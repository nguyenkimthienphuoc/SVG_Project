#include "stdafx.h"
#include "SVGCircle.h"

// Function helps double dispatch
void SVGCircle::accept(SVGVisitor* visitor){
    visitor->visit(this);
}

//Constructors
SVGCircle::SVGCircle(Gdiplus::PointF center, float radius, const PaintStyle& s)
    : center(center), radius(radius) {
    style = s;
}



// Draw the circle using GDI+
// This uses the style's fill color, stroke color, and stroke width,
// making it flexible and consistent with other SVG elements.
Gdiplus::RectF SVGCircle::localBounds() const {
    float left = center.X - radius;
    float top = center.Y - radius;
    float d = radius * 2;
    return Gdiplus::RectF(left, top, d, d);
}

void SVGCircle::draw(Graphics* graphics) const {
    if (!graphics) return;
    auto state = graphics->Save();
    graphics->MultiplyTransform(&getTransform());

    RectF bb = localBounds();

    if (!style.fillNone && style.fillOpacity > 0.0f) {
        if (gradientRegistry && !style.fillUrlId.empty()) {
            auto brush = gradientRegistry->makeBrush(style.fillUrlId, bb);
            if (brush) graphics->FillEllipse(brush.get(), bb);
        }
        else {
            BYTE a = static_cast<BYTE>(style.fillColor.GetA() * style.fillOpacity);
            Color c(a, style.fillColor.GetR(), style.fillColor.GetG(), style.fillColor.GetB());
            SolidBrush br(c);
            graphics->FillEllipse(&br, bb);
        }
    }

    if (!style.strokeNone && style.strokeWidth > 0 && style.strokeOpacity > 0) {
        BYTE a = static_cast<BYTE>(style.strokeColor.GetA() * style.strokeOpacity);
        Color c(a, style.strokeColor.GetR(), style.strokeColor.GetG(), style.strokeColor.GetB());
        Pen pen(c, style.strokeWidth);
        graphics->DrawEllipse(&pen, bb);
    }
    graphics->Restore(state);
}