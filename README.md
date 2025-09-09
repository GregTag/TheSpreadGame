## TheSpreadGame

Client–server game with a C++ backend (Boost.Beast & Asio, spdlog, nlohmann_json) and a Svelte + Vite frontend. The frontend talks to the backend via WebSocket (default ws://localhost:8080). An AsyncAPI description lives at `./async_api.yaml`.

### Stack
- Backend: C++20, CMake, Conan 2, Boost.Beast & Asio, spdlog, nlohmann_json
- Frontend: Svelte 4, Vite 5, TailwindCSS 3

## Prerequisites
- Linux, macOS, or Windows (WSL works)
- CMake ≥ 3.26
- A C++20 compiler (e.g., GCC ≥ 12 or Clang ≥ 15)
- Conan 2.x
- Node.js ≥ 18 and npm ≥ 9

## Quick start

### 1. Backend (Conan + CMake)
The backend uses Conan generators for dependency setup.

```zsh
# From repo root
cd backend

# One-time (detect your default profile)
conan profile detect --force

# Install deps and generate toolchain for Debug
conan install . \
	--output-folder=. \
	--build=missing

# Configure and build (Release)
cmake --preset conan-release
cmake --build build/Release --parallel

# Run (port is optional; defaults to 8080)
./build/Release/spread_server 8080
```

Useful env vars:
- `LOG_LEVEL` (e.g., `trace`, `debug`, `info`, `warn`, `err`)

### 2. Frontend (Vite + Svelte)
```zsh
# From repo root
cd frontend
npm install
npm run dev
# Open http://localhost:5173
```

The frontend auto-connects to `ws://<host>:8080`. Ensure the backend is running.

## Project layout
```
TheSpreadGame/
├─ async_api.yaml           # AsyncAPI contract for the WS API
├─ backend/                 # C++20 backend
│  ├─ include/, src/        # server, lobby, models, etc.
│  ├─ lib/                  # game logic (library target)
│  ├─ playground/           # small C++ demo app
│  ├─ CMakeLists.txt, conanfile.py
│  └─ build/                # out-of-source build trees (Debug/Release)
└─ frontend/                # Svelte + Vite app
	 ├─ src/
	 ├─ vite.config.js
	 └─ package.json
```

## Configuration
- Backend port: pass as argv to `spread_server` (default 8080).
- Frontend WebSocket URL: defined in `frontend/src/stores/WebSocketStore.js`. Default is `ws://<hostname>:8080` or `wss://` on HTTPS.

## API
- The WebSocket API is described in `async_api.yaml`.
- On connect, the client typically sends a `list_lobbies` message; the server responds with state updates and may emit `server_ready` with a `player_id`.
