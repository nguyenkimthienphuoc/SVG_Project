#include "stdafx.h"
#include "SVGPolyline.h"

SVGPolyline::SVGPolyline(const std::vector<Gdiplus::PointF>& pts, const PaintStyle& s) {
	points = pts;
	this->style = s;
}

void SVGPolyline::addPoint(const Gdiplus::PointF& point) {
   points.push_back(point);
}

void SVGPolyline::draw(Graphics* graphics) const {
    if (points.size() < 2) return;

    // Chuyển vector sang mảng
    int count = static_cast<int>(points.size());
    std::unique_ptr<PointF[]> arr(new PointF[count]);
    for (int i = 0; i < count; ++i) arr[i] = points[i];

    // Fill nếu có opacity
    if (style.fillOpacity > 0.0f) {
        BYTE fillA = static_cast<BYTE>(style.fillColor.GetA() * style.fillOpacity);
        Color fillC(style.fillColor.GetR(), style.fillColor.GetG(), style.fillColor.GetB());
        fillC.SetValue((fillA << 24) | (fillC.GetR() << 16) | (fillC.GetG() << 8) | fillC.GetB());
        SolidBrush brush(fillC);
        // Sử dụng FillPolygon để fill vùng giữa các đoạn thẳng
        graphics->FillPolygon(&brush, arr.get(), count);
    }

    // Stroke nếu cần
    if (style.strokeWidth > 0.0f && style.strokeOpacity > 0.0f) {
        BYTE strokeA = static_cast<BYTE>(style.strokeColor.GetA() * style.strokeOpacity);
        Color strokeC(style.strokeColor.GetR(), style.strokeColor.GetG(), style.strokeColor.GetB());
        strokeC.SetValue((strokeA << 24) | (strokeC.GetR() << 16) | (strokeC.GetG() << 8) | strokeC.GetB());
        Pen pen(strokeC, style.strokeWidth);
        graphics->DrawLines(&pen, arr.get(), count);
    }
}
