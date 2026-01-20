#include "world-view.hh"
#include <cmath>
#include <algorithm>

WorldView::WorldView(int screenWidth, int screenHeight, int textAreaHeight) 
    : screenWidth(screenWidth), screenHeight(screenHeight), textAreaHeight(textAreaHeight) {}

const NPC* WorldView::getNPCInCrosshair(const Map& map, const Vec2& playerPos, float viewAngle, float maxDistance) {
    // Cast a ray from the center of the screen (where crosshair is)
    Vec2 rayDir(std::cos(viewAngle), std::sin(viewAngle));
    
    const float rayStep = 0.1f;
    
    // Check along the ray
    for (float dist = 0.1f; dist < maxDistance; dist += rayStep) {
        Vec2 checkPos = playerPos + rayDir * dist;
        
        // Check NPCs
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
    // Render the 1D side view as a horizontal line in the center
    int viewHeight = 60;
    int viewStartY = (screenHeight - textAreaHeight) / 2 - viewHeight / 2;
    
    // Calculate view direction
    Vec2 viewDir(std::cos(viewAngle), std::sin(viewAngle));
    
    // Render the horizontal line - 180 degree field of view
    int centerY = viewStartY + viewHeight / 2;
    const float FOV = M_PI/2; // 180 degrees
    const float maxRayDistance = 50.0f;
    const float rayStep = 0.2f;
    
    for (int i = 0; i < screenWidth; i++) {
        // Map pixel to angle within FOV (-90 to +90 degrees from view direction)
        float screenPercent = (float)i / screenWidth;
        float angle = viewAngle + (screenPercent - 0.5f) * FOV;
        
        // Cast a ray in this direction
        Vec2 rayDir(std::cos(angle), std::sin(angle));
        
        // Ray marching to find intersection
        bool hitWall = false;
        bool hitNPC = false;
        float minDist = maxRayDistance;
        
        // Check at multiple distances along the ray
        for (float dist = 0.1f; dist < maxRayDistance; dist += rayStep) {
            Vec2 checkPos = playerPos + rayDir * dist;
            
            // Check shapes (walls)
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
            
            // Check NPCs
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
        }
        
        done_checking:
        
        // Draw the pixel on the horizontal line
        if (hitWall || hitNPC) {
            // Calculate brightness based on distance
            float brightness = 1.0f - (minDist / 30.0f);
            brightness = std::max(0.2f, std::min(1.0f, brightness));
            
            if (hitWall) {
                SDL_SetRenderDrawColor(renderer, 
                                     (Uint8)(255 * brightness), 
                                     (Uint8)(255 * brightness), 
                                     (Uint8)(255 * brightness), 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 
                                     (Uint8)(255 * brightness), 
                                     (Uint8)(100 * brightness), 
                                     (Uint8)(100 * brightness), 255);
            }
            
            SDL_RenderDrawPoint(renderer, i, centerY);
        } else {
            // Draw black/empty space
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawPoint(renderer, i, centerY);
        }
    }
    
    // Draw orange crosshair at center
    int centerX = screenWidth / 2;
    int crosshairSize = 8;
    SDL_SetRenderDrawColor(renderer, 255, 165, 0, 180); // Orange with some transparency
    
    // Horizontal line
    SDL_RenderDrawLine(renderer, centerX - crosshairSize, centerY, centerX + crosshairSize, centerY);
    
    // Vertical line
    SDL_RenderDrawLine(renderer, centerX, centerY - crosshairSize, centerX, centerY + crosshairSize);
}