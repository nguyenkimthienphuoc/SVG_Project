#include "stdafx.h"
#include "SVGRect.h"

//Constructors
SVGRect::SVGRect(PointF topLeft, float width, float height, const PaintStyle &s)
   : topLeft(topLeft), width(width), height(height)
{
   style = s;
}

// Hàm draw: vẽ hình chữ nhật bằng GDI+
void SVGRect::draw(Graphics* graphics) const {
    // 1. Vùng hình chữ nhật
    RectF rect(
        static_cast<Gdiplus::REAL>(topLeft.X),
        static_cast<Gdiplus::REAL>(topLeft.Y),
        static_cast<Gdiplus::REAL>(width),
        static_cast<Gdiplus::REAL>(height)
    );

    // 2. Fill nếu cần
    if (style.fillOpacity > 0.0f) {
        Color fillColor(
            static_cast<BYTE>(style.fillOpacity * 255),
            style.fillColor.GetRed(),
            style.fillColor.GetGreen(),
            style.fillColor.GetBlue()
        );
        SolidBrush brush(fillColor);
        graphics->FillRectangle(&brush, rect);
    }

    // 3. Stroke nếu cần
    if (style.strokeWidth > 0.0f && style.strokeOpacity > 0.0f) {
        Color strokeColor(
            static_cast<BYTE>(style.strokeOpacity * 255),
            style.strokeColor.GetRed(),
            style.strokeColor.GetGreen(),
            style.strokeColor.GetBlue()
        );
        Pen pen(strokeColor, style.strokeWidth);
        graphics->DrawRectangle(&pen, rect);
    }
}

