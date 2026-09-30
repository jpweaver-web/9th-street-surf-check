#include "tideTest.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iomanip>
#include <sstream>
#include <ctime>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string getDateOffset(int offsetDays) {
    std::time_t now = std::time(nullptr) + offsetDays * 86400;
    std::tm* tm = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y%m%d");
    return oss.str();
}

std::string getNOAATideURL(const std::string& startDate, const std::string& endDate) {
    const std::string station = "9410660";
    return "https://api.tidesandcurrents.noaa.gov/api/prod/datagetter?"
           "product=predictions&application=surfapp&begin_date=" + startDate +
           "&end_date=" + endDate +
           "&datum=MLLW&station=" + station +
           "&time_zone=lst_ldt&units=english&interval=10&format=json";
}

std::string fetchDataFromNOAA(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            std::cerr << "[NOAA] cURL failed: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

double interpolateTide(const TideEvent& a, const TideEvent& b, std::time_t now) {
    double t = difftime(now, a.timestamp) / difftime(b.timestamp, a.timestamp);
    double c = (1 - std::cos(t * M_PI)) / 2;
    return a.height + (b.height - a.height) * c;
}

// Cache — one fetch per calendar day
static std::vector<TideEvent> cachedEvents;
static std::string cachedDate = "";

std::vector<TideEvent> getTideEvents() {
    std::string today = getDateOffset(0);

    if (!cachedEvents.empty() && cachedDate == today) {
        return cachedEvents;
    }

    cachedEvents.clear();
    cachedDate = "";

    std::string url = getNOAATideURL(today, getDateOffset(1));
    std::cerr << "[NOAA] Fetching tides for " << today << std::endl;

    try {
        std::string response = fetchDataFromNOAA(url);
        auto json = nlohmann::json::parse(response);

        if (!json.contains("predictions")) {
            std::cerr << "[NOAA] Missing predictions: " << response.substr(0, 300) << std::endl;
            return cachedEvents;
        }

        for (const auto& entry : json["predictions"]) {
            std::tm tm = {};
            std::istringstream ss(entry["t"].get<std::string>());
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
            tm.tm_isdst = -1;
            std::time_t timestamp = mktime(&tm);
            double height = std::stod(entry["v"].get<std::string>());
            char type = 'U';
            if (entry.contains("type") && !entry["type"].is_null()) {
                std::string t = entry["type"].get<std::string>();
                type = t.empty() ? 'U' : t[0];
            }
            cachedEvents.push_back({ timestamp, height, type });
        }

        cachedDate = today;
        std::cerr << "[NOAA] Fetched " << cachedEvents.size() << " events, cached for " << today << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[NOAA] Parse error: " << e.what() << std::endl;
    }

    return cachedEvents;
}

double getCurrentTide() {
    auto events = getTideEvents();
    std::time_t now = std::time(nullptr);

    if (events.size() < 2) return -9999.0;
    if (now <= events.front().timestamp) return events.front().height;
    if (now >= events.back().timestamp)  return events.back().height;

    for (size_t i = 0; i + 1 < events.size(); ++i) {
        if (events[i].timestamp <= now && now <= events[i+1].timestamp)
            return interpolateTide(events[i], events[i+1], now);
    }
    return -9999.0;
}

std::string printTides() {
    std::ostringstream oss;
    auto events = getTideEvents();
    oss << std::fixed << std::setprecision(1);
    for (const auto& t : events)
        oss << std::put_time(std::localtime(&t.timestamp), "%Y-%m-%d %I:%M %p")
            << " - " << t.height << " ft\n";
    oss << "\nCurrent: " << getCurrentTide() << " ft\n";
    return oss.str();
}