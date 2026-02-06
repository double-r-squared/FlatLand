#ifndef WORLD_VIEW_HH
#define WORLD_VIEW_HH

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "../../map/map.hh"
#include "../../Vec2.hh"
#include "../../npc/npc.hh"

class WorldView {
private:
    int posX;
    int posY;
    int width;
    int height;
    
    // Font for NPC prompt
    TTF_Font* promptFont = nullptr;
    
    // Current prompt state
    std::string currentPrompt;
    bool showPrompt = false;
    
public:
    WorldView(int posX, int posY, int width, int height);
    ~WorldView();
    void render(SDL_Renderer* renderer, const Map& map, const Vec2& playerPos, float viewAngle);
    void setPrompt(const std::string& prompt, bool visible);
    const NPC* getNPCInCrosshair(const Map& map, const Vec2& playerPos, float viewAngle, float maxDistance = 3.0f);
};

#endif // WORLD_VIEW_HH