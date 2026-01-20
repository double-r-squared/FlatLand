#ifndef WORLD_VIEW_HH
#define WORLD_VIEW_HH

#include <SDL2/SDL.h>
#include "../../Vec2.hh"
#include "../../map/map.hh"
#include "../../npc/npc.hh"

class WorldView {
private:
    int screenWidth;
    int screenHeight;
    int textAreaHeight;
    
public:
    WorldView(int screenWidth, int screenHeight, int textAreaHeight);
    void render(SDL_Renderer* renderer, const Map& map, const Vec2& playerPos, float viewAngle);
    
    // Check if an NPC is in the crosshair within maxDistance
    const NPC* getNPCInCrosshair(const Map& map, const Vec2& playerPos, float viewAngle, float maxDistance = 3.0f);
};

#endif // WORLD_VIEW_HH