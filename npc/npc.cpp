#include "npc.hh"

NPC::NPC(std::shared_ptr<Shape> s, Vec2 vel) : shape(s), velocity(vel) {}

void NPC::update(float dt) {
    shape->position = shape->position + velocity * dt;
}