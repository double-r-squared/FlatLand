#ifndef PLAYER_VIEW_HH
#define PLAYER_VIEW_HH

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>

class PlayerStatsView {
private:
    int x, y, width, height;
    
    // Player info
    std::string playerName;
    SDL_Texture* playerAvatar;
    
    // NPC portrait (when in conversation)
    SDL_Texture* npcPortrait;
    std::string npcName;
    std::string npcDialogue;  // Current NPC dialogue text
    bool showingNPC;
    
    // Font
    TTF_Font* font;
    TTF_Font* dialogueFont;  // Font for dialogue text
    SDL_Color textColor;
    SDL_Color dialogueColor;
    
    // Helper for text wrapping
    std::vector<std::string> wrapText(const std::string& text, int maxWidth, TTF_Font* font);
    
public:
    PlayerStatsView(int x, int y, int width, int height);
    ~PlayerStatsView();
    
    bool loadFont(const std::string& fontPath, int fontSize);
    bool loadAvatar(SDL_Renderer* renderer, const std::string& avatarPath);
    bool loadNPCPortrait(SDL_Renderer* renderer, const std::string& npcAvatarPath);
    
    void setPlayerName(const std::string& name);
    void showNPC(const std::string& name);
    void hideNPC();
    
    // New: Set NPC dialogue text
    void setNPCDialogue(const std::string& dialogue);
    
    void render(SDL_Renderer* renderer);
};

#endif // PLAYER_VIEW_HH