#ifndef MINI_MAP_HH
#define MINI_MAP_HH

#include <SDL2/SDL.h>
#include "../../map/map.hh"
#include "../../Vec2.hh"

class MiniMap {
private:
    int size;
    int screenWidth;
    
public:
    MiniMap(int size, int screenWidth);
    void render(SDL_Renderer* renderer, const Map& map, const Vec2& playerPos, float viewAngle);
};

#endif // MINI_MAP_HH