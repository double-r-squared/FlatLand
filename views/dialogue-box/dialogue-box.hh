#ifndef DIALOGUE_BOX_HH
#define DIALOGUE_BOX_HH

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

class DialogueBox {
private:
    std::string content;
    std::string promptText;
    bool showPrompt;
    int x, y, width, height;
    TTF_Font* font;
    TTF_Font* promptFont;
    SDL_Color textColor;
    int padding;
    int lineHeight;
    
    std::vector<std::string> wrapText(const std::string& text, int maxWidth);
    
public:
    DialogueBox(int x, int y, int width, int height);
    ~DialogueBox();
    
    bool loadFont(const std::string& fontPath, int fontSize);
    void setContent(const std::string& text);
    void setPrompt(const std::string& text, bool show);
    void render(SDL_Renderer* renderer);
    void renderPrompt(SDL_Renderer* renderer, int centerX, int promptY);
    void setPosition(int newX, int newY);
    void setSize(int newWidth, int newHeight);
};

#endif // DIALOGUE_BOX_HH