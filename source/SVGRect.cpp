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
   // 1. Tạo màu với alpha theo fillOpacity
   Color fillColor(
       static_cast<BYTE>(style.fillOpacity * 255),
       style.fillColor.GetRed(),
       style.fillColor.GetGreen(),
       style.fillColor.GetBlue()
   );

   // 2. Tạo màu viền (stroke) với alpha theo strokeOpacity
   Color strokeColor(
       static_cast<BYTE>(style.strokeOpacity * 255),
       style.strokeColor.GetRed(),
       style.strokeColor.GetGreen(),
       style.strokeColor.GetBlue()
   );

   // 3. Tạo Pen và Brush từ màu đã có alpha
   Pen pen(strokeColor, style.strokeWidth);
   SolidBrush brush(fillColor);

   // 4. Vẽ hình chữ nhật
   RectF rect(
       static_cast<Gdiplus::REAL>(topLeft.X),
       static_cast<Gdiplus::REAL>(topLeft.Y),
       static_cast<Gdiplus::REAL>(width),
       static_cast<Gdiplus::REAL>(height)
   );

   // 5. Tô nền và viền
   graphics->FillRectangle(&brush, rect);
   graphics->DrawRectangle(&pen, rect);
}
