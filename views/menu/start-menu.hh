#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class StartMenu {
public:
    enum class Result {
        NONE,
        NEW_GAME,
        CONTINUE,
        QUIT
    };

    StartMenu(int w, int h);
    ~StartMenu();  // ← ADD THIS

    void handleEvent(const SDL_Event& e);
    void update(float dt);
    void render(SDL_Renderer* renderer);

    Result getResult() const;

private:
    int screenW;
    int screenH;

    Result result;
    bool hasSave;
    bool selection;
    float barProgress;
    bool transitioning;

    // ✅ ADD THESE (nothing else)
    TTF_Font* titleFont;
    TTF_Font* optionFont;
};
