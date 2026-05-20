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

- **C backend** (`backend/`): Adjacency-matrix graph (`Graph` struct, max 20 vertices), Dijkstra O(V²), hand-rolled JSON output (no third-party JSON library). The executable takes start/end indices as CLI args and writes `result.json`.
- **Python bridge** (`server.py`): Extends `SimpleHTTPRequestHandler`, serves static files from `frontend/`, proxies `/api/route` to the C backend subprocess. Zero pip dependencies.
- **Frontend** (`frontend/`): Cytoscape.js graph visualization with a custom distance-proportional force-directed layout (not Cytoscape's built-in layouts). The layout iteratively applies spring forces so that visual edge length ≈ real road distance × scaling factor.

## Key implementation details

### Dijkstra is duplicated
The Dijkstra algorithm logic exists in **two places**: `backend/src/dijkstra.c` (standalone with console output) and `backend/src/main.c` (inlined into `main()` for JSON output). Changes to the core algorithm must be applied to both locations, or the `dijkstra.c` version should be refactored to return the `dist[]`/`prev[]` arrays so `main.c` can reuse it.

### Map data is duplicated
The map data (20 locations, ~40 roads) exists in two places that must stay consistent:
1. `data/map.txt` — canonical source, loaded by the C backend and used by `make test`
2. `frontend/script.js` — `LOCATIONS[]` and `ROADS[]` arrays used by the custom layout and Cytoscape initialization

When adding/removing locations or roads, update both.

To generate a map for a different city, use the prompt template at `prompts/city_map_generator.md` with any AI — it produces both `map.txt` and `layout.json` for the target city.

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
`computeDistanceBasedLayout()` in `script.js` implements a custom force-directed layout with springs that pull edge lengths toward `weight × SCALE` pixels. It also uses Floyd-Warshall all-pairs shortest path for initial position estimation. The layout runs synchronously at page load in the browser.
