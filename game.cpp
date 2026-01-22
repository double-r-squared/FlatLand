#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sys/stat.h>  // Add this for mkdir
#include <cstdlib>     // Add this for system()
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
    int mouseX;  // Removed mouseY since not used
    int lastMouseX;
    
    // NPC interaction state
    bool inConversation;
    bool eKeyWasPressed;
    NPC* currentTalkingNPC;  // Track which NPC we're talking to
    
    // View components
    std::unique_ptr<DialogueBox> dialogueBox;
    std::unique_ptr<MiniMap> miniMap;
    std::unique_ptr<WorldView> worldView;
    std::unique_ptr<PlayerStatsView> playerStatsView;
    
public:
    Game() : window(nullptr), renderer(nullptr), running(true), 
             playerPos(5, 2.5), viewAngle(0), lastMouseX(SCREEN_WIDTH/2),
             inConversation(false), eKeyWasPressed(false), currentTalkingNPC(nullptr) {
        
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
        
        window = SDL_CreateWindow("FlatLand", 
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
        std::vector<std::string> avatarPaths = { "assets/player"};
        
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
        std::ifstream testFile("map/caarlHouse.map");
        
        // After loading the map, update the dialogue file paths:
        if (testFile.good()) {
            testFile.close();
            map = Map::load("map/caarlHouse.map");
            std::cout << "Loaded map from caarlHouse.map" << std::endl;
            
            // FIX: Check and fix NPC avatar paths if they're empty
            std::cout << "\n=== CHECKING NPC AVATAR PATHS ===" << std::endl;
            for (auto& npc : map.npcs) {
                std::cout << "NPC: " << npc.name << " (ID: " << npc.id << ")" << std::endl;
                std::cout << "Original avatar path: '" << npc.avatarPath << "'" << std::endl;
                
                // If avatar path is empty, set it to the default NPC directory
                if (npc.avatarPath.empty()) {
                    npc.avatarPath = "assets/npc";
                    std::cout << "  -> Fixed: Set avatar path to: " << npc.avatarPath << std::endl;
                }
                
                // Note: Removed directory checking code that was causing compile errors
                // The actual directory check happens in PlayerStatsView::loadNPCPortrait
            }
            std::cout << "=== END AVATAR CHECK ===\n" << std::endl;
            
            std::cout << "\n=== DIALOGUE LOADING DEBUG ===" << std::endl;
            for (auto& npc : map.npcs) {
                std::cout << "\nNPC: " << npc.name << " (ID: " << npc.id << ")" << std::endl;
                std::cout << "Avatar path: " << npc.avatarPath << std::endl;
                
                // Fix common path issues
                std::string originalPath = npc.dialogueFile;
                
                // Try multiple possible paths
                std::vector<std::string> possiblePaths;
                
                // 1. Try the path as-is
                possiblePaths.push_back(npc.dialogueFile);
                
                // 2. Try removing ".." prefix if present
                if (npc.dialogueFile.find("../") == 0) {
                    possiblePaths.push_back(npc.dialogueFile.substr(3)); // Remove "../"
                    possiblePaths.push_back("dialogues/" + npc.dialogueFile.substr(3));
                }
                
                // 3. Try standard dialogues folder
                possiblePaths.push_back("dialogues/" + npc.id + ".dialogue");
                possiblePaths.push_back("dialogues/" + npc.name + ".dialogue");
                possiblePaths.push_back("dialogues/" + npc.id + ".txt");
                
                // 4. Try with .dialogue extension if missing
                if (npc.dialogueFile.find(".dialogue") == std::string::npos) {
                    possiblePaths.push_back(npc.dialogueFile + ".dialogue");
                }
                
                bool dialogueLoaded = false;
                for (const auto& path : possiblePaths) {
                    std::ifstream test(path);
                    if (test.good()) {
                        std::cout << "  Found dialogue at: " << path << std::endl;
                        npc.dialogueFile = path;
                        npc.loadDialogue(path);
                        
                        if (npc.hasDialogue()) {
                            std::cout << "  SUCCESS: Loaded " << npc.dialogueNodes.size() << " dialogue nodes" << std::endl;
                            dialogueLoaded = true;
                            break;
                        } else {
                            std::cout << "  WARNING: File exists but no dialogue nodes parsed" << std::endl;
                        }
                    }
                }
                
                if (!dialogueLoaded && !originalPath.empty()) {
                    std::cout << "  ERROR: Could not find or load dialogue file" << std::endl;
                    std::cout << "  Tried paths:" << std::endl;
                    for (const auto& path : possiblePaths) {
                        std::cout << "    - " << path << std::endl;
                    }
                }
            }
            std::cout << "=== END DEBUG ===\n" << std::endl;
        }
        
        textContent = "Welcome! Use WASD to move, mouse to look around. Look at NPCs and press E to talk.";
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
        const NPC* targetNPCConst = worldView->getNPCInCrosshair(map, playerPos, viewAngle, 3.0f);

        // Check for NPC in crosshair
        NPC* targetNPC = nullptr;
        if (targetNPCConst) {
            // Find the corresponding NPC in our map (non-const version)
            for (auto& npc : map.npcs) {
                if (&npc == targetNPCConst) {
                    targetNPC = &npc;
                    break;
                }
            }
        }
        
        // Handle E key for interaction
        bool eKeyPressed = keyState[SDL_SCANCODE_E];
        
        // CONVERSATION TREE IMPLEMENTATION
        if (eKeyPressed && !eKeyWasPressed) {
            // E key was just pressed
            if (inConversation) {
                // We're already in conversation - advance dialogue
                if (currentTalkingNPC && currentTalkingNPC->hasDialogue()) {
                    currentTalkingNPC->advanceDialogue();
                    
                    // Check if we're back at start (conversation ended)
                    if (currentTalkingNPC->currentNodeId == currentTalkingNPC->startNodeId) {
                        // Only end conversation if we've completed at least one full cycle
                        if (currentTalkingNPC->conversationCount > 0) {
                            // End conversation
                            inConversation = false;
                            currentTalkingNPC = nullptr;
                            dialogueBox->setPrompt("", false);
                            textContent = "You finished talking with the NPC.";
                            
                            // Hide NPC portrait
                            playerStatsView->hideNPC();
                        } else {
                            // Still in first conversation, update text
                            dialogueBox->setContent(currentTalkingNPC->getCurrentDialogueText());
                        }
                    } else {
                        // Still in conversation, update text
                        dialogueBox->setContent(currentTalkingNPC->getCurrentDialogueText());
                        dialogueBox->setPrompt("E - Continue", true);
                    }
                } else {
                    // No valid dialogue, end conversation
                    inConversation = false;
                    currentTalkingNPC = nullptr;
                    dialogueBox->setPrompt("", false);
                    
                    // Hide NPC portrait
                    playerStatsView->hideNPC();
                }
            } else if (targetNPC) {
                // DEBUG: Print NPC info before loading
                std::cout << "\n=== STARTING CONVERSATION WITH NPC ===" << std::endl;
                std::cout << "NPC Name: " << targetNPC->name << std::endl;
                std::cout << "NPC Avatar Path: '" << targetNPC->avatarPath << "'" << std::endl;
                
                // Start conversation with NPC
                inConversation = true;
                currentTalkingNPC = targetNPC;
                
                // Load and show NPC portrait
                if (!targetNPC->avatarPath.empty()) {
                    std::cout << "DEBUG: Calling loadNPCPortrait with path: " << targetNPC->avatarPath << std::endl;
                    bool success = playerStatsView->loadNPCPortrait(renderer, targetNPC->avatarPath);
                    std::cout << "DEBUG: loadNPCPortrait returned: " << (success ? "SUCCESS" : "FAILED") << std::endl;
                } else {
                    std::cout << "DEBUG: NPC avatarPath is EMPTY!" << std::endl;
                    // Try to load from default location
                    bool success = playerStatsView->loadNPCPortrait(renderer, "assets/npc");
                    std::cout << "DEBUG: Tried loading from default location: " 
                              << (success ? "SUCCESS" : "FAILED") << std::endl;
                }
                
                std::cout << "DEBUG: Calling showNPC with name: " << targetNPC->name << std::endl;
                playerStatsView->showNPC(targetNPC->name);
                std::cout << "=== CONVERSATION STARTED ===\n" << std::endl;
                
                // Load NPC's dialogue if not already loaded
                if (!targetNPC->dialogueFile.empty() && !targetNPC->hasDialogue()) {
                    targetNPC->loadDialogue(targetNPC->dialogueFile);
                }
                
                // Reset to start of dialogue (or start if new conversation)
                targetNPC->resetDialogue();
                
                // Set initial dialogue text
                if (targetNPC->hasDialogue()) {
                    dialogueBox->setContent(targetNPC->getCurrentDialogueText());
                } else {
                    dialogueBox->setContent("Hello, I'm " + targetNPC->name + ".");
                }
                
                dialogueBox->setPrompt("E - Continue", true);
            }
        }
        
        eKeyWasPressed = eKeyPressed;
        
        // Only allow movement when not in conversation
        if (!inConversation) {
            float moveSpeed = 10.0f * dt;
            
            // Calculate forward and right vectors based on view angle
            Vec2 forward(std::cos(viewAngle), std::sin(viewAngle));
            Vec2 right(-std::sin(viewAngle), std::cos(viewAngle));
            
            // Store old position for collision detection (unused but kept for reference)
            // Vec2 oldPos = playerPos;  // Commented out since not used
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
                textContent = targetNPC->name + 
                             " - Position: (" + 
                             std::to_string((int)playerPos.x) + ", " + 
                             std::to_string((int)playerPos.y) + ")";
                dialogueBox->setPrompt("E - Talk", true);
            } else {
                textContent = "Exploring... Position: (" + 
                             std::to_string((int)playerPos.x) + ", " + 
                             std::to_string((int)playerPos.y) + ")";
                dialogueBox->setPrompt("", false);
            }
            
            // Only update dialogue box content if we're not in conversation
            if (!inConversation) {
                dialogueBox->setContent(textContent);
            }
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
    (void)argc;  // Mark unused parameters
    (void)argv;
    
    Game game;
    game.run();
    return 0;
}