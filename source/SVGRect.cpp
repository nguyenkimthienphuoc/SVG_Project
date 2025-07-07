#include "stdafx.h"
#include "SVGRect.h"

//Constructors
SVGRect::SVGRect(PointF topLeft, float width, float height)
    : topLeft(topLeft), width(width), height(height) {}

// Hàm draw: vẽ hình chữ nhật bằng GDI+
void SVGRect::draw(Graphics* graphics) const {
    // 1. Tạo bút vẽ viền (Pen) với màu strokeColor và độ dày strokeWidth
    Pen pen(style.strokeColor, style.strokeWidth);

    // 2. Tạo cọ (Brush) để tô màu nền với fillColor
    SolidBrush brush(style.fillColor);

    // 3. Tạo một hình chữ nhật theo GDI+ (tọa độ và kích thước dùng kiểu REAL)
    RectF rect(
        static_cast<Gdiplus::REAL>(topLeft.X),
        static_cast<Gdiplus::REAL>(topLeft.Y),
        static_cast<Gdiplus::REAL>(width),
        static_cast<Gdiplus::REAL>(height)
    );

    // 4. Vẽ phần màu nền (fill)
    graphics->FillRectangle(&brush, rect);

    // 5. Vẽ phần viền (stroke)
    graphics->DrawRectangle(&pen, rect);
}