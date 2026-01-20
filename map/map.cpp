#include "map.hh"
#include "../npc/Shapes/Rectangle.hh"
#include "../npc/Shapes/Triangle.hh"
#include "../npc/Shapes/Circle.hh"
#include <fstream>
#include <sstream>

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
    
    for (const auto& npc : npcs) {
        if (auto circ = dynamic_cast<Circle*>(npc.shape.get())) {
            file << "NPC_CIRC," << circ->position.x << "," << circ->position.y << ","
                 << circ->radius << "," << npc.velocity.x << "," << npc.velocity.y << "\n";
        }
    }
}

Map Map::load(const std::string& filename) {
    Map map;
    std::ifstream file(filename);
    std::string line;
    
    if (std::getline(file, line)) {
        if (line.substr(0, 4) == "MAP:") {
            map.name = line.substr(4);
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
            map.addShape(std::make_shared<Rectangle>(Vec2(x, y), w, h));
        } else if (type == "TRI") {
            float x1, y1, x2, y2, x3, y3;
            char comma;
            iss >> x1 >> comma >> y1 >> comma >> x2 >> comma >> y2 >> comma >> x3 >> comma >> y3;
            map.addShape(std::make_shared<Triangle>(Vec2(x1, y1), Vec2(x2, y2), Vec2(x3, y3)));
        } else if (type == "CIRC") {
            float x, y, r;
            char comma;
            iss >> x >> comma >> y >> comma >> r;
            map.addShape(std::make_shared<Circle>(Vec2(x, y), r));
        } else if (type == "NPC_CIRC") {
            float x, y, r, vx, vy;
            char comma;
            iss >> x >> comma >> y >> comma >> r >> comma >> vx >> comma >> vy;
            auto shape = std::make_shared<Circle>(Vec2(x, y), r);
            map.addNPC(NPC(shape, Vec2(vx, vy)));
        }
    }
    
    return map;
}