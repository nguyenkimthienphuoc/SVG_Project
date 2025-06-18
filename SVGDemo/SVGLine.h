#ifndef SVGLINE_H
#define SVGLINE_H
#include "SVGElement.h"

class SVGLine : public SVGElement
{
private:
    float x1, x2, y1, y2;

public:
    SVGLine(float x1, float x2, float y1, float y2, const PaintStyle &s);
    float getter_x1() const { return x1; };
    float getter_x2() const { return x2; };
    float getter_y1() const { return y1; };
    float getter_y2() const { return y2; };
    void setStart(float x, float y) { x1 = x; y1 = y; }
    void setEnd(float x, float y) { x2 = x; y2 = y; }
};

#endif