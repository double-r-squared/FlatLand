#include "Rectangle.hh"

Rectangle::Rectangle(Vec2 pos, float w, float h) : width(w), height(h) {
    position = pos;
}

bool Rectangle::intersectsVerticalLine(float x, float& minY, float& maxY) const {
    if (x >= position.x && x <= position.x + width) {
        minY = position.y;
        maxY = position.y + height;
        return true;
    }
    return false;
}

std::string Rectangle::getType() const { 
    return "rectangle"; 
}