#define NOMINMAX
#include "RaycastRenderer.h"
#include <cmath>
#include <algorithm>

namespace DungeonGame {

    RaycastRenderer::RaycastRenderer()
        : m_lines(sf::Lines) {

        // wall/floor/ceiling — still loaded directly as before
        m_wallImage.loadFromFile("assets/texture_wall.png");
        m_floorImage.loadFromFile("assets/texture_floor.png");
        m_ceilImage.loadFromFile("assets/texture_ceiling.png");

        // create floor pixel buffer
        m_floorBuffer.create(MAP_DRAW_WIDTH, SCREEN_HEIGHT, sf::Color::Black);
        m_floorTexture.create(MAP_DRAW_WIDTH, SCREEN_HEIGHT);
        m_floorSprite.setTexture(m_floorTexture);

        // preload sprite textures into cache
        getSpriteImage("assets/texture_chest_closed.png");
        getSpriteImage("assets/texture_chest_opened.png");
        getSpriteImage("assets/texture_doors.png");
        getSpriteImage("assets/enemy_grunt.png");
        getSpriteImage("assets/enemy_trickster.png");
        getSpriteImage("assets/enemy_brute.png");
        getSpriteImage("assets/texture_merchant.png");
    }

    const sf::Image* RaycastRenderer::getSpriteImage(const std::string& path) {
        if (path.empty()) return nullptr;
        auto it = m_spriteImages.find(path);
        if (it != m_spriteImages.end())
            return &it->second;
        sf::Image img;
        if (img.loadFromFile(path))
            m_spriteImages[path] = std::move(img);
        else
            return nullptr;
        return &m_spriteImages[path];
    }

    bool RaycastRenderer::isWall(const Dungeon& dungeon, int tx, int ty) const {
        if (tx < 0 || tx >= MAP_WIDTH || ty < 0 || ty >= MAP_HEIGHT)
            return true; // out of bounds = solid
        const Tile& tile = dungeon.getGrid()[ty][tx];
        return tile.type != TileType::Floor;
    }

    sf::Color RaycastRenderer::wallColor(float distance, float brightness, float lightRadius) const {
        float fog = std::max(0.f, 1.f - distance / lightRadius);
        fog = fog * fog; // quadratic falloff — darker faster
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

        for (int col = 0; col < MAP_DRAW_WIDTH; ++col) {
            // camera x in [-1, 1]
            float cameraX = 2.f * col / (float)MAP_DRAW_WIDTH - 1.f;

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
            int  side = 0; // 0 = X side hit, 1 = Y side hit
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

            // perpendicular distance — eliminates fisheye
            float perpDist = (side == 0)
                ? (sideDistX - deltaDistX)
                : (sideDistY - deltaDistY);

            if (perpDist < 0.0001f) perpDist = std::max(0.5f, perpDist);

            m_zBuffer[col] = perpDist;

            int wallH = (int)((float)SCREEN_HEIGHT / perpDist);
            int wallTop = SCREEN_HEIGHT / 2 - wallH / 2;
            int wallBot = SCREEN_HEIGHT / 2 + wallH / 2;

            float wallX;
            if (side == 0) wallX = py + perpDist * rayDirY;
            else           wallX = px + perpDist * rayDirX;
            wallX -= std::floor(wallX);

            int texX = (int)(wallX * TEX_SIZE);
            texX = std::clamp(texX, 0, TEX_SIZE - 1);

            float fog = std::max(0.f, 1.f - perpDist / lightRadius);
            fog = fog * fog;
            float brightness = (side == 0) ? 1.0f : 0.65f;

            for (int y = std::max(0, wallTop); y < std::min(SCREEN_HEIGHT, wallBot); ++y) {
                int texY = (int)(((float)(y - wallTop) / (float)(wallBot - wallTop)) * TEX_SIZE);
                texY = std::clamp(texY, 0, TEX_SIZE - 1);

                sf::Color texColor = m_wallImage.getPixel(texX, texY);
                texColor.r = (sf::Uint8)(texColor.r * brightness * fog);
                texColor.g = (sf::Uint8)(texColor.g * brightness * fog);
                texColor.b = (sf::Uint8)(texColor.b * brightness * fog);

                m_floorBuffer.setPixel(col, y, texColor);
            }
        }
        // one upload, one draw call
        m_floorTexture.update(m_floorBuffer);
        window.draw(m_floorSprite);

        drawSprites(window, dungeon, player, lightRadius);
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
            default:                   entityColors[key] = sf::Color::White;         break;
            }
        }

        // chest lookup
        const auto& chests = dungeon.getChests();

        sf::VertexArray cells(sf::Quads);

        for (int row = 0; row < MAP_HEIGHT; ++row) {
            for (int col = 0; col < MAP_WIDTH; ++col) {
                const Tile& tile = grid[row][col];
                int key = row * MAP_WIDTH + col;

                sf::Color color;

                if (row == player.y && col == player.x)
                    color = sf::Color::Yellow;
                else if (entityColors.count(key))
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

                float x = (float)(MINIMAP_X + col * MINIMAP_SCALE);
                float y = (float)(MINIMAP_Y + row * MINIMAP_SCALE);
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
            const sf::Image* img = getSpriteImage(e->getTextureName());
            sprites.push_back({
                e->visualX + 0.5f,
                e->visualY + 0.5f,
                sf::Color::White, 0.5f, img, 0.25f
                });
        }

        for (const auto& [key, items] : dungeon.getChests()) {
            int cx = key % MAP_WIDTH;
            int cy = key / MAP_WIDTH;

            const sf::Image* img = getSpriteImage(
                items.empty() ? "assets/texture_chest_opened.png"
                : "assets/texture_chest_closed.png");

            sprites.push_back({ (float)cx + 0.5f, (float)cy + 0.5f,
                sf::Color::White, 0.5f, img, 0.4f });
        }


        if (dungeon.getExitX() != -1)
            sprites.push_back({
                (float)dungeon.getExitX() + 0.5f,
                (float)dungeon.getExitY() + 0.5f,
                sf::Color::White, 0.5f,
                getSpriteImage("assets/texture_doors.png")
                });

        std::sort(sprites.begin(), sprites.end(), [&](const Sprite& a, const Sprite& b) {
            float dax = a.worldX - px, day = a.worldY - py;
            float dbx = b.worldX - px, dby = b.worldY - py;
            return (dax * dax + day * day) > (dbx * dbx + dby * dby);
            });

        sf::VertexArray spriteStrips(sf::Lines);

        for (const auto& sprite : sprites) {
            float sx = sprite.worldX - px;
            float sy = sprite.worldY - py;

            float invDet = 1.f / (planeX * dirY - dirX * planeY);
            float transformX = invDet * (dirY * sx - dirX * sy);
            float transformY = invDet * (-planeY * sx + planeX * sy);

            if (transformY < 0.5f) continue;

            // lighting
            float fog = std::max(0.f, 1.f - transformY / lightRadius);
            fog = fog * fog;

            int spriteScreenX = (int)((MAP_DRAW_WIDTH / 2) * (1.f + transformX / transformY));
            int spriteH = std::abs((int)(SCREEN_HEIGHT / transformY));
            int spriteW = (int)(spriteH * sprite.width);

            int drawStartY = SCREEN_HEIGHT / 2 - spriteH / 2 + (int)(spriteH * sprite.verticalOffset);
            int drawEndY = SCREEN_HEIGHT / 2 + spriteH / 2 + (int)(spriteH * sprite.verticalOffset);
            int drawStartX = spriteScreenX - spriteW / 2;
            int drawEndX = spriteScreenX + spriteW / 2;

            for (int col = drawStartX; col < drawEndX; ++col) {
                if (col < 0 || col >= MAP_DRAW_WIDTH) continue;
                if (transformY >= m_zBuffer[col]) continue;

                if (sprite.image != nullptr) {
                    // textured sprite — sample column from image
                    float u = (float)(col - drawStartX) / (float)(drawEndX - drawStartX);
                    int texW = (int)sprite.image->getSize().x;
                    int texH = (int)sprite.image->getSize().y;

                    int texX = (int)(u * texW);
                    texX = std::clamp(texX, 0, texW - 1);

                    for (int y = std::max(0, drawStartY); y < std::min(SCREEN_HEIGHT, drawEndY); ++y) {
                        float v = (float)(y - drawStartY) / (float)(drawEndY - drawStartY);
                        int texY = (int)(v * texH);
                        texY = std::clamp(texY, 0, texH - 1);

                        sf::Color texColor = sprite.image->getPixel(texX, texY);

                        // skip transparent pixels
                        if (texColor.a < 128) continue;

                        texColor.r = (sf::Uint8)(texColor.r * fog);
                        texColor.g = (sf::Uint8)(texColor.g * fog);
                        texColor.b = (sf::Uint8)(texColor.b * fog);

                        m_floorBuffer.setPixel(col, y, texColor);
                    }
                }
                else {
                    // colored sprite — draw vertical strip
                    sf::Color c = sprite.color;
                    c.r = (sf::Uint8)(c.r * fog);
                    c.g = (sf::Uint8)(c.g * fog);
                    c.b = (sf::Uint8)(c.b * fog);

                    for (int y = std::max(0, drawStartY); y < std::min(SCREEN_HEIGHT, drawEndY); ++y)
                        m_floorBuffer.setPixel(col, y, c);
                }
            }
        }

        m_floorTexture.update(m_floorBuffer);
        window.draw(m_floorSprite);
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

        int halfH = SCREEN_HEIGHT / 2;

        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
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
            float stepX = rowDist * (rayDirX1 - rayDirX0) / (float)MAP_DRAW_WIDTH;
            float stepY = rowDist * (rayDirY1 - rayDirY0) / (float)MAP_DRAW_WIDTH;

            // starting world position for this row
            float floorX = px + rowDist * rayDirX0;
            float floorY = py + rowDist * rayDirY0;

            // distance fog
            float fog = std::max(0.f, 1.f - rowDist / lightRadius);
            fog = fog * fog;
            sf::Uint8 fogVal = (sf::Uint8)(fog * 255.f);

            for (int x = 0; x < MAP_DRAW_WIDTH; ++x) {
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
}