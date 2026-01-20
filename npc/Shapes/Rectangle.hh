#ifndef RECTANGLE_HH
#define RECTANGLE_HH

#include "../Shape.hh"

class Rectangle : public Shape {
public:
    float width, height;
    
    Rectangle(Vec2 pos, float w, float h);
    bool intersectsVerticalLine(float x, float& minY, float& maxY) const override;
    std::string getType() const override;
};

#endif // RECTANGLE_HH