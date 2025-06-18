#include "stdafx.h"
#include "SVGText.h"

SVGText::SVGText(const std::wstring& content, float x, float y, const PaintStyle& style, float fontSize)
    : text(content), x(x), y(y), fontSize(fontSize) {
    this->style = style;
}

void SVGText::setPosition(float x, float y) {
    this->x = x;
    this->y = y;
}

void SVGText::setText(const std::wstring& content) {
    text = content;
}

void SVGText::setFontSize(float size) {
    fontSize = size;
}

std::wstring SVGText::getText() const {
    return text;
}

void SVGText::draw(Graphics* graphics) const {
    SolidBrush brush(style.fillColor);

    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, fontSize, FontStyleRegular, UnitPixel);

    graphics->DrawString(
        text.c_str(),
        -1,
        &font,
        PointF(x, y),
        &brush
    );
}
