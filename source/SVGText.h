#pragma once
#include "SVGElement.h"
#include <string>

class SVGText : public SVGElement {
private:
    std::wstring textContent;   // Text content
    Gdiplus::PointF startPoint; // Start Point
    float fontSize;             // size
    TextPaintStyle style;       // style
public:
    SVGText(const std::wstring& textContent, Gdiplus::PointF startPoint, const TextPaintStyle& style, float fontSize);

    void setPosition(float x, float y);
    void setText(const std::wstring& content);
    void setFontSize(float size);

    std::wstring getText() const;

    void draw(Graphics* graphics) const override;
};