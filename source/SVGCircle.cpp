#include "stdafx.h"
#include "SVGCircle.h"

// Constructor
SVGCircle::SVGCircle(Gdiplus::PointF center, float radius)
    : center(center), radius(radius) {
}

// Draw the circle using GDI+
// This uses the style's fill color, stroke color, and stroke width,
// making it flexible and consistent with other SVG elements.
void SVGCircle::draw(Gdiplus::Graphics* graphics) const {
    if (!graphics) return; // Safety check to avoid null pointer dereference

    // Create a solid brush with the fill color specified in style
    Gdiplus::SolidBrush brush(style.fillColor);

    // Create a pen with the stroke color and stroke width specified in style
    Gdiplus::Pen pen(style.strokeColor, style.strokeWidth);

    // Calculate the top-left corner and diameter of the bounding box for the circle
    float left = center.X - radius;
    float top = center.Y - radius;
    float diameter = radius * 2;

    // Fill the circle with the specified fill color
    graphics->FillEllipse(&brush, left, top, diameter, diameter);

    // Draw the outline of the circle with the specified stroke color and width
    graphics->DrawEllipse(&pen, left, top, diameter, diameter);
}
