#include "stdafx.h"
#include "SVGPolygon.h"

SVGPolygon::SVGPolygon(const std::vector<PointF>& pts, const PaintStyle& style) {
	points = pts;
	this->style = style;
}
void SVGPolygon::addPoint(const PointF& point) {
	points.push_back(point);
}

void SVGPolygon::draw(Graphics* graphics) const {
    if (points.empty()) return;

    // Prepare fill brush
    BYTE fillAlpha = static_cast<BYTE>(style.fillColor.GetA() * style.fillOpacity);
    Color fillColor(style.fillColor.GetR(), style.fillColor.GetG(), style.fillColor.GetB());
    fillColor.SetValue((fillAlpha << 24) |
        (fillColor.GetR() << 16) |
        (fillColor.GetG() << 8) |
        (fillColor.GetB()));
    SolidBrush fillBrush(fillColor);

    // Convert vector<PointF> to array
    int count = static_cast<int>(points.size());
    std::unique_ptr<PointF[]> arr(new PointF[count]);
    for (int i = 0; i < count; ++i) {
        arr[i] = points[i];
    }

    // Fill polygon
    if (style.fillOpacity > 0)
        graphics->FillPolygon(&fillBrush, arr.get(), count);

    // Stroke polygon (only if stroke is visible)
    if (style.strokeWidth > 0 && style.strokeOpacity > 0) {
        BYTE strokeAlpha = static_cast<BYTE>(style.strokeColor.GetA() * style.strokeOpacity);
        Color strokeColor(style.strokeColor.GetR(), style.strokeColor.GetG(), style.strokeColor.GetB());
        strokeColor.SetValue((strokeAlpha << 24) |
            (strokeColor.GetR() << 16) |
            (strokeColor.GetG() << 8) |
            (strokeColor.GetB()));
        Pen strokePen(strokeColor, style.strokeWidth);
        graphics->DrawPolygon(&strokePen, arr.get(), count);
    }
}
