#include "stdafx.h"
#include "SVGPolyline.h"

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

void SVGPolyline::draw(Graphics* graphics) const {
    if (points.size() < 2) return;

    // Lưu trạng thái gốc của Graphics để khôi phục sau khi transform
    GraphicsState state = graphics->Save();

    // Áp dụng transform nếu có
    graphics->MultiplyTransform(&getTransform());

    // Chuyển vector<PointF> thành mảng để GDI+ dùng
    int count = static_cast<int>(points.size());
    std::unique_ptr<PointF[]> arr(new PointF[count]);
    for (int i = 0; i < count; ++i) {
        arr[i] = points[i];
    }

    // 1. Fill (nếu có fillOpacity > 0)
    if (style.fillOpacity > 0.0f) {
        // Tạo màu với alpha
        BYTE alpha = static_cast<BYTE>(style.fillColor.GetA() * style.fillOpacity);
        Color fillColor(alpha,
            style.fillColor.GetR(),
            style.fillColor.GetG(),
            style.fillColor.GetB());
        SolidBrush brush(fillColor);

        // FillPolygon sẽ tự đóng kín mảng điểm
        graphics->FillPolygon(&brush, arr.get(), count);
    }

    // 2. Stroke (nếu cần)
    if (style.strokeWidth > 0.0f && style.strokeOpacity > 0.0f) {
        BYTE alpha = static_cast<BYTE>(style.strokeColor.GetA() * style.strokeOpacity);
        Color strokeColor(alpha,
            style.strokeColor.GetR(),
            style.strokeColor.GetG(),
            style.strokeColor.GetB());
        Pen pen(strokeColor, style.strokeWidth);

        // Vẽ đường nối các điểm theo thứ tự (mở)
        graphics->DrawLines(&pen, arr.get(), count);
    }

    // Khôi phục lại trạng thái ban đầu để các element khác không bị ảnh hưởng
    graphics->Restore(state);
}

