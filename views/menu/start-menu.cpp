#include "start-menu.hh"
#include <SDL2/SDL_ttf.h>
#include <iostream>

// Helper: render text centered in a rect
static void renderTextCentered(
    SDL_Renderer* renderer,
    TTF_Font* font,
    const std::string& text,
    SDL_Color color,
    const SDL_Rect& rect
) {
    if (!font) return;

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

    SDL_Rect dst {
        rect.x + (rect.w - w) / 2,
        rect.y + (rect.h - h) / 2,
        w,
        h
    };

    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

StartMenu::StartMenu(int w, int h)
    : screenW(w),
      screenH(h),
      result(Result::NONE),
      hasSave(false),          // hardcoded for now
      selection(false),
      titleFont(nullptr),
      optionFont(nullptr)
{
    // Ensure TTF is initialized (safe if already done)
    if (TTF_WasInit() == 0) {
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
            return;
        }
    }

    const char* fontPath = "assets/fonts/stitch-warrior/StitchWarrior_demo.ttf";

    titleFont  = TTF_OpenFont(fontPath, 96);
    optionFont = TTF_OpenFont(fontPath, 36);

    if (!titleFont || !optionFont) {
        std::cerr << "Failed to load start menu font: "
                  << TTF_GetError() << std::endl;
    }
}

StartMenu::~StartMenu() {
    if (titleFont) {
        TTF_CloseFont(titleFont);
        titleFont = nullptr;
    }
    if (optionFont) {
        TTF_CloseFont(optionFont);
        optionFont = nullptr;
    }
}

void StartMenu::handleEvent(const SDL_Event& e) {
    if (transitioning) return;

    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_0) {
            selection = false;
            transitioning = true;
        }
        if (e.key.keysym.sym == SDLK_1 && hasSave) {
            selection = true;
            transitioning = true;
        }
        if (e.key.keysym.sym == SDLK_ESCAPE) {
            result = Result::QUIT;
        }
    }
}

void StartMenu::update(float dt) {
    if (transitioning) {
        barProgress += dt * 1.5f;
        if (barProgress >= 1.f) {
            barProgress = 1.f;
            result = selection ? Result::CONTINUE : Result::NEW_GAME;
        }
    }
}

void StartMenu::render(SDL_Renderer* renderer) {
    // background
    SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
    SDL_RenderClear(renderer);

    SDL_Color white { 255, 255, 255, 255 };

    // --- Render title ---
    std::string titleText = "FlatLand";
    SDL_Rect titleRect {
        screenW / 2 - 300, // center horizontally approx
        screenH / 4 - 60,  // vertical position
        600, 120            // width, height for centering
    };
    renderTextCentered(renderer, titleFont, titleText, white, titleRect);

    // --- Render options ---
    std::string newGameText = "New Game";
    std::string continueText = "Continue";

    SDL_Rect newGameRect {
        screenW / 2 - 260,
        screenH / 2,
        240,
        80
    };

    SDL_Rect continueRect {
        screenW / 2 + 20,
        screenH / 2,
        240,
        80
    };

    renderTextCentered(renderer, optionFont, newGameText, white, newGameRect);
    renderTextCentered(renderer, optionFont, continueText, white, continueRect);

    // --- Transition bars ---
    if (transitioning) {
        int barHeight = static_cast<int>((screenH / 2) * barProgress);

        SDL_Rect topBar { 0, 0, screenW, barHeight };
        SDL_Rect bottomBar { 0, screenH - barHeight, screenW, barHeight };

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &topBar);
        SDL_RenderFillRect(renderer, &bottomBar);
    }
}

StartMenu::Result StartMenu::getResult() const {
    return result;
}
