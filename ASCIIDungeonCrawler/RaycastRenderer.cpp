#include "RaycastRenderer.h"
#include <cmath>
#include <algorithm>

namespace DungeonGame {

    RaycastRenderer::RaycastRenderer()
        : m_lines(sf::Lines) {
    }

    bool RaycastRenderer::isWall(const Dungeon& dungeon, int tx, int ty) const {
        if (tx < 0 || tx >= MAP_WIDTH || ty < 0 || ty >= MAP_HEIGHT)
            return true; // out of bounds = solid
        const Tile& tile = dungeon.getGrid()[ty][tx];
        return tile.type != TileType::Floor;
    }

    sf::Color RaycastRenderer::wallColor(float distance, float brightness) const {
        // base wall color scaled by brightness and distance fog
        float fog = std::max(0.f, 1.f - distance / 16.f);
        int   value = (int)(brightness * fog * 255.f);
        value = std::clamp(value, 0, 255);
        return sf::Color(value, value, value);
    }

    void RaycastRenderer::draw(sf::RenderWindow& window,
        const Dungeon& dungeon,
        const Player& player) {
        // ceiling
        sf::RectangleShape ceiling(sf::Vector2f((float)MAP_DRAW_WIDTH, (float)(SCREEN_HEIGHT / 2)));
        ceiling.setPosition(0.f, 0.f);
        ceiling.setFillColor(sf::Color(30, 30, 30));
        window.draw(ceiling);

        // floor
        sf::RectangleShape floorRect(sf::Vector2f((float)MAP_DRAW_WIDTH, (float)(SCREEN_HEIGHT / 2)));
        floorRect.setPosition(0.f, (float)(SCREEN_HEIGHT / 2));
        floorRect.setFillColor(sf::Color(50, 40, 40));
        window.draw(floorRect);

        float px = (float)player.x + 0.5f;
        float py = (float)player.y + 0.5f;

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

            if (perpDist < 0.0001f) perpDist = 0.0001f;

            m_zBuffer[col] = perpDist;

            int wallH = (int)((float)SCREEN_HEIGHT / perpDist);
            int wallTop = SCREEN_HEIGHT / 2 - wallH / 2;
            int wallBot = SCREEN_HEIGHT / 2 + wallH / 2;

            // N/S vs E/W shading
            float brightness = (side == 0) ? 1.0f : 0.65f;
            sf::Color color = wallColor(perpDist, brightness);

            m_lines.append(sf::Vertex(sf::Vector2f((float)col, (float)wallTop), color));
            m_lines.append(sf::Vertex(sf::Vector2f((float)col, (float)wallBot), color));
        }

        window.draw(m_lines);
        drawSprites(window, dungeon, player);
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
        const Player& player) {

        sf::Clock spriteClock;

        float px = (float)player.x + 0.5f;
        float py = (float)player.y + 0.5f;
        float dirX = std::cos(player.angle);
        float dirY = std::sin(player.angle);
        float planeX = -dirY * 0.66f;
        float planeY = dirX * 0.66f;

        std::vector<Sprite> sprites;

        for (const auto& e : dungeon.getEntities()) {
            if (!e->isAlive()) continue;
            sf::Color color;
            switch (e->getType()) {
            case EntityType::Enemy:    color = sf::Color(200, 50, 50);  break;
            case EntityType::Merchant: color = sf::Color(50, 200, 50);  break;
            default:                   color = sf::Color::White;         break;
            }
            sprites.push_back({ (float)e->getX() + 0.5f, (float)e->getY() + 0.5f, color, 1.0f });
        }

        for (const auto& [key, items] : dungeon.getChests()) {
            int cx = key % MAP_WIDTH;
            int cy = key / MAP_WIDTH;
            sf::Color color = items.empty() ? sf::Color(100, 100, 50) : sf::Color(255, 215, 0);
            sprites.push_back({ (float)cx + 0.5f, (float)cy + 0.5f, color, 0.6f });
        }

        if (dungeon.getExitX() != -1)
            sprites.push_back({
                (float)dungeon.getExitX() + 0.5f,
                (float)dungeon.getExitY() + 0.5f,
                sf::Color(50, 255, 50), 0.5f
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

            if (transformY <= 0.f) continue;

            int spriteScreenX = (int)((MAP_DRAW_WIDTH / 2) * (1.f + transformX / transformY));
            int spriteH = std::abs((int)(SCREEN_HEIGHT / transformY));
            int spriteW = (int)(spriteH * sprite.width);

            int drawStartY = SCREEN_HEIGHT / 2 - spriteH / 2;
            int drawEndY = SCREEN_HEIGHT / 2 + spriteH / 2;
            int drawStartX = spriteScreenX - spriteW / 2;
            int drawEndX = spriteScreenX + spriteW / 2;

            for (int col = drawStartX; col < drawEndX; ++col) {
                if (col < 0 || col >= MAP_DRAW_WIDTH) continue;
                if (transformY >= m_zBuffer[col]) continue;

                spriteStrips.append(sf::Vertex(
                    sf::Vector2f((float)col, (float)drawStartY), sprite.color));
                spriteStrips.append(sf::Vertex(
                    sf::Vector2f((float)col, (float)drawEndY), sprite.color));
            }
        }

        window.draw(spriteStrips);

        float elapsed = spriteClock.getElapsedTime().asSeconds();
        if (elapsed > 0.016f)
            sf::err() << "drawSprites spike: " << elapsed << "s, sprites: "
            << sprites.size() << "\n";
    }

}