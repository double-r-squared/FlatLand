#include "Triangle.hh"
#include <vector>
#include <algorithm>
#include <cmath>

Triangle::Triangle(Vec2 a, Vec2 b, Vec2 c) : p1(a), p2(b), p3(c) {
    position = Vec2((a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3);
}

bool Triangle::intersectsVerticalLine(float x, float& minY, float& maxY) const {
    std::vector<float> yIntersections;
    
    auto checkEdge = [&](Vec2 a, Vec2 b) {
        if ((a.x <= x && b.x >= x) || (a.x >= x && b.x <= x)) {
            if (std::abs(b.x - a.x) < 0.001f) {
                yIntersections.push_back(a.y);
                yIntersections.push_back(b.y);
            } else {
                float t = (x - a.x) / (b.x - a.x);
                float y = a.y + t * (b.y - a.y);
                yIntersections.push_back(y);
            }
        }
    };
    
    checkEdge(p1, p2);
    checkEdge(p2, p3);
    checkEdge(p3, p1);
    
    if (yIntersections.empty()) return false;
    
    minY = *std::min_element(yIntersections.begin(), yIntersections.end());
    maxY = *std::max_element(yIntersections.begin(), yIntersections.end());
    return true;
}

std::string Triangle::getType() const { 
    return "triangle"; 
}