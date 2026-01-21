#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <cmath>
#include "Vec2.hh"
#include "npc/Shape.hh"
#include "npc/Shapes/Rectangle.hh"
#include "npc/Shapes/Triangle.hh"
#include "npc/Shapes/Circle.hh"
#include "npc/npc.hh"

const int WINDOW_WIDTH = 1400;
const int WINDOW_HEIGHT = 800;
const int TOOLBAR_WIDTH = 300;
const int PREVIEW_HEIGHT = 200;

enum class Tool {
    RECTANGLE,
    TRIANGLE,
    CIRCLE,
    NPC,
    SELECT,
    DELETE
};

class MapBuilder {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    
    std::vector<std::shared_ptr<Shape>> shapes;
    std::vector<NPC> npcs;
    
    Tool currentTool;
    Vec2 clickStart;
    bool isDragging;
    int selectedShape;
    int selectedNPC;  // Track selected NPC for editing
    
    // Camera
    Vec2 cameraOffset;
    float zoom;
    
    // For triangle creation
    std::vector<Vec2> trianglePoints;
    
    // Map name
    std::string mapName;
    
public:
    MapBuilder() : window(nullptr), renderer(nullptr), running(true),
                   currentTool(Tool::RECTANGLE), isDragging(false),
                   selectedShape(-1), selectedNPC(-1), 
                   cameraOffset(50, 400), zoom(10.0f),
                   mapName("Untitled Map") {
        
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
            running = false;
            return;
        }
        
        window = SDL_CreateWindow("Map Builder", 
                                  SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT, 
                                  SDL_WINDOW_SHOWN);
        
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
        
        std::cout << "Map Builder Controls:" << std::endl;
        std::cout << "  R - Rectangle tool" << std::endl;
        std::cout << "  T - Triangle tool (click 3 points)" << std::endl;
        std::cout << "  C - Circle tool" << std::endl;
        std::cout << "  N - NPC tool" << std::endl;
        std::cout << "  D - Delete tool" << std::endl;
        std::cout << "  E - Edit selected NPC properties" << std::endl;
        std::cout << "  S - Save map" << std::endl;
        std::cout << "  L - Load map" << std::endl;
        std::cout << "  Arrow Keys - Pan camera" << std::endl;
        std::cout << "  +/- - Zoom in/out" << std::endl;
        std::cout << "  ESC - Quit" << std::endl;
    }
    
    ~MapBuilder() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
    Vec2 screenToWorld(int x, int y) {
        float worldX = (x - cameraOffset.x) / zoom;
        float worldY = (cameraOffset.y - y) / zoom;
        return Vec2(worldX, worldY);
    }
    
    Vec2 worldToScreen(Vec2 world) {
        int screenX = (int)(world.x * zoom + cameraOffset.x);
        int screenY = (int)(cameraOffset.y - world.y * zoom);
        return Vec2(screenX, screenY);
    }
    
    void editNPCProperties(int npcIndex) {
        if (npcIndex < 0 || npcIndex >= (int)npcs.size()) {
            std::cout << "No NPC selected" << std::endl;
            return;
        }
        
        NPC& npc = npcs[npcIndex];
        
        std::cout << "\n=== Editing NPC ===" << std::endl;
        std::cout << "Current ID: " << npc.id << std::endl;
        std::cout << "Current Name: " << npc.name << std::endl;
        std::cout << "Current Dialogue File: " << npc.dialogueFile << std::endl;
        
        std::cout << "\nEnter new ID (or press Enter to keep current): ";
        std::string newId;
        std::getline(std::cin, newId);
        if (!newId.empty()) {
            npc.id = newId;
        }
        
        std::cout << "Enter new Name (or press Enter to keep current): ";
        std::string newName;
        std::getline(std::cin, newName);
        if (!newName.empty()) {
            npc.name = newName;
        }
        
        std::cout << "Enter dialogue file path (or press Enter to keep current): ";
        std::string newDialogue;
        std::getline(std::cin, newDialogue);
        if (!newDialogue.empty()) {
            npc.dialogueFile = newDialogue;
        }
        
        std::cout << "\nNPC updated!" << std::endl;
        std::cout << "  ID: " << npc.id << std::endl;
        std::cout << "  Name: " << npc.name << std::endl;
        std::cout << "  Dialogue: " << npc.dialogueFile << std::endl;
    }
    
    void selectNPCAtPosition(Vec2 worldPos) {
        selectedNPC = -1;
        
        for (size_t i = 0; i < npcs.size(); i++) {
            float minY, maxY;
            if (npcs[i].shape->intersectsVerticalLine(worldPos.x, minY, maxY)) {
                if (worldPos.y >= minY && worldPos.y <= maxY) {
                    selectedNPC = i;
                    std::cout << "Selected NPC: " << npcs[i].name << " (" << npcs[i].id << ")" << std::endl;
                    return;
                }
            }
        }
    }
    
    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: running = false; break;
                    case SDLK_r: currentTool = Tool::RECTANGLE; std::cout << "Rectangle tool selected" << std::endl; break;
                    case SDLK_t: currentTool = Tool::TRIANGLE; trianglePoints.clear(); std::cout << "Triangle tool selected (click 3 points)" << std::endl; break;
                    case SDLK_c: currentTool = Tool::CIRCLE; std::cout << "Circle tool selected" << std::endl; break;
                    case SDLK_n: currentTool = Tool::NPC; std::cout << "NPC tool selected" << std::endl; break;
                    case SDLK_d: currentTool = Tool::DELETE; std::cout << "Delete tool selected" << std::endl; break;
                    case SDLK_e: editNPCProperties(selectedNPC); break;
                    case SDLK_s: saveMap(); break;
                    case SDLK_l: loadMap(); break;
                    case SDLK_LEFT: cameraOffset.x += 20; break;
                    case SDLK_RIGHT: cameraOffset.x -= 20; break;
                    case SDLK_UP: cameraOffset.y += 20; break;
                    case SDLK_DOWN: cameraOffset.y -= 20; break;
                    case SDLK_EQUALS: 
                    case SDLK_PLUS: zoom *= 1.2f; break;
                    case SDLK_MINUS: zoom /= 1.2f; break;
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mx = event.button.x;
                    int my = event.button.y;
                    
                    if (mx < WINDOW_WIDTH - TOOLBAR_WIDTH) {
                        Vec2 worldPos = screenToWorld(mx, my);
                        
                        if (currentTool == Tool::TRIANGLE) {
                            trianglePoints.push_back(worldPos);
                            std::cout << "Triangle point " << trianglePoints.size() << ": (" 
                                     << worldPos.x << ", " << worldPos.y << ")" << std::endl;
                            
                            if (trianglePoints.size() == 3) {
                                shapes.push_back(std::make_shared<Triangle>(
                                    trianglePoints[0], trianglePoints[1], trianglePoints[2]
                                ));
                                std::cout << "Triangle created!" << std::endl;
                                trianglePoints.clear();
                            }
                        } else if (currentTool == Tool::DELETE) {
                            // Find and delete shape at click position
                            for (int i = shapes.size() - 1; i >= 0; i--) {
                                float minY, maxY;
                                if (shapes[i]->intersectsVerticalLine(worldPos.x, minY, maxY)) {
                                    if (worldPos.y >= minY && worldPos.y <= maxY) {
                                        shapes.erase(shapes.begin() + i);
                                        std::cout << "Shape deleted" << std::endl;
                                        break;
                                    }
                                }
                            }
                            
                            // Also check NPCs
                            for (int i = npcs.size() - 1; i >= 0; i--) {
                                float minY, maxY;
                                if (npcs[i].shape->intersectsVerticalLine(worldPos.x, minY, maxY)) {
                                    if (worldPos.y >= minY && worldPos.y <= maxY) {
                                        std::cout << "Deleted NPC: " << npcs[i].name << std::endl;
                                        npcs.erase(npcs.begin() + i);
                                        if (selectedNPC == i) selectedNPC = -1;
                                        break;
                                    }
                                }
                            }
                        } else if (currentTool == Tool::SELECT) {
                            selectNPCAtPosition(worldPos);
                        } else {
                            clickStart = worldPos;
                            isDragging = true;
                        }
                    }
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    // Right click to select NPC
                    int mx = event.button.x;
                    int my = event.button.y;
                    if (mx < WINDOW_WIDTH - TOOLBAR_WIDTH) {
                        Vec2 worldPos = screenToWorld(mx, my);
                        selectNPCAtPosition(worldPos);
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT && isDragging) {
                    int mx = event.button.x;
                    int my = event.button.y;
                    Vec2 worldEnd = screenToWorld(mx, my);
                    
                    if (currentTool == Tool::RECTANGLE) {
                        float x = std::min(clickStart.x, worldEnd.x);
                        float y = std::min(clickStart.y, worldEnd.y);
                        float w = std::abs(worldEnd.x - clickStart.x);
                        float h = std::abs(worldEnd.y - clickStart.y);
                        
                        if (w > 0.1f && h > 0.1f) {
                            shapes.push_back(std::make_shared<Rectangle>(Vec2(x, y), w, h));
                            std::cout << "Rectangle created at (" << x << ", " << y << ") size " << w << "x" << h << std::endl;
                        }
                    } else if (currentTool == Tool::CIRCLE || currentTool == Tool::NPC) {
                        Vec2 center = clickStart;
                        float radius = (worldEnd - clickStart).length();
                        
                        if (radius > 0.1f) {
                            auto circle = std::make_shared<Circle>(center, radius);
                            
                            if (currentTool == Tool::NPC) {
                                // Prompt for NPC details
                                std::cout << "\n=== Creating New NPC ===" << std::endl;
                                std::cout << "Enter NPC ID: ";
                                std::string npcId;
                                std::getline(std::cin, npcId);
                                
                                std::cout << "Enter NPC Name: ";
                                std::string npcName;
                                std::getline(std::cin, npcName);
                                
                                std::cout << "Enter dialogue file path (optional): ";
                                std::string dialoguePath;
                                std::getline(std::cin, dialoguePath);
                                
                                // Default velocity: moving right
                                NPC newNPC(circle, Vec2(2, 0), npcId, npcName);
                                if (!dialoguePath.empty()) {
                                    newNPC.dialogueFile = dialoguePath;
                                }
                                
                                npcs.push_back(newNPC);
                                selectedNPC = npcs.size() - 1;
                                
                                std::cout << "NPC created: " << npcName << " (" << npcId << ")" << std::endl;
                            } else {
                                shapes.push_back(circle);
                                std::cout << "Circle created at (" << center.x << ", " << center.y << ") radius " << radius << std::endl;
                            }
                        }
                    }
                    
                    isDragging = false;
                }
            }
        }
    }
    
    void saveMap() {
        std::cout << "Enter map filename (without .map extension): ";
        std::string filename;
        std::getline(std::cin, filename);
        
        if (filename.empty()) {
            filename = "level";
        }
        
        filename = "map/" + filename + ".map";
        
        std::ofstream file(filename);
        file << "MAP:" << mapName << "\n";
        
        for (const auto& shape : shapes) {
            if (auto rect = dynamic_cast<Rectangle*>(shape.get())) {
                file << "RECT," << rect->position.x << "," << rect->position.y << ","
                     << rect->width << "," << rect->height << "\n";
            } else if (auto tri = dynamic_cast<Triangle*>(shape.get())) {
                file << "TRI," << tri->p1.x << "," << tri->p1.y << ","
                     << tri->p2.x << "," << tri->p2.y << ","
                     << tri->p3.x << "," << tri->p3.y << "\n";
            } else if (auto circ = dynamic_cast<Circle*>(shape.get())) {
                file << "CIRC," << circ->position.x << "," << circ->position.y << ","
                     << circ->radius << "\n";
            }
        }
        
        // Here the NPC file is stored 
        for (const auto& npc : npcs) {
            if (auto circ = dynamic_cast<Circle*>(npc.shape.get())) {
                file << "NPC_CIRC," 
                     << circ->position.x << "," << circ->position.y << ","
                     << circ->radius << "," 
                     << npc.velocity.x << "," << npc.velocity.y << ","
                     << npc.id << "," << npc.name << "," << npc.dialogueFile << "\n";
            }
        }
        
        std::cout << "Map saved to " << filename << std::endl;
    }
    
    void loadMap() {
        std::cout << "Enter map filename (without .map extension): ";
        std::string filename;
        std::getline(std::cin, filename);
        
        if (filename.empty()) return;
        
        filename = "map/" + filename + ".map";
        
        std::ifstream file(filename);
        if (!file.good()) {
            std::cout << "File not found: " << filename << std::endl;
            return;
        }
        
        shapes.clear();
        npcs.clear();
        selectedNPC = -1;
        
        std::string line;
        if (std::getline(file, line)) {
            if (line.substr(0, 4) == "MAP:") {
                mapName = line.substr(4);
            }
        }
        
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string type;
            std::getline(iss, type, ',');
            
            if (type == "RECT") {
                float x, y, w, h;
                char comma;
                iss >> x >> comma >> y >> comma >> w >> comma >> h;
                shapes.push_back(std::make_shared<Rectangle>(Vec2(x, y), w, h));
            } else if (type == "TRI") {
                float x1, y1, x2, y2, x3, y3;
                char comma;
                iss >> x1 >> comma >> y1 >> comma >> x2 >> comma >> y2 >> comma >> x3 >> comma >> y3;
                shapes.push_back(std::make_shared<Triangle>(Vec2(x1, y1), Vec2(x2, y2), Vec2(x3, y3)));
            } else if (type == "CIRC") {
                float x, y, r;
                char comma;
                iss >> x >> comma >> y >> comma >> r;
                shapes.push_back(std::make_shared<Circle>(Vec2(x, y), r));
            } else if (type == "NPC_CIRC") {
                float x, y, r, vx, vy;
                std::string npcId, npcName, dialoguePath;
                char comma;
                
                iss >> x >> comma >> y >> comma >> r >> comma >> vx >> comma >> vy >> comma;
                std::getline(iss, npcId, ',');
                std::getline(iss, npcName, ',');
                std::getline(iss, dialoguePath);
                
                auto shape = std::make_shared<Circle>(Vec2(x, y), r);
                NPC npc(shape, Vec2(vx, vy), npcId, npcName);
                npc.dialogueFile = dialoguePath;
                npcs.push_back(npc);
            }
        }
        
        std::cout << "Map loaded from " << filename << std::endl;
        std::cout << "Loaded " << shapes.size() << " shapes and " << npcs.size() << " NPCs" << std::endl;
    }
    
    void render() {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderClear(renderer);
        
        // Draw grid
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        for (int i = -50; i <= 100; i += 5) {
            Vec2 start = worldToScreen(Vec2(i, -50));
            Vec2 end = worldToScreen(Vec2(i, 50));
            SDL_RenderDrawLine(renderer, start.x, start.y, end.x, end.y);
        }
        for (int i = -50; i <= 50; i += 5) {
            Vec2 start = worldToScreen(Vec2(-50, i));
            Vec2 end = worldToScreen(Vec2(100, i));
            SDL_RenderDrawLine(renderer, start.x, start.y, end.x, end.y);
        }
        
        // Draw Y=0 axis (ground)
        SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
        Vec2 axisStart = worldToScreen(Vec2(-50, 0));
        Vec2 axisEnd = worldToScreen(Vec2(100, 0));
        SDL_RenderDrawLine(renderer, axisStart.x, axisStart.y, axisEnd.x, axisEnd.y);
        
        // Draw shapes
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        for (const auto& shape : shapes) {
            if (auto rect = dynamic_cast<Rectangle*>(shape.get())) {
                Vec2 tl = worldToScreen(Vec2(rect->position.x, rect->position.y + rect->height));
                Vec2 br = worldToScreen(Vec2(rect->position.x + rect->width, rect->position.y));
                SDL_Rect r = {(int)tl.x, (int)tl.y, (int)(br.x - tl.x), (int)(br.y - tl.y)};
                SDL_RenderDrawRect(renderer, &r);
            } else if (auto circ = dynamic_cast<Circle*>(shape.get())) {
                Vec2 center = worldToScreen(circ->position);
                int r = (int)(circ->radius * zoom);
                for (int i = 0; i < 32; i++) {
                    float angle1 = i * 2 * M_PI / 32;
                    float angle2 = (i + 1) * 2 * M_PI / 32;
                    SDL_RenderDrawLine(renderer,
                        center.x + r * std::cos(angle1), center.y - r * std::sin(angle1),
                        center.x + r * std::cos(angle2), center.y - r * std::sin(angle2));
                }
            } else if (auto tri = dynamic_cast<Triangle*>(shape.get())) {
                Vec2 p1 = worldToScreen(tri->p1);
                Vec2 p2 = worldToScreen(tri->p2);
                Vec2 p3 = worldToScreen(tri->p3);
                SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y);
                SDL_RenderDrawLine(renderer, p2.x, p2.y, p3.x, p3.y);
                SDL_RenderDrawLine(renderer, p3.x, p3.y, p1.x, p1.y);
            }
        }
        
        // Draw NPCs
        for (size_t i = 0; i < npcs.size(); i++) {
            const auto& npc = npcs[i];
            
            // Highlight selected NPC
            if ((int)i == selectedNPC) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
            }
            
            if (auto circ = dynamic_cast<Circle*>(npc.shape.get())) {
                Vec2 center = worldToScreen(circ->position);
                int r = (int)(circ->radius * zoom);
                for (int j = 0; j < 32; j++) {
                    float angle1 = j * 2 * M_PI / 32;
                    float angle2 = (j + 1) * 2 * M_PI / 32;
                    SDL_RenderDrawLine(renderer,
                        center.x + r * std::cos(angle1), center.y - r * std::sin(angle1),
                        center.x + r * std::cos(angle2), center.y - r * std::sin(angle2));
                }
                
                // Draw velocity arrow
                Vec2 arrowEnd = worldToScreen(Vec2(circ->position.x + npc.velocity.x, circ->position.y + npc.velocity.y));
                SDL_RenderDrawLine(renderer, center.x, center.y, arrowEnd.x, arrowEnd.y);
            }
        }
        
        // Draw triangle points in progress
        if (!trianglePoints.empty()) {
            SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
            for (const auto& pt : trianglePoints) {
                Vec2 screen = worldToScreen(pt);
                SDL_Rect r = {(int)screen.x - 3, (int)screen.y - 3, 6, 6};
                SDL_RenderFillRect(renderer, &r);
            }
        }
        
        // Draw current drag
        if (isDragging) {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            
            SDL_SetRenderDrawColor(renderer, 100, 255, 255, 128);
            if (currentTool == Tool::RECTANGLE) {
                Vec2 start = worldToScreen(clickStart);
                SDL_Rect r = {
                    (int)std::min(start.x, (float)mx),
                    (int)std::min(start.y, (float)my),
                    (int)std::abs(mx - start.x),
                    (int)std::abs(my - start.y)
                };
                SDL_RenderDrawRect(renderer, &r);
            } else if (currentTool == Tool::CIRCLE || currentTool == Tool::NPC) {
                Vec2 center = worldToScreen(clickStart);
                float dx = mx - center.x;
                float dy = my - center.y;
                int r = (int)std::sqrt(dx * dx + dy * dy);
                for (int i = 0; i < 32; i++) {
                    float angle1 = i * 2 * M_PI / 32;
                    float angle2 = (i + 1) * 2 * M_PI / 32;
                    SDL_RenderDrawLine(renderer,
                        center.x + r * std::cos(angle1), center.y + r * std::sin(angle1),
                        center.x + r * std::cos(angle2), center.y + r * std::sin(angle2));
                }
            }
        }
        
        // Draw toolbar
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_Rect toolbar = {WINDOW_WIDTH - TOOLBAR_WIDTH, 0, TOOLBAR_WIDTH, WINDOW_HEIGHT - PREVIEW_HEIGHT};
        SDL_RenderFillRect(renderer, &toolbar);
        
        // Draw preview
        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
        SDL_Rect preview = {WINDOW_WIDTH - TOOLBAR_WIDTH, WINDOW_HEIGHT - PREVIEW_HEIGHT, TOOLBAR_WIDTH, PREVIEW_HEIGHT};
        SDL_RenderFillRect(renderer, &preview);
        
        // Draw preview label
        SDL_SetRenderDrawColor(renderer, 100, 100, 150, 255);
        SDL_RenderDrawRect(renderer, &preview);
        
        // Draw 1D preview (simplified version of game view)
        renderPreview();
        
        SDL_RenderPresent(renderer);
    }
    
    void renderPreview() {
        int previewY = WINDOW_HEIGHT - PREVIEW_HEIGHT / 2;
        int previewWidth = TOOLBAR_WIDTH - 40;
        int previewX = WINDOW_WIDTH - TOOLBAR_WIDTH + 20;
        
        // Simulate player position
        Vec2 playerPos(5, 2.5);
        float viewAngle = 0;
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 150, 255);
        SDL_RenderDrawLine(renderer, previewX, previewY, previewX + previewWidth, previewY);
        
        // Simple preview rendering
        for (int i = 0; i < previewWidth; i++) {
            float screenPercent = (float)i / previewWidth;
            float angle = viewAngle + (screenPercent - 0.5f) * M_PI;
            
            Vec2 rayDir(std::cos(angle), std::sin(angle));
            bool hit = false;
            
            for (float dist = 0.1f; dist < 50.0f && !hit; dist += 0.5f) {
                Vec2 checkPos = playerPos + rayDir * dist;
                
                for (const auto& shape : shapes) {
                    float minY, maxY;
                    if (shape->intersectsVerticalLine(checkPos.x, minY, maxY)) {
                        if (checkPos.y >= minY && checkPos.y <= maxY) {
                            hit = true;
                            break;
                        }
                    }
                }
                
                for (const auto& npc : npcs) {
                    float minY, maxY;
                    if (npc.shape->intersectsVerticalLine(checkPos.x, minY, maxY)) {
                        if (checkPos.y >= minY && checkPos.y <= maxY) {
                            hit = true;
                            break;
                        }
                    }
                }
            }
            
            if (hit) {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                SDL_RenderDrawPoint(renderer, previewX + i, previewY);
            }
        }
    }
    
    void run() {
        while (running) {
            handleEvents();
            render();
            SDL_Delay(16);
        }
    }
};

int main(int argc, char* argv[]) {
    MapBuilder builder;
    builder.run();
    return 0;
}