#ifndef MAP_HH
#define MAP_HH

#include "../npc/Shape.hh"
#include "../npc/npc.hh"
#include <vector>
#include <memory>
#include <string>

class Map {
public:
    std::vector<std::shared_ptr<Shape>> shapes;
    std::vector<NPC> npcs;
    std::string name;
    
    void addShape(std::shared_ptr<Shape> shape);
    void addNPC(const NPC& npc);
    void update(float dt);
    void save(const std::string& filename) const;
    static Map load(const std::string& filename);
};

#endif // MAP_HH