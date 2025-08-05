#include "stdafx.h"
#include "SVGScale.h"

// Scale the circle's radius by the given times
void SVGScaleByTimes::visit(SVGCircle *circle)
{
    float newRadius = circle->getRadius() * times;
    circle->setRadius(newRadius);
}

// Scale the rectangle's width and height by the given times
void SVGScaleByTimes::visit(SVGRect *rectangle)
{
    float newWidth = rectangle->getWidth() * times;
    float newHeight = rectangle->getHeight() * times;
    rectangle->setWidth(newWidth);
    rectangle->setHeight(newHeight);
}

// Scale the line's start and end points by the given times
void SVGScaleByTimes::visit(SVGLine *line)
{
    float newX1 = line->getter_x1() * times;
    float newY1 = line->getter_y1() * times;
    float newX2 = line->getter_x2() * times;
    float newY2 = line->getter_y2() * times;
    line->setStart(newX1, newY1);
    line->setEnd(newX2, newY2);
}

// Scale the text's starting point and font size by the given times
void SVGScaleByTimes::visit(SVGText *text)
{
    Gdiplus::PointF startPoint = text->getStartPoint();
    float newX = startPoint.X * times;
    float newY = startPoint.Y * times;
    text->setPosition(newX, newY);
    text->setFontSize(text->getSize() * times);
}

// Scale the ellipse's radii by the given times
void SVGScaleByTimes::visit(SVGEllipse *ellipse)
{
    float newRx = ellipse->getRx() * times;
    float newRy = ellipse->getRy() * times;
    ellipse->setRadius(newRx, newRy);
}

// Scale each point of the polygon by the given times
void SVGScaleByTimes::visit(SVGPolygon *polygon)
{
    std::vector<PointF> points = polygon->getPoints();
    for (auto &point : points)
    {
        point.X *= times;
        point.Y *= times;
    }
    polygon->setPoints(points);
}

// Scale each point of the polyline by the given times
void SVGScaleByTimes::visit(SVGPolyline *polyline)
{
    std::vector<PointF> points = polyline->getPoints();
    for (auto &point : points)
    {
        point.X *= times;
        point.Y *= times;
    }
    polyline->setPoints(points);
}

// Scale the circle's radius by the x factor
void SVGScaleByXY::visit(SVGCircle *circle)
{
    float newRadius = circle->getRadius() * x; // Assuming uniform scaling for simplicity
    circle->setRadius(newRadius);
}

// Scale the line's start and end points by the x and y factors
void SVGScaleByXY::visit(SVGRect *rectangle)
{
    float newWidth = rectangle->getWidth() * x;
    float newHeight = rectangle->getHeight() * y;
    rectangle->setWidth(newWidth);
    rectangle->setHeight(newHeight);
}

// Scale the line's start and end points by the x and y factors
void SVGScaleByXY::visit(SVGLine *line)
{
    float newX1 = line->getter_x1() * x;
    float newY1 = line->getter_y1() * y;
    float newX2 = line->getter_x2() * x;
    float newY2 = line->getter_y2() * y;
    line->setStart(newX1, newY1);
    line->setEnd(newX2, newY2);
}

// Scale the text's starting point and font size by the x and y factors
void SVGScaleByXY::visit(SVGText *text)
{
    Gdiplus::PointF startPoint = text->getStartPoint();
    float newX = startPoint.X * x;
    float newY = startPoint.Y * y;
    text->setPosition(newX, newY);
    text->setFontSize(text->getSize() * max(x, y)); // Scale font size by the larger factor
}

// Scale the ellipse's radii by the x and y factors
void SVGScaleByXY::visit(SVGEllipse *ellipse)
{
    float newRx = ellipse->getRx() * x;
    float newRy = ellipse->getRy() * y;
    ellipse->setRadius(newRx, newRy);
}

// Scale each point of the polygon by the x and y factors
void SVGScaleByXY::visit(SVGPolygon *polygon)
{
    std::vector<PointF> points = polygon->getPoints();
    for (auto &point : points)
    {
        point.X *= x;
        point.Y *= y;
    }
    polygon->setPoints(points);
}

// Scale each point of the polyline by the x and y factors
void SVGScaleByXY::visit(SVGPolyline *polyline)
{
    std::vector<PointF> points = polyline->getPoints();
    for (auto &point : points)
    {
        point.X *= x;
        point.Y *= y;
    }
    polyline->setPoints(points);
}
