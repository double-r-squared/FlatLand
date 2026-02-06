#include "player-view.hh"
#include "../../player/player.hh"
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

void PlayerStatsView::render(SDL_Renderer* renderer, Player* player) {
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
        
        // Display player stats from Player object
        if (player && dialogueFont) {
            int statsX = playerAvatarX + avatarSize + padding + 10;
            int statsY = y + padding + 10;
            int statLineHeight = 24;
            
            // HP Bar
            std::string hpText = "HP: " + std::to_string(player->getHitPoints()) + "/" + std::to_string(player->getMaxHitPoints());
            SDL_Surface* hpSurface = TTF_RenderText_Blended(dialogueFont, hpText.c_str(), dialogueColor);
            if (hpSurface) {
                SDL_Texture* hpTexture = SDL_CreateTextureFromSurface(renderer, hpSurface);
                if (hpTexture) {
                    SDL_Rect hpRect = {statsX, statsY, hpSurface->w, hpSurface->h};
                    SDL_RenderCopy(renderer, hpTexture, nullptr, &hpRect);
                    SDL_DestroyTexture(hpTexture);
                }
                SDL_FreeSurface(hpSurface);
            }
            
            // HP Health bar visualization
            int healthBarWidth = 150;
            int healthBarHeight = 12;
            float hpPercent = (float)player->getHitPoints() / player->getMaxHitPoints();
            int filledWidth = (int)(healthBarWidth * hpPercent);
            
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_Rect healthBarBg = {statsX, statsY + statLineHeight, healthBarWidth, healthBarHeight};
            SDL_RenderFillRect(renderer, &healthBarBg);
            
            // Color based on health
            int r = (hpPercent < 0.5f) ? 255 : 100;
            int g = (hpPercent > 0.5f) ? 200 : 100;
            int b = 100;
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_Rect healthBarFill = {statsX, statsY + statLineHeight, filledWidth, healthBarHeight};
            SDL_RenderFillRect(renderer, &healthBarFill);
            
            // Healing Potions
            std::string healText = "Healing Potions: " + std::to_string(player->getHealingPotions());
            SDL_Surface* healSurface = TTF_RenderText_Blended(dialogueFont, healText.c_str(), dialogueColor);
            if (healSurface) {
                SDL_Texture* healTexture = SDL_CreateTextureFromSurface(renderer, healSurface);
                if (healTexture) {
                    SDL_Rect healRect = {statsX, statsY + (statLineHeight * 2), healSurface->w, healSurface->h};
                    SDL_RenderCopy(renderer, healTexture, nullptr, &healRect);
                    SDL_DestroyTexture(healTexture);
                }
                SDL_FreeSurface(healSurface);
            }
            
            // Vision Potions
            std::string visionText = "Vision Potions: " + std::to_string(player->getVisionPotions());
            SDL_Surface* visionSurface = TTF_RenderText_Blended(dialogueFont, visionText.c_str(), dialogueColor);
            if (visionSurface) {
                SDL_Texture* visionTexture = SDL_CreateTextureFromSurface(renderer, visionSurface);
                if (visionTexture) {
                    SDL_Rect visionRect = {statsX, statsY + (statLineHeight * 3), visionSurface->w, visionSurface->h};
                    SDL_RenderCopy(renderer, visionTexture, nullptr, &visionRect);
                    SDL_DestroyTexture(visionTexture);
                }
                SDL_FreeSurface(visionSurface);
            }
            
            // Pillars Found
            const auto& pillars = player->getPillarsPieces();
            std::string pillarText = "Pillars Found: " + std::to_string(pillars.size());
            SDL_Surface* pillarSurface = TTF_RenderText_Blended(dialogueFont, pillarText.c_str(), dialogueColor);
            if (pillarSurface) {
                SDL_Texture* pillarTexture = SDL_CreateTextureFromSurface(renderer, pillarSurface);
                if (pillarTexture) {
                    SDL_Rect pillarRect = {statsX, statsY + (statLineHeight * 4), pillarSurface->w, pillarSurface->h};
                    SDL_RenderCopy(renderer, pillarTexture, nullptr, &pillarRect);
                    SDL_DestroyTexture(pillarTexture);
                }
                SDL_FreeSurface(pillarSurface);
            }
            
            // List pillars found
            int pillarListY = statsY + (statLineHeight * 5);
            for (const auto& pillar : pillars) {
                SDL_Surface* pSurface = TTF_RenderText_Blended(dialogueFont, ("  - " + pillar).c_str(), textColor);
                if (pSurface) {
                    SDL_Texture* pTexture = SDL_CreateTextureFromSurface(renderer, pSurface);
                    if (pTexture) {
                        SDL_Rect pRect = {statsX, pillarListY, pSurface->w, pSurface->h};
                        SDL_RenderCopy(renderer, pTexture, nullptr, &pRect);
                        SDL_DestroyTexture(pTexture);
                    }
                    SDL_FreeSurface(pSurface);
                }
                pillarListY += statLineHeight;
            }
        }
    }
}