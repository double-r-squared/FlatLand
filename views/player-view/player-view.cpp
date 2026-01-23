#include "player-view.hh"
#include <iostream>
#include <dirent.h>
#include <algorithm>
#include <vector>
#include <sstream>
#include <sys/stat.h>  // for stat() to check file existence

PlayerStatsView::PlayerStatsView(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height),
      playerAvatar(nullptr), npcPortrait(nullptr), 
      showingNPC(false), font(nullptr), dialogueFont(nullptr),
      textColor({255, 255, 255, 255}), dialogueColor({220, 220, 220, 255}) {
    
    if (!TTF_WasInit()) {
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        }
    }
    
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << std::endl;
    }
}

PlayerStatsView::~PlayerStatsView() {
    if (playerAvatar) SDL_DestroyTexture(playerAvatar);
    if (npcPortrait)  SDL_DestroyTexture(npcPortrait);
    if (font)         TTF_CloseFont(font);
    if (dialogueFont) TTF_CloseFont(dialogueFont);
}

bool PlayerStatsView::loadFont(const std::string& fontPath, int fontSize) {
    if (font)         TTF_CloseFont(font);
    if (dialogueFont) TTF_CloseFont(dialogueFont);
    
    font = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }
    
    dialogueFont = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!dialogueFont) {
        std::cerr << "Failed to load dialogue font: " << TTF_GetError() << std::endl;
    }
    
    return true;
}

bool PlayerStatsView::loadAvatar(SDL_Renderer* renderer, const std::string& avatarPath) {
    if (playerAvatar) {
        SDL_DestroyTexture(playerAvatar);
        playerAvatar = nullptr;
    }
    
    // Player avatar is loaded from a directory (picks first .png)
    DIR* dir = opendir(avatarPath.c_str());
    if (!dir) {
        std::cerr << "Could not open player avatar directory: " << avatarPath << std::endl;
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
        std::cerr << "No PNG files found in player avatar directory: " << avatarPath << std::endl;
        return false;
    }
    
    // Sort to make loading deterministic (optional)
    std::sort(pngFiles.begin(), pngFiles.end());
    
    SDL_Surface* surface = IMG_Load(pngFiles[0].c_str());
    if (!surface) {
        std::cerr << "Failed to load player avatar image: " << pngFiles[0] 
                  << " - " << IMG_GetError() << std::endl;
        return false;
    }
    
    playerAvatar = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!playerAvatar) {
        std::cerr << "Failed to create texture from player avatar: " << SDL_GetError() << std::endl;
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
    
    if (npcAvatarPath.empty()) {
        std::cout << "No NPC avatar path provided → skipping load" << std::endl;
        return false;
    }
    
    // Check if the file actually exists
    struct stat buffer;
    if (stat(npcAvatarPath.c_str(), &buffer) != 0) {
        std::cerr << "NPC avatar file not found: " << npcAvatarPath << std::endl;
        return false;
    }
    
    SDL_Surface* surface = IMG_Load(npcAvatarPath.c_str());
    if (!surface) {
        std::cerr << "Failed to load NPC portrait from file: " << npcAvatarPath 
                  << " - " << IMG_GetError() << std::endl;
        return false;
    }
    
    npcPortrait = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!npcPortrait) {
        std::cerr << "Failed to create texture from NPC portrait: " << SDL_GetError() << std::endl;
        return false;
    }
    
    std::cout << "Successfully loaded NPC portrait: " << npcAvatarPath << std::endl;
    return true;
}

void PlayerStatsView::setPlayerName(const std::string& name) {
    playerName = name;
}

void PlayerStatsView::showNPC(const std::string& npcId) {
    this->npcId = npcId;
    showingNPC = true;
    std::cout << "[PlayerStatsView] Showing NPC: " << npcId << std::endl;
}

void PlayerStatsView::hideNPC() {
    showingNPC = false;
    npcId = "";
    npcDialogue = "";
    std::cout << "[PlayerStatsView] Hid NPC" << std::endl;
}

void PlayerStatsView::setNPCDialogue(const std::string& dialogue) {
    npcDialogue = dialogue;
}

std::vector<std::string> PlayerStatsView::wrapText(const std::string& text, int maxWidth, TTF_Font* fontToUse) {
    std::vector<std::string> lines;
    if (!fontToUse) return lines;
    
    std::istringstream stream(text);
    std::string word;
    std::string currentLine;
    
    while (stream >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        
        int textWidth;
        TTF_SizeText(fontToUse, testLine.c_str(), &textWidth, nullptr);
        
        if (textWidth > maxWidth) {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine = word;
            } else {
                lines.push_back(word);
                currentLine = "";
            }
        } else {
            currentLine = testLine;
        }
    }
    
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    
    return lines;
}

void PlayerStatsView::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect bgRect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &bgRect);
    
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    
    int padding = 20;
    int nameHeight = 24;
    int avatarSize = height - (2 * padding) - nameHeight;
    
    if (showingNPC) {
        // CONVERSATION MODE
        int separatorX = width / 2;
        int dialogueAreaWidth = (width / 2) - avatarSize - (3 * padding);
        
        // LEFT - Player
        int playerAvatarX = x + padding;
        if (playerAvatar) {
            SDL_Rect playerAvatarRect = {playerAvatarX, y + padding, avatarSize, avatarSize};
            SDL_RenderCopy(renderer, playerAvatar, nullptr, &playerAvatarRect);
        }
        
        // Player Name
        if (font && !playerName.empty()) {
            int nameW, nameH;
            TTF_SizeText(font, playerName.c_str(), &nameW, &nameH);
            int nameX = playerAvatarX + (avatarSize - nameW) / 2;
            int nameY = y + padding + avatarSize + 5;
            
            SDL_Surface* surface = TTF_RenderText_Blended(font, playerName.c_str(), textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect textRect = {nameX, nameY, nameW, nameH};
                    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
        
        // Separator line
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderDrawLine(renderer, x + separatorX, y, x + separatorX, y + height);
        
        // RIGHT - NPC portrait
        int npcAvatarX = x + width - padding - avatarSize;
        if (npcPortrait) {
            SDL_Rect npcAvatarRect = {npcAvatarX, y + padding, avatarSize, avatarSize};
            SDL_RenderCopy(renderer, npcPortrait, nullptr, &npcAvatarRect);
        } else {
            // Optional: draw placeholder rectangle if no portrait
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            SDL_Rect placeholder = {npcAvatarX, y + padding, avatarSize, avatarSize};
            SDL_RenderFillRect(renderer, &placeholder);
        }
        
        // NPC ID (display name)
        if (font && !npcId.empty()) {
            int nameW, nameH;
            TTF_SizeText(font, npcId.c_str(), &nameW, &nameH);
            int nameX = npcAvatarX + (avatarSize - nameW) / 2;
            int nameY = y + padding + avatarSize + 5;
            
            SDL_Surface* surface = TTF_RenderText_Blended(font, npcId.c_str(), textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect textRect = {nameX, nameY, nameW, nameH};
                    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
        
        // NPC Dialogue
        int npcSectionX = x + separatorX + padding;
        int npcDialogueStartY = y + padding + 10;
        
        if (dialogueFont && !npcDialogue.empty()) {
            std::vector<std::string> lines = wrapText(npcDialogue, dialogueAreaWidth, dialogueFont);
            int lineHeight = TTF_FontLineSkip(dialogueFont);
            int currentY = npcDialogueStartY;
            
            for (const auto& line : lines) {
                if (currentY + lineHeight > y + height - padding) break;
                
                SDL_Surface* surface = TTF_RenderText_Blended(dialogueFont, line.c_str(), dialogueColor);
                if (surface) {
                    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                    if (texture) {
                        SDL_Rect textRect = {npcSectionX, currentY, surface->w, surface->h};
                        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                        SDL_DestroyTexture(texture);
                    }
                    SDL_FreeSurface(surface);
                }
                currentY += lineHeight;
            }
        }
    } else {
        // NORMAL MODE (player only)
        int playerAvatarX = x + padding;
        if (playerAvatar) {
            SDL_Rect playerAvatarRect = {playerAvatarX, y + padding, avatarSize, avatarSize};
            SDL_RenderCopy(renderer, playerAvatar, nullptr, &playerAvatarRect);
        }
        
        if (font && !playerName.empty()) {
            int nameW, nameH;
            TTF_SizeText(font, playerName.c_str(), &nameW, &nameH);
            int nameX = playerAvatarX + (avatarSize - nameW) / 2;
            int nameY = y + padding + avatarSize + 5;
            
            SDL_Surface* surface = TTF_RenderText_Blended(font, playerName.c_str(), textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect textRect = {nameX, nameY, nameW, nameH};
                    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
    }
}