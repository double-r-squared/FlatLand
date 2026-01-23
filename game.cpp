#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <cstdlib>

#include "Vec2.hh"
#include "map/map.hh"
#include "npc/npc.hh"
#include "npc/Shapes/Rectangle.hh"
#include "npc/Shapes/Triangle.hh"
#include "npc/Shapes/Circle.hh"
#include "views/mini-map/mini-map.hh"
#include "views/world-view/world-view.hh"
#include "views/player-view/player-view.hh"
#include "views/menu/start-menu.hh"

enum class GameState {
    MENU,
    PLAYING
};

int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;
const int MINIMAP_SIZE = 200;

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;

    GameState state;
    std::unique_ptr<StartMenu> startMenu;

    Map map;
    Vec2 playerPos;
    float viewAngle;

    const Uint8* keyState;
    int mouseX;

    // NPC interaction state
    bool inConversation;
    bool eKeyWasPressed;
    NPC* currentTalkingNPC;

    // View components
    std::unique_ptr<MiniMap> miniMap;
    std::unique_ptr<WorldView> worldView;
    std::unique_ptr<PlayerStatsView> playerStatsView;

public:
    Game()
        : window(nullptr),
          renderer(nullptr),
          running(true),

          state(GameState::MENU),

          playerPos(5, 2.5),
          viewAngle(0),
          mouseX(SCREEN_WIDTH / 2),
          inConversation(false),
          eKeyWasPressed(false),
          currentTalkingNPC(nullptr)
    {
        // ================= SDL INIT =================
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
            running = false;
            return;
        }

        SDL_DisplayMode dm;
        SDL_GetCurrentDisplayMode(0, &dm);
        SCREEN_WIDTH = dm.w;
        SCREEN_HEIGHT = dm.h;

        window = SDL_CreateWindow(
            "FlatLand",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_FULLSCREEN_DESKTOP
        );

        if (!window) {
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            running = false;
            return;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        if (!renderer) {
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
            running = false;
            return;
        }

        startMenu = std::make_unique<StartMenu>(SCREEN_WIDTH, SCREEN_HEIGHT);

        // ================= PLAYER STATS VIEW =================
        int statsX      = 40;
        int statsY      = SCREEN_HEIGHT - 290;
        int statsWidth  = SCREEN_WIDTH - 80;
        int statsHeight = 270;

        playerStatsView = std::make_unique<PlayerStatsView>(
            statsX, statsY, statsWidth, statsHeight
        );

        // ================= PLAYER AVATAR =================
        std::vector<std::string> avatarPaths = { "assets/player" };
        bool avatarLoaded = false;

        for (const auto& avatarPath : avatarPaths) {
            std::cout << "DEBUG: Trying to load player avatar from: " << avatarPath << std::endl;
            if (playerStatsView->loadAvatar(renderer, avatarPath)) {
                avatarLoaded = true;
                std::cout << "DEBUG: Successfully loaded player avatar" << std::endl;
                break;
            }
        }

        if (!avatarLoaded) {
            std::cerr << "Warning: Could not load player avatar" << std::endl;
        }

        // ================= FONT LOADING =================
        std::vector<std::string> fontPaths = {
            "assets/fonts/Minecraft/Minecraft-Regular.otf",
            "assets/fonts/Minecraft/Minecraft-Bold.otf",
            "assets/fonts/Minecraft/Minecraft-BoldItalic.otf",
            "assets/fonts/Minecraft/Minecraft-Italic.otf",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
            "/System/Library/Fonts/Helvetica.ttc",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
        };

        bool fontLoaded = false;
        for (const auto& fontPath : fontPaths) {
            if (playerStatsView->loadFont(fontPath, 24)) {
                std::cout << "Successfully loaded font for player view: " << fontPath << std::endl;
                fontLoaded = true;
                break;
            }
        }

        if (!fontLoaded) {
            std::cerr << "Warning: Could not load font for player stats view" << std::endl;
        }

        playerStatsView->setPlayerName("Square");

        // ================= VIEWS =================
        miniMap   = std::make_unique<MiniMap>(MINIMAP_SIZE, SCREEN_WIDTH);
        worldView = std::make_unique<WorldView>(SCREEN_WIDTH, SCREEN_HEIGHT);

        // ================= MAP LOAD =================
        map.name = "Town";

        std::ifstream testFile("map/caarlHouse.map");
        if (testFile.good()) {
            testFile.close();
            map = Map::load("map/caarlHouse.map");
            std::cout << "Loaded map from caarlHouse.map" << std::endl;

            // ===== NPC AVATAR FIXUP =====
            std::cout << "\n=== CHECKING NPC AVATAR PATHS ===\n";
            for (auto& npc : map.npcs) {
                std::cout << "NPC: " << npc.name << " (ID: " << npc.id << ")\n";
                std::cout << "Original avatar path: '" << npc.avatarPath << "'\n";

                if (npc.avatarPath.empty()) {
                    npc.avatarPath = "assets/npc";
                    std::cout << "  -> Fixed: Set avatar path to: " << npc.avatarPath << "\n";
                }
            }
            std::cout << "=== END AVATAR CHECK ===\n\n";

            // ===== DIALOGUE RESOLUTION =====
            std::cout << "\n=== DIALOGUE LOADING DEBUG ===\n";
            for (auto& npc : map.npcs) {
                std::cout << "\nNPC: " << npc.name << " (ID: " << npc.id << ")\n";
                std::cout << "Avatar path: " << npc.avatarPath << "\n";
                std::cout << "Dialogue file: " << npc.dialogueFile << "\n";

                std::string originalPath = npc.dialogueFile;
                std::vector<std::string> possiblePaths;
                possiblePaths.push_back(npc.dialogueFile);

                if (npc.dialogueFile.find("../") == 0) {
                    possiblePaths.push_back(npc.dialogueFile.substr(3));
                    possiblePaths.push_back("dialogues/" + npc.dialogueFile.substr(3));
                }

                possiblePaths.push_back("dialogues/" + npc.id + ".dialogue");
                possiblePaths.push_back("dialogues/" + npc.name + ".dialogue");

                if (npc.dialogueFile.find(".dialogue") == std::string::npos) {
                    possiblePaths.push_back(npc.dialogueFile + ".dialogue");
                }

                bool dialogueFound = false;
                for (const auto& path : possiblePaths) {
                    std::ifstream test(path);
                    if (test.good()) {
                        std::cout << "  Found dialogue file at: " << path << "\n";
                        npc.dialogueFile = path;
                        dialogueFound = true;
                        break;
                    }
                }

                if (!dialogueFound && !originalPath.empty()) {
                    std::cout << "  WARNING: Could not find dialogue file\n";
                    std::cout << "  Tried: " << originalPath << "\n";
                }
            }
            std::cout << "=== END DEBUG ===\n\n";
        }

        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    ~Game() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }

    // ================= EVENTS =================
    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            // MENU HANDLING
            if (state == GameState::MENU) {
                startMenu->handleEvent(event);
                continue;
            }

            // ===== ORIGINAL INPUT =====
            if (event.type == SDL_MOUSEMOTION) {
                if (!inConversation) {
                    mouseX += event.motion.xrel;
                    viewAngle += event.motion.xrel * 0.003f;
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }

        keyState = SDL_GetKeyboardState(nullptr);
    }

    // ================= UPDATE =================
    void update(float dt) {
        if (state == GameState::MENU) {
            startMenu->update(dt);
            if (startMenu->getResult() != StartMenu::Result::NONE) {
                state = GameState::PLAYING;
                SDL_SetRelativeMouseMode(SDL_TRUE); // restore original behavior
            }
            return;
        }

        // ===== ORIGINAL UPDATE CODE (UNCHANGED) =====
        const NPC* targetNPCConst =
            worldView->getNPCInCrosshair(map, playerPos, viewAngle, 3.0f);

        NPC* targetNPC = nullptr;
        if (targetNPCConst) {
            for (auto& npc : map.npcs) {
                if (&npc == targetNPCConst) {
                    targetNPC = &npc;
                    break;
                }
            }
        }

        bool eKeyPressed = keyState[SDL_SCANCODE_E];

        if (eKeyPressed && !eKeyWasPressed) {
            if (inConversation && currentTalkingNPC) {
                bool continues = currentTalkingNPC->advanceConversation();
                if (continues) {
                    playerStatsView->setNPCDialogue(
                        currentTalkingNPC->getCurrentText()
                    );
                    worldView->setPrompt(
                        currentTalkingNPC->getPrompt(), true
                    );
                } else {
                    currentTalkingNPC->endConversation();
                    inConversation = false;
                    currentTalkingNPC = nullptr;
                    worldView->setPrompt("", false);
                    playerStatsView->hideNPC();
                }
            }
            else if (targetNPC && targetNPC->canTalk()) {
                targetNPC->startConversation();
                inConversation = true;
                currentTalkingNPC = targetNPC;

                playerStatsView->loadNPCPortrait(
                    renderer,
                    targetNPC->avatarPath.empty() ? "assets/npc"
                                                  : targetNPC->avatarPath
                );

                playerStatsView->showNPC(targetNPC->name);
                playerStatsView->setNPCDialogue(
                    targetNPC->getCurrentText()
                );
                worldView->setPrompt(
                    targetNPC->getPrompt(), true
                );
            }
        }

        eKeyWasPressed = eKeyPressed;

        if (!inConversation) {
            float moveSpeed = 10.0f * dt;

            Vec2 forward(std::cos(viewAngle), std::sin(viewAngle));
            Vec2 right(-std::sin(viewAngle), std::cos(viewAngle));

            Vec2 newPos = playerPos;

            if (keyState[SDL_SCANCODE_W]) newPos = newPos + forward * moveSpeed;
            if (keyState[SDL_SCANCODE_S]) newPos = newPos - forward * moveSpeed;
            if (keyState[SDL_SCANCODE_A]) newPos = newPos - right * moveSpeed;
            if (keyState[SDL_SCANCODE_D]) newPos = newPos + right * moveSpeed;

            bool collision = false;
            const float playerRadius = 0.5f;

            for (const auto& shape : map.shapes) {
                float minY, maxY;
                if (shape->intersectsVerticalLine(newPos.x, minY, maxY)) {
                    if (newPos.y >= minY - playerRadius &&
                        newPos.y <= maxY + playerRadius) {
                        collision = true;
                        break;
                    }
                }
            }

            if (!collision) {
                for (const auto& npc : map.npcs) {
                    float minY, maxY;
                    if (npc.shape->intersectsVerticalLine(newPos.x, minY, maxY)) {
                        if (newPos.y >= minY - playerRadius &&
                            newPos.y <= maxY + playerRadius) {
                            collision = true;
                            break;
                        }
                    }
                }
            }

            if (!collision) {
                playerPos = newPos;
            }

            map.update(dt);

            for (auto& npc : map.npcs) {
                if (npc.shape->position.x < 0 ||
                    npc.shape->position.x > 50) {
                    npc.velocity.x *= -1;
                }
            }

            if (targetNPC) {
                worldView->setPrompt(targetNPC->getPrompt(), true);
            } else {
                worldView->setPrompt("", false);
            }
        }
    }

    // ================= RENDER =================
    void render() {
        if (state == GameState::MENU) {
            startMenu->render(renderer);
            SDL_RenderPresent(renderer);
            return;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        worldView->render(renderer, map, playerPos, viewAngle);
        miniMap->render(renderer, map, playerPos, viewAngle);
        playerStatsView->render(renderer);

        SDL_RenderPresent(renderer);
    }

    void run() {
        Uint32 lastTime = SDL_GetTicks();
        while (running) {
            Uint32 currentTime = SDL_GetTicks();
            float dt = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;

            handleEvents();
            update(dt);
            render();

            SDL_Delay(16);
        }
    }
};

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    Game game;
    game.run();
    return 0;
}
