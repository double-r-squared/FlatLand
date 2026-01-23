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
    
    // NPC info (shown during conversation)
    SDL_Texture* npcPortrait;
    std::string npcId;          // NPC's identifier (also used as the display name under portrait)
    std::string npcDialogue;    // Current dialogue text being shown
    bool showingNPC;
    
    // Fonts & colors
    TTF_Font* font;             // For names
    TTF_Font* dialogueFont;     // For multi-line dialogue text
    SDL_Color textColor;
    SDL_Color dialogueColor;
    
    // Text wrapping helper
    std::vector<std::string> wrapText(const std::string& text, int maxWidth, TTF_Font* font);
    
public:
    PlayerStatsView(int x, int y, int width, int height);
    ~PlayerStatsView();
    
    bool loadFont(const std::string& fontPath, int fontSize);
    
    // Loads player avatar from a directory (picks first .png found)
    bool loadAvatar(SDL_Renderer* renderer, const std::string& avatarDirPath);
    
    // Loads NPC portrait — expects either a full file path (e.g. "assets/npcs/npc_0.png")
    // or a directory path (fallback behavior)
    bool loadNPCPortrait(SDL_Renderer* renderer, const std::string& npcAvatarPath);
    
    void setPlayerName(const std::string& name);
    
    // Sets the NPC to show — pass npc.id (which doubles as the visible name)
    void showNPC(const std::string& npcId);
    
    void hideNPC();
    
    void setNPCDialogue(const std::string& dialogue);
    
    void render(SDL_Renderer* renderer);
};

#endif // PLAYER_VIEW_HH