#include "player-view.hh"
#include <iostream>
#include <algorithm>

PlayerStatsView::PlayerStatsView(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height), 
      avatarTexture(nullptr), renderer(nullptr), font(nullptr),
      playerName("Square") {
}

PlayerStatsView::~PlayerStatsView() {
    if (avatarTexture) {
        SDL_DestroyTexture(avatarTexture);
    }
    if (font) {
        TTF_CloseFont(font);
    }
}

bool PlayerStatsView::loadAvatar(SDL_Renderer* renderer, const std::string& imagePath) {
    this->renderer = renderer;
    
    // Try multiple image formats
    std::vector<std::string> extensions = {".png", ".jpg", ".jpeg", ".bmp"};
    
    for (const auto& ext : extensions) {
        std::string fullPath = imagePath + ext;
        SDL_Surface* surface = IMG_Load(fullPath.c_str());
        
        if (surface) {
            avatarTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            
            if (avatarTexture) {
                std::cout << "Successfully loaded avatar: " << fullPath << std::endl;
                return true;
            }
        }
    }
    
    std::cerr << "Failed to load avatar from: " << imagePath << std::endl;
    return false;
}

bool PlayerStatsView::loadFont(const std::string& fontPath, int fontSize) {
    font = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!font) {
        std::cerr << "Failed to load font: " << fontPath << std::endl;
        return false;
    }
    return true;
}

void PlayerStatsView::setPlayerName(const std::string& name) {
    playerName = name;
}

void PlayerStatsView::render(SDL_Renderer* renderer) {
    // Draw background
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect bgRect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Draw avatar - scale to fill most of the height while maintaining aspect ratio
    if (avatarTexture) {
        // Calculate avatar size - leave room for padding and name text
        int availableHeight = height - (PADDING * 3) - 40; // 40px for name text
        int avatarSize = std::min(availableHeight, width - (PADDING * 2));
        
        // Position avatar on the LEFT side
        int avatarX = x + PADDING;
        int avatarY = y + PADDING;
        
        SDL_Rect avatarRect = {
            avatarX,
            avatarY,
            avatarSize,
            avatarSize
        };
        
        // SDL_image will scale the texture to fit the destination rect
        SDL_RenderCopy(renderer, avatarTexture, nullptr, &avatarRect);
        
        // Render player name below avatar on the left side
        if (font) {
            SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, playerName.c_str(), textColor);
            
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                
                if (textTexture) {
                    // Position text below the avatar, centered under the avatar
                    int textY = avatarY + avatarSize + PADDING;
                    int textX = avatarX + (avatarSize - textSurface->w) / 2; // Center text under avatar
                    
                    SDL_Rect textRect = {
                        textX,
                        textY,
                        textSurface->w,
                        textSurface->h
                    };
                    
                    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                
                SDL_FreeSurface(textSurface);
            }
        } else {
            // Fallback to console if no font loaded
            static bool nameShown = false;
            if (!nameShown) {
                std::cout << "Player Name: " << playerName << std::endl;
                nameShown = true;
            }
        }
    }
}