#include "stdafx.h"
#include "SVGPolyline.h"
#include <algorithm>
// Function helps double dispatch
void SVGPolyline::accept(SVGVisitor* visitor){
    visitor->visit(this);
}

SVGPolyline::SVGPolyline(const std::vector<Gdiplus::PointF>& pts, const PaintStyle& s) {
	points = pts;
	this->style = s;
}

void SVGPolyline::addPoint(const Gdiplus::PointF& point) {
   points.push_back(point);
}

Gdiplus::RectF SVGPolyline::localBounds() const {
    if (points.empty()) return Gdiplus::RectF(0, 0, 0, 0);

    float minx = points[0].X, maxx = points[0].X;
    float miny = points[0].Y, maxy = points[0].Y;
    for (const auto& p : points) {
        minx = (std::min)(minx, p.X);
        maxx = (std::max)(maxx, p.X);
        miny = (std::min)(miny, p.Y);
        maxy = (std::max)(maxy, p.Y);
    }
    return Gdiplus::RectF(minx, miny, maxx - minx, maxy - miny);
}

// Vẽ polyline: Fill (nếu có) như Polygon, và Stroke như bình thường
void SVGPolyline::draw(Gdiplus::Graphics* g) const {
    if (!g || points.size() < 2) return;

    auto state = g->Save();
    g->MultiplyTransform(&getTransform());

    const Gdiplus::RectF bb = localBounds();

    const int count = static_cast<int>(points.size());
    std::unique_ptr<Gdiplus::PointF[]> arr(new Gdiplus::PointF[count]);
    for (int i = 0; i < count; ++i) arr[i] = points[i];

    // Fill như Polygon (chỉ khi đủ >=3 điểm mới fill được)
    if (!style.fillNone && style.fillOpacity > 0.0f && count >= 3) {
        if (gradientRegistry && !style.fillUrlId.empty()) {
            auto brush = gradientRegistry->makeBrush(style.fillUrlId, bb);
            if (brush) g->FillPolygon(brush.get(), arr.get(), count);
        }
        else {
            BYTE a = static_cast<BYTE>(style.fillColor.GetA() * style.fillOpacity);
            Gdiplus::Color c(a,
                style.fillColor.GetR(),
                style.fillColor.GetG(),
                style.fillColor.GetB());
            Gdiplus::SolidBrush br(c);
            g->FillPolygon(&br, arr.get(), count);
        }
    }

    // Stroke polyline
    if (!style.strokeNone && style.strokeWidth > 0.0f && style.strokeOpacity > 0.0f) {
        BYTE a = static_cast<BYTE>(style.strokeColor.GetA() * style.strokeOpacity);
        Gdiplus::Color c(a,
            style.strokeColor.GetR(),
            style.strokeColor.GetG(),
            style.strokeColor.GetB());
        Gdiplus::Pen pen(c, style.strokeWidth);
        g->DrawLines(&pen, arr.get(), count);
    }

    g->Restore(state);
}

