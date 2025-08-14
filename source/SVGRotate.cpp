#include "stdafx.h"
#include "SVGRotate.h"
#include "SVGGroup.h"

void SVGRotate::visit(SVGCircle *circle)
{
    // Apply rotation using matrix transform
    Gdiplus::Matrix rotateMatrix;
    
    // Get circle center for rotation
    PointF center = circle->getCenter();
    
    // Rotate around circle center
    rotateMatrix.RotateAt(degree, center);
    
    // Apply the rotation transform to the element
    circle->applyTransform(rotateMatrix);
    
    std::cout << "Applied matrix rotation " << degree << " degrees to circle around center (" 
              << center.X << ", " << center.Y << ")" << std::endl;
}

void SVGRotate::visit(SVGRect *rectangle)
{
    // Apply rotation using matrix transform instead of changing coordinates
    Gdiplus::Matrix rotateMatrix;
    
    // Calculate center of rectangle for rotation
    float centerX = rectangle->topLeft.X + rectangle->width / 2;
    float centerY = rectangle->topLeft.Y + rectangle->height / 2;
    
    // Rotate around rectangle center
    rotateMatrix.RotateAt(degree, Gdiplus::PointF(centerX, centerY));
    
    // Apply the rotation transform to the element
    rectangle->applyTransform(rotateMatrix);
    
    std::cout << "Applied matrix rotation " << degree << " degrees around center (" 
              << centerX << ", " << centerY << ")" << std::endl;
}

void SVGRotate::visit(SVGLine *line)
{
    // Apply rotation using matrix transform
    Gdiplus::Matrix rotateMatrix;
    
    // Calculate line center for rotation
    float x1 = line->getter_x1();
    float y1 = line->getter_y1();
    float x2 = line->getter_x2();
    float y2 = line->getter_y2();
    float centerX = (x1 + x2) / 2;
    float centerY = (y1 + y2) / 2;
    
    // Rotate around line center
    rotateMatrix.RotateAt(degree, Gdiplus::PointF(centerX, centerY));
    
    // Apply the rotation transform to the element
    line->applyTransform(rotateMatrix);
    
    std::cout << "Applied matrix rotation " << degree << " degrees to line around center (" 
              << centerX << ", " << centerY << ")" << std::endl;
}

void SVGRotate::visit(SVGText *text)
{
    // Apply rotation using matrix transform
    Gdiplus::Matrix rotateMatrix;
    
    // Get text position for rotation
    PointF startPoint = text->getStartPoint();
    
    // Rotate around text position
    rotateMatrix.RotateAt(degree, startPoint);
    
    // Apply the rotation transform to the element
    text->applyTransform(rotateMatrix);
    
    std::cout << "Applied matrix rotation " << degree << " degrees to text around position (" 
              << startPoint.X << ", " << startPoint.Y << ")" << std::endl;
}

void SVGRotate::visit(SVGEllipse *ellipse)
{
    // Apply rotation using matrix transform
    Gdiplus::Matrix rotateMatrix;
    
    // Get ellipse center for rotation
    float cx = ellipse->getCx();
    float cy = ellipse->getCy();
    
    // Rotate around ellipse center
    rotateMatrix.RotateAt(degree, Gdiplus::PointF(cx, cy));
    
    // Apply the rotation transform to the element
    ellipse->applyTransform(rotateMatrix);
    
    std::cout << "Applied matrix rotation " << degree << " degrees to ellipse around center (" 
              << cx << ", " << cy << ")" << std::endl;
}

/* void SVGRotate::visit(SVGPath *path)
{   
}
 */

void SVGRotate::visit(SVGPolygon *polygon)
{
    // Apply rotation using matrix transform
    Gdiplus::Matrix rotateMatrix;
    
    // Calculate polygon centroid for rotation
    std::vector<PointF> points = polygon->getPoints();
    if (points.empty()) return;
    
    float centerX = 0, centerY = 0;
    for (const auto& point : points) {
        centerX += point.X;
        centerY += point.Y;
    }
    centerX /= points.size();
    centerY /= points.size();
    
    // Rotate around polygon center
    rotateMatrix.RotateAt(degree, Gdiplus::PointF(centerX, centerY));
    
    // Apply the rotation transform to the element
    polygon->applyTransform(rotateMatrix);
    
    std::cout << "Applied matrix rotation " << degree << " degrees to polygon around center (" 
              << centerX << ", " << centerY << ")" << std::endl;
}

void SVGRotate::visit(SVGPolyline *polyline)
{
    // Apply rotation using matrix transform
    Gdiplus::Matrix rotateMatrix;
    
    // Calculate polyline centroid for rotation
    std::vector<PointF> points = polyline->getPoints();
    if (points.empty()) return;
    
    float centerX = 0, centerY = 0;
    for (const auto& point : points) {
        centerX += point.X;
        centerY += point.Y;
    }
    centerX /= points.size();
    centerY /= points.size();
    
    // Rotate around polyline center
    rotateMatrix.RotateAt(degree, Gdiplus::PointF(centerX, centerY));
    
    // Apply the rotation transform to the element
    polyline->applyTransform(rotateMatrix);
    
    std::cout << "Applied matrix rotation " << degree << " degrees to polyline around center (" 
              << centerX << ", " << centerY << ")" << std::endl;
}

void SVGRotate::visit(SVGPath *path)
{
    // For paths, we can apply transformation matrix
    // Since path data is string-based, we'll use the transform matrix approach
    Gdiplus::Matrix rotateMatrix;
    rotateMatrix.Rotate(degree);
    path->applyTransform(rotateMatrix);
}

void SVGRotate::visit(SVGGroup *group)
{   
    Gdiplus::Matrix rotateMatrix;
    rotateMatrix.Rotate(degree);
    group->applyTransform(rotateMatrix);
		std::cout << "Applied matrix rotation " << degree << " degrees to group" << std::endl;
    // Apply rotation to all children in the group
    for (auto child : group->getChildren()) {
        child->accept(this);
    }
}
