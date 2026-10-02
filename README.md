9th Street Surf Conditions 

A full-stack surf conditions dashboard for 9th Street in Huntington Beach, California.

I built this project as a fun way to learn computer science skills beyond what has been taught in my courses. I decided the best way to go about this was to merge my passion for surfing with my passion for programming.
Live site: hosted on GitHub Pages
Backend: hosted on Render
Monitoring: UptimeRobot

What I Built

The project is a browser-based frontend backed by a custom C++ HTTP server.

The frontend requests a single endpoint from the C++ backend. The backend combines cached surf/weather data with NOAA tide predictions, calculates the current surf rating, and returns the data as JSON.

The frontend then uses that response to dynamically update:

Surf-quality rating

Wave-height visualization

Tide graph

Wind compass

Swell direction

Air temperature

Water temperature

High/low tide information

Rating-specific music and artwork

Highlights

Custom Surf Rating

I designed a custom scoring algorithm that evaluates conditions using:

Wave height

Tide height

Wind direction

Wind speed

Water temperature

Air temperature

The result is normalized to a 0–10 rating and displayed and then given a song equivalent. 

Dynamic Wave Visualization

The frontend generates an SVG visualization based on the current wave height.

It includes:

A dynamically scaled wave

A height measurement bracket

A person used as a visual size reference

Current wave-height information

This makes the numerical wave-height value easier to understand visually.

Interactive Tide Graph

Tide predictions come from NOAA and are rendered as a dynamic graph.

The graph includes:

High and low tide events

A smooth interpolated tide curve

Current time marker

Current tide height

Hover/touch information

A defined surfing window

Instead of simply drawing straight lines between tide events, the backend uses cosine interpolation to generate smoother transitions.

Wind & Swell

The dashboard displays:

Wind speed

Wind direction

Visual wind compass

Swell direction

Swell direction in degrees

Music-Based Rating UI

Each rating range is associated with a song, album artwork, and short audio clip.

The selected music changes with the calculated surf rating, giving the rating system a more personalized interface than a standard weather dashboard.

Architecture

                    ┌─────────────────────────┐
                    │       GitHub Pages      │
                    │                         │
                    │  HTML / CSS / JavaScript│
                    └────────────┬────────────┘
                                 │
                                 │ HTTPS
                                 ▼
                    ┌─────────────────────────┐
                    │         Render          │
                    │                         │
                    │     C++ HTTP Server     │
                    │       cpp-httplib       │
                    └────────────┬────────────┘
                                 │
                    ┌────────────┴────────────┐
                    │                         │
                    ▼                         ▼
             ┌──────────────┐         ┌──────────────┐
             │ conditions   │         │     NOAA     │
             │    .json     │         │ Tide API     │
             │              │         │              │
             │ Cached surf/ │         │ Tide events  │
             │ weather data │         │ & predictions│
             └──────────────┘         └──────────────┘

                         UptimeRobot
                              │
                              ▼
                       /health monitoring

Request Flow

When the user requests updated conditions:

The browser sends a request to /api/conditions.

The C++ server receives the request using cpp-httplib.

The server reads the available cached surf/weather data.

NOAA tide predictions are retrieved when the tide cache needs to be refreshed.

The backend determines the current tide using interpolation between surrounding tide events.

The rating algorithm evaluates the conditions.

The backend returns the combined data as JSON.

JavaScript processes the response.

The dashboard updates without requiring a full page reload.

Technical Stack

Area

Technologies

Frontend

HTML, CSS, JavaScript

Backend

C++

HTTP Server

cpp-httplib

JSON

nlohmann/json

HTTP/API Requests

libcurl

Tide Data

NOAA Tides & Currents

Surf/Weather Data

Stormglass-derived data stored in conditions.json

Visualization

SVG, Canvas, JavaScript

Deployment

GitHub Pages, Render

CI/CD

GitHub Actions

Containerization

Docker

Monitoring

UptimeRobot

Version Control

Git / GitHub

Backend

The backend is written in C++ and uses cpp-httplib to expose HTTP endpoints.

API Endpoints

GET /

Basic backend response used to confirm that the server is running.

GET /health

Lightweight health-check endpoint used by UptimeRobot.

ok

GET /api/conditions

Returns the data consumed by the frontend, including:

Wave height

Wind speed

Wind direction

Swell direction

Air temperature

Water temperature

Current tide

High/low tide events

Tide graph data

Surf rating

Rating reaction

Rating Algorithm

The rating calculation is implemented in the C++ backend.

The algorithm evaluates several environmental factors and assigns each one a weighted contribution.

Wave Height + Tide

Wave height determines which tide scoring rules apply.

Very small waves result in a zero rating. For larger waves, different tide ranges receive different scores based on how the current tide interacts with the wave conditions.

Wind

Wind direction and speed are used together.

The algorithm gives a strong contribution to favorable/offshore wind directions. For other directions, the contribution depends on wind speed, with sufficiently strong winds able to reduce the overall rating to zero.

Water Temperature

Warmer water receives a higher contribution to the rating.

Air Temperature

Air temperature also contributes to the final score, with warmer conditions receiving a higher value.

Final Score

The individual components are combined and divided by five:

return rating / 5.0;

This produces the application's 0–10 rating scale.

The frontend then uses that score to select the appropriate rating display, reaction, artwork, and music.

Tide Data & Interpolation

NOAA provides discrete high- and low-tide events.

The backend converts these events into a smoother tide curve using cosine interpolation:

double c = (1.0 - std::cos(t * M_PI)) / 2.0;

The interpolation produces intermediate tide values between NOAA events, which allows the frontend to draw a continuous-looking tide graph.

The project generates points at regular intervals for the graph and marks the current time separately.

Caching

Caching reduces unnecessary API requests and improves response speed.

Tide Cache

NOAA tide predictions are cached for the current calendar day rather than requested repeatedly for every frontend request.

Response Cache

The backend also maintains a short-lived in-memory response cache:

static const int CACHE_TTL = 30;

This allows repeated requests within the cache window to reuse the existing response instead of rebuilding it every time.

Project Structure

SurfSpotApp/
│
├── index.html
├── ratingMap.js
│
├── main.cpp
├── tideTest.cpp
├── tideTest.h
├── httplib.h
│
├── conditions.json
├── Makefile
├── Dockerfile
├── .gitignore
│
├── .github/
│   └── workflows/
│       └── pages.yml
│
├── images/
├── musicImages/
├── songs/
└── audio/

Key Files

main.cpp

The main C++ backend. It:

Starts the HTTP server

Handles API requests

Reads cached condition data

Calculates the surf rating

Combines tide and surf/weather information

Returns JSON to the frontend

tideTest.cpp / tideTest.h

Contains the NOAA tide integration, tide event structures, current-tide calculation, and interpolation logic.

index.html

Contains the main frontend interface and browser-side functionality.

ratingMap.js

Maps rating ranges to their associated songs, artwork, and audio information.

conditions.json

Contains the cached surf/weather condition data consumed by the backend.

Dockerfile

Defines the containerized environment used to build and run the C++ server for deployment.

.github/workflows/pages.yml

Automates deployment of the frontend to GitHub Pages.

Deployment

GitHub Pages

The frontend is hosted on GitHub Pages.

A GitHub Actions workflow handles deployment when changes are pushed to the main branch.

The workflow:

Checks out the repository

Configures GitHub Pages

Creates the Pages deployment artifact

Deploys the frontend

Render

The C++ backend is deployed separately on Render.

This keeps the frontend hosting and backend hosting independent.

The backend uses the PORT environment variable supplied by Render rather than relying on a hard-coded production port.

UptimeRobot

UptimeRobot monitors the backend's /health endpoint.

This provides a simple way to detect when the deployed server stops responding.

Docker

The backend includes a Dockerfile for deployment.

The container build separates the compilation environment from the runtime environment so the final runtime image only needs the components required to execute the compiled server.

The server reads the deployment port from the environment and starts the C++ HTTP service inside the container.

Running Locally

Requirements

C++17-compatible compiler

Make

libcurl

OpenSSL

nlohmann/json

Build

make

This creates the server executable.

Run

./server

The local server runs on port 8080 unless configured otherwise.

The API can then be accessed at:

http://localhost:8080/api/conditions

Environment Variables

The project uses environment variables for sensitive/deployment-specific configuration.

For example:

STORMGLASS_API_KEY=your_api_key_here
PORT=8080

Real API keys should never be committed to the public repository.

What I Learned

This project gave me experience working across several parts of a software system rather than only building a frontend.

Backend Development

I worked with:

C++ HTTP servers

REST-style endpoints

JSON serialization/deserialization

External API requests

File-based caching

Environment variables

Data Processing

I worked with:

Weather and surf data

Tide-event data

Unit conversions

Time handling

Interpolation

Custom scoring logic

Frontend Development

I worked with:

JavaScript

Dynamic DOM updates

SVG

Canvas

Interactive graphs

Responsive UI

Audio playback

Deployment

I also worked with:

GitHub Actions

GitHub Pages

Render

Docker

UptimeRobot

Separating frontend and backend deployments

Future Improvements

Some areas I would like to improve as the project continues to develop:

Add support for multiple surf spots

Store and visualize historical conditions

Add forecast trends

Improve automated testing of the rating algorithm

Make the rating criteria configurable

Improve error handling when external data sources are unavailable

Further separate frontend and backend configuration

Add more robust backend logging

Author

John Weaver
Computer Science — UC Santa Cruz

This project was built as a personal software project to combine my interests in surfing and software development while gaining practical experience with C++, APIs, frontend development, and cloud deployment.
