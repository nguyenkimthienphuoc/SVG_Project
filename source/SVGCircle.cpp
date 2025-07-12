#include "stdafx.h"
#include "SVGCircle.h"

//Constructors
SVGCircle::SVGCircle(Gdiplus::PointF center, float radius, const PaintStyle& s)
    : center(center), radius(radius) {
    style = s;
}

// Draw the circle using GDI+
// This uses the style's fill color, stroke color, and stroke width,
// making it flexible and consistent with other SVG elements.
void SVGCircle::draw(Gdiplus::Graphics* graphics) const {
    if (!graphics) return; // Safety check to avoid null pointer dereference

    // Calculate the top-left corner and diameter of the bounding box for the circle
    float left = center.X - radius;
    float top = center.Y - radius;
    float diameter = radius * 2;

    // Fill circle
    if (style.fillOpacity > 0.0f) {
        BYTE fillAlpha = static_cast<BYTE>(style.fillColor.GetA() * style.fillOpacity);
        Color fillColor(fillAlpha,
            style.fillColor.GetR(),
            style.fillColor.GetG(),
            style.fillColor.GetB());
        SolidBrush fillBrush(fillColor);
        graphics->FillEllipse(&fillBrush, left, top, diameter, diameter);
    }

    // Stroke circle (only if stroke is visible)
    if (style.strokeWidth > 0.0f && style.strokeOpacity > 0.0f) {
        BYTE strokeAlpha = static_cast<BYTE>(style.strokeColor.GetA() * style.strokeOpacity);
        Color strokeColor(strokeAlpha,
            style.strokeColor.GetR(),
            style.strokeColor.GetG(),
            style.strokeColor.GetB());
        Pen strokePen(strokeColor, style.strokeWidth);
        graphics->DrawEllipse(&strokePen, left, top, diameter, diameter);
    }
}
