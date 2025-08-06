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

    Gdiplus::FontFamily fontFamily(L"Arial");
    Gdiplus::Font font(&fontFamily, fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

    Gdiplus::StringFormat stringFormat;
    stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);

    // Tính chiều cao văn bản
    Gdiplus::RectF layoutRect;
    graphics->MeasureString(
        textContent.c_str(),
        -1,
        &font,
        Gdiplus::PointF(startPoint.X, startPoint.Y),
        &stringFormat,
        &layoutRect
    );

    // Apply dx and dy offsets
    float finalX = startPoint.X + style.dx;
    float finalY = startPoint.Y + style.dy;

    // Dịch text lên (trừ bớt y) theo layoutRect.Height
    float adjustedY = finalY - layoutRect.Height;

    // Apply text-anchor alignment
    if (style.textAnchor == "middle") {
        finalX -= layoutRect.Width / 2;
    } else if (style.textAnchor == "end") {
        finalX -= layoutRect.Width;
    }

    // Draw fill if not transparent
    if (style.fillColor.GetA() > 0) {
        Gdiplus::SolidBrush brush(style.fillColor);
        graphics->DrawString(
            textContent.c_str(),
            -1,
            &font,
            Gdiplus::PointF(finalX, adjustedY),
            &stringFormat,
            &brush
        );
    }

    // Draw stroke if not transparent and stroke width > 0
    if (style.strokeColor.GetA() > 0 && style.strokeWidth > 0) {
        Gdiplus::Pen pen(style.strokeColor, style.strokeWidth);
        // Note: GDI+ doesn't have direct text stroke, we would need to use GraphicsPath
        // For simplicity, we'll skip stroke on text for now unless specifically needed
    }

    // Khôi phục lại trạng thái ban đầu để các element khác không bị ảnh hưởng
    graphics->Restore(state);
}

void SVGText::accept(SVGVisitor* visitor) {
    // Implementation for visitor pattern
    if (visitor) {
        visitor->visit(this);
    }
}
