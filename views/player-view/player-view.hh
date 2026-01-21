#ifndef PLAYER_VIEW_HH
#define PLAYER_VIEW_HH

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <memory>

class PlayerStatsView {
private:
    int x, y, width, height;
    SDL_Texture* avatarTexture;
    SDL_Renderer* renderer;
    TTF_Font* font;
    
    std::string playerName;
    
    const int PADDING = 20;
    
public:
    PlayerStatsView(int x, int y, int width, int height);
    ~PlayerStatsView();
    
    bool loadAvatar(SDL_Renderer* renderer, const std::string& imagePath);
    bool loadFont(const std::string& fontPath, int fontSize);
    void setPlayerName(const std::string& name);
    
    void render(SDL_Renderer* renderer);
};

#endif // PLAYER_VIEW_HH