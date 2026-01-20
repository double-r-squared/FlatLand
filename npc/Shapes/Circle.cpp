#include "Circle.hh"
#include <cmath>

Circle::Circle(Vec2 pos, float r) : radius(r) {
    position = pos;
}

bool Circle::intersectsVerticalLine(float x, float& minY, float& maxY) const {
    float dx = x - position.x;
    if (std::abs(dx) <= radius) {
        float dy = std::sqrt(radius * radius - dx * dx);
        minY = position.y - dy;
        maxY = position.y + dy;
        return true;
    }
    return false;
}

std::string Circle::getType() const { 
    return "circle"; 
}