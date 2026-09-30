#pragma once

#include <vector>
#include <ctime>
#include <string>

struct TideEvent {
    std::time_t timestamp;
    double height;
    char type; // 'H' = high, 'L' = low
};

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

std::vector<TideEvent> getTideEvents();
double getCurrentTide();
double interpolateTide(const TideEvent& a, const TideEvent& b, std::time_t now);
std::string printTides();