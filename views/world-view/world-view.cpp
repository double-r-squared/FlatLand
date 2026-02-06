#include "world-view.hh"
#include "../../npc/Shapes/Rectangle.hh"
#include "../../npc/Shapes/Triangle.hh"
#include "../../npc/Shapes/Circle.hh"
#include "../../npc/Shapes/Line.hh"
#include <cmath>
#include <iostream>

WorldView::WorldView(int posX, int posY, int width, int height) : posX(posX), posY(posY), width(width), height(height) {
    // Load a font for the floating prompt
    std::vector<std::string> fontPaths = {
        "assets/fonts/Minecraft/Minecraft-Regular.otf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"
    };

    for (const auto& path : fontPaths) {
        promptFont = TTF_OpenFont(path.c_str(), 20);
        if (promptFont) {
            std::cout << "WorldView: Loaded prompt font: " << path << std::endl;
            break;
        }
    }

    if (!promptFont) {
        std::cerr << "WorldView: Warning — could not load any font for prompt\n";
    }
}

WorldView::~WorldView() {
    if (promptFont) TTF_CloseFont(promptFont);
}

void WorldView::setPrompt(const std::string& prompt, bool visible) {
    currentPrompt = prompt;
    showPrompt = visible && !prompt.empty();
}

const NPC* WorldView::getNPCInCrosshair(const Map& map, const Vec2& playerPos, float viewAngle, float maxDistance) {
    Vec2 rayDir(std::cos(viewAngle), std::sin(viewAngle));
    const float rayStep = 0.1f;

    for (float dist = 0.1f; dist < maxDistance; dist += rayStep) {
        Vec2 checkPos = playerPos + rayDir * dist;

        for (const auto& npc : map.npcs) {
            float minY, maxY;
            if (npc.shape->intersectsVerticalLine(checkPos.x, minY, maxY)) {
                if (checkPos.y >= minY && checkPos.y <= maxY) {
                    return &npc;
                }
            }
        }
    }
    return nullptr;
}

void WorldView::render(SDL_Renderer* renderer, const Map& map, const Vec2& playerPos, float viewAngle) {
    // Draw minimap background
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect minimapRect = {posX, posY, width, height};
    SDL_RenderFillRect(renderer, &minimapRect);
    
    // Enable clipping to minimap area
    SDL_RenderSetClipRect(renderer, &minimapRect);
    
    // Calculate scale and offset for minimap
    float scale = 4.0f;  // Larger scale for full-screen view
    float offsetX = posX + width / 2 - playerPos.x * scale;
    float offsetY = posY + height / 2 - playerPos.y * scale;
    
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
    int px = posX + width / 2;
    int py = posY + height / 2;
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
    
    // Draw orange crosshair on main view
    int centerX = posX + width / 2;
    int centerY = posY + height / 2;
    int crosshairSize = 8;
    SDL_SetRenderDrawColor(renderer, 255, 165, 0, 200);
    SDL_RenderDrawLine(renderer, centerX - crosshairSize, centerY, centerX + crosshairSize, centerY);
    SDL_RenderDrawLine(renderer, centerX, centerY - crosshairSize, centerX, centerY + crosshairSize);

    // Floating prompt under crosshair
    if (showPrompt && promptFont) {
        SDL_Color textColor = {255, 220, 100, 255};  // Light yellow
        SDL_Color bgColor   = {0, 0, 0, 160};

        SDL_Surface* textSurface = TTF_RenderText_Blended(promptFont, currentPrompt.c_str(), textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

            int textW = textSurface->w;
            int textH = textSurface->h;

            int promptY = centerY + 35;  // ~35px below crosshair

            // Text
            SDL_Rect dstRect = {centerX - textW/2, promptY, textW, textH};
            SDL_RenderCopy(renderer, textTexture, nullptr, &dstRect);

            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
    }
}