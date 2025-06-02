#pragma once
#include "SVGElement.h"
// Class mẫu
class SVGRect : public SVGElement { // Kế thừa từ class SVGElement
private:
    float x, y, width, height;
    Gdiplus::Color fillColor;
    Gdiplus::Color strokeColor;
    float strokeWidth;

public:
    SVGRect(float x, float y, float w, float h, Gdiplus::Color fill, Gdiplus::Color stroke, float strokeW);
    void draw(Gdiplus::Graphics* graphics) override;
};