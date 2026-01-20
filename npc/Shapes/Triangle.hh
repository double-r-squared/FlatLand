#ifndef TRIANGLE_HH
#define TRIANGLE_HH

#include "../Shape.hh"

class Triangle : public Shape {
public:
    Vec2 p1, p2, p3;
    
    Triangle(Vec2 a, Vec2 b, Vec2 c);
    bool intersectsVerticalLine(float x, float& minY, float& maxY) const override;
    std::string getType() const override;
};

#endif // TRIANGLE_HH