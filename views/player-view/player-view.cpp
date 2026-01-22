#include "player-view.hh"
#include <iostream>
#include <dirent.h>
#include <algorithm>
#include <vector>
#include <sstream>

PlayerStatsView::PlayerStatsView(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height),
      playerAvatar(nullptr), npcPortrait(nullptr), 
      showingNPC(false), font(nullptr), dialogueFont(nullptr),
      textColor({255, 255, 255, 255}), dialogueColor({220, 220, 220, 255}) {
    
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
    if (dialogueFont) {
        TTF_CloseFont(dialogueFont);
    }
}

bool PlayerStatsView::loadFont(const std::string& fontPath, int fontSize) {
    if (font) {
        TTF_CloseFont(font);
    }
    if (dialogueFont) {
        TTF_CloseFont(dialogueFont);
    }
    
    font = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }
    
    // Load smaller font for dialogue text
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
    
    if (npcAvatarPath.empty()) {
        return true;
    }
    
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
    npcDialogue = "";
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
    // Draw background
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect bgRect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    
    int padding = 20;
    int nameHeight = 24;  // smaller reserve for names
    int avatarSize = height - (2 * padding) - nameHeight;
    
    if (showingNPC && npcPortrait) {
        // CONVERSATION MODE
        int separatorX = width / 2;
        int dialogueAreaWidth = (width / 2) - avatarSize - (3 * padding);
        
        // LEFT SIDE - Player
        int playerAvatarX = x + padding;
        if (playerAvatar) {
            SDL_Rect playerAvatarRect = {
                playerAvatarX,
                y + padding,
                avatarSize,
                avatarSize
            };
            SDL_RenderCopy(renderer, playerAvatar, nullptr, &playerAvatarRect);
        }
        
        // Player Name - CENTERED UNDER avatar
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
        
        // CENTER - Vertical Separator
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderDrawLine(renderer, x + separatorX, y, x + separatorX, y + height);
        
        // RIGHT SIDE - NPC
        int npcAvatarX = x + width - padding - avatarSize;
        if (npcPortrait) {
            SDL_Rect npcAvatarRect = {
                npcAvatarX,
                y + padding,
                avatarSize,
                avatarSize
            };
            SDL_RenderCopy(renderer, npcPortrait, nullptr, &npcAvatarRect);
        }
        
        // NPC Name - CENTERED UNDER avatar
        if (font && !npcName.empty()) {
            int nameW, nameH;
            TTF_SizeText(font, npcName.c_str(), &nameW, &nameH);
            
            int nameX = npcAvatarX + (avatarSize - nameW) / 2;
            int nameY = y + padding + avatarSize + 5;
            
            SDL_Surface* surface = TTF_RenderText_Blended(font, npcName.c_str(), textColor);
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
        
        // NPC Dialogue Text - start higher to ensure visibility
        int npcSectionX = x + separatorX + padding;
        int npcDialogueStartY = y + padding + 10;  // ← start near top so it's visible
        
        if (dialogueFont && !npcDialogue.empty()) {
            std::vector<std::string> lines = wrapText(npcDialogue, dialogueAreaWidth, dialogueFont);
            int lineHeight = TTF_FontLineSkip(dialogueFont);
            int currentY = npcDialogueStartY;
            
            for (const auto& line : lines) {
                if (currentY + lineHeight > y + height - padding) {
                    break;
                }
                
                SDL_Surface* surface = TTF_RenderText_Blended(dialogueFont, line.c_str(), dialogueColor);
                if (surface) {
                    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                    if (texture) {
                        SDL_Rect textRect = {
                            npcSectionX,
                            currentY,
                            surface->w,
                            surface->h
                        };
                        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                        SDL_DestroyTexture(texture);
                    }
                    SDL_FreeSurface(surface);
                }
                
                currentY += lineHeight;
            }
        }
    } else {
        // NORMAL MODE
        int playerAvatarX = x + padding;
        if (playerAvatar) {
            SDL_Rect playerAvatarRect = {
                playerAvatarX,
                y + padding,
                avatarSize,
                avatarSize
            };
            SDL_RenderCopy(renderer, playerAvatar, nullptr, &playerAvatarRect);
        }
        
        // Player Name - CENTERED UNDER avatar
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