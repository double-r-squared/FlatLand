#ifndef WORLD_VIEW_HH
#define WORLD_VIEW_HH

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "../../Vec2.hh"
#include "../../map/map.hh"
#include "../../npc/npc.hh"

class WorldView {
private:
    int screenWidth;
    int screenHeight;

    // Font for floating prompt
    TTF_Font* promptFont = nullptr;

    // Current prompt state
    std::string currentPrompt;
    bool showPrompt = false;

public:
    WorldView(int screenWidth, int screenHeight);
    ~WorldView();

    void render(SDL_Renderer* renderer, const Map& map, const Vec2& playerPos, float viewAngle);

    const NPC* getNPCInCrosshair(const Map& map, const Vec2& playerPos, float viewAngle, float maxDistance = 3.0f);

    // Called every frame from Game to update the floating prompt
    void setPrompt(const std::string& prompt, bool visible);
};

#endif // WORLD_VIEW_HH