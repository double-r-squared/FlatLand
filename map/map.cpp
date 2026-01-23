#include "map.hh"
#include "../npc/Shapes/Rectangle.hh"
#include "../npc/Shapes/Triangle.hh"
#include "../npc/Shapes/Circle.hh"
#include "../npc/Shapes/Line.hh"
#include <fstream>
#include <sstream>
#include <iostream>

void Map::addShape(std::shared_ptr<Shape> shape) {
    shapes.push_back(shape);
}

void Map::addNPC(const NPC& npc) {
    npcs.push_back(npc);
}

void Map::update(float dt) {
    for (auto& npc : npcs) {
        npc.update(dt);
    }
}

void Map::save(const std::string& filename) const {
    std::ofstream file(filename);
    file << "MAP:" << name << "\n";
    
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
    
    // Save all NPCs
    for (const auto& npc : npcs) {
        if (auto circ = dynamic_cast<Circle*>(npc.shape.get())) {
            file << "NPC_CIRC," << circ->position.x << "," << circ->position.y << ","
                 << circ->radius << "," << npc.velocity.x << "," << npc.velocity.y << "\n";
        }
    }
    
    // Save lines
    for (const auto& line : lines) {
        file << "LINE";
        for (const auto& pt : line->getPoints()) {
            file << "," << pt.x << "," << pt.y;
        }
        file << "\n";
    }
}

Map Map::load(const std::string& filename) {
    Map map;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open map file: " << filename << std::endl;
        return map;
    }

    std::string line;

    // Read MAP: line
    if (std::getline(file, line)) {
        if (line.substr(0, 4) == "MAP:") {
            map.name = line.substr(4);
        } else {
            std::cerr << "Invalid map file - expected 'MAP:' on first line" << std::endl;
            return map;
        }
    }

    int npc_count = 0;
    int shape_count = 0;
    int line_count = 0;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string type;
        std::getline(iss, type, ',');

        char comma;

        if (type == "RECT") {
            float x, y, w, h;
            iss >> x >> comma >> y >> comma >> w >> comma >> h;
            if (iss) {
                map.addShape(std::make_shared<Rectangle>(Vec2(x, y), w, h));
                shape_count++;
            }
        }
        else if (type == "TRI") {
            float x1, y1, x2, y2, x3, y3;
            iss >> x1 >> comma >> y1 >> comma >> x2 >> comma >> y2 >> comma >> x3 >> comma >> y3;
            if (iss) {
                map.addShape(std::make_shared<Triangle>(
                    Vec2(x1, y1), Vec2(x2, y2), Vec2(x3, y3)
                ));
                shape_count++;
            }
        }
        else if (type == "CIRC") {
            float x, y, r;
            iss >> x >> comma >> y >> comma >> r;
            if (iss) {
                map.addShape(std::make_shared<Circle>(Vec2(x, y), r));
                shape_count++;
            }
        }
        else if (type == "NPC_CIRC") {
            float x, y, r, vx, vy;
            std::string npc_id;

            iss >> x >> comma >> y >> comma >> r >> comma >> vx >> comma >> vy;

            // Read optional identifier/name after the last comma
            if (iss >> comma) {
                std::getline(iss, npc_id);
                // Clean up whitespace
                npc_id.erase(0, npc_id.find_first_not_of(" \t\r\n"));
                npc_id.erase(npc_id.find_last_not_of(" \t\r\n") + 1);
            }

            if (iss) {
                auto shape = std::make_shared<Circle>(Vec2(x, y), r);
                // MARK: NPC FACTORY
                NPC new_npc(shape, Vec2(vx, vy));

                // Store the name/ID if present
                if (!npc_id.empty()) {
                    new_npc.id = npc_id;
                    // You can also do: new_npc.name = npc_id;  // if your NPC has a 'name' field
                }

                map.addNPC(new_npc);
                npc_count++;
            }
        }
        else if (type == "LINE") {
            std::vector<Vec2> linePoints;
            std::string coord;
            while (std::getline(iss, coord, ',')) {
                float x, y;
                std::istringstream coordIss(coord);
                coordIss >> x;
                if (coordIss) {
                    std::getline(iss, coord, ',');
                    std::istringstream yIss(coord);
                    yIss >> y;
                    if (yIss) {
                        linePoints.push_back(Vec2(x, y));
                    }
                }
            }
            if (linePoints.size() >= 2) {
                map.lines.push_back(std::make_shared<Line>(linePoints));
                line_count++;
            }
        }
    }

    // Improved debug output with NPC names/IDs
    std::cout << "\n=== MAP LOADED DEBUG ===\n";
    std::cout << "Map name: " << map.name << "\n";
    std::cout << "Static shapes loaded: " << shape_count << "\n";
    std::cout << "Lines loaded: " << line_count << "\n";
    std::cout << "NPCs loaded: " << npc_count << "\n";

    if (!map.npcs.empty()) {
        std::cout << "NPC list:\n";
        for (const auto& npc : map.npcs) {
            std::string id_str = npc.id.empty() ? "(no id)" : npc.id;
            float x = npc.shape ? npc.shape->position.x : 0.0f;
            float y = npc.shape ? npc.shape->position.y : 0.0f;
            std::cout << "  - " << id_str 
                      << " at (" << x << ", " << y << ")\n";
        }
    }

    std::cout << "========================\n\n";

    return map;
}