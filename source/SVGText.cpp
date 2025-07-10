#include "stdafx.h"
#include "SVGText.h"

SVGText::SVGText(const std::wstring& textContent, Gdiplus::PointF startPoint,
    const TextPaintStyle& style, float fontSize)
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
    Gdiplus::SolidBrush brush(style.fillColor);

    Gdiplus::FontFamily fontFamily(L"Arial");
    Gdiplus::Font font(&fontFamily, fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

    graphics->DrawString(
        textContent.c_str(),
        -1,
        &font,
        startPoint,
        &brush
    );
}
