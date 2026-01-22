#ifndef PLAYER_VIEW_HH
#define PLAYER_VIEW_HH

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>

class PlayerStatsView {
private:
    int x, y, width, height;
    
    // Player info
    std::string playerName;
    SDL_Texture* playerAvatar;
    
    // NPC portrait (when in conversation)
    SDL_Texture* npcPortrait;
    std::string npcName;
    bool showingNPC;
    
    // Font
    TTF_Font* font;
    SDL_Color textColor;
    
public:
    PlayerStatsView(int x, int y, int width, int height);
    ~PlayerStatsView();
    
    bool loadFont(const std::string& fontPath, int fontSize);
    bool loadAvatar(SDL_Renderer* renderer, const std::string& avatarPath);
    bool loadNPCPortrait(SDL_Renderer* renderer, const std::string& npcAvatarPath);
    
    void setPlayerName(const std::string& name);
    void showNPC(const std::string& name);  // Show NPC portrait on right side
    void hideNPC();  // Hide NPC portrait, return to normal view
    
    void render(SDL_Renderer* renderer);
};

#endif // PLAYER_VIEW_HH