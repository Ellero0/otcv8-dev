# Changelog

All notable changes to EZODUS (OTCv8 fork for Gunzodus) are documented in this file.

## [1.0.0] - 2025-12-16

### Added

#### Native Minimap Markers System
- **100x faster marker loading** - 6000+ markers load in ~0.5s vs ~80s with Lua widgets
- Native C++ `MinimapMarker` struct with position, icon, and description
- `std::unordered_map` storage with O(1) lookup using position hash
- Direct rendering in `Minimap::draw()` with viewport culling
- JSON marker loading via nlohmann-json library

#### New Lua API (`g_minimap`)
- `g_minimap.loadMarkersFromJson(path)` - Load markers from JSON file
- `g_minimap.addMarker(position, iconId, description)` - Add single marker
- `g_minimap.removeMarker(position)` - Remove marker at position
- `g_minimap.clearMarkers()` - Clear all markers
- `g_minimap.getMarkerCount()` - Get total marker count
- `g_minimap.hasMarker(position)` - Check if marker exists

#### JSON Marker Format
```json
[
  {"x": 32000, "y": 32000, "z": 7, "icon": "flag", "description": "My marker"}
]
```

**Supported Icons:**

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

### Fixed

#### MSVC Build Compatibility
- Added const template overloads in `luabinder.h` for const member functions
- Fixes C2672/C2782 template resolution errors when binding `getMarkerCount() const` and `hasMarker() const`

#### GitHub Actions Workflow
- Updated deprecated actions to v4 (`actions/checkout`, `actions/upload-artifact`)
- Updated `microsoft/setup-msbuild` to v2
- Updated runner images (`macos-13`, `ubuntu-22.04`)
- Updated vcpkg to `2024.07.12` release - fixes MSYS2 404 mirror errors
- Added `nlohmann-json` to vcpkg dependencies

### Changed

#### Modified Files
| File | Changes |
|------|---------|
| `src/client/minimap.h` | Added `MinimapMarker` struct, marker method declarations |
| `src/client/minimap.cpp` | Marker storage, CRUD operations, JSON loading, rendering |
| `src/client/luafunctions_client.cpp` | Lua bindings for `g_minimap` marker functions |
| `src/framework/luaengine/luabinder.h` | Const template overloads for MSVC |
| `.github/workflows/build-on-request.yml` | Updated actions and vcpkg version |
| `.github/workflows/build-windows.yml` | Updated vcpkg version |

## Performance Comparison

| Operation | Lua Widgets | Native C++ | Improvement |
|-----------|-------------|------------|-------------|
| Load 6000 markers | ~80 seconds | ~0.5 seconds | **160x faster** |
| Memory per marker | ~2KB (widget overhead) | ~64 bytes | **32x smaller** |
| Add single marker | ~15ms | <0.1ms | **150x faster** |
| Remove marker | ~10ms | <0.1ms | **100x faster** |

## Migration Guide

### From Lua Widget Markers

**Before (slow):**
```lua
-- Creating 6000 widgets takes ~80 seconds
for _, marker in ipairs(markers) do
    local widget = g_ui.createWidget('MinimapMarker', minimapWidget)
    widget:setPosition(marker.pos)
    widget:setIcon(marker.icon)
end
```

**After (fast):**
```lua
-- Native loading takes ~0.5 seconds
g_minimap.loadMarkersFromJson('/markers.json')

-- Or add individually
g_minimap.addMarker(pos, 8, "Flag marker")  -- 8 = flag icon
```

### Icon ID Reference

| ID | Icon | Name | ID | Icon | Name |
|:--:|:----:|------|:--:|:----:|------|
| 0 | ![checkmark](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/checkmark.png) | checkmark | 10 | ![skull](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/skull.png) | skull |
| 1 | ![?](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/question.png) | ? | 11 | ![$](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/dollar.png) | $ |
| 2 | ![!](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/exclamation.png) | ! | 12 | ![red-up](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/red-up.png) | red up |
| 3 | ![star](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/star.png) | star | 13 | ![red-down](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/red-down.png) | red down |
| 4 | ![crossmark](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/crossmark.png) | crossmark | 14 | ![red-right](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/red-right.png) | red right |
| 5 | ![temple](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/cross.png) | temple | 15 | ![red-left](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/red-left.png) | red left |
| 6 | ![brush](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/bag.png) | brush | 16 | ![green-up](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/up.png) | green up |
| 7 | ![sword](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/sword.png) | sword | 17 | ![green-down](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/down.png) | green down |
| 8 | ![flag](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/flag.png) | flag | | | |
| 9 | ![lock](https://raw.githubusercontent.com/tibiamaps/tibia-map/main/src/_img/marker-icons/lock.png) | lock | | | |
