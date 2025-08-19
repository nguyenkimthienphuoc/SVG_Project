#include "stdafx.h"
#include "SVGEllipse.h"

// function helps double dispatch
void SVGEllipse::accept(SVGVisitor* visitor){
    visitor->visit(this);
}

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

float SVGEllipse::getCx() const { return cx; }
float SVGEllipse::getCy() const { return cy; }
float SVGEllipse::getRx() const { return rx; }
float SVGEllipse::getRy() const { return ry; }

Gdiplus::RectF SVGEllipse::localBounds() const {
    return Gdiplus::RectF(cx - rx, cy - ry, 2 * rx, 2 * ry);
}

void SVGEllipse::draw(Gdiplus::Graphics* graphics) const {
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