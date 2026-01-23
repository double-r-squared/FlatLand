#ifndef LINE_HH
#define LINE_HH

#include "../Shape.hh"
#include "../../Vec2.hh"
#include <vector>

class Line : public Shape {
public:
    std::vector<Vec2> points;
    
    Line(const std::vector<Vec2>& points = {});
    
    bool intersectsVerticalLine(float x, float& minY, float& maxY) const override;
    std::string getType() const override { return "Line"; }
    
    // Additional method to get all points
    const std::vector<Vec2>& getPoints() const { return points; }
    
    // New method: check closest point on line to given position
    float getClosestDistanceToPoint(const Vec2& point) const;
};

#endif // LINE_HH
