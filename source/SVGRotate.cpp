#include "stdafx.h"
#include "SVGRotate.h"

void SVGRotate::visit(SVGCircle *circle)
{
    // Rotate the circle around its center
    PointF center = circle->getCenter();
    float radius = circle->getRadius();
    
    // Calculate new position after rotation
    float radian = degree * (M_PI / 180.0f);
    float newX = center.X * cos(radian) - center.Y * sin(radian);
    float newY = center.X * sin(radian) + center.Y * cos(radian);
    
    // Update the circle's position
    circle->setCenter(PointF(newX, newY));
}

void SVGRotate::visit(SVGRect *rectangle)
{
    // Rotate the rectangle around its top-left corner
    PointF topLeft = rectangle->topLeft;
    float width = rectangle->width;
    float height = rectangle->height;

    // Calculate new position after rotation
    float radian = degree * (M_PI / 180.0f);
    float newX = topLeft.X * cos(radian) - topLeft.Y * sin(radian);
    float newY = topLeft.X * sin(radian) + topLeft.Y * cos(radian);

    // Update the rectangle's position
    rectangle->topLeft = PointF(newX, newY);
}

void SVGRotate::visit(SVGLine *line)
{
    // Rotate the line's start and end points
    float x1 = line->getter_x1();
    float y1 = line->getter_y1();
    float x2 = line->getter_x2();
    float y2 = line->getter_y2();

    // Calculate new positions after rotation
    float radian = degree * (M_PI / 180.0f);
    float newX1 = x1 * cos(radian) - y1 * sin(radian);
    float newY1 = x1 * sin(radian) + y1 * cos(radian);
    float newX2 = x2 * cos(radian) - y2 * sin(radian);
    float newY2 = x2 * sin(radian) + y2 * cos(radian);

    // Update the line's start and end points
    line->setStart(newX1, newY1);
    line->setEnd(newX2, newY2);
}

void SVGRotate::visit(SVGText *text)
{
    // Rotate the text's starting point
    PointF startPoint = text->getStartPoint();
    float size = text->getSize();

    // Calculate new position after rotation
    float radian = degree * (M_PI / 180.0f);
    float newX = startPoint.X * cos(radian) - startPoint.Y * sin(radian);
    float newY = startPoint.X * sin(radian) + startPoint.Y * cos(radian);

    // Update the text's starting point
    text->setStart(PointF(newX, newY));
}

void SVGRotate::visit(SVGEllipse *ellipse)
{
    // Rotate the ellipse around its center
    float rx = ellipse->getRx();
    float ry = ellipse->getRy();
    float cx = ellipse->getCx();
    float cy = ellipse->getCy();


    // Calculate new position after rotation
    float radian = degree * (M_PI / 180.0f);
    float newX = cx * cos(radian) - cy * sin(radian);
    float newY = cx * sin(radian) + cy * cos(radian);

    // Update the ellipse's center
    ellipse->setCenter(newX, newY);
}

/* void SVGRotate::visit(SVGPath *path)
{   
}
 */

void SVGRotate::visit(SVGPolygon *polygon)
{
    // Rotate each point of the polygon
    std::vector<PointF> points = polygon->getPoints();
    float radian = degree * (M_PI / 180.0f);
    
    for (auto& point : points) {
        float newX = point.X * cos(radian) - point.Y * sin(radian);
        float newY = point.X * sin(radian) + point.Y * cos(radian);
        point = PointF(newX, newY);
    }
}

void SVGRotate::visit(SVGPolyline *polyline)
{
    // Rotate each point of the polyline
    std::vector<PointF> points = polyline->getPoints();
    float radian = degree * (M_PI / 180.0f);
    
    for (auto& point : points) {
        float newX = point.X * cos(radian) - point.Y * sin(radian);
        float newY = point.X * sin(radian) + point.Y * cos(radian);
        point = PointF(newX, newY);
    }
}
