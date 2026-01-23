#include "Line.hh"
#include <algorithm>
#include <cmath>

Line::Line(const std::vector<Vec2>& pts) : points(pts) {
    // Update position to be the average of all points
    if (!points.empty()) {
        float avgX = 0, avgY = 0;
        for (const auto& pt : points) {
            avgX += pt.x;
            avgY += pt.y;
        }
        position = Vec2(avgX / points.size(), avgY / points.size());
    }
}

bool Line::intersectsVerticalLine(float x, float& minY, float& maxY) const {
    if (points.size() < 2) {
        return false;
    }
    
    std::vector<float> yValues;
    
    // Check each line segment
    for (size_t i = 0; i + 1 < points.size(); ++i) {
        const Vec2& p1 = points[i];
        const Vec2& p2 = points[i + 1];
        
        // Get x range for this segment
        float minX = std::min(p1.x, p2.x);
        float maxX = std::max(p1.x, p2.x);
        
        // Add small tolerance to allow passing by endpoints
        const float tolerance = 0.05f;
        
        // Skip if x is outside the segment's x range (with tolerance)
        if (x < minX - tolerance || x > maxX + tolerance) {
            continue;
        }
        
        // Handle vertical line segment
        if (std::abs(p2.x - p1.x) < 1e-6f) {
            if (std::abs(p1.x - x) < tolerance) {
                yValues.push_back(std::min(p1.y, p2.y));
                yValues.push_back(std::max(p1.y, p2.y));
            }
            continue;
        }
        
        // Linear interpolation to find y at x
        float t = (x - p1.x) / (p2.x - p1.x);
        // Clamp t to [0, 1] but with slight tolerance for numerical precision
        if (t >= -tolerance && t <= 1.0f + tolerance) {
            float y = p1.y + t * (p2.y - p1.y);
            yValues.push_back(y);
        }
    }
    
    if (yValues.empty()) {
        return false;
    }
    
    minY = *std::min_element(yValues.begin(), yValues.end());
    maxY = *std::max_element(yValues.begin(), yValues.end());
    
    return true;
}

float Line::getClosestDistanceToPoint(const Vec2& point) const {
    if (points.size() < 2) {
        return 1e6f;  // No collision if line doesn't have segments
    }
    
    float minDist = 1e6f;
    
    // Check distance to each line segment
    for (size_t i = 0; i + 1 < points.size(); ++i) {
        const Vec2& p1 = points[i];
        const Vec2& p2 = points[i + 1];
        
        // Vector from p1 to p2
        Vec2 seg = p2 - p1;
        float segLenSq = seg.x * seg.x + seg.y * seg.y;
        
        if (segLenSq < 1e-6f) {
            // Segment is essentially a point
            Vec2 diff = point - p1;
            float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
            minDist = std::min(minDist, dist);
            continue;
        }
        
        // Vector from p1 to point
        Vec2 toPoint = point - p1;
        
        // Project toPoint onto segment
        float t = (toPoint.x * seg.x + toPoint.y * seg.y) / segLenSq;
        
        // Clamp t to [0, 1] to stay on segment
        t = std::max(0.0f, std::min(1.0f, t));
        
        // Find closest point on segment
        Vec2 closest = p1 + seg * t;
        
        // Calculate distance
        Vec2 diff = point - closest;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        
        minDist = std::min(minDist, dist);
    }
    
    return minDist;
}
