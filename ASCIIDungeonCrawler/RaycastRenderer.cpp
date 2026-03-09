#define NOMINMAX
#include "RaycastRenderer.h"
#include <cmath>
#include <algorithm>

namespace DungeonGame {

    RaycastRenderer::RaycastRenderer(sf::RenderWindow& window)
        : m_lines(sf::Lines) {

        m_screenW = (int)window.getSize().x;
        m_screenH = (int)window.getSize().y;
        m_drawWidth = (int)(m_screenW * 0.62f);

        m_minimapX = m_screenW - (MAP_WIDTH * MINIMAP_SCALE) + 10;
        m_minimapY = m_screenH - (MAP_HEIGHT * MINIMAP_SCALE) - 10;

        m_zBuffer.resize(m_drawWidth, 0.f);

        m_wallImage.loadFromFile("assets/texture_wall.png");
        m_floorImage.loadFromFile("assets/texture_floor.png");
        m_ceilImage.loadFromFile("assets/texture_ceiling.png");

        m_floorBuffer.create(m_drawWidth, m_screenH, sf::Color::Black);
        m_floorTexture.create(m_drawWidth, m_screenH);
        m_floorSprite.setTexture(m_floorTexture);

        // preload sprite textures into cache
        getSpriteTexture("assets/texture_chest_closed.png");
        getSpriteTexture("assets/texture_chest_opened.png");
        getSpriteTexture("assets/texture_doors.png");
        getSpriteTexture("assets/enemy_grunt.png");
        getSpriteTexture("assets/enemy_trickster.png");
        getSpriteTexture("assets/enemy_brute.png");
        getSpriteTexture("assets/texture_merchant.png");
    }

    const sf::Texture* RaycastRenderer::getSpriteTexture(const std::string& path) {
        if (path.empty()) return nullptr;
        auto it = m_spriteTextures.find(path);
        if (it != m_spriteTextures.end())
            return &it->second;
        sf::Texture tex;
        if (tex.loadFromFile(path)) {
            tex.setSmooth(true);
            m_spriteTextures[path] = std::move(tex);
        }
        else {
            return nullptr;
        }
        return &m_spriteTextures[path];
    }

    bool RaycastRenderer::isWall(const Dungeon& dungeon, int tx, int ty) const {
        if (tx < 0 || tx >= MAP_WIDTH || ty < 0 || ty >= MAP_HEIGHT)
            return true; // out of bounds = solid
        const Tile& tile = dungeon.getGrid()[ty][tx];
        return tile.type != TileType::Floor;
    }

    sf::Color RaycastRenderer::wallColor(float distance, float brightness, float lightRadius) const {
        float fog = std::max(0.f, 1.f - distance / lightRadius);
        fog = fog * fog; // quadratic falloff — darker the further away the player is
        int   value = (int)(brightness * fog * 255.f);
        value = std::clamp(value, 0, 255);
        return sf::Color(value, value, value);
    }

    void RaycastRenderer::draw(sf::RenderWindow& window,
        const Dungeon& dungeon,
        const Player& player,
        float dt) {

        m_time += dt;

        // light radius — torch equipped = 6 tiles, no torch = 1.5 tiles
        float lightRadius = player.equipment.torch.has_value() ? 6.f : 1.5f;

        // flicker — only when torch equipped
        if (player.equipment.torch.has_value()) {
            float flicker = std::sin(m_time * 8.f) * 0.08f;
            lightRadius += flicker;
        }

        // draw floor and ceiling
        drawFloorCeiling(window, player, lightRadius);

        float px = player.visualX + 0.5f;
        float py = player.visualY + 0.5f;

        // direction vector and camera plane from player angle
        float dirX = std::cos(player.angle);
        float dirY = std::sin(player.angle);
        // camera plane perpendicular to direction, length controls FOV
        float planeX = -dirY * 0.66f;
        float planeY = dirX * 0.66f;

        m_lines.clear();

        for (int col = 0; col < m_drawWidth; ++col) {
            // camera x in [-1, 1]
            float cameraX = 2.f * col / (float)m_drawWidth - 1.f;

            float rayDirX = dirX + planeX * cameraX;
            float rayDirY = dirY + planeY * cameraX;

            // current tile
            int mapX = (int)px;
            int mapY = (int)py;

            // length of ray from one x/y side to next
            float deltaDistX = (rayDirX == 0.f) ? 1e30f : std::abs(1.f / rayDirX);
            float deltaDistY = (rayDirY == 0.f) ? 1e30f : std::abs(1.f / rayDirY);

            float sideDistX, sideDistY;
            int stepX, stepY;

            // step direction and initial side distances
            if (rayDirX < 0.f) { stepX = -1; sideDistX = (px - mapX) * deltaDistX; }
            else { stepX = 1; sideDistX = (mapX + 1.f - px) * deltaDistX; }
            if (rayDirY < 0.f) { stepY = -1; sideDistY = (py - mapY) * deltaDistY; }
            else { stepY = 1; sideDistY = (mapY + 1.f - py) * deltaDistY; }

            // DDA
            int  side = 0; 
            bool hit = false;
            for (int i = 0; i < 64 && !hit; ++i) {
                if (sideDistX < sideDistY) {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                }
                else {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }
                if (isWall(dungeon, mapX, mapY)) hit = true;
            }

            // perpendicular distance
            float perpDist = (side == 0)
                ? (sideDistX - deltaDistX)
                : (sideDistY - deltaDistY);

            if (perpDist < 0.0001f) perpDist = std::max(0.5f, perpDist);

            m_zBuffer[col] = perpDist;

            int wallH = (int)((float)m_screenH / perpDist);
            int wallTop = m_screenH / 2 - wallH / 2;
            int wallBot = m_screenH / 2 + wallH / 2;

            float wallX;
            if (side == 0) wallX = py + perpDist * rayDirY;
            else           wallX = px + perpDist * rayDirX;
            wallX -= std::floor(wallX);

            int texX = (int)(wallX * TEX_SIZE);
            texX = std::clamp(texX, 0, TEX_SIZE - 1);

            float fog = std::max(0.f, 1.f - perpDist / lightRadius);
            fog = fog * fog;
            float brightness = (side == 0) ? 1.0f : 0.65f;

            for (int y = std::max(0, wallTop); y < std::min(m_screenH, wallBot); ++y) {
                int texY = (int)(((float)(y - wallTop) / (float)(wallBot - wallTop)) * TEX_SIZE);
                texY = std::clamp(texY, 0, TEX_SIZE - 1);

                sf::Color texColor = m_wallImage.getPixel(texX, texY);
                texColor.r = (sf::Uint8)(texColor.r * brightness * fog);
                texColor.g = (sf::Uint8)(texColor.g * brightness * fog);
                texColor.b = (sf::Uint8)(texColor.b * brightness * fog);

                m_floorBuffer.setPixel(col, y, texColor);
            }
        }
        // texture upload
        m_floorTexture.update(m_floorBuffer);
        window.draw(m_floorSprite);
        // sprite draw call
        drawSprites(window, dungeon, player, lightRadius);
        // for VFX
        updateOverlays(window, dt);
    }

    void RaycastRenderer::drawMinimap(sf::RenderWindow& window,
        const Dungeon& dungeon,
        const Player& player) {
        const auto& grid = dungeon.getGrid();

        // build entity lookup
        std::unordered_map<int, sf::Color> entityColors;
        for (const auto& e : dungeon.getEntities()) {
            if (!e->isAlive()) continue;
            int key = e->getY() * MAP_WIDTH + e->getX();
            switch (e->getType()) {
            case EntityType::Enemy:    entityColors[key] = sf::Color(200, 50, 50);  break;
            case EntityType::Merchant: entityColors[key] = sf::Color(50, 200, 50);  break;
            default:                   entityColors[key] = sf::Color::White;        break;
            }
        }

        const auto& chests = dungeon.getChests();
        sf::VertexArray cells(sf::Quads);

        for (int row = 0; row < MAP_HEIGHT; ++row) {
            for (int col = 0; col < MAP_WIDTH; ++col) {
                const Tile& tile = grid[row][col];
                int key = row * MAP_WIDTH + col;
                sf::Color color;

                if (!tile.visited && !tile.visible) {
                    color = sf::Color::Black;
                }
                else if (row == player.y && col == player.x) {
                    color = sf::Color::Yellow;
                }
                else if (tile.visited) {
                    // permanent — full color
                    if (entityColors.count(key))
                        color = entityColors[key];
                    else if (chests.count(key))
                        color = chests.at(key).empty()
                        ? sf::Color(100, 100, 50)
                        : sf::Color(255, 215, 0);
                    else if (tile.isExit)
                        color = sf::Color(50, 255, 50);
                    else if (tile.type == TileType::Floor)
                        color = sf::Color(80, 80, 80);
                    else
                        color = sf::Color(20, 20, 20);
                }
                else {
                    // visible but not visited — dim
                    if (tile.type == TileType::Floor)
                        color = sf::Color(40, 40, 40);
                    else
                        color = sf::Color(15, 15, 15);
                }

                float x = (float)(m_minimapX + col * MINIMAP_SCALE);
                float y = (float)(m_minimapY + row * MINIMAP_SCALE);
                float s = (float)(MINIMAP_SCALE - 1);
                cells.append(sf::Vertex(sf::Vector2f(x, y), color));
                cells.append(sf::Vertex(sf::Vector2f(x + s, y), color));
                cells.append(sf::Vertex(sf::Vector2f(x + s, y + s), color));
                cells.append(sf::Vertex(sf::Vector2f(x, y + s), color));
            }
        }
        window.draw(cells);
    }

    void RaycastRenderer::drawSprites(sf::RenderWindow& window,
        const Dungeon& dungeon,
        const Player& player,
        float lightRadius) {

        float px = player.visualX + 0.5f;
        float py = player.visualY + 0.5f;
        float dirX = std::cos(player.angle);
        float dirY = std::sin(player.angle);
        float planeX = -dirY * 0.66f;
        float planeY = dirX * 0.66f;

        std::vector<Sprite> sprites;

        for (const auto& e : dungeon.getEntities()) {
            if (!e->isAlive()) continue;
            const sf::Texture* tex = getSpriteTexture(e->getTextureName());
            sprites.push_back({
                e->visualX + 0.5f,
                e->visualY + 0.5f,
                sf::Color::White, 0.5f, tex, 0.25f
                });
        }

        for (const auto& [key, items] : dungeon.getChests()) {
            int cx = key % MAP_WIDTH;
            int cy = key / MAP_WIDTH;
            const sf::Texture* tex = getSpriteTexture(
                items.empty() ? "assets/texture_chest_opened.png"
                : "assets/texture_chest_closed.png");
            sprites.push_back({ (float)cx + 0.5f, (float)cy + 0.5f,
                sf::Color::White, 0.5f, tex, 0.4f });
        }

        if (dungeon.getExitX() != -1)
            sprites.push_back({
                (float)dungeon.getExitX() + 0.5f,
                (float)dungeon.getExitY() + 0.5f,
                sf::Color::White, 0.5f,
                getSpriteTexture("assets/texture_doors.png")
                });

        std::sort(sprites.begin(), sprites.end(), [&](const Sprite& a, const Sprite& b) {
            float dax = a.worldX - px, day = a.worldY - py;
            float dbx = b.worldX - px, dby = b.worldY - py;
            return (dax * dax + day * day) > (dbx * dbx + dby * dby);
            });

        for (const auto& sprite : sprites) {
            float sx = sprite.worldX - px;
            float sy = sprite.worldY - py;

            float invDet = 1.f / (planeX * dirY - dirX * planeY);
            float transformX = invDet * (dirY * sx - dirX * sy);
            float transformY = invDet * (-planeY * sx + planeX * sy);

            if (transformY < 0.5f) continue;

            float fog = std::max(0.f, 1.f - transformY / lightRadius);
            fog = fog * fog;
            sf::Uint8 fogVal = (sf::Uint8)(fog * 255.f);
            sf::Color fogColor(fogVal, fogVal, fogVal, 255);

            int spriteScreenX = (int)((m_drawWidth / 2) * (1.f + transformX / transformY));
            int spriteH = std::abs((int)(m_screenH / transformY));
            int spriteW = (int)(spriteH * sprite.width);

            int drawStartY = m_screenH / 2 - spriteH / 2 + (int)(spriteH * sprite.verticalOffset);
            int drawEndY = m_screenH / 2 + spriteH / 2 + (int)(spriteH * sprite.verticalOffset);
            int drawStartX = spriteScreenX - spriteW / 2;
            int drawEndX = spriteScreenX + spriteW / 2;

            if (sprite.texture != nullptr) {
                sf::Vector2u texSize = sprite.texture->getSize();

                // build a vertex quad per visible column
                sf::VertexArray strip(sf::Quads);

                for (int col = drawStartX; col < drawEndX; ++col) {
                    if (col < 0 || col >= m_drawWidth) continue;
                    if (transformY >= m_zBuffer[col]) continue;

                    float u = (float)(col - drawStartX) / (float)(drawEndX - drawStartX);
                    float texLeft = u * texSize.x;
                    float texRight = texLeft + (texSize.x / (float)(drawEndX - drawStartX));

                    int y0 = std::max(0, drawStartY);
                    int y1 = std::min(m_screenH, drawEndY);

                    float vTop = (float)(y0 - drawStartY) / (float)(drawEndY - drawStartY);
                    float vBottom = (float)(y1 - drawStartY) / (float)(drawEndY - drawStartY);
                    float texTop = vTop * texSize.y;
                    float texBottom = vBottom * texSize.y;

                    strip.append(sf::Vertex(sf::Vector2f((float)col, (float)y0), fogColor,
                        sf::Vector2f(texLeft, texTop)));
                    strip.append(sf::Vertex(sf::Vector2f((float)(col + 1), (float)y0), fogColor,
                        sf::Vector2f(texRight, texTop)));
                    strip.append(sf::Vertex(sf::Vector2f((float)(col + 1), (float)y1), fogColor,
                        sf::Vector2f(texRight, texBottom)));
                    strip.append(sf::Vertex(sf::Vector2f((float)col, (float)y1), fogColor,
                        sf::Vector2f(texLeft, texBottom)));
                }

                sf::RenderStates states;
                states.texture = sprite.texture;
                window.draw(strip, states);

            }
            else {
                // colored fallback — still writes to pixel buffer
                sf::Color c = sprite.color;
                c.r = (sf::Uint8)(c.r * fog);
                c.g = (sf::Uint8)(c.g * fog);
                c.b = (sf::Uint8)(c.b * fog);

                for (int col = drawStartX; col < drawEndX; ++col) {
                    if (col < 0 || col >= m_drawWidth) continue;
                    if (transformY >= m_zBuffer[col]) continue;
                    for (int y = std::max(0, drawStartY); y < std::min(m_screenH, drawEndY); ++y)
                        m_floorBuffer.setPixel(col, y, c);
                }
            }
        }
    }

    void RaycastRenderer::drawFloorCeiling(sf::RenderWindow& window,
        const Player& player,
        float lightRadius) {
        float px = player.visualX + 0.5f;
        float py = player.visualY + 0.5f;
        float dirX = std::cos(player.angle);
        float dirY = std::sin(player.angle);
        float planeX = -dirY * 0.66f;
        float planeY = dirX * 0.66f;

        int halfH = m_screenH / 2;

        for (int y = 0; y < m_screenH; ++y) {
            bool isFloor = y > halfH;

            // ray direction for leftmost and rightmost column
            float rayDirX0 = dirX - planeX;
            float rayDirY0 = dirY - planeY;
            float rayDirX1 = dirX + planeX;
            float rayDirY1 = dirY + planeY;

            // current row distance
            float rowDist;
            if (y == halfH) continue; // horizon — skip
            rowDist = (float)halfH / std::abs(y - halfH);

            // step vector per pixel
            float stepX = rowDist * (rayDirX1 - rayDirX0) / (float)m_drawWidth;
            float stepY = rowDist * (rayDirY1 - rayDirY0) / (float)m_drawWidth;

            // starting world position for this row
            float floorX = px + rowDist * rayDirX0;
            float floorY = py + rowDist * rayDirY0;

            // distance fog
            float fog = std::max(0.f, 1.f - rowDist / lightRadius);
            fog = fog * fog;
            sf::Uint8 fogVal = (sf::Uint8)(fog * 255.f);

            for (int x = 0; x < m_drawWidth; ++x) {
                // texture coordinates
                int tx = (int)(floorX * TEX_SIZE) & (TEX_SIZE - 1);
                int ty = (int)(floorY * TEX_SIZE) & (TEX_SIZE - 1);

                floorX += stepX;
                floorY += stepY;

                sf::Color texColor = isFloor
                    ? m_floorImage.getPixel(tx, ty)
                    : m_ceilImage.getPixel(tx, ty);

                // apply fog/lighting
                texColor.r = (sf::Uint8)(texColor.r * fog);
                texColor.g = (sf::Uint8)(texColor.g * fog);
                texColor.b = (sf::Uint8)(texColor.b * fog);

                m_floorBuffer.setPixel(x, y, texColor);
            }
        }

    }

    void RaycastRenderer::triggerHitFlash() { m_hitVignetteTimer = HIT_VIGNETTE_DURATION; }
    void RaycastRenderer::triggerCritFlash() { m_critFlashTimer = CRIT_FLASH_DURATION; }

    void RaycastRenderer::updateOverlays(sf::RenderWindow& window, float dt) {
        // red vignette — player took damage
        if (m_hitVignetteTimer > 0.f) {
            m_hitVignetteTimer -= dt;
            float alpha = (m_hitVignetteTimer / HIT_VIGNETTE_DURATION) * 180.f;
            sf::RectangleShape vignette(sf::Vector2f((float)m_drawWidth, (float)m_screenH));
            vignette.setFillColor(sf::Color(180, 0, 0, (sf::Uint8)alpha));
            window.draw(vignette);
        }

        // white flash — crit
        if (m_critFlashTimer > 0.f) {
            m_critFlashTimer -= dt;
            float alpha = (m_critFlashTimer / CRIT_FLASH_DURATION) * 200.f;
            sf::RectangleShape flash(sf::Vector2f((float)m_drawWidth, (float)m_screenH));
            flash.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));
            window.draw(flash);
        }
    }
}