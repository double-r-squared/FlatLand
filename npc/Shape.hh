#ifndef SHAPE_HH
#define SHAPE_HH

#include "../Vec2.hh"
#include <string>

// Abstract shape base class
class Shape {
public:
    Vec2 position;
    virtual ~Shape() = default;
    virtual bool intersectsVerticalLine(float x, float& minY, float& maxY) const = 0;
    virtual std::string getType() const = 0;
};

#endif // SHAPE_HH