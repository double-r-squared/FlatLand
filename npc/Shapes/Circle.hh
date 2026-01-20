#ifndef CIRCLE_HH
#define CIRCLE_HH

#include "../Shape.hh"

class Circle : public Shape {
public:
    float radius;
    
    Circle(Vec2 pos, float r);
    bool intersectsVerticalLine(float x, float& minY, float& maxY) const override;
    std::string getType() const override;
};

#endif // CIRCLE_HH