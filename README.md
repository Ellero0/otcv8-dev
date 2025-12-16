# EZODUS - OTCv8 Fork for Gunzodus

Fork of OTCv8 with **Native Minimap Markers** - 100x faster marker loading (6000+ markers in ~0.5s vs ~80s with Lua widgets).

## Key Features

- **Native C++ Markers** - Bypass Lua widget overhead for minimap markers
- **JSON Marker Loading** - Load thousands of markers from JSON files instantly
- **Full OTCv8 Compatibility** - All original features preserved

## Native Markers API

```lua
g_minimap.loadMarkersFromJson("/markers.json")  -- Load from JSON
g_minimap.addMarker(position, iconId, "desc")   -- Add single marker
g_minimap.removeMarker(position)                -- Remove marker
g_minimap.clearMarkers()                        -- Clear all
g_minimap.getMarkerCount()                      -- Get count
g_minimap.hasMarker(position)                   -- Check existence
```

## JSON Marker Format

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

---

This repository uses Github Actions to build and test automatically.

Check Actions tab to see test results or download latest binaries. ![Workflow status](https://github.com/OTCv8/otcv8-dev/actions/workflows/ci-cd.yml/badge.svg)

## Compilation

### Automatic

You can clone repoistory and use github action build-on-request workload.

### Windows

You need visual studio 2019 and vcpkg with commit `3b3bd424827a1f7f4813216f6b32b6c61e386b2e` ([download](https://github.com/microsoft/vcpkg/archive/3b3bd424827a1f7f4813216f6b32b6c61e386b2e.zip)).

Then you install vcpkg dependencies:
```bash
vcpkg install boost-iostreams:x86-windows-static boost-asio:x86-windows-static boost-beast:x86-windows-static boost-system:x86-windows-static boost-variant:x86-windows-static boost-lockfree:x86-windows-static boost-process:x86-windows-static boost-program-options:x86-windows-static luajit:x86-windows-static glew:x86-windows-static boost-filesystem:x86-windows-static boost-uuid:x86-windows-static physfs:x86-windows-static openal-soft:x86-windows-static libogg:x86-windows-static libvorbis:x86-windows-static zlib:x86-windows-static libzip:x86-windows-static openssl:x86-windows-static
```

and then you can compile static otcv8 version.

### Linux

on linux you need:
- vcpkg from commit `761c81d43335a5d5ccc2ec8ad90bd7e2cbba734e`
- boost >=1.67 and libzip-dev, physfs >= 3
- gcc >=9

Then just run mkdir build && cd build && cmake .. && make -j8

### Android

To compile on android you need to create C:\android with
- android-ndk-r21b https://dl.google.com/android/repository/android-ndk-r21d-windows-x86_64.zip
- libs from android_libs.7z

Also install android extension for visual studio
In visual studio go to options -> cross platform -> c++ and set Android NDK to C:\android\android-ndk-r21b
Right click on otclientv8 -> proporties -> general and change target api level to android-25

Put data.zip in android/otclientv8/assets
You can use powershell script create_android_assets.ps1 to create them automaticly (won't be encrypted)

## Useful tips

- To run tests manually, unpack tests.7z and use command `otclient_debug.exe --test`
- To test mobile UI use command `otclient_debug.exe --mobile`

## Links

- Discord: https://discord.gg/feySup6
- Forum: http://otclient.net
- Email: otclient@otclient.ovh
