#ifndef NPC_HH
#define NPC_HH

#include "Shape.hh"
#include "../Vec2.hh"
#include <memory>

class NPC {
public:
    std::shared_ptr<Shape> shape;
    Vec2 velocity;
    
    NPC(std::shared_ptr<Shape> s, Vec2 vel = Vec2());
    void update(float dt);
};

#endif // NPC_HH