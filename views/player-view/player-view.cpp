#include "player-view.hh"
#include <iostream>
#include <dirent.h>
#include <algorithm>
#include <vector>

PlayerStatsView::PlayerStatsView(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height),
      playerAvatar(nullptr), npcPortrait(nullptr), font(nullptr),
      textColor({255, 255, 255, 255}), showingNPC(false) {
    
    // Initialize SDL_ttf if not already initialized
    if (!TTF_WasInit()) {
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        }
    }
    
    // Initialize SDL_image for PNG support
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << std::endl;
    }
}

PlayerStatsView::~PlayerStatsView() {
    if (playerAvatar) {
        SDL_DestroyTexture(playerAvatar);
    }
    if (npcPortrait) {
        SDL_DestroyTexture(npcPortrait);
    }
    if (font) {
        TTF_CloseFont(font);
    }
}

bool PlayerStatsView::loadFont(const std::string& fontPath, int fontSize) {
    if (font) {
        TTF_CloseFont(font);
    }
    
    font = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }
    
    return true;
}

bool PlayerStatsView::loadAvatar(SDL_Renderer* renderer, const std::string& avatarPath) {
    if (playerAvatar) {
        SDL_DestroyTexture(playerAvatar);
        playerAvatar = nullptr;
    }
    
    // Try to find a PNG file in the avatar directory
    DIR* dir = opendir(avatarPath.c_str());
    if (!dir) {
        std::cerr << "Could not open avatar directory: " << avatarPath << std::endl;
        return false;
    }
    
    struct dirent* entry;
    std::vector<std::string> pngFiles;
    
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".png") {
            pngFiles.push_back(avatarPath + "/" + filename);
        }
    }
    closedir(dir);
    
    if (pngFiles.empty()) {
        std::cerr << "No PNG files found in: " << avatarPath << std::endl;
        return false;
    }
    
    // Load the first PNG found
    SDL_Surface* surface = IMG_Load(pngFiles[0].c_str());
    if (!surface) {
        std::cerr << "Failed to load avatar image: " << IMG_GetError() << std::endl;
        return false;
    }
    
    playerAvatar = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!playerAvatar) {
        std::cerr << "Failed to create texture from avatar: " << SDL_GetError() << std::endl;
        return false;
    }
    
    std::cout << "Loaded player avatar from: " << pngFiles[0] << std::endl;
    return true;
}

bool PlayerStatsView::loadNPCPortrait(SDL_Renderer* renderer, const std::string& npcAvatarPath) {
    if (npcPortrait) {
        SDL_DestroyTexture(npcPortrait);
        npcPortrait = nullptr;
    }
    
    // If empty path, just return true (no portrait to load)
    if (npcAvatarPath.empty()) {
        return true;
    }
    
    // Try to find a PNG file in the NPC avatar directory
    DIR* dir = opendir(npcAvatarPath.c_str());
    if (!dir) {
        std::cerr << "Could not open NPC avatar directory: " << npcAvatarPath << std::endl;
        return false;
    }
    
    struct dirent* entry;
    std::vector<std::string> pngFiles;
    
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".png") {
            pngFiles.push_back(npcAvatarPath + "/" + filename);
        }
    }
    closedir(dir);
    
    if (pngFiles.empty()) {
        std::cerr << "No PNG files found in: " << npcAvatarPath << std::endl;
        return false;
    }
    
    // Load the first PNG found
    SDL_Surface* surface = IMG_Load(pngFiles[0].c_str());
    if (!surface) {
        std::cerr << "Failed to load NPC portrait: " << IMG_GetError() << std::endl;
        return false;
    }
    
    npcPortrait = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!npcPortrait) {
        std::cerr << "Failed to create texture from NPC portrait: " << SDL_GetError() << std::endl;
        return false;
    }
    
    std::cout << "Loaded NPC portrait from: " << pngFiles[0] << std::endl;
    return true;
}

void PlayerStatsView::setPlayerName(const std::string& name) {
    playerName = name;
}

void PlayerStatsView::showNPC(const std::string& name) {
    npcName = name;
    showingNPC = true;
}

void PlayerStatsView::hideNPC() {
    showingNPC = false;
    npcName = "";
}

void PlayerStatsView::render(SDL_Renderer* renderer) {
    // Draw background
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect bgRect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    
    int padding = 20;
    int avatarSize = 128;  // Size for avatar squares
    
    if (showingNPC && npcPortrait) {
        // Split view: Player on left, NPC on right
        int halfWidth = width / 2;
        
        // Draw divider line
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderDrawLine(renderer, x + halfWidth, y, x + halfWidth, y + height);
        
        // LEFT SIDE - Player
        if (playerAvatar) {
            SDL_Rect playerAvatarRect = {
                x + padding,
                y + padding,
                avatarSize,
                avatarSize
            };
            SDL_RenderCopy(renderer, playerAvatar, nullptr, &playerAvatarRect);
        }
        
        // Player name
        if (font && !playerName.empty()) {
            SDL_Surface* surface = TTF_RenderText_Blended(font, playerName.c_str(), textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect textRect = {
                        x + padding,
                        y + padding + avatarSize + 10,
                        surface->w,
                        surface->h
                    };
                    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
        
        // RIGHT SIDE - NPC
        if (npcPortrait) {
            SDL_Rect npcAvatarRect = {
                x + halfWidth + padding,
                y + padding,
                avatarSize,
                avatarSize
            };
            SDL_RenderCopy(renderer, npcPortrait, nullptr, &npcAvatarRect);
        }
        
        // NPC name
        if (font && !npcName.empty()) {
            SDL_Surface* surface = TTF_RenderText_Blended(font, npcName.c_str(), textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect textRect = {
                        x + halfWidth + padding,
                        y + padding + avatarSize + 10,
                        surface->w,
                        surface->h
                    };
                    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
    } else {
        // Normal view: Just player
        if (playerAvatar) {
            SDL_Rect playerAvatarRect = {
                x + padding,
                y + padding,
                avatarSize,
                avatarSize
            };
            SDL_RenderCopy(renderer, playerAvatar, nullptr, &playerAvatarRect);
        }
        
        // Player name
        if (font && !playerName.empty()) {
            SDL_Surface* surface = TTF_RenderText_Blended(font, playerName.c_str(), textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect textRect = {
                        x + padding,
                        y + padding + avatarSize + 10,
                        surface->w,
                        surface->h
                    };
                    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
    }
}