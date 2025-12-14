/*
 * Copyright (c) 2010-2017 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "minimap.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "tile.h"
#include "game.h"
#include "spritemanager.h"

#include <framework/graphics/image.h>
#include <framework/graphics/texture.h>
#include <framework/graphics/painter.h>
#include <framework/graphics/image.h>
#include <framework/graphics/framebuffermanager.h>
#include <framework/graphics/texturemanager.h>
#include <framework/core/resourcemanager.h>
#include <framework/core/filestream.h>
#include <zlib.h>

#include <framework/util/stats.h>

Minimap g_minimap;

void MinimapBlock::clean()
{
    m_tiles.fill(MinimapTile());
    m_texture.reset();
    m_mustUpdate = false;
}

void MinimapBlock::update()
{
    if(!m_mustUpdate)
        return;

    ImagePtr image(new Image(Size(MMBLOCK_SIZE, MMBLOCK_SIZE)));

    bool shouldDraw = false;
    for(int x=0;x<MMBLOCK_SIZE;++x) {
        for(int y=0;y<MMBLOCK_SIZE;++y) {
            uint8 c = getTile(x, y).color;
            Color col = Color::alpha;
            if(c != 255) {
                col = Color::from8bit(c);
                shouldDraw = true;
            }
            image->setPixel(x, y, col);
        }
    }

    if(shouldDraw) {
        m_texture = TexturePtr(new Texture(image));
    } else
        m_texture.reset();

    m_mustUpdate = false;
}

void MinimapBlock::updateTile(int x, int y, const MinimapTile& tile)
{
    if(m_tiles[getTileIndex(x,y)].color != tile.color)
        m_mustUpdate = true;

    m_tiles[getTileIndex(x,y)] = tile;
}

void Minimap::init()
{
}

void Minimap::terminate()
{
    clean();
}

void Minimap::clean()
{
    std::lock_guard<std::mutex> lock(m_lock);
    for(int i=0;i<=Otc::MAX_Z;++i)
        m_tileBlocks[i].clear();
}

void Minimap::draw(const Rect& screenRect, const Position& mapCenter, float scale, const Color& color)
{
    if(screenRect.isEmpty())
        return;

    Rect mapRect = calcMapRect(screenRect, mapCenter, scale);
    g_drawQueue->addFilledRect(screenRect, color);

    if(MMBLOCK_SIZE*scale <= 1 || !mapCenter.isMapPosition()) {
        return;
    }

    size_t drawQueueStart = g_drawQueue->size();
    Point blockOff = getBlockOffset(mapRect.topLeft());
    Point off = Point((mapRect.size() * scale).toPoint() - screenRect.size().toPoint())/2;
    Point start = screenRect.topLeft() -(mapRect.topLeft() - blockOff)*scale - off;

    for(int y = blockOff.y, ys = start.y;ys<screenRect.bottom();y += MMBLOCK_SIZE, ys += MMBLOCK_SIZE*scale) {
        if(y < 0 || y >= 65536)
            continue;

        for(int x = blockOff.x, xs = start.x;xs<screenRect.right();x += MMBLOCK_SIZE, xs += MMBLOCK_SIZE*scale) {
            if(x < 0 || x >= 65536)
                continue;

            Position blockPos(x, y, mapCenter.z);
            if(!hasBlock(blockPos))
                continue;

            MinimapBlock& block = getBlock(Position(x, y, mapCenter.z));
            block.update();

            const TexturePtr& tex = block.getTexture();
            if(tex) {
                Rect src(0, 0, MMBLOCK_SIZE, MMBLOCK_SIZE);
                Rect dest(xs, ys, MMBLOCK_SIZE * scale, MMBLOCK_SIZE * scale);

                g_drawQueue->addTexturedRect(dest, tex, src);
            }
        }
    }

    
    // Render native markers
    if (!m_markers.empty()) {
        int tileSize = g_sprites.spriteSize() * scale;
        Point screenCenter = screenRect.center();

        static TexturePtr markerIcons = nullptr;
        if (!markerIcons) {
            markerIcons = g_textures.getTexture("/images/game/minimap/mapflags");
        }

        if (markerIcons) {
            int iconSize = 11;
            int iconsPerRow = markerIcons->getWidth() / iconSize;
            int tilesX = screenRect.width() / tileSize + 2;
            int tilesY = screenRect.height() / tileSize + 2;

            for (const auto& pair : m_markers) {
                const MinimapMarker& marker = pair.second;
                if (marker.pos.z != mapCenter.z)
                    continue;

                int dx = (int)marker.pos.x - (int)mapCenter.x;
                int dy = (int)marker.pos.y - (int)mapCenter.y;

                if (std::abs(dx) > tilesX / 2 || std::abs(dy) > tilesY / 2)
                    continue;

                int sx = screenCenter.x + dx * tileSize - iconSize / 2;
                int sy = screenCenter.y + dy * tileSize - iconSize / 2;

                int iconX = (marker.icon % iconsPerRow) * iconSize;
                int iconY = (marker.icon / iconsPerRow) * iconSize;

                Rect dest(sx, sy, iconSize, iconSize);
                Rect src(iconX, iconY, iconSize, iconSize);
                g_drawQueue->addTexturedRect(dest, markerIcons, src);
            }
        }
    }

    g_drawQueue->setClip(drawQueueStart, screenRect);
}

Point Minimap::getTilePoint(const Position& pos, const Rect& screenRect, const Position& mapCenter, float scale)
{
    if(screenRect.isEmpty() || pos.z != mapCenter.z)
        return Point(-1,-1);

    Rect mapRect = calcMapRect(screenRect, mapCenter, scale);
    Point off = Point((mapRect.size() * scale).toPoint() - screenRect.size().toPoint())/2;
    Point posoff = (Point(pos.x,pos.y) - mapRect.topLeft())*scale;
    return posoff + screenRect.topLeft() - off + (Point(1,1)*scale)/2;
}

Position Minimap::getTilePosition(const Point& point, const Rect& screenRect, const Position& mapCenter, float scale)
{
    if(screenRect.isEmpty())
        return Position();

    Rect mapRect = calcMapRect(screenRect, mapCenter, scale);
    Point off = Point((mapRect.size() * scale).toPoint() - screenRect.size().toPoint())/2;
    Point pos2d = (point - screenRect.topLeft() + off)/scale + mapRect.topLeft();
    return Position(pos2d.x, pos2d.y, mapCenter.z);
}

Rect Minimap::getTileRect(const Position& pos, const Rect& screenRect, const Position& mapCenter, float scale)
{
    if(screenRect.isEmpty() || pos.z != mapCenter.z)
        return Rect();

    int tileSize = g_sprites.spriteSize() * scale;
    Rect tileRect(0,0,tileSize, tileSize);
    tileRect.moveCenter(getTilePoint(pos, screenRect, mapCenter, scale));
    return tileRect;
}

Rect Minimap::calcMapRect(const Rect& screenRect, const Position& mapCenter, float scale)
{
    int w = screenRect.width() / scale, h = std::ceil(screenRect.height() / scale);
    Rect mapRect(0,0,w,h);
    mapRect.moveCenter(Point(mapCenter.x, mapCenter.y));
    return mapRect;
}

void Minimap::updateTile(const Position& pos, const TilePtr& tile)
{
    MinimapTile minimapTile;
    if(tile) {
        minimapTile.color = tile->getMinimapColorByte();
        minimapTile.flags |= MinimapTileWasSeen;
        if(!tile->isWalkable(true))
            minimapTile.flags |= MinimapTileNotWalkable;
        if(!tile->isPathable())
            minimapTile.flags |= MinimapTileNotPathable;
        minimapTile.speed = std::min<int>((int)std::ceil(tile->getGroundSpeed() / 10.0f), 255);
    } else {
        minimapTile.color = 255;
        minimapTile.flags |= MinimapTileEmpty;
        minimapTile.speed = 1;
    }

    if(minimapTile != MinimapTile()) {
        MinimapBlock& block = getBlock(pos);
        Point offsetPos = getBlockOffset(Point(pos.x, pos.y));
        block.updateTile(pos.x - offsetPos.x, pos.y - offsetPos.y, minimapTile);
        block.justSaw();
    }
}

const MinimapTile& Minimap::getTile(const Position& pos)
{
    static MinimapTile nulltile;
    if(pos.z <= Otc::MAX_Z && hasBlock(pos)) {
        MinimapBlock& block = getBlock(pos);
        Point offsetPos = getBlockOffset(Point(pos.x, pos.y));
        return block.getTile(pos.x - offsetPos.x, pos.y - offsetPos.y);
    }
    return nulltile;
}

std::pair<MinimapBlock_ptr, MinimapTile> Minimap::threadGetTile(const Position& pos) {
    std::lock_guard<std::mutex> lock(m_lock);
    static MinimapTile nulltile;
    
    if (pos.z <= Otc::MAX_Z && hasBlock(pos)) {
        MinimapBlock_ptr block = m_tileBlocks[pos.z][getBlockIndex(pos)];
        if (block) {
            Point offsetPos = getBlockOffset(Point(pos.x, pos.y));
            return std::make_pair(block, block->getTile(pos.x - offsetPos.x, pos.y - offsetPos.y));
        }
    }
    return std::make_pair(nullptr, nulltile);
}

bool Minimap::loadImage(const std::string& fileName, const Position& topLeft, float colorFactor)
{
    if(colorFactor <= 0.01f)
        colorFactor = 1.0f;

    try {
        ImagePtr image = Image::load(fileName);

        uint8 waterc = Color::to8bit(std::string("#3300cc"));

        // non pathable colors
        Color nonPathableColors[] = {
            std::string("#ffff00"), // yellow
        };

        // non walkable colors
        Color nonWalkableColors[] = {
            std::string("#000000"), // oil, black
            std::string("#006600"), // trees, dark green
            std::string("#ff3300"), // walls, red
            std::string("#666666"), // mountain, grey
            std::string("#ff6600"), // lava, orange
            std::string("#00ff00"), // positon
            std::string("#ccffff"), // ice, very light blue
        };

        for(int y=0;y<image->getHeight();++y) {
            for(int x=0;x<image->getWidth();++x) {
                Color color = *(uint32*)image->getPixel(x,y);
                uint8 c = Color::to8bit(color * colorFactor);
                int flags = 0;

                if(c == waterc || color.a() == 0) {
                    flags |= MinimapTileNotWalkable;
                    c = 255; // alpha
                }

                if(flags != 0) {
                    for(Color &col : nonWalkableColors) {
                        if(col == color) {
                            flags |= MinimapTileNotWalkable;
                            break;
                        }
                    }
                }

                if(flags != 0) {
                    for(Color &col : nonPathableColors) {
                        if(col == color) {
                            flags |= MinimapTileNotPathable;
                            break;
                        }
                    }
                }

                if(c == 255)
                    continue;

                Position pos(topLeft.x + x, topLeft.y + y, topLeft.z);
                MinimapBlock& block = getBlock(pos);
                Point offsetPos = getBlockOffset(Point(pos.x, pos.y));
                MinimapTile& tile = block.getTile(pos.x - offsetPos.x, pos.y - offsetPos.y);
                if(!(tile.flags & MinimapTileWasSeen)) {
                    tile.color = c;
                    tile.flags = flags;
                    block.mustUpdate();
                }
            }
        }
        return true;
    } catch(stdext::exception& e) {
        g_logger.error(stdext::format("failed to load OTMM minimap: %s", e.what()));
        return false;
    }
}

void Minimap::saveImage(const std::string& fileName, const Rect& mapRect)
{
    //TODO
}

bool Minimap::loadOtmm(const std::string& fileName)
{
    try {
        FileStreamPtr fin = g_resources.openFile(fileName, g_game.getFeature(Otc::GameDontCacheFiles));
        if(!fin)
            stdext::throw_exception("unable to open file");

        uint32 signature = fin->getU32();
        if(signature != OTMM_SIGNATURE)
            stdext::throw_exception("invalid OTMM file");

        uint16 start = fin->getU16();
        uint16 version = fin->getU16();
        fin->getU32(); // flags

        switch(version) {
            case 1: {
                fin->getString(); // description
                break;
            }
            default:
                stdext::throw_exception("OTMM version not supported");
        }

        fin->seek(start);

        uint blockSize = MMBLOCK_SIZE * MMBLOCK_SIZE * sizeof(MinimapTile);
        std::vector<uchar> compressBuffer(compressBound(blockSize));
        std::vector<uchar> decompressBuffer(blockSize);

        while(true) {
            Position pos;
            pos.x = fin->getU16();
            pos.y = fin->getU16();
            pos.z = fin->getU8();

            // end of file or file is corrupted
            if(!pos.isValid() || pos.z >= Otc::MAX_Z+1)
                break;

            MinimapBlock& block = getBlock(pos);
            ulong len = fin->getU16();
            ulong destLen = blockSize;
            fin->read(compressBuffer.data(), len);
            int ret = uncompress(decompressBuffer.data(), &destLen, compressBuffer.data(), len);
            if(ret != Z_OK || destLen != blockSize)
                break;

            memcpy((uchar*)&block.getTiles(), decompressBuffer.data(), blockSize);
            block.mustUpdate();
            block.justSaw();
        }

        fin->close();
        return true;
    } catch(stdext::exception& e) {
        g_logger.error(stdext::format("failed to load OTMM minimap: %s", e.what()));
        return false;
    }
}

void Minimap::saveOtmm(const std::string& fileName)
{
    try {
        stdext::timer saveTimer;

#ifndef ANDROID
        std::string tmpFileName = fileName;
        tmpFileName += ".tmp";
        FileStreamPtr fin = g_resources.createFile(tmpFileName);
#else
        FileStreamPtr fin = g_resources.createFile(fileName);
#endif

        //TODO: compression flag with zlib
        uint32 flags = 0;

        // header
        fin->addU32(OTMM_SIGNATURE);
        fin->addU16(0); // data start, will be overwritten later
        fin->addU16(OTMM_VERSION);
        fin->addU32(flags);

        // version 1 header
        fin->addString("OTMM 1.0"); // description

        // go back and rewrite where the map data starts
        uint32 start = fin->tell();
        fin->seek(4);
        fin->addU16(start);
        fin->seek(start);

        uint blockSize = MMBLOCK_SIZE * MMBLOCK_SIZE * sizeof(MinimapTile);
        std::vector<uchar> compressBuffer(compressBound(blockSize));
        const int COMPRESS_LEVEL = 3;

        for(uint8_t z = 0; z <= Otc::MAX_Z; ++z) {
            for(auto& it : m_tileBlocks[z]) {
                int index = it.first;
                MinimapBlock& block = *it.second;
                if(!block.wasSeen())
                    continue;

                Position pos = getIndexPosition(index, z);
                fin->addU16(pos.x);
                fin->addU16(pos.y);
                fin->addU8(pos.z);

                ulong len = blockSize;
                int ret = compress2(compressBuffer.data(), &len, (uchar*)&block.getTiles(), blockSize, COMPRESS_LEVEL);
                VALIDATE(ret == Z_OK);
                fin->addU16(len);
                fin->write(compressBuffer.data(), len);
            }
        }

        // end of file
        Position invalidPos;
        fin->addU16(invalidPos.x);
        fin->addU16(invalidPos.y);
        fin->addU8(invalidPos.z);

        fin->flush();

        fin->close();
#ifndef ANDROID
        std::filesystem::path filePath(g_resources.getWriteDir()), tmpFilePath(g_resources.getWriteDir());
        filePath += fileName;
        tmpFilePath += tmpFileName;
        if(std::filesystem::file_size(tmpFilePath) > 1024) {
            std::filesystem::rename(tmpFilePath, filePath);
        }
#endif
    } catch (stdext::exception& e) {
        g_logger.error(stdext::format("failed to save OTMM minimap: %s", e.what()));
    } catch (std::exception& e) {
        g_logger.error(stdext::format("failed to save OTMM minimap: %s", e.what()));
    }
}


// ============== Native Marker Methods ==============

void Minimap::addMarker(const Position& pos, uint8_t icon, const std::string& description)
{
    uint64_t key = getMarkerKey(pos);
    m_markers[key] = MinimapMarker(pos, icon, description);
}

void Minimap::removeMarker(const Position& pos)
{
    uint64_t key = getMarkerKey(pos);
    m_markers.erase(key);
}

void Minimap::clearMarkers()
{
    m_markers.clear();
}

bool Minimap::hasMarker(const Position& pos) const
{
    uint64_t key = getMarkerKey(pos);
    return m_markers.find(key) != m_markers.end();
}

MinimapMarker Minimap::getMarker(const Position& pos) const
{
    uint64_t key = getMarkerKey(pos);
    auto it = m_markers.find(key);
    if (it != m_markers.end())
        return it->second;
    return MinimapMarker();
}

std::vector<MinimapMarker> Minimap::getMarkersInRange(const Position& center, int range) const
{
    std::vector<MinimapMarker> result;
    for (const auto& pair : m_markers) {
        const MinimapMarker& m = pair.second;
        if (m.pos.z == center.z &&
            std::abs((int)m.pos.x - (int)center.x) <= range &&
            std::abs((int)m.pos.y - (int)center.y) <= range) {
            result.push_back(m);
        }
    }
    return result;
}

uint8_t Minimap::parseIconString(const std::string& iconStr)
{
    static std::unordered_map<std::string, uint8_t> iconMap = {
        {"checkmark", 0}, {"?", 1}, {"!", 2}, {"star", 3},
        {"crossmark", 4}, {"temple", 5}, {"brush", 6}, {"sword", 7},
        {"flag", 8}, {"lock", 9}, {"skull", 10}, {"$", 11},
        {"dollar", 11}, {"red up", 12}, {"red down", 13},
        {"red right", 14}, {"red left", 15}, {"green up", 16},
        {"green down", 17}, {"green right", 18}, {"green left", 19},
        {"up", 12}, {"down", 13}, {"right", 14}, {"left", 15}
    };

    std::string lower = iconStr;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    auto it = iconMap.find(lower);
    if (it != iconMap.end())
        return it->second;
    return 8; // default flag
}

void Minimap::loadMarkersFromJson(const std::string& jsonPath)
{
    try {
        std::string jsonData = g_resources.readFileContents(jsonPath);
        if (jsonData.empty()) {
            g_logger.error(stdext::format("Failed to read markers file: %s", jsonPath));
            return;
        }

        auto markers = json::parse(jsonData);

        clearMarkers();
        int loaded = 0;
        for (const auto& m : markers) {
            if (m.contains("x") && m.contains("y") && m.contains("z")) {
                Position pos;
                pos.x = m["x"].get<int>();
                pos.y = m["y"].get<int>();
                pos.z = m["z"].get<int>();

                uint8_t icon = 8;
                if (m.contains("icon") && m["icon"].is_string()) {
                    icon = parseIconString(m["icon"].get<std::string>());
                }

                std::string desc = "NO_DESCRIPTION";
                if (m.contains("description") && m["description"].is_string()) {
                    desc = m["description"].get<std::string>();
                    if (desc.empty()) desc = "NO_DESCRIPTION";
                }

                addMarker(pos, icon, desc);
                loaded++;
            }
        }

        m_markersLoaded = true;
        g_logger.info(stdext::format("[Minimap] Loaded %d native markers from %s", loaded, jsonPath));

    } catch (const std::exception& e) {
        g_logger.error(stdext::format("Failed to parse markers JSON: %s", e.what()));
    }
}

