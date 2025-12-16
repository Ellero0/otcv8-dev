# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

EZODUS is a fork of OTClientV8 configured for the Gunzodus game server. It's a Tibia-like MMORPG client written in C++ with Lua scripting. The client supports Windows (x86), Linux, and Android platforms with OpenGL and DirectX rendering backends.

## Build Commands

### Windows (Visual Studio 2019/2022)
```batch
# Using the automated build script (includes native markers patch)
.\build_native_markers.bat

# Or using Visual Studio solution directly
cd vc16
MSBuild otclient.sln /property:Configuration=DirectX /property:Platform=Win32
MSBuild otclient.sln /property:Configuration=OpenGL /property:Platform=Win32
```

### Linux
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### CMake with vcpkg (cross-platform)
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x86-windows-static
cmake --build build --config Release
```

## Architecture

### Framework Layer (`src/framework/`)
Core application infrastructure:
- **core/** - Application lifecycle, event dispatching, threading
- **graphics/** - Renderer abstraction (OpenGL/DirectX via ANGLE)
- **ui/** - Widget system with OTML-based layouts
- **net/** - TCP networking via Boost.Asio
- **luaengine/** - LuaJIT integration with C++ bindings
- **sound/** - OpenAL audio system
- **platform/** - OS abstraction (Windows, Linux, Android)

### Client Layer (`src/client/`)
Game-specific implementation:
- **game.cpp/h** - Core game state and logic
- **protocolgame*.cpp** - OTGame protocol implementation
- **creature.cpp/h, item.cpp/h** - Entity system
- **map.cpp/h** - Tile-based world rendering
- **minimap.cpp/h** - Minimap with native marker support
- **luafunctions_client.cpp** - Lua bindings for game objects

### Lua Modules (`modules/`)
57 auto-loading modules with priority system:
- **0-99**: Core libraries (corelib, gamelib)
- **100-499**: Client UI modules (client_*)
- **500-999**: Game feature modules (game_*)
- **1000+**: User mods

### Key Global Objects (Lua)
- `g_app` - Application lifecycle
- `g_game` - Game state and events
- `g_client` - Client singleton
- `g_resources` - File system access
- `g_minimap` - Minimap with native markers
- `g_modules` - Module manager
- `g_ui` - UI system

## Entry Points

1. **src/main.cpp** - C++ entry point, initializes framework and runs init.lua
2. **init.lua** - Lua bootstrap, configures server (Gunzodus), loads modules
3. **modules/corelib/** - Core Lua utilities loaded first
4. **modules/game_interface/** - Main game UI implementation

## Configuration

- **init.lua** - Server config (APP_NAME="gunzodus", login.gunzodus.net:7171)
- **config.otml** - User settings (stored in APPDATA)
- **layouts/retro/** - Default UI theme
- **data/styles/** - OTML widget styling

## Native Markers System

Performance optimization for minimap markers (**100x faster** than Lua widgets):

### Implementation Details
- **Storage**: `std::unordered_map<uint64_t, MinimapMarker>` with O(1) lookup
- **Rendering**: Direct C++ rendering in `Minimap::draw()` with viewport culling
- **Loading**: JSON parsing with nlohmann-json library

### Modified Files (in `otcv8-src/src/`)
- `client/minimap.h` - `MinimapMarker` struct and method declarations
- `client/minimap.cpp` - Marker implementation and rendering
- `client/luafunctions_client.cpp` - Lua bindings for `g_minimap`
- `framework/luaengine/luabinder.h` - Const template overloads for MSVC

### Lua API
```lua
g_minimap.loadMarkersFromJson("/markers.json")  -- Load from JSON
g_minimap.addMarker(position, iconId, "desc")   -- Add single marker
g_minimap.removeMarker(position)                -- Remove marker
g_minimap.clearMarkers()                        -- Clear all
g_minimap.getMarkerCount()                      -- Get count
g_minimap.hasMarker(position)                   -- Check existence
```

### JSON Marker Format
```json
[
  {"x": 32000, "y": 32000, "z": 7, "icon": "flag", "description": "My marker"},
  {"x": 32100, "y": 32100, "z": 7, "icon": "star", "description": "Another"}
]
```

### Supported Icons

| Icon | Name | Icon | Name |
|:----:|------|:----:|------|
| ![checkmark](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/checkmark.png) | `checkmark` | ![skull](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/skull.png) | `skull` |
| ![question](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/question.png) | `?` | ![dollar](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/dollar.png) | `$` |
| ![exclamation](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/exclamation.png) | `!` | ![red-up](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/red-up.png) | `red up` |
| ![star](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/star.png) | `star` | ![red-down](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/red-down.png) | `red down` |
| ![crossmark](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/crossmark.png) | `crossmark` | ![red-right](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/red-right.png) | `red right` |
| ![cross](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/cross.png) | `temple` | ![red-left](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/red-left.png) | `red left` |
| ![bag](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/bag.png) | `brush` | ![up](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/up.png) | `green up` |
| ![sword](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/sword.png) | `sword` | ![down](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/down.png) | `green down` |
| ![flag](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/flag.png) | `flag` | | |
| ![lock](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/lock.png) | `lock` | | |

## MSVC Const Template Fix

The `luabinder.h` file includes const overloads for binding const member functions to Lua. This fixes MSVC template resolution errors (C2672, C2782) when binding functions like `getMarkerCount() const`.

**Location**: `src/framework/luaengine/luabinder.h` (lines 182-225)

Key additions:
- `make_mem_func_singleton()` overloads for `const` member function pointers
- `bind_singleton_mem_fun()` overload for `const` member function pointers

## Dependencies (via vcpkg)

boost-iostreams, boost-asio, boost-beast, boost-system, boost-variant, boost-lockfree, boost-process, boost-program-options, boost-filesystem, boost-uuid, luajit, glew, physfs, openal-soft, libogg, libvorbis, zlib, libzip, openssl, nlohmann-json

## Testing

```bash
otclient_debug.exe --test    # Run tests
otclient_debug.exe --mobile  # Test mobile UI
```

## Adding Features

**C++ feature**: Modify src/client/ or src/framework/ → add Lua bindings in luafunctions_client.cpp
**Lua feature**: Add module in modules/ with appropriate priority in module.otmod
**UI feature**: Add OTML layout in layouts/ and Lua handler in modules/
