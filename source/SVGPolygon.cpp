#include "stdafx.h"
#include "SVGPolygon.h"

// Function helps double dispatch
void SVGPolygon::accept(SVGVisitor* visitor){
    visitor->visit(this);
}

SVGPolygon::SVGPolygon(const std::vector<PointF>& pts, const PaintStyle& style) {
	points = pts;
	this->style = style;
}
void SVGPolygon::addPoint(const PointF& point) {
	points.push_back(point);
}

void SVGPolygon::draw(Graphics* graphics) const {
    if (points.empty()) return;

    // Convert vector<PointF> to array
    int count = static_cast<int>(points.size());
    std::unique_ptr<PointF[]> arr(new PointF[count]);
    for (int i = 0; i < count; ++i) {
        arr[i] = points[i];
    }

    // Fill polygon
    if (style.fillOpacity > 0.0f) {
        BYTE fillAlpha = static_cast<BYTE>(style.fillColor.GetA() * style.fillOpacity);
        Color fillColor(fillAlpha,
            style.fillColor.GetR(),
            style.fillColor.GetG(),
            style.fillColor.GetB());
        SolidBrush fillBrush(fillColor);
        graphics->FillPolygon(&fillBrush, arr.get(), count);
    }

    // Stroke polygon (only if stroke is visible)
    if (style.strokeWidth > 0.0f && style.strokeOpacity > 0.0f) {
        BYTE strokeAlpha = static_cast<BYTE>(style.strokeColor.GetA() * style.strokeOpacity);
        Color strokeColor(strokeAlpha,
            style.strokeColor.GetR(),
            style.strokeColor.GetG(),
            style.strokeColor.GetB());
        Pen strokePen(strokeColor, style.strokeWidth);
        graphics->DrawPolygon(&strokePen, arr.get(), count);
    }
}
