#include "stdafx.h"
#include "SVGEllipse.h"

using namespace Gdiplus;

SVGEllipse::SVGEllipse(float cx, float cy, float rx, float ry, const PaintStyle& style)
    : cx(cx), cy(cy), rx(rx), ry(ry) {
    this->style = style;
}

void SVGEllipse::setCenter(float cx, float cy) {
    this->cx = cx;
    this->cy = cy;
}

void SVGEllipse::setRadius(float rx, float ry) {
    this->rx = rx;
    this->ry = ry;
}

float SVGEllipse::getCx() const {
    return cx;
}

float SVGEllipse::getCy() const {
    return cy;
}

float SVGEllipse::getRx() const {
    return rx;
}

float SVGEllipse::getRy() const {
    return ry;
}

void SVGEllipse::draw(Graphics* graphics) const {
    Color fill(style.fillColor, style.fillColor, style.fillColor, style.fillColor);
    Color stroke(style.strokeColor, style.strokeColor, style.strokeColor, style.strokeColor);

    SolidBrush brush(fill);
    Pen pen(stroke, style.strokeWidth);

    float left = cx - rx;
    float top = cy - ry;
    float width = 2 * rx;
    float height = 2 * ry;

    graphics->FillEllipse(&brush, left, top, width, height);
    graphics->DrawEllipse(&pen, left, top, width, height);
}
