#include "dialogue-box.hh"
#include <iostream>
#include <sstream>

DialogueBox::DialogueBox(int x, int y, int width, int height) 
    : x(x), y(y), width(width), height(height), font(nullptr),
      textColor({255, 255, 255, 255}), padding(10), lineHeight(20),
      promptFont(nullptr), showPrompt(false) {
    
    // Initialize SDL_ttf if not already initialized
    if (!TTF_WasInit()) {
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        }
    }
}

DialogueBox::~DialogueBox() {
    if (font) {
        TTF_CloseFont(font);
    }
    if (promptFont) {
        TTF_CloseFont(promptFont);
    }
}

bool DialogueBox::loadFont(const std::string& fontPath, int fontSize) {
    if (font) {
        TTF_CloseFont(font);
    }
    
    font = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }
    
    // Also load a slightly larger font for prompts
    promptFont = TTF_OpenFont(fontPath.c_str(), fontSize + 2);
    
    lineHeight = TTF_FontLineSkip(font);
    return true;
}

void DialogueBox::setContent(const std::string& text) {
    content = text;
}

void DialogueBox::setPrompt(const std::string& text, bool show) {
    promptText = text;
    showPrompt = show;
}

void DialogueBox::setPosition(int newX, int newY) {
    x = newX;
    y = newY;
}

void DialogueBox::setSize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
}

std::vector<std::string> DialogueBox::wrapText(const std::string& text, int maxWidth) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string word;
    std::string currentLine;
    
    while (stream >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        
        if (font) {
            int textWidth;
            TTF_SizeText(font, testLine.c_str(), &textWidth, nullptr);
            
            if (textWidth > maxWidth) {
                if (!currentLine.empty()) {
                    lines.push_back(currentLine);
                    currentLine = word;
                } else {
                    // Single word is too long, add it anyway
                    lines.push_back(word);
                    currentLine = "";
                }
            } else {
                currentLine = testLine;
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

void DialogueBox::renderPrompt(SDL_Renderer* renderer, int centerX, int promptY) {
    if (!showPrompt || promptText.empty() || !promptFont) {
        return;
    }
    
    SDL_Color promptColor = {255, 200, 100, 255}; // Yellow-orange color
    SDL_Surface* surface = TTF_RenderText_Blended(promptFont, promptText.c_str(), promptColor);
    
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            // Center the prompt text
            SDL_Rect destRect = {
                centerX - surface->w / 2,
                promptY,
                surface->w,
                surface->h
            };
            SDL_RenderCopy(renderer, texture, nullptr, &destRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

void DialogueBox::render(SDL_Renderer* renderer) {
    // Draw background
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect rect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // Render text if font is loaded
    if (font && !content.empty()) {
        int maxTextWidth = width - (padding * 2);
        std::vector<std::string> lines = wrapText(content, maxTextWidth);
        
        int currentY = y + padding;
        
        for (const auto& line : lines) {
            if (currentY + lineHeight > y + height - padding) {
                break; // Don't render text outside the box
            }
            
            SDL_Surface* surface = TTF_RenderText_Blended(font, line.c_str(), textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect destRect = {x + padding, currentY, surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
            
            currentY += lineHeight;
        }
    } else if (!content.empty()) {
        // Fallback: print to console if font isn't loaded
        static std::string lastText;
        if (content != lastText) {
            std::cout << content << std::endl;
            lastText = content;
        }
    }
}