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

Gdiplus::RectF SVGPolygon::localBounds() const {
    if (points.empty()) return Gdiplus::RectF(0, 0, 0, 0);
    float minx = points[0].X, maxx = points[0].X, miny = points[0].Y, maxy = points[0].Y;
    for (auto& p : points) {
        minx = (std::min)(minx, p.X); maxx = (std::max)(maxx, p.X);
        miny = (std::min)(miny, p.Y); maxy = (std::max)(maxy, p.Y);
    }
    return Gdiplus::RectF(minx, miny, maxx - minx, maxy - miny);
}

void SVGPolygon::draw(Graphics* g) const {
    if (points.empty()) return;
    auto state = g->Save();
    g->MultiplyTransform(&getTransform());

    const RectF bb = localBounds();

    int count = static_cast<int>(points.size());
    std::unique_ptr<PointF[]> arr(new PointF[count]);
    for (int i = 0; i < count; ++i) arr[i] = points[i];

    if (!style.fillNone && style.fillOpacity > 0.0f) {
        if (gradientRegistry && !style.fillUrlId.empty()) {
            auto brush = gradientRegistry->makeBrush(style.fillUrlId, bb);
            if (brush) g->FillPolygon(brush.get(), arr.get(), count);
        }
        else {
            BYTE a = static_cast<BYTE>(style.fillColor.GetA() * style.fillOpacity);
            Color c(a, style.fillColor.GetR(), style.fillColor.GetG(), style.fillColor.GetB());
            SolidBrush br(c);
            g->FillPolygon(&br, arr.get(), count);
        }
    }

    if (!style.strokeNone && style.strokeWidth > 0 && style.strokeOpacity > 0) {
        BYTE a = static_cast<BYTE>(style.strokeColor.GetA() * style.strokeOpacity);
        Color c(a, style.strokeColor.GetR(), style.strokeColor.GetG(), style.strokeColor.GetB());
        Pen pen(c, style.strokeWidth);
        g->DrawPolygon(&pen, arr.get(), count);
    }
    g->Restore(state);
}
