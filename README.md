# 9th Street Surf Conditions

A full-stack surf conditions dashboard for 9th Street in Huntington Beach, California.

I built this project as a fun way to learn computer science skills beyond what has been taught in my courses. I decided the best way to go about this was to merge my passion for surfing with my passion for programming.

- **Live site:** Hosted on GitHub Pages  
- **Backend:** Hosted on Render  
- **Monitoring:** UptimeRobot  

---

## What I Built

The project is a browser-based frontend backed by a custom C++ HTTP server.

The frontend requests a single endpoint from the C++ backend. The backend combines cached surf/weather data with NOAA tide predictions, calculates the current surf rating, and returns the data as JSON.

The frontend then uses that response to dynamically update:
- Surf-quality rating
- Wave-height visualization
- Tide graph
- Wind compass
- Swell direction
- Air temperature
- Water temperature
- High/low tide information
- Rating-specific music and artwork

---

## Highlights

### Custom Surf Rating
I designed a custom scoring algorithm that evaluates conditions using:
- Wave height
- Tide height
- Wind direction
- Wind speed
- Water temperature
- Air temperature

The result is normalized to a 0–10 rating and displayed alongside a corresponding song match.

### Dynamic Wave Visualization
The frontend generates an SVG visualization based on the current wave height. It includes:
- A dynamically scaled wave
- A height measurement bracket
- A person used as a visual size reference
- Current wave-height information

This makes the numerical wave-height value easier to understand visually.

### Interactive Tide Graph
Tide predictions come from NOAA and are rendered as a dynamic graph. It features:
- High and low tide events
- A smooth interpolated tide curve (using cosine interpolation)
- Current time marker and tide height
- Hover/touch information
- A defined surfing window

### Wind & Swell
The dashboard displays:
- Wind speed and direction
- Visual wind compass
- Swell direction (including degrees)

### Music-Based Rating UI
Each rating range is associated with a song, album artwork, and short audio clip. The selected music changes with the calculated surf rating, creating a personalized experience.

---

## Architecture

```text
                    ┌─────────────────────────┐
                    │      GitHub Pages       │
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
             ┌──────────────┐          ┌──────────────┐
             │ conditions   │          │     NOAA     │
             │   .json      │          │  Tide API    │
             │              │          │              │
             │ Cached surf/ │          │ Tide events  │
             │ weather data │          │ & predictions│
             └──────────────┘          └──────────────┘

                               UptimeRobot
                                    │
                                    ▼
                             /health monitoring
```

## Request Flow

1. The browser sends a request to `/api/conditions`.
2. The C++ server receives the request using `cpp-httplib`.
3. The server reads the available cached surf/weather data.
4. NOAA tide predictions are retrieved when the tide cache needs to be refreshed.
5. The backend determines the current tide using interpolation between surrounding tide events.
6. The rating algorithm evaluates the conditions.
7. The backend returns the combined data as JSON.
8. JavaScript processes the response and updates the DOM dynamically without a page reload.

---

## Technical Stack

| Area | Technologies |
| --- | --- |
| **Frontend** | HTML5, CSS3, JavaScript (ES6+), SVG, Canvas |
| **Backend** | C++17, `cpp-httplib`, `nlohmann/json`, `libcurl` |
| **APIs & Data** | NOAA Tides & Currents API, Stormglass-derived data |
| **Deployment** | GitHub Pages, Render, Docker |
| **CI/CD & Monitoring** | GitHub Actions, UptimeRobot |
| **Version Control** | Git, GitHub |

---

## Backend Architecture & Key Features

### API Endpoints
- `GET /` — Basic status response to verify backend operation.
- `GET /health` — Lightweight health-check endpoint for UptimeRobot.
- `GET /api/conditions` — Returns JSON containing surf ratings, environmental metrics, and tide graph coordinates.

### Rating Scoring Logic
Implemented in C++, the rating algorithm calculates component scores based on:
- **Wave Height & Tide:** Scoring thresholds change dynamically depending on wave scale.
- **Wind Vector:** Factors direction and velocity; strong onshore winds can drop score to zero.
- **Temperatures:** Water and air temperatures contribute positively to higher values.
- **Scale:** Aggregated scores are normalized to a standard **0–10 scale** (`rating / 5.0`).

### Tide Interpolation
To convert NOAA's discrete high/low tide predictions into a continuous curve, the backend uses cosine interpolation:
```cpp
double c = (1.0 - std::cos(t * M_PI)) / 2.0;
```

Caching Strategy
Tide Cache: Cached daily to reduce NOAA API overhead.
Response Cache: Short-lived in-memory cache (CACHE_TTL = 30 seconds) to handle rapid requests efficiently.

---

## What I Learned
- **Backend Engineering:** Writing an HTTP server in C++, managing REST endpoints, parsing JSON natively, handling HTTP client requests, and building caching layers.
- **Data Processing:** Applying mathematical interpolation algorithms to environmental time-series data.
- **Frontend & Visualization:** Constructing interactive SVG components and Canvas graphs without external UI libraries.
- **DevOps:** Setting up containerized deployments with Docker on Render and automated CI/CD deployment pipelines using GitHub Actions.

---

## Future Improvements
- Let user choose surf spot.
- Store historical condition data for trend analysis.
- Add configurable rating criteria.
- Implement robust backend logging and failure recovery modes for external APIs.

---

## Author

**John Weaver**  
*Computer Science — UC Santa Cruz*
