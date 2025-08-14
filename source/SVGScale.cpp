#include "stdafx.h"
#include "SVGScale.h"
#include "SVGGroup.h"

// SVGScaleByTimes implementations

// Scale the circle's radius by the given times
void SVGScaleByTimes::visit(SVGCircle *circle)
{
    /* float newRadius = circle->getRadius() * times;
    circle->setRadius(newRadius); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(times, times);
    circle->applyTransform(scaleMatrix);
}

// Scale the rectangle's width and height by the given times
void SVGScaleByTimes::visit(SVGRect *rectangle)
{
    /* float newWidth = rectangle->getWidth() * times;
    float newHeight = rectangle->getHeight() * times;
    rectangle->setWidth(newWidth);
    rectangle->setHeight(newHeight); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(times, times);
    rectangle->applyTransform(scaleMatrix);
}

// Scale the line's start and end points by the given times
void SVGScaleByTimes::visit(SVGLine *line)
{
    /* float newX1 = line->getter_x1() * times;
    float newY1 = line->getter_y1() * times;
    float newX2 = line->getter_x2() * times;
    float newY2 = line->getter_y2() * times;
    line->setStart(newX1, newY1);
    line->setEnd(newX2, newY2); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(times, times);
    line->applyTransform(scaleMatrix);
}

// Scale the text's starting point and font size by the given times
void SVGScaleByTimes::visit(SVGText *text)
{
    /* Gdiplus::PointF startPoint = text->getStartPoint();
    float newX = startPoint.X * times;
    float newY = startPoint.Y * times;
    text->setPosition(newX, newY);
    text->setFontSize(text->getSize() * times); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(times, times);
    text->applyTransform(scaleMatrix);
}

// Scale the ellipse's radii by the given times
void SVGScaleByTimes::visit(SVGEllipse *ellipse)
{
    /* float newRx = ellipse->getRx() * times;
    float newRy = ellipse->getRy() * times;
    ellipse->setRadius(newRx, newRy); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(times, times);
    ellipse->applyTransform(scaleMatrix);
}

// Scale each point of the polygon by the given times
void SVGScaleByTimes::visit(SVGPolygon *polygon)
{
    /* std::vector<PointF> points = polygon->getPoints();
    for (auto &point : points)
    {
        point.X *= times;
        point.Y *= times;
    }
    polygon->setPoints(points); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(times, times);
    polygon->applyTransform(scaleMatrix);
}

// Scale each point of the polyline by the given times
void SVGScaleByTimes::visit(SVGPolyline *polyline)
{
    /* std::vector<PointF> points = polyline->getPoints();
    for (auto &point : points)
    {
        point.X *= times;
        point.Y *= times;
    }
    polyline->setPoints(points); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(times, times);
    polyline->applyTransform(scaleMatrix);
}

void SVGScaleByTimes::visit(SVGPath *path)
{
    // For paths, use transformation matrix
    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(times, times);
    path->applyTransform(scaleMatrix);
}

void SVGScaleByTimes::visit(SVGGroup *group)
{
    // Apply scaling to all children in the group
    for (auto child : group->getChildren()) {
        child->accept(this);
    }
}

// SVGScaleByXY implementations

// Scale the circle's radius by the x factor (assuming uniform scaling for simplicity)
void SVGScaleByXY::visit(SVGCircle *circle)
{
   /*  float newRadius = circle->getRadius() * x; // Using x factor for radius
    circle->setRadius(newRadius); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(x, y);
    circle->applyTransform(scaleMatrix);
}

// Scale the rectangle's width and height by the x and y factors
void SVGScaleByXY::visit(SVGRect *rectangle)
{
    /* float newWidth = rectangle->getWidth() * x;
    float newHeight = rectangle->getHeight() * y;
    rectangle->setWidth(newWidth);
    rectangle->setHeight(newHeight); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(x, y);
    rectangle->applyTransform(scaleMatrix);
}

// Scale the line's start and end points by the x and y factors
void SVGScaleByXY::visit(SVGLine *line)
{
    /* float newX1 = line->getter_x1() * x;
    float newY1 = line->getter_y1() * y;
    float newX2 = line->getter_x2() * x;
    float newY2 = line->getter_y2() * y;
    line->setStart(newX1, newY1);
    line->setEnd(newX2, newY2); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(x, y);
    line->applyTransform(scaleMatrix);
}

// Scale the text's starting point and font size by the x and y factors
void SVGScaleByXY::visit(SVGText *text)
{
    /* Gdiplus::PointF startPoint = text->getStartPoint();
    float newX = startPoint.X * x;
    float newY = startPoint.Y * y;
    text->setPosition(newX, newY);
    text->setFontSize(text->getSize() * max(x, y)); // Scale font size by the larger factor */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(x, y);
    text->applyTransform(scaleMatrix);
}

// Scale the ellipse's radii by the x and y factors
void SVGScaleByXY::visit(SVGEllipse *ellipse)
{
    /* float newRx = ellipse->getRx() * x;
    float newRy = ellipse->getRy() * y;
    ellipse->setRadius(newRx, newRy); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(x, y);
    ellipse->applyTransform(scaleMatrix);
}

// Scale each point of the polygon by the x and y factors
void SVGScaleByXY::visit(SVGPolygon *polygon)
{
    /* std::vector<PointF> points = polygon->getPoints();
    for (auto &point : points)
    {
        point.X *= x;
        point.Y *= y;
    }
    polygon->setPoints(points); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(x, y);
    polygon->applyTransform(scaleMatrix);
}

// Scale each point of the polyline by the x and y factors
void SVGScaleByXY::visit(SVGPolyline *polyline)
{
    /* std::vector<PointF> points = polyline->getPoints();
    for (auto &point : points)
    {
        point.X *= x;
        point.Y *= y;
    }
    polyline->setPoints(points); */

    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(x, y);
    polyline->applyTransform(scaleMatrix);
}

void SVGScaleByXY::visit(SVGPath *path)
{
    // For paths, use transformation matrix
    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(x, y);
    path->applyTransform(scaleMatrix);
}

void SVGScaleByXY::visit(SVGGroup *group)
{
    // Apply scaling to all children in the group
    for (auto child : group->getChildren()) {
        child->accept(this);
    }
}
