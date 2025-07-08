#include "stdafx.h"
#include "SVGText.h"

SVGText::SVGText(const std::wstring& textContent, Gdiplus::PointF startPoint, const TextPaintStyle& style, float fontSize)
    : textContent(textContent), startPoint(startPoint), fontSize(fontSize), style(style){}

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

void SVGText::draw(Graphics* graphics) const {
    SolidBrush brush(style.fillColor);

    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, fontSize, FontStyleRegular, UnitPixel);

    graphics->DrawString(
        textContent.c_str(),
        -1,
        &font,
        startPoint,
        &brush
    );
}
