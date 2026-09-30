#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iomanip>
#include <sstream>
#include "tideTest.h"
#include <fstream>

// Function to handle the API response

// Convert degrees to cardinal direction
std::string degreesToCompass(double degrees) {
    static const char* directions[] = {
        "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
        "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"
    };
    int index = static_cast<int>((degrees + 11.25) / 22.5) % 16;
    return directions[index];
}

// Function to fetch data from the API
std::string fetchDataFromSG(const std::string& url, const std::string& apiKey) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    if (curl) {
        struct curl_slist* chunk = nullptr;
        std::string header = "Authorization: " + apiKey;
        chunk = curl_slist_append(chunk, header.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL request failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(chunk);
    }
    return readBuffer;
}
    double waveRating(double waveHeight, double windSpeed, double windDirection, double waterTemp, double airTemp, double currentTide)
{
    double rating = 0.0;
    if(waveHeight < 1.2) //flat
    {
        return 0;
    }
    else if(waveHeight > 5) //big surf
    {
        if(currentTide > 3.5)
        {
            rating += 18;
        }
        else if(currentTide < 2.0)
        {
            rating += 8;
        }
        else
        {
            rating += 14;
        }
    }
    else if(waveHeight < 2) //small surf
    {
        if(currentTide > 4)
        {
            rating += 8;
        }
        else if(currentTide < 1.5)
        {
            rating += 8;
        }
        else
        {
            rating += 12;
        }
    }
    else // medium size surf
    {
        if(currentTide < 2.0)
        {
            rating+=14;
        }
        else if(currentTide > 4.5)
        {
            rating += 16;
        }
        else
        {
            rating += 20;
        }
    }
    if(windDirection >= 0 && windDirection <= 105) //offshore
    {
        rating += 10;
    }
    else if(windSpeed < 5)
    {
        rating += 10;
    }
    else if (windSpeed < 7.5)
    {
        rating += 3;
    }
    else
    {
       return 0; 
    }
    if(waterTemp > 61)
    {
        rating += 10;
    }
    else if(waterTemp > 59)
    {
        rating += 8;
    }
    else
    {
        rating += 5;
    }
    if(airTemp >= 72)
    {
        rating += 10;
    }
    else
    {
        rating += 8.5;
    }
    return (rating / 5);
}

std::string ratingReaction(double waveRating)
{
    if(waveRating >= 9.5)
    {
        return "Perfect conditions, get out there!";
    }
    else if(waveRating >= 8.7)
    {
        return "It's nice out";
    }
    else if(waveRating >= 7.5)
    {
        return "It's decent";
    }
    else if(waveRating >= 6.0)
    {
        return "Not great";
    }
    else
    {
        return "Trash";
    }
}


int main() {
   // Replace hardcoded string with std::getenv
const char* envKey = std::getenv("STORMGLASS_API_KEY");
std::string apiKey = envKey ? std::string(envKey) : "";

if (apiKey.empty()) {
    std::cerr << "Error: STORMGLASS_API_KEY environment variable not set!" << std::endl;
    return 1;
}
    // Fetch weather/surf data
    std::string weatherUrl = "https://api.stormglass.io/v2/weather/point?"
                             "lat=33.658698&lng=-118.007683&params=waveHeight,windSpeed,windDirection,"
                             "airTemperature,waterTemperature,swellDirection";
    std::string weatherData = fetchDataFromSG(weatherUrl, apiKey);

    try {
        // Parse JSON for weather data
        auto jsonWeather = nlohmann::json::parse(weatherData);
        // Find closest forecast hour to now
        std::time_t now = std::time(nullptr);
        double minDifference = std::numeric_limits<double>::max();
        size_t bestIndex = 0;

        for (size_t i = 0; i < jsonWeather["hours"].size(); ++i) {
            std::string timeStr = jsonWeather["hours"][i]["time"].get<std::string>();
            std::tm tm = {};
            std::istringstream ss(timeStr);
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
            std::time_t forecastTime = timegm(&tm);

    double diff = std::abs(std::difftime(now, forecastTime));
    if (diff < minDifference) {
        minDifference = diff;
        bestIndex = i;
    }
}
// Convert and extract values from the closest matching forecast
        double waveHeight = jsonWeather["hours"][bestIndex]["waveHeight"]["sg"].get<double>() * 3.28; // meters to feet
        double windSpeed = jsonWeather["hours"][bestIndex]["windSpeed"]["sg"].get<double>() * 1.9438; // m/s to knots
        double airTemp = (jsonWeather["hours"][bestIndex]["airTemperature"]["sg"].get<double>() * 9.0 / 5.0) + 32; // C to F
        double waterTemp = (jsonWeather["hours"][bestIndex]["waterTemperature"]["sg"].get<double>() * 9.0 / 5.0) + 32; // C to F
        double windDirDeg = jsonWeather["hours"][bestIndex]["windDirection"]["sg"].get<double>();
        double swellDirDeg = jsonWeather["hours"][bestIndex]["swellDirection"]["sg"].get<double>(); 
        std::string windDirCard = degreesToCompass(windDirDeg);
        std::string swellDirCard = degreesToCompass(swellDirDeg);
        double currentTide = getCurrentTide();
        double Rating = waveRating(waveHeight, windSpeed, windDirDeg, waterTemp, airTemp, currentTide);

        //wave rating

        // Output
nlohmann::json out;

out["spot"] = "9th Street, Huntington Beach";
out["waveHeight"] = waveHeight;
out["windSpeed"] = windSpeed;
out["windDirectionDegrees"] = windDirDeg;
out["windDirection"] = windDirCard;
out["swellDirectionDegrees"] = swellDirDeg;
out["swellDirection"] = swellDirCard;
out["waterTemp"] = waterTemp;
out["airTemp"] = airTemp;
out["currentTide"] = currentTide;
out["rating"] = Rating;
out["reaction"] = ratingReaction(Rating);

// Timestamp (nice for UI)
out["updated"] = std::string(std::asctime(std::localtime(&now)));

std::ofstream file("conditions.json");
file << out.dump(2);  // pretty print with indentation
file.close();
std::cout << out.dump(2) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }

    return 0;
}

