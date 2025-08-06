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

    // Apply dx, dy offsets to position
    float finalX = startPoint.X + style.dx;
    float finalY = startPoint.Y + style.dy;

    Gdiplus::SolidBrush brush(style.fillColor);

    // Font setup with style support
    // Convert string to wstring for font family
    std::wstring fontFamilyName(style.fontFamily.begin(), style.fontFamily.end());
    Gdiplus::FontFamily fontFamily(fontFamilyName.c_str());
    
    int fontStyle = Gdiplus::FontStyleRegular;
    if (style.fontStyle == "italic") {
        fontStyle |= Gdiplus::FontStyleItalic;
    }
    if (style.fontWeight == "bold") {
        fontStyle |= Gdiplus::FontStyleBold;
    }
    
    Gdiplus::Font font(&fontFamily, fontSize, fontStyle, Gdiplus::UnitPixel);

    // Text anchor alignment
    Gdiplus::StringFormat stringFormat;
    if (style.textAnchor == "middle") {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
    } else if (style.textAnchor == "end") {
        stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
    } else { // "start" or default
        stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
    }

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
        Gdiplus::PointF(finalX, finalY),
        &stringFormat,
        &layoutRect
    );

    // Dịch text lên (trừ bớt y) theo layoutRect.Height
    float adjustedY = finalY - layoutRect.Height;

    graphics->DrawString(
        textContent.c_str(),
        -1,
        &font,
        Gdiplus::PointF(finalX, adjustedY),
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
