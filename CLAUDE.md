# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

A full-stack map navigation system: C backend computes shortest paths via Dijkstra's algorithm on an adjacency-matrix graph, a Python stdlib HTTP server bridges to the frontend, and a vanilla HTML/CSS/JS frontend visualizes the graph with Cytoscape.js using a custom distance-proportional force-directed layout.

## Commands

```bash
# Build C backend (with make)
cd backend && make

# Build C backend (without make — single gcc command)
cd backend && gcc -Wall -Wextra -std=c99 -pedantic -O2 -Iinclude -o map_navigation_backend src/main.c src/graph.c src/dijkstra.c src/json_output.c

# Clean / rebuild
cd backend && make clean
cd backend && make rebuild

# Quick test (station 0 → airport 8)
cd backend && make test

# Manual C backend invocation
./backend/map_navigation_backend <start_index> <end_index> [map_file] [output_json]

# Generate layout coordinates for a city map
python tools/generate_layout.py maps/<city>.txt

# Start the bridge server (default port 8080)
python server.py
python server.py 3000
python server.py 8080 --debug    # enables verbose request/response logging
```

Open `http://localhost:8080` after starting the server.

## Architecture

Three layers, strictly separated:

```
Browser (Cytoscape.js) ──GET /api/route?start=X&end=Y──▶ Python server (server.py)
                                                            │ subprocess.run()
                                                            ▼
                                                       C backend (backend/)
                                                            │ writes result.json
                                                            ▼
Browser ◀──────────── JSON response ───────────────── Python reads result.json
```

- **C backend** (`backend/`): Adjacency-matrix graph (`Graph` struct, max 50 vertices), Dijkstra O(V²), hand-rolled JSON output (no third-party JSON library). The executable takes start/end indices as CLI args and writes `result.json`.
- **Python bridge** (`server.py`): Extends `SimpleHTTPRequestHandler`, serves static files from `frontend/`, proxies `/api/route` to the C backend subprocess, serves map data via `/api/map` and `/api/maps`, and serves layout files from `/data/layouts/`. Zero pip dependencies.
- **Frontend** (`frontend/`): Cytoscape.js graph visualization. Prefers `preset` layout from `data/layouts/<city>.json` for stable coordinates; falls back to a custom distance-proportional force-directed layout if no layout file exists.

## Key implementation details

### Dijkstra is duplicated
The Dijkstra algorithm logic exists in **two places**: `backend/src/dijkstra.c` (standalone with console output) and `backend/src/main.c` (inlined into `main()` for JSON output). Changes to the core algorithm must be applied to both locations, or the `dijkstra.c` version should be refactored to return the `dist[]`/`prev[]` arrays so `main.c` can reuse it.

### Map data
The primary data source is `maps/<city>.txt` — loaded by the C backend and served to the frontend via `/api/map?city=X`. `data/map.txt` is a legacy fallback used when no city parameter is provided.

Multiple city maps are stored in `maps/` directory (e.g., `maps/shanghai.txt`). The frontend supports switching cities at runtime via the city selector. The `/api/maps` endpoint returns available cities.

**Layout**: Precomputed node coordinates are stored in `data/layouts/<city>.json`. The frontend loads these and uses Cytoscape.js `preset` layout for stable, repeatable rendering. If no layout file exists, the force-directed algorithm runs as a fallback. Generate layouts with: `python tools/generate_layout.py maps/<city>.txt`

To generate a map for a different city, use the prompt template at `prompts/city_map_generator.md` with any AI — it produces `map.txt` for the target city. Place the file in `maps/<city>.txt` and it will appear in the frontend's city selector automatically. Run `generate_layout.py` to create its layout file.

### Graph is undirected
`addEdge()` sets `edge[start][end] = edge[end][start] = weight` — the graph is always undirected. `INF` (99999) represents no edge, `0` represents self-loops (diagonal).

### Map file format
```
V <location name>
E <start_index> <end_index> <weight>
# lines starting with # are comments
```
Vertices are indexed by definition order (first `V` = index 0, second = 1, etc.). Edges reference these 0-based indices.

### Frontend layout algorithm
`computeDistanceBasedLayout()` in `script.js` implements a custom force-directed layout with springs that pull edge lengths toward `weight × SCALE` pixels. It uses Floyd-Warshall all-pairs shortest path for initial position estimation. This now runs only as a **fallback** when no `data/layouts/<city>.json` file exists. When a layout file is present, Cytoscape.js `preset` layout is used instead for stable, repeatable rendering.
