#include "world-view.hh"
#include <cmath>
#include <algorithm>
#include <iostream>

WorldView::WorldView(int w, int h)
    : screenWidth(w), screenHeight(h), currentPrompt(""), showPrompt(false) {

    // Load a font for the floating prompt — try the same paths as before
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
        std::cerr << "WorldView: Warning — could not load any font for floating prompt\n";
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
    int viewHeight = 60;
    int viewStartY = (screenHeight / 2) - viewHeight / 2;  // centered vertically
    int centerY = viewStartY + viewHeight / 2;

    Vec2 viewDir(std::cos(viewAngle), std::sin(viewAngle));

    const float FOV = M_PI / 2;
    const float maxRayDistance = 50.0f;
    const float rayStep = 0.2f;

    for (int i = 0; i < screenWidth; i++) {
        float screenPercent = (float)i / screenWidth;
        float angle = viewAngle + (screenPercent - 0.5f) * FOV;
        Vec2 rayDir(std::cos(angle), std::sin(angle));

        bool hitWall = false;
        bool hitNPC = false;
        float minDist = maxRayDistance;

        for (float dist = 0.1f; dist < maxRayDistance; dist += rayStep) {
            Vec2 checkPos = playerPos + rayDir * dist;

            for (const auto& shape : map.shapes) {
                float minY, maxY;
                if (shape->intersectsVerticalLine(checkPos.x, minY, maxY)) {
                    if (checkPos.y >= minY && checkPos.y <= maxY) {
                        minDist = dist;
                        hitWall = true;
                        hitNPC = false;
                        goto done_checking;
                    }
                }
            }

            for (const auto& npc : map.npcs) {
                float minY, maxY;
                if (npc.shape->intersectsVerticalLine(checkPos.x, minY, maxY)) {
                    if (checkPos.y >= minY && checkPos.y <= maxY) {
                        minDist = dist;
                        hitWall = false;
                        hitNPC = true;
                        goto done_checking;
                    }
                }
            }

            for (const auto& line : map.lines) {
                float dist_to_line = line->getClosestDistanceToPoint(checkPos);
                const float lineThickness = 0.5f;  // Raycast hits if within this distance
                if (dist_to_line < lineThickness) {
                    minDist = dist;
                    hitWall = true;
                    hitNPC = false;
                    goto done_checking;
                }
            }
        }
    done_checking:

        if (hitWall || hitNPC) {
            float brightness = 1.0f - (minDist / 30.0f);
            brightness = std::max(0.2f, std::min(1.0f, brightness));

            if (hitWall) {
                SDL_SetRenderDrawColor(renderer, (Uint8)(255 * brightness), (Uint8)(255 * brightness), (Uint8)(255 * brightness), 255);
            } else {
                SDL_SetRenderDrawColor(renderer, (Uint8)(255 * brightness), (Uint8)(100 * brightness), (Uint8)(100 * brightness), 255);
            }
            SDL_RenderDrawPoint(renderer, i, centerY);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawPoint(renderer, i, centerY);
        }
    }

    // Orange crosshair
    int centerX = screenWidth / 2;
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

            // Background rectangle
            SDL_Rect bgRect = {centerX - textW/2 - 10, promptY - 5, textW + 20, textH + 10};
            SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
            SDL_RenderFillRect(renderer, &bgRect);

            // Text
            SDL_Rect dstRect = {centerX - textW/2, promptY, textW, textH};
            SDL_RenderCopy(renderer, textTexture, nullptr, &dstRect);

            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
    }
}