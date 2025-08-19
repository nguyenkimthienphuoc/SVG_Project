#include "stdafx.h"
#include "SVGGradient.h"
#include <algorithm>

using namespace Gdiplus;

Color SVGGradientRegistry::applyOpacity(const Color& c, float mult) {
    BYTE a = static_cast<BYTE>(std::clamp<int>(int(c.GetA() * mult), 0, 255));
    return Color(a, c.GetR(), c.GetG(), c.GetB());
}

void SVGGradientRegistry::sortStops(std::vector<SVGGradientStop>& stops) {
    std::sort(stops.begin(), stops.end(),
        [](const SVGGradientStop& a, const SVGGradientStop& b) { return a.offset < b.offset; });
}

void SVGGradientRegistry::resolveInheritanceLinear(SVGLinearGradientDef& g) const {
    // GIỮ LẠI để phù hợp header, nhưng ta KHÔNG GỌI hàm này trong makeBrush
    if (g.href.empty()) return;
    auto it = linear.find(g.href);
    if (it != linear.end()) {
        const auto& base = it->second;
        // chỉ kế thừa STOPs nếu local trống (KHÔNG copy Matrix để tránh copy non-copyable)
        if (g.stops.empty())
            const_cast<std::vector<SVGGradientStop>&>(g.stops) = base.stops;
    }
    else {
        auto itR = radial.find(g.href);
        if (itR != radial.end() && g.stops.empty()) {
            const_cast<std::vector<SVGGradientStop>&>(g.stops) = itR->second.stops;
        }
    }
}

void SVGGradientRegistry::resolveInheritanceRadial(SVGRadialGradientDef& g) const {
    // GIỮ LẠI để phù hợp header, nhưng ta KHÔNG GỌI hàm này trong makeBrush
    if (g.href.empty()) return;
    auto it = radial.find(g.href);
    if (it != radial.end()) {
        const auto& base = it->second;
        if (g.stops.empty())
            const_cast<std::vector<SVGGradientStop>&>(g.stops) = base.stops;
    }
    else {
        auto itL = linear.find(g.href);
        if (itL != linear.end() && g.stops.empty()) {
            const_cast<std::vector<SVGGradientStop>&>(g.stops) = itL->second.stops;
        }
    }
}

// ===== Helpers nội bộ để "resolve stops" mà không copy struct có Matrix =====
static std::vector<SVGGradientStop> resolvedStopsLinear(
    const SVGGradientRegistry& reg, const SVGLinearGradientDef& g)
{
    if (!g.stops.empty()) return g.stops;
    if (!g.href.empty()) {
        // trước thử linear
        auto itL = reg.linear.find(g.href);
        if (itL != reg.linear.end() && !itL->second.stops.empty())
            return itL->second.stops;
        // rồi radial
        auto itR = reg.radial.find(g.href);
        if (itR != reg.radial.end() && !itR->second.stops.empty())
            return itR->second.stops;
    }
    return {}; // trống -> sẽ được bù mặc định 0..1 trong hàm brush
}

static std::vector<SVGGradientStop> resolvedStopsRadial(
    const SVGGradientRegistry& reg, const SVGRadialGradientDef& g)
{
    if (!g.stops.empty()) return g.stops;
    if (!g.href.empty()) {
        auto itR = reg.radial.find(g.href);
        if (itR != reg.radial.end() && !itR->second.stops.empty())
            return itR->second.stops;
        auto itL = reg.linear.find(g.href);
        if (itL != reg.linear.end() && !itL->second.stops.empty())
            return itL->second.stops;
    }
    return {};
}

// ===== Linear =====
std::unique_ptr<Brush> SVGGradientRegistry::makeLinearBrush(
    const SVGLinearGradientDef& g, const RectF& bb) const
{
    auto resolve = [&](float v, bool isX) -> float {
        if (g.userSpaceOnUse) return v;
        return (isX ? bb.X + v * bb.Width : bb.Y + v * bb.Height);
        };

    PointF p1(resolve(g.x1, true), resolve(g.y1, false));
    PointF p2(resolve(g.x2, true), resolve(g.y2, false));

    auto stops = resolvedStopsLinear(*this, g);
    if (stops.empty()) {
        stops.push_back({ 0.f, Color(255,0,0,0) });
        stops.push_back({ 1.f, Color(255,0,0,0) });
    }
    sortStops(stops);

    // Tạo brush + set interpolation qua mảng (tránh ColorBlend)
    auto brush = std::make_unique<LinearGradientBrush>(p1, p2, Color::Black, Color::White);

    const int n = static_cast<int>(stops.size());
    std::vector<Color> cols(n);
    std::vector<REAL>  pos(n);
    for (int i = 0; i < n; ++i) {
        cols[i] = stops[i].color;
        pos[i] = std::clamp(stops[i].offset, 0.0f, 1.0f);
    }
    brush->SetInterpolationColors(cols.data(), pos.data(), n);

    // gradientTransform
    if (g.transform) {
        brush->MultiplyTransform(g.transform.get(), Gdiplus::MatrixOrderAppend);
    }
    return brush;
}

// ===== Radial =====
std::unique_ptr<Brush> SVGGradientRegistry::makeRadialBrush(
    const SVGRadialGradientDef& g, const RectF& bb) const
{
    auto resolve = [&](float v, bool isX) -> float {
        if (g.userSpaceOnUse) return v;
        return (isX ? bb.X + v * bb.Width : bb.Y + v * bb.Height);
        };

    float cx = resolve(g.cx, true);
    float cy = resolve(g.cy, false);
    float r = g.userSpaceOnUse ? g.r : ((bb.Width + bb.Height) * 0.5f * g.r);

    RectF ellipseRect(cx - r, cy - r, 2 * r, 2 * r);

    GraphicsPath path;
    path.AddEllipse(ellipseRect);
    auto brush = std::make_unique<PathGradientBrush>(&path);

    // center/focal
    if (g.fx >= 0 && g.fy >= 0) {
        brush->SetCenterPoint(PointF(resolve(g.fx, true), resolve(g.fy, false)));
    }
    else {
        brush->SetCenterPoint(PointF(cx, cy));
    }

    auto stops = resolvedStopsRadial(*this, g);
    if (stops.empty()) {
        stops.push_back({ 0.f, Color(255,0,0,0) });
        stops.push_back({ 1.f, Color(255,0,0,0) });
    }
    sortStops(stops);

    const int n = static_cast<int>(stops.size());
    std::vector<Color> cols(n);
    std::vector<REAL>  pos(n);
    for (int i = 0; i < n; ++i) {
        cols[i] = stops[i].color;
        pos[i] = std::clamp(stops[i].offset, 0.0f, 1.0f);
    }
    brush->SetInterpolationColors(cols.data(), pos.data(), n);

    if (g.transform) {
        brush->MultiplyTransform(g.transform.get(), Gdiplus::MatrixOrderAppend);
    }
    return brush;
}

// ===== Chọn brush theo id (KHÔNG COPY struct có Matrix) =====
std::unique_ptr<Brush> SVGGradientRegistry::makeBrush(
    const std::string& id, const RectF& bb) const
{
    auto itL = linear.find(id);
    if (itL != linear.end()) {
        // dùng trực tiếp const ref để tránh copy
        return makeLinearBrush(itL->second, bb);
    }
    auto itR = radial.find(id);
    if (itR != radial.end()) {
        return makeRadialBrush(itR->second, bb);
    }
    return nullptr;
}
