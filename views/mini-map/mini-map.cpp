#include "mini-map.hh"
#include "../../npc/Shapes/Rectangle.hh"
#include "../../npc/Shapes/Triangle.hh"
#include "../../npc/Shapes/Circle.hh"
#include "../../npc/Shapes/Line.hh"
#include <cmath>

MiniMap::MiniMap(int size, int screenWidth) : size(size), screenWidth(screenWidth) {}

void MiniMap::render(SDL_Renderer* renderer, const Map& map, const Vec2& playerPos, float viewAngle) {
    int minimapX = screenWidth - size - 20;
    int minimapY = 20;
    
    // Draw minimap background
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 200);
    SDL_Rect minimapRect = {minimapX, minimapY, size, size};
    SDL_RenderFillRect(renderer, &minimapRect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &minimapRect);
    
    // Enable clipping to minimap area
    SDL_RenderSetClipRect(renderer, &minimapRect);
    
    // Calculate scale and offset for minimap
    float scale = 3.0f;
    float offsetX = minimapX + size / 2 - playerPos.x * scale;
    float offsetY = minimapY + size / 2 - playerPos.y * scale;
    
    // Draw shapes on minimap
    for (const auto& shape : map.shapes) {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        
        if (auto rect = dynamic_cast<Rectangle*>(shape.get())) {
            SDL_Rect r = {
                (int)(offsetX + rect->position.x * scale),
                (int)(offsetY + rect->position.y * scale),
                (int)(rect->width * scale),
                (int)(rect->height * scale)
            };
            SDL_RenderFillRect(renderer, &r);
        } else if (auto circ = dynamic_cast<Circle*>(shape.get())) {
            int cx = (int)(offsetX + circ->position.x * scale);
            int cy = (int)(offsetY + circ->position.y * scale);
            int r = (int)(circ->radius * scale);
            // Draw circle as octagon approximation
            for (int i = 0; i < 8; i++) {
                float angle1 = i * M_PI / 4;
                float angle2 = (i + 1) * M_PI / 4;
                SDL_RenderDrawLine(renderer,
                    cx + r * std::cos(angle1), cy + r * std::sin(angle1),
                    cx + r * std::cos(angle2), cy + r * std::sin(angle2));
            }
        } else if (auto tri = dynamic_cast<Triangle*>(shape.get())) {
            SDL_Point points[4] = {
                {(int)(offsetX + tri->p1.x * scale), (int)(offsetY + tri->p1.y * scale)},
                {(int)(offsetX + tri->p2.x * scale), (int)(offsetY + tri->p2.y * scale)},
                {(int)(offsetX + tri->p3.x * scale), (int)(offsetY + tri->p3.y * scale)},
                {(int)(offsetX + tri->p1.x * scale), (int)(offsetY + tri->p1.y * scale)}
            };
            SDL_RenderDrawLines(renderer, points, 4);
        }
    }
    
    // Draw lines on minimap
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    for (const auto& line : map.lines) {
        const auto& points = line->getPoints();
        for (size_t i = 0; i + 1 < points.size(); ++i) {
            int x1 = (int)(offsetX + points[i].x * scale);
            int y1 = (int)(offsetY + points[i].y * scale);
            int x2 = (int)(offsetX + points[i + 1].x * scale);
            int y2 = (int)(offsetY + points[i + 1].y * scale);
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
    
    // Draw NPCs on minimap
    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
    for (const auto& npc : map.npcs) {
        if (auto circ = dynamic_cast<Circle*>(npc.shape.get())) {
            int cx = (int)(offsetX + circ->position.x * scale);
            int cy = (int)(offsetY + circ->position.y * scale);
            int r = (int)(circ->radius * scale);
            SDL_Rect npcRect = {cx - r, cy - r, r * 2, r * 2};
            SDL_RenderFillRect(renderer, &npcRect);
        }
    }
    
    // Draw player on minimap
    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
    int px = minimapX + size / 2;
    int py = minimapY + size / 2;
    SDL_Rect playerRect = {px - 3, py - 3, 6, 6};
    SDL_RenderFillRect(renderer, &playerRect);
    
    // Draw view direction indicator
    int dirLength = 15;
    SDL_RenderDrawLine(renderer, px, py,
        px + (int)(dirLength * std::cos(viewAngle)),
        py + (int)(dirLength * std::sin(viewAngle)));
    
    // Draw FOV cone (180 degrees)
    float fov = M_PI / 2; // 180 degrees
    SDL_RenderDrawLine(renderer, px, py,
        px + (int)(dirLength * std::cos(viewAngle - fov/2)),
        py + (int)(dirLength * std::sin(viewAngle - fov/2)));
    SDL_RenderDrawLine(renderer, px, py,
        px + (int)(dirLength * std::cos(viewAngle + fov/2)),
        py + (int)(dirLength * std::sin(viewAngle + fov/2)));
    
    // Disable clipping
    SDL_RenderSetClipRect(renderer, nullptr);
}