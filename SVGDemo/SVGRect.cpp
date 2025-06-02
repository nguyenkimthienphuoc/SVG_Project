#include "SVGRect.h"

// Constructor: lưu lại thông tin về vị trí, kích thước, màu sắc và đường viền
SVGRect::SVGRect(float x, float y, float w, float h,
    Gdiplus::Color fill,
    Gdiplus::Color stroke,
    float strokeW)
    : x(x), y(y), width(w), height(h),
    fillColor(fill), strokeColor(stroke), strokeWidth(strokeW) {
    // Dùng member initializer list để gán giá trị trực tiếp
}

// Hàm draw: vẽ hình chữ nhật bằng GDI+
void SVGRect::draw(Gdiplus::Graphics* graphics) {
    // 1. Tạo bút vẽ viền (Pen) với màu strokeColor và độ dày strokeWidth
    Gdiplus::Pen pen(strokeColor, strokeWidth);

    // 2. Tạo cọ (Brush) để tô màu nền với fillColor
    Gdiplus::SolidBrush brush(fillColor);

    // 3. Tạo một hình chữ nhật theo GDI+ (tọa độ và kích thước dùng kiểu REAL)
    Gdiplus::RectF rect(
        static_cast<Gdiplus::REAL>(x),
        static_cast<Gdiplus::REAL>(y),
        static_cast<Gdiplus::REAL>(width),
        static_cast<Gdiplus::REAL>(height)
    );

    // 4. Vẽ phần màu nền (fill)
    graphics->FillRectangle(&brush, rect);

    // 5. Vẽ phần viền (stroke)
    graphics->DrawRectangle(&pen, rect);
}