#pragma once
#include "SVGElement.h"
#include <string>

class SVGText : public SVGElement {
private:
    std::wstring text;  // Text content
    float x, y;         // Start Point
    float fontSize;     // size

public:
    SVGText(const std::wstring& content, float x, float y, const PaintStyle& style, float fontSize);

    void setPosition(float x, float y);
    void setText(const std::wstring& content);
    void setFontSize(float size);

    std::wstring getText() const;

    void draw(Graphics* graphics) const override;
};