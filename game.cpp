#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "Vec2.hh"
#include "map/map.hh"
#include "npc/npc.hh"
#include "npc/Shapes/Rectangle.hh"
#include "npc/Shapes/Triangle.hh"
#include "npc/Shapes/Circle.hh"
#include "views/dialogue-box/dialogue-box.hh"
#include "views/mini-map/mini-map.hh"
#include "views/world-view/world-view.hh"
#include "views/player-view/player-view.hh"

int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;
const int VIEW_HEIGHT = 400;
const int TEXT_AREA_HEIGHT = 300;
const int MINIMAP_SIZE = 200;

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    Map map;
    Vec2 playerPos;
    float viewAngle;
    std::string textContent;
    
    const Uint8* keyState;
    int mouseX, mouseY;
    int lastMouseX;
    
    // NPC interaction state
    bool inConversation;
    bool eKeyWasPressed;
    
    // View components
    std::unique_ptr<DialogueBox> dialogueBox;
    std::unique_ptr<MiniMap> miniMap;
    std::unique_ptr<WorldView> worldView;
    std::unique_ptr<PlayerStatsView> playerStatsView;
    
public:
    Game() : window(nullptr), renderer(nullptr), running(true), 
             playerPos(5, 2.5), viewAngle(0), lastMouseX(SCREEN_WIDTH/2),
             inConversation(false), eKeyWasPressed(false) {
        
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
            running = false;
            return;
        }
        
        // Get display mode for fullscreen
        SDL_DisplayMode dm;
        SDL_GetCurrentDisplayMode(0, &dm);
        SCREEN_WIDTH = dm.w;
        SCREEN_HEIGHT = dm.h;
        
        window = SDL_CreateWindow("Side View Game", 
                                  SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED,
                                  SCREEN_WIDTH, SCREEN_HEIGHT, 
                                  SDL_WINDOW_FULLSCREEN_DESKTOP);
        
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
        
        // Initialize view components
        int viewHeight = 60;
        int viewStartY = (SCREEN_HEIGHT - TEXT_AREA_HEIGHT) / 2 - viewHeight / 2;
        int worldViewBottom = viewStartY + viewHeight;
        
        // Position dialogue box beneath world view with some spacing
        int dialogueBoxY = worldViewBottom + 30;
        int dialogueBoxHeight = 150;
        
        dialogueBox = std::make_unique<DialogueBox>(
            50,  // x position with left margin
            dialogueBoxY, 
            SCREEN_WIDTH - 100,  // width with margins on both sides
            dialogueBoxHeight
        );
        
        // Try multiple font paths - prioritize assets/fonts folder
        // Supports both .ttf and .otf font formats
        bool fontLoaded = false;
        std::vector<std::string> fontPaths = {
            "assets/fonts/Minecraft/Minecraft-Regular.otf",      // Primary path (OTF)
            "assets/fonts/Minecraft/Minecraft-Bold.otf",         // Another alternative
            "assets/fonts/Minecraft/Minecraft-BoldItalic.otf",   // Another alternative
            "assets/fonts/Minecraft/Minecraft-Italic.otf",       // Another alternative
            "/System/Library/Fonts/Supplemental/Arial.ttf",      // macOS fallback
            "/System/Library/Fonts/Helvetica.ttc",               // macOS alternative
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"    // Linux fallback
        };
        
        for (const auto& fontPath : fontPaths) {
            if (dialogueBox->loadFont(fontPath, 18)) {
                std::cout << "Successfully loaded font: " << fontPath << std::endl;
                fontLoaded = true;
                break;
            }
        }
        
        if (!fontLoaded) {
            std::cerr << "Warning: Could not load any font, text will appear in console only" << std::endl;
            std::cerr << "Try placing a .ttf font file in your project directory" << std::endl;
        }
        
        // Position player stats view beneath dialogue box - span to bottom of screen
        int statsViewY = dialogueBoxY + dialogueBoxHeight + 20;  // 20px spacing
        int statsViewHeight = SCREEN_HEIGHT - statsViewY - 50;  // Span to bottom with margin
        
        playerStatsView = std::make_unique<PlayerStatsView>(
            50,  // x position with left margin
            statsViewY,
            SCREEN_WIDTH - 100,  // width with margins
            statsViewHeight
        );
        
        // Try to load player avatar
        std::vector<std::string> avatarPaths = {
            "assets/player/normal",
            "assets/player/Normal"
        };
        
        bool avatarLoaded = false;
        for (const auto& avatarPath : avatarPaths) {
            if (playerStatsView->loadAvatar(renderer, avatarPath)) {
                avatarLoaded = true;
                break;
            }
        }
        
        if (!avatarLoaded) {
            std::cerr << "Warning: Could not load player avatar" << std::endl;
        }
        
        // Load font for player name
        for (const auto& fontPath : fontPaths) {
            if (playerStatsView->loadFont(fontPath, 24)) {
                std::cout << "Successfully loaded font for player view: " << fontPath << std::endl;
                break;
            }
        }
        
        // Set player name
        playerStatsView->setPlayerName("Square");
        
        miniMap = std::make_unique<MiniMap>(MINIMAP_SIZE, SCREEN_WIDTH);
        worldView = std::make_unique<WorldView>(SCREEN_WIDTH, SCREEN_HEIGHT, TEXT_AREA_HEIGHT);
        
        // Create test map
        map.name = "Town";
        
        // Try to load from file first
        std::ifstream testFile("map/castle.map");
        if (testFile.good()) {
            testFile.close();
            map = Map::load("map/castle.map");
            std::cout << "Loaded map from castle.map" << std::endl;
        } else {
            // If file doesn't exist, create default map
            std::cout << "castle.map not found, creating default map" << std::endl;
            map.addShape(std::make_shared<Rectangle>(Vec2(0, 0), 10, 5));
            map.addShape(std::make_shared<Rectangle>(Vec2(15, 0), 8, 6));
            map.addShape(std::make_shared<Triangle>(Vec2(25, 0), Vec2(30, 0), Vec2(27.5, 5)));
            map.addShape(std::make_shared<Circle>(Vec2(35, 3), 3));
            map.addShape(std::make_shared<Rectangle>(Vec2(40, 0), 5, 8));
            
            auto npcShape = std::make_shared<Circle>(Vec2(20, 3), 1.5);
            map.addNPC(NPC(npcShape, Vec2(2, 0)));
        }
        
        textContent = "Welcome! Use WASD to move, mouse to look around.";
        dialogueBox->setContent(textContent);
        
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
    
    ~Game() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEMOTION) {
                if (!inConversation) {  // Only allow camera movement when not in conversation
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
    
    void update(float dt) {
        // Check for NPC in crosshair
        const NPC* targetNPC = worldView->getNPCInCrosshair(map, playerPos, viewAngle, 3.0f);
        
        // Handle E key for interaction
        bool eKeyPressed = keyState[SDL_SCANCODE_E];
        
        // @todo
        // MARK: CONVERSATION TREE
        if (eKeyPressed && !eKeyWasPressed) {
            // E key was just pressed
            if (inConversation) {
                // Exit conversation
                inConversation = false;
                dialogueBox->setPrompt("", false);
            } else if (targetNPC) {
                // Enter conversation
                inConversation = true;
                dialogueBox->setContent("Hello World");
                dialogueBox->setPrompt("", false);
            }
        }
        
        eKeyWasPressed = eKeyPressed;
        
        // Only allow movement when not in conversation
        if (!inConversation) {
            float moveSpeed = 10.0f * dt;
            
            // Calculate forward and right vectors based on view angle
            Vec2 forward(std::cos(viewAngle), std::sin(viewAngle));
            Vec2 right(-std::sin(viewAngle), std::cos(viewAngle));
            
            // Store old position for collision detection
            Vec2 oldPos = playerPos;
            Vec2 newPos = playerPos;
            
            // WASD movement relative to view direction
            if (keyState[SDL_SCANCODE_W]) newPos = newPos + forward * moveSpeed;
            if (keyState[SDL_SCANCODE_S]) newPos = newPos - forward * moveSpeed;
            if (keyState[SDL_SCANCODE_A]) newPos = newPos - right * moveSpeed;
            if (keyState[SDL_SCANCODE_D]) newPos = newPos + right * moveSpeed;
            
            // Check collision with shapes (walls)
            bool collision = false;
            const float playerRadius = 0.5f; // Player collision radius
            
            for (const auto& shape : map.shapes) {
                float minY, maxY;
                // Check if the player's new position would intersect with a wall
                if (shape->intersectsVerticalLine(newPos.x, minY, maxY)) {
                    if (newPos.y >= minY - playerRadius && newPos.y <= maxY + playerRadius) {
                        collision = true;
                        break;
                    }
                }
            }
            
            // Check collision with NPCs
            if (!collision) {
                for (const auto& npc : map.npcs) {
                    float minY, maxY;
                    if (npc.shape->intersectsVerticalLine(newPos.x, minY, maxY)) {
                        if (newPos.y >= minY - playerRadius && newPos.y <= maxY + playerRadius) {
                            collision = true;
                            break;
                        }
                    }
                }
            }
            
            // Only update position if no collision
            if (!collision) {
                playerPos = newPos;
            }
            
            // Update NPCs
            map.update(dt);
            
            // Bounce NPCs at boundaries
            for (auto& npc : map.npcs) {
                if (npc.shape->position.x < 0 || npc.shape->position.x > 50) {
                    npc.velocity.x *= -1;
                }
            }
            
            // Update text based on position or NPC targeting
            if (targetNPC) {
                textContent = "Exploring... Position: (" + 
                             std::to_string((int)playerPos.x) + ", " + 
                             std::to_string((int)playerPos.y) + ")";
                dialogueBox->setPrompt("E - Talk", true);
            } else {
                textContent = "Exploring... Position: (" + 
                             std::to_string((int)playerPos.x) + ", " + 
                             std::to_string((int)playerPos.y) + ")";
                dialogueBox->setPrompt("", false);
            }
            
            dialogueBox->setContent(textContent);
        }
    }
    
    void render() {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Render world view
        worldView->render(renderer, map, playerPos, viewAngle);
        
        // Draw minimap
        miniMap->render(renderer, map, playerPos, viewAngle);
        
        // Draw dialogue box
        dialogueBox->render(renderer);
        
        // Draw player stats view
        playerStatsView->render(renderer);
        
        // Draw prompt text below crosshair if needed
        int viewHeight = 60;
        int viewStartY = (SCREEN_HEIGHT - TEXT_AREA_HEIGHT) / 2 - viewHeight / 2;
        int centerY = viewStartY + viewHeight / 2;
        int promptY = centerY + 20; // 20 pixels below crosshair
        dialogueBox->renderPrompt(renderer, SCREEN_WIDTH / 2, promptY);
        
        // Present
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
            
            SDL_Delay(16); // Cap at ~60 FPS
        }
    }
};

int main(int argc, char* argv[]) {
    Game game;
    game.run();
    return 0;
}