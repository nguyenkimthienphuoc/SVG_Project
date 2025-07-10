#include "stdafx.h"
#include "SVGCircle.h"

//Constructors
SVGCircle::SVGCircle(Gdiplus::PointF center, float radius, const PaintStyle& s)
    : center(center), radius(radius) {
    style = s;
}

void SVGCircle::draw(Gdiplus::Graphics* graphics) const {
    if (!graphics) return;

    // Fill with solid red
    Gdiplus::SolidBrush fillBrush(Gdiplus::Color(255, 255, 0, 0)); // opaque red
    graphics->FillEllipse(&fillBrush,
        center.X - radius, center.Y - radius,
        radius * 2, radius * 2);

    // Draw black outline
    Gdiplus::Pen outlinePen(Gdiplus::Color(255, 0, 0, 0), 1.0f);
    graphics->DrawEllipse(&outlinePen,
        center.X - radius, center.Y - radius,
        radius * 2, radius * 2);
}
