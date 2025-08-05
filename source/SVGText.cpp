#include "stdafx.h"
#include "SVGText.h"
#include <iostream>

SVGText::SVGText(const std::wstring& textContent, Gdiplus::PointF startPoint, const TextPaintStyle& style, float fontSize)
    : textContent(textContent), startPoint(startPoint),
    fontSize(fontSize), style(style) {
}

void SVGText::setPosition(float x, float y) {
    this->startPoint.X = x;
    this->startPoint.Y = y;
}

void SVGText::setText(const std::wstring& content) {
    textContent = content;
}

void SVGText::setFontSize(float size) {
    fontSize = size;
}
std::wstring SVGText::getText() const {
    return textContent;
}

void SVGText::draw(Gdiplus::Graphics* graphics) const {
    if (!graphics) return;

    // Lưu trạng thái gốc của Graphics để khôi phục sau khi transform
    GraphicsState state = graphics->Save();

    // Áp dụng transform nếu có
    graphics->MultiplyTransform(&getTransform());

    Gdiplus::SolidBrush brush(style.fillColor);

    Gdiplus::FontFamily fontFamily(L"Arial");
    Gdiplus::Font font(&fontFamily, fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

    Gdiplus::StringFormat stringFormat;
    stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);

    // Lưu ý về hệ trục tọa độ:
    // Trong SVG: (x, y) là vị trí baseline (dưới cùng) của text.
    // Trong GDI+: (x, y) là top-left corner của text bounding box.
    // Vì vậy, nếu ta dùng trực tiếp (x, y), text sẽ bị thấp hơn dự kiến (do chiều dương hướng xuống).
    // Giải pháp: tính chiều cao dòng chữ, rồi dịch text lên (tức là trừ bớt y) một đoạn tương ứng.

    Gdiplus::RectF layoutRect;

    // Tính chiều cao văn bản
    graphics->MeasureString(
        textContent.c_str(),
        -1,
        &font,
        Gdiplus::PointF(startPoint.X, startPoint.Y),
        &stringFormat,
        &layoutRect
    );

    // Dịch text lên (trừ bớt y) theo layoutRect.Height
    float adjustedY = startPoint.Y - layoutRect.Height;

    graphics->DrawString(
        textContent.c_str(),
        -1,
        &font,
        Gdiplus::PointF(startPoint.X, adjustedY),
        &stringFormat,
        &brush
    );

    // Khôi phục lại trạng thái ban đầu để các element khác không bị ảnh hưởng
    graphics->Restore(state);
}

void SVGText::accept(SVGVisitor* visitor) {
    // Implementation for visitor pattern
    if (visitor) {
        visitor->visit(this);
    }
}
