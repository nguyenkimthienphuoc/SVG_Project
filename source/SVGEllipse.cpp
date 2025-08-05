#include "stdafx.h"
#include "SVGEllipse.h"

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

void SVGEllipse::draw(Gdiplus::Graphics* graphics) const {
    // Lưu trạng thái gốc của Graphics để khôi phục sau khi transform
    GraphicsState state = graphics->Save();

    // Áp dụng transform nếu có
    graphics->MultiplyTransform(&getTransform());

    float left = cx - rx;
    float top = cy - ry;
    float width = 2 * rx;
    float height = 2 * ry;

    // Fill ellipse
    if (style.fillOpacity > 0.0f) {
        BYTE fillAlpha = static_cast<BYTE>(style.fillColor.GetA() * style.fillOpacity);
        Color fillColor(fillAlpha,
            style.fillColor.GetR(),
            style.fillColor.GetG(),
            style.fillColor.GetB());
        SolidBrush fillBrush(fillColor);
        graphics->FillEllipse(&fillBrush, left, top, width, height);
    }

    // Stroke ellipse (only if stroke is visible)
    if (style.strokeWidth > 0.0f && style.strokeOpacity > 0.0f) {
        BYTE strokeAlpha = static_cast<BYTE>(style.strokeColor.GetA() * style.strokeOpacity);
        Color strokeColor(strokeAlpha,
            style.strokeColor.GetR(),
            style.strokeColor.GetG(),
            style.strokeColor.GetB());
        Pen strokePen(strokeColor, style.strokeWidth);
        graphics->DrawEllipse(&strokePen, left, top, width, height);
    }
    // Khôi phục lại trạng thái ban đầu để các element khác không bị ảnh hưởng
    graphics->Restore(state);
}
