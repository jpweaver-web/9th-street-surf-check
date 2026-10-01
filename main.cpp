#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <nlohmann/json.hpp>
#include "httplib.h"
#include "tideTest.h"
#include <vector>
#include <cmath>
#include <iomanip>

// Path to the JSON file written by conditionsTest
static const std::string CACHE_FILE = "conditions.json";

// Per-request cache — rebuilt once per button press, reused within same second
static nlohmann::json requestCache;
static std::time_t   requestCacheTime = 0;
static const int     CACHE_TTL = 30; // seconds

// ── Tide helpers ──────────────────────────────────────────────────────────────

static std::string fmtTime(std::time_t t) {
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%I:%M %p");
    std::string s = oss.str();
    if (!s.empty() && s[0] == '0') s.erase(0, 1);
    return s;
}

static double cosineInterp(double h0, double h1, double t01) {
    double c = (1.0 - std::cos(t01 * M_PI)) / 2.0;
    return h0 + (h1 - h0) * c;
}

static void buildTideCurveArrays(std::vector<long long>& times,
                                  std::vector<double>& heights) {
    auto events = getTideEvents();
    if (events.size() < 2) return;

    const int step = 10 * 60; // 10 minutes

    for (size_t i = 0; i + 1 < events.size(); ++i) {
        const auto& a = events[i];
        const auto& b = events[i + 1];
        if (b.timestamp <= a.timestamp) continue;

        double dt = std::difftime(b.timestamp, a.timestamp);
        for (std::time_t t = a.timestamp; t <= b.timestamp; t += step) {
            double t01 = std::difftime(t, a.timestamp) / dt;
            heights.push_back(cosineInterp(a.height, b.height, t01));
            times.push_back((long long)t);
        }
    }
}

// ── Rating ────────────────────────────────────────────────────────────────────

double waveRating(double waveHeight, double windSpeed, double windDirection,
                  double waterTemp, double airTemp, double currentTide) {
    double rating = 0.0;
    if (waveHeight < 1.2) return 0;

    if (waveHeight > 5) {
        rating += currentTide > 3.5 ? 18 : currentTide < 2.0 ? 8 : 14;
    } else if (waveHeight < 2) {
        rating += (currentTide > 4 || currentTide < 1.5) ? 8 : 12;
    } else {
        rating += currentTide < 2.0 ? 14 : currentTide > 4.5 ? 16 : 20;
    }

    if (windDirection >= 0 && windDirection <= 105) rating += 10;
    else if (windSpeed < 5)   rating += 10;
    else if (windSpeed < 7.5) rating += 3;
    else return 0;

    rating += waterTemp > 61 ? 10 : waterTemp > 59 ? 8 : 5;
    rating += airTemp >= 72  ? 10 : 8.5;
    return rating / 5.0;
}

std::string ratingReaction(double r) {
    if (r >= 9.5) return "Perfect conditions, get out there!";
    if (r >= 8.7) return "It's nice out";
    if (r >= 7.5) return "It's decent";
    if (r >= 6.0) return "Not great";
    return "Trash";
}

std::string degreesToCompass(double deg) {
    static const char* d[] = {
        "N","NNE","NE","ENE","E","ESE","SE","SSE",
        "S","SSW","SW","WSW","W","WNW","NW","NNW"
    };
    return d[static_cast<int>((deg + 11.25) / 22.5) % 16];
}

// ── Build response JSON ───────────────────────────────────────────────────────
// Reads StormGlass data from conditions.json (written by conditionsTest).
// Only calls NOAA for live tide data (free, no daily limit).

nlohmann::json buildResponseJson() {
    // Return cached response if it was built within the last 30 seconds
    std::time_t now = std::time(nullptr);
    if (!requestCache.empty() && (now - requestCacheTime) < CACHE_TTL) {
        std::cerr << "[server] Serving from request cache" << std::endl;
        return requestCache;
    }
    std::cerr << "[server] Building fresh response" << std::endl;
    // 1. Read cached StormGlass data
    std::ifstream f(CACHE_FILE);
    if (!f.is_open())
        throw std::runtime_error(
            "conditions.json not found — run ./conditionsTest first");

    std::ostringstream ss;
    ss << f.rdbuf();
    auto cached = nlohmann::json::parse(ss.str());

    // 2. Get live tide data from NOAA (free, no limit)
    double currentTide = getCurrentTide();
    auto events = getTideEvents();

    // Get today's date boundaries in local time
    std::time_t nowForFilter = std::time(nullptr);
    std::tm* localNow = std::localtime(&nowForFilter);
    std::tm startOfDay = *localNow;
    startOfDay.tm_hour = 0; startOfDay.tm_min = 0; startOfDay.tm_sec = 0;
    std::time_t todayStart = std::mktime(&startOfDay);
    std::tm endOfDay = *localNow;
    endOfDay.tm_hour = 23; endOfDay.tm_min = 59; endOfDay.tm_sec = 59;
    std::time_t todayEnd = std::mktime(&endOfDay);

    std::vector<std::string> highs, lows;
    for (size_t i = 0; i < events.size(); ++i) {
        // Only include events from today
        if (events[i].timestamp < todayStart || events[i].timestamp > todayEnd) continue;
        std::ostringstream entry;
        entry << fmtTime(events[i].timestamp)
              << " (" << std::fixed << std::setprecision(1)
              << events[i].height << " ft)";
        if (events[i].type == 'H') {
            highs.push_back(entry.str());
        } else if (events[i].type == 'L') {
            lows.push_back(entry.str());
        }
    }

    std::vector<long long> tideTimes;
    std::vector<double>    tideHeights;
    buildTideCurveArrays(tideTimes, tideHeights);

    // 3. Re-compute rating with live tide
    double waveHeight  = cached.value("waveHeight",  0.0);
    double windSpeed   = cached.value("windSpeed",   0.0);
    double windDir     = cached.value("windDirectionDegrees", 0.0);
    double waterTemp   = cached.value("waterTemp",   0.0);
    double airTemp     = cached.value("airTemp",     0.0);

    double rating   = waveRating(waveHeight, windSpeed, windDir,
                                 waterTemp, airTemp, currentTide);
    std::string reaction = ratingReaction(rating);

    // 4. Build response — pass through StormGlass fields, add live tide
    nlohmann::json out = cached; // start with everything from conditionsTest
    out["currentTide"]  = currentTide;
    out["highTides"]    = highs;
    out["lowTides"]     = lows;
    out["tideTimes"]    = tideTimes;
    out["tideHeights"]  = tideHeights;
    out["rating"]       = rating;
    out["reaction"]     = reaction;
    out["updatedEpoch"] = (long long)std::time(nullptr);

    // Cache the result
    requestCache = out;
    requestCacheTime = std::time(nullptr);
    return out;
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    httplib::Server svr;

    svr.Get("/api/conditions", [](const httplib::Request&,
                                   httplib::Response& res) {
        try {
            auto j = buildResponseJson();
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_content(j.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 503;
            nlohmann::json err;
            err["error"] = e.what();
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_content(err.dump(), "application/json");
        }
    });
    // Root route to prevent 404 on base URL
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("SurfSpotApp C++ Backend is Live!", "text/plain");
    });

    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("ok", "text/plain");
    });

    // Read dynamic port assigned by Render (defaults to 8080 locally)
    const char* port_env = std::getenv("PORT");
    int port = port_env ? std::atoi(port_env) : 8080;

    std::cout << "Server starting on port " << port << "...\n";
    std::cout << "Reads StormGlass data from: " << CACHE_FILE << "\n";
    std::cout << "NOAA tide data fetched live (no daily limit)\n";

    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Error: Server failed to listen on port " << port << std::endl;
        return 1;
    }
}