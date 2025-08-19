#pragma once
#include "SVGElement.h"

class SVGEllipse : public SVGElement {
private:
    float cx, cy;   // Tâm ellipse
    float rx, ry;   // Bán kính trục x và y

public:
    SVGEllipse(float cx, float cy, float rx, float ry, const PaintStyle& style);

    void setCenter(float cx, float cy);
    void setRadius(float rx, float ry);

    float getCx() const;
    float getCy() const;
    float getRx() const;
    float getRy() const;

    void draw(Graphics* graphics) const override;
    void accept(SVGVisitor* visitor) override;

    Gdiplus::RectF localBounds() const override;
};
