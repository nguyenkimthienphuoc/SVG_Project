#include "stdafx.h"
#include "SVGGradient.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

using namespace Gdiplus;

Gdiplus::Color SVGGradientRegistry::applyOpacity(const Gdiplus::Color& c, float mult) {
    BYTE a = static_cast<BYTE>(std::clamp<int>(int(c.GetA() * mult), 0, 255));
    return Gdiplus::Color(a, c.GetR(), c.GetG(), c.GetB());
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

// Merge chain of referenced gradients into a single resolved linear definition
static SVGLinearGradientDef mergedLinearDef(const SVGGradientRegistry& reg, const SVGLinearGradientDef& src)
{
    SVGLinearGradientDef out;
    // copy scalar/simple fields
    out.id = src.id;
    out.userSpaceOnUse = src.userSpaceOnUse;
    out.unitsSpecified = src.unitsSpecified;
    out.x1 = src.x1; out.y1 = src.y1; out.x2 = src.x2; out.y2 = src.y2;
    out.x1Specified = src.x1Specified; out.y1Specified = src.y1Specified; out.x2Specified = src.x2Specified; out.y2Specified = src.y2Specified;
    out.stops = src.stops; // vector copy
    out.href = src.href;
    out.spreadMethod = src.spreadMethod;
    out.transformSpecified = src.transformSpecified;
    if (src.transform) out.transform.reset(src.transform->Clone());

    const SVGLinearGradientDef* cur = &src;
    int depth = 0;
    while (!cur->href.empty() && depth < 10) {
        auto it = reg.linear.find(cur->href);
        if (it == reg.linear.end()) {
            // maybe href points to radial -> stop for coords but stops may be inherited elsewhere
            break;
        }
        const SVGLinearGradientDef& base = it->second;

        if (!out.unitsSpecified && base.unitsSpecified) { out.userSpaceOnUse = base.userSpaceOnUse; out.unitsSpecified = true; }
        if (!out.x1Specified && base.x1Specified) { out.x1 = base.x1; out.x1Specified = true; }
        if (!out.y1Specified && base.y1Specified) { out.y1 = base.y1; out.y1Specified = true; }
        if (!out.x2Specified && base.x2Specified) { out.x2 = base.x2; out.x2Specified = true; }
        if (!out.y2Specified && base.y2Specified) { out.y2 = base.y2; out.y2Specified = true; }
        if (!out.transformSpecified && base.transform) { out.transform.reset(base.transform->Clone()); out.transformSpecified = true; }
        if (out.spreadMethod == "pad" && base.spreadMethod != "pad") out.spreadMethod = base.spreadMethod;
        if (out.stops.empty() && !base.stops.empty()) out.stops = base.stops;

        cur = &base; ++depth;
    }
    return out;
}

// Merge chain for radial
static SVGRadialGradientDef mergedRadialDef(const SVGGradientRegistry& reg, const SVGRadialGradientDef& src)
{
    SVGRadialGradientDef out;
    out.id = src.id;
    out.userSpaceOnUse = src.userSpaceOnUse;
    out.unitsSpecified = src.unitsSpecified;
    out.cx = src.cx; out.cy = src.cy; out.r = src.r;
    out.cxSpecified = src.cxSpecified; out.cySpecified = src.cySpecified; out.rSpecified = src.rSpecified;
    out.fx = src.fx; out.fy = src.fy; out.fxSpecified = src.fxSpecified; out.fySpecified = src.fySpecified;
    out.stops = src.stops;
    out.href = src.href;
    out.spreadMethod = src.spreadMethod;
    out.transformSpecified = src.transformSpecified;
    if (src.transform) out.transform.reset(src.transform->Clone());

    const SVGRadialGradientDef* cur = &src;
    int depth = 0;
    while (!cur->href.empty() && depth < 10) {
        auto it = reg.radial.find(cur->href);
        if (it == reg.radial.end()) break;
        const SVGRadialGradientDef& base = it->second;

        if (!out.unitsSpecified && base.unitsSpecified) { out.userSpaceOnUse = base.userSpaceOnUse; out.unitsSpecified = true; }
        if (!out.cxSpecified && base.cxSpecified) { out.cx = base.cx; out.cxSpecified = true; }
        if (!out.cySpecified && base.cySpecified) { out.cy = base.cy; out.cySpecified = true; }
        if (!out.rSpecified && base.rSpecified)   { out.r = base.r; out.rSpecified = true; }
        if (!out.fxSpecified && base.fxSpecified) { out.fx = base.fx; out.fxSpecified = true; }
        if (!out.fySpecified && base.fySpecified) { out.fy = base.fy; out.fySpecified = true; }
        if (!out.transformSpecified && base.transform) { out.transform.reset(base.transform->Clone()); out.transformSpecified = true; }
        if (out.spreadMethod == "pad" && base.spreadMethod != "pad") out.spreadMethod = base.spreadMethod;
        if (out.stops.empty() && !base.stops.empty()) out.stops = base.stops;

        cur = &base; ++depth;
    }
    return out;
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

// Normalize stops: handle unspecified offsets (NaN) per SVG spec
static void normalizeStops(std::vector<SVGGradientStop>& stops)
{
    using std::isnan;
    const int n = static_cast<int>(stops.size());
    if (n == 0) return;

    // If first offset unspecified -> 0
    if (std::isnan(stops[0].offset)) stops[0].offset = 0.0f;
    // If last offset unspecified -> 1
    if (std::isnan(stops[n-1].offset)) stops[n-1].offset = 1.0f;

    // Walk and interpolate runs of NaN between two known offsets
    int i = 0;
    while (i < n) {
        if (!std::isnan(stops[i].offset)) { ++i; continue; }
        // find start (prev known) and end (next known)
        int j = i;
        while (j < n && std::isnan(stops[j].offset)) ++j;
        if (j == n) break; // should not happen because last handled
        float startVal = stops[i-1].offset;
        float endVal = stops[j].offset;
        int gap = j - i + 1; // number of intervals including endpoints
        for (int k = i; k < j; ++k) {
            float t = float(k - (i-1)) / float(gap);
            stops[k].offset = startVal + t * (endVal - startVal);
        }
        i = j;
    }

    // Clamp just in case
    for (auto &s : stops) s.offset = std::clamp(s.offset, 0.0f, 1.0f);
}

// ===== Linear =====
std::unique_ptr<Brush> SVGGradientRegistry::makeLinearBrush(
    const SVGLinearGradientDef& g, const RectF& bb) const
{
    // merge inheritance chain first
    auto mg = mergedLinearDef(*this, g);

    auto resolve = [&](float v, bool isX) -> float {
        if (mg.userSpaceOnUse) return v;
        return (isX ? bb.X + v * bb.Width : bb.Y + v * bb.Height);
        };

    PointF p1(resolve(mg.x1, true), resolve(mg.y1, false));
    PointF p2(resolve(mg.x2, true), resolve(mg.y2, false));

    auto stops = resolvedStopsLinear(*this, mg);
    if (stops.empty()) {
        stops.push_back({ 0.f, Color(255,0,0,0) });
        stops.push_back({ 1.f, Color(255,0,0,0) });
    }
    // normalize unspecified offsets then sort
    normalizeStops(stops);
    sortStops(stops);

    // Tạo brush + set interpolation qua mảng (tránh ColorBlend)
    auto brush = std::make_unique<LinearGradientBrush>(p1, p2, Color::Black, Color::White);

    // DEBUG: dump gradient creation info
    std::cout << "makeLinearBrush id=" << mg.id << " userSpaceOnUse=" << mg.userSpaceOnUse
        << " p1=(" << p1.X << "," << p1.Y << ") p2=(" << p2.X << "," << p2.Y << ") stops=" << stops.size()
        << " spread=" << mg.spreadMethod << std::endl;

    // (cols/pos arrays are prepared below; avoid duplicated declarations)

    // Prepare arrays and set interpolation colors for linear gradient
    const int n = static_cast<int>(stops.size());
    std::vector<Color> cols(n);
    std::vector<REAL>  pos(n);
    for (int i = 0; i < n; ++i) {
        cols[i] = stops[i].color;
        pos[i] = std::clamp(stops[i].offset, 0.0f, 1.0f);
    }
    // Dump stops for debugging (after cols/pos are populated)
    for (int i = 0; i < n; ++i) {
        Color c = cols[i];
        std::cout << "  stop[" << i << "] pos=" << pos[i]
            << " rgba=(" << (int)c.GetR() << "," << (int)c.GetG() << "," << (int)c.GetB() << "," << (int)c.GetA() << ")\n";
    }
    brush->SetInterpolationColors(cols.data(), pos.data(), n);

    // gradientTransform
    if (mg.transform) {
        brush->MultiplyTransform(mg.transform.get(), Gdiplus::MatrixOrderAppend);
    }
    // spreadMethod -> WrapMode
    if (!mg.spreadMethod.empty()) {
        if (mg.spreadMethod == "reflect") brush->SetWrapMode(WrapModeTileFlipXY);
        else if (mg.spreadMethod == "repeat") brush->SetWrapMode(WrapModeTile);
        else brush->SetWrapMode(WrapModeClamp);
    }
    return brush;
}

// ===== Radial =====
std::unique_ptr<Brush> SVGGradientRegistry::makeRadialBrush(
    const SVGRadialGradientDef& g, const RectF& bb) const
{
    // merge inheritance chain
    auto mg = mergedRadialDef(*this, g);

    auto resolve = [&](float v, bool isX) -> float {
        if (mg.userSpaceOnUse) return v;
        return (isX ? bb.X + v * bb.Width : bb.Y + v * bb.Height);
        };

    float cx = resolve(mg.cx, true);
    float cy = resolve(mg.cy, false);
    float r = mg.userSpaceOnUse ? mg.r : ((bb.Width + bb.Height) * 0.5f * mg.r);

    RectF ellipseRect(cx - r, cy - r, 2 * r, 2 * r);

    GraphicsPath path;
    path.AddEllipse(ellipseRect);

    // Diagnostics: print ellipse rect and path bounds before transform
    RectF prBefore;
    path.GetBounds(&prBefore);
    std::cout << "  ellipseRect before transform = (" << ellipseRect.X << "," << ellipseRect.Y << "," << ellipseRect.Width << "," << ellipseRect.Height << ")\n";
    std::cout << "  path bounds before transform = (" << prBefore.X << "," << prBefore.Y << "," << prBefore.Width << "," << prBefore.Height << ")\n";

    // If gradientTransform present, we do NOT mutate the path here; we'll apply the transform to the brush
    if (mg.transform) {
        std::cout << "  (gradientTransform present) will apply transform to brush instead of path\n";
    }

    auto stops = resolvedStopsRadial(*this, mg);
    if (stops.empty()) {
        stops.push_back({ 0.f, Color(255,0,0,0) });
        stops.push_back({ 1.f, Color(255,0,0,0) });
    }
    normalizeStops(stops);
    sortStops(stops);

    auto brush = std::make_unique<PathGradientBrush>(&path);

    std::cout << "makeRadialBrush id=" << mg.id << " cx=" << cx << " cy=" << cy << " r=" << r << " stops=" << stops.size() << " spread=" << mg.spreadMethod << std::endl;

    // Extra diagnostics: units, fx/fy, transform matrix, and stop colors
    std::cout << "  userSpaceOnUse=" << mg.userSpaceOnUse << " cxSpecified=" << mg.cxSpecified << " cySpecified=" << mg.cySpecified << " rSpecified=" << mg.rSpecified
        << " fxSpecified=" << mg.fxSpecified << " fySpecified=" << mg.fySpecified << std::endl;

    if (mg.transform) {
        float elems[6] = {0};
        mg.transform->GetElements(elems);
        std::cout << "  transform matrix = [" << elems[0] << "," << elems[1] << "," << elems[2] << "," << elems[3] << "," << elems[4] << "," << elems[5] << "]\n";
    } else {
        std::cout << "  transform = (none)\n";
    }


    // center/focal: use cx/cy as the brush center (SVG center). fx/fy is the focal point.
    PointF centerPt(cx, cy);
    PointF focalPt = centerPt;
    if (mg.fx >= 0 && mg.fy >= 0) {
        focalPt = PointF(resolve(mg.fx, true), resolve(mg.fy, false));
    }

    // Set center point in the (untransformed) gradient coordinate space; if a gradientTransform
    // exists we'll apply it to the brush below so the brush and geometry remain aligned.
    brush->SetCenterPoint(centerPt);
    // Diagnostic: report focal point if different
    if (focalPt.X != centerPt.X || focalPt.Y != centerPt.Y) {
        std::cout << "  focalPt after transform = (" << focalPt.X << "," << focalPt.Y << ")\n";
    }

    // Try to approximate SVG's focal point by setting focus scales on PathGradientBrush.
    // Compute normalized offset of focal relative to center in the path bounds.
    RectF pathBounds;
    path.GetBounds(&pathBounds);
    float halfW = pathBounds.Width * 0.5f;
    float halfH = pathBounds.Height * 0.5f;
    if (halfW <= 0) halfW = 1.0f;
    if (halfH <= 0) halfH = 1.0f;
    float dx = (focalPt.X - centerPt.X) / halfW;
    float dy = (focalPt.Y - centerPt.Y) / halfH;
    auto clampf = [](float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); };
    dx = clampf(dx, -1.0f, 1.0f);
    dy = clampf(dy, -1.0f, 1.0f);
    // PathGradientBrush focus scales are (scaleX, scaleY) where 1.0 means focus at center,
    // lower values move focus towards edges. We approximate with 1 - |offset|.
    float focusScaleX = 1.0f - fabsf(dx);
    if (focusScaleX < 0.001f) focusScaleX = 0.001f;
    float focusScaleY = 1.0f - fabsf(dy);
    if (focusScaleY < 0.001f) focusScaleY = 0.001f;
    brush->SetFocusScales(focusScaleX, focusScaleY);
    std::cout << "  focus scales set = (" << focusScaleX << "," << focusScaleY << ") from normalized offset (" << dx << "," << dy << ")\n";

    // Prepare arrays for evaluating colors

    // Prepare arrays for evaluating colors
    const int n = static_cast<int>(stops.size());
    std::vector<Color> cols(n);
    std::vector<REAL>  pos(n);
    for (int i = 0; i < n; ++i) {
        cols[i] = stops[i].color;
        pos[i] = std::clamp(stops[i].offset, 0.0f, 1.0f);
    }

    // For radial, set center color and surround colors on the PathGradientBrush
    auto evalColorAt = [&](REAL t) -> Color {
        if (n == 0) return Color(255,0,0,0);
        if (t <= pos.front()) return cols.front();
        if (t >= pos.back()) return cols.back();
        for (int i = 1; i < n; ++i) {
            if (t <= pos[i]) {
                float span = pos[i] - pos[i-1];
                float local = (t - pos[i-1]) / (span > 1e-6f ? span : 1.0f);
                Color a = cols[i-1]; Color b = cols[i];
                BYTE ra = static_cast<BYTE>(a.GetA() + local * (b.GetA() - a.GetA()));
                BYTE rr = static_cast<BYTE>(a.GetR() + local * (b.GetR() - a.GetR()));
                BYTE rg = static_cast<BYTE>(a.GetG() + local * (b.GetG() - a.GetG()));
                BYTE rb = static_cast<BYTE>(a.GetB() + local * (b.GetB() - a.GetB()));
                return Color(ra, rr, rg, rb);
            }
        }
        return cols.back();
    };

    Color centerColor = evalColorAt(0.0f);
    Color outerColor = evalColorAt(1.0f);
    std::cout << "  centerColor rgba=(" << (int)centerColor.GetR() << "," << (int)centerColor.GetG() << "," << (int)centerColor.GetB() << "," << (int)centerColor.GetA() << ")\n";
    std::cout << "  outerColor rgba=(" << (int)outerColor.GetR() << "," << (int)outerColor.GetG() << "," << (int)outerColor.GetB() << "," << (int)outerColor.GetA() << ")\n";
    brush->SetCenterColor(centerColor);
    int pointCount = path.GetPointCount();
    if (pointCount <= 0) pointCount = 1;
    // Compute surround colors per boundary point by evaluating the gradient at the
    // normalized distance from the center to that point. This produces a better
    // match to SVG stop distribution than a single outer color.
    std::vector<PointF> pts(pointCount);
    path.GetPathPoints(pts.data(), pointCount);
    std::vector<Color> surround(pointCount);

    // We'll evaluate points in the gradient's local coordinate space. If a gradientTransform
    // exists we invert it and apply to the sampled points and the center so all are in the
    // same coordinate system. Then compute evalRadius in that space.
    PointF centerLocal = centerPt;
    float evalRadius = r;
    std::unique_ptr<Gdiplus::Matrix> inv;
    if (mg.transform) {
        auto tmp = mg.transform->Clone();
        if (tmp && tmp->Invert() == Ok) inv.reset(tmp);
        else delete tmp;
    }
    if (inv) {
        inv->TransformPoints(pts.data(), pointCount);
        PointF tmpC[1] = { centerLocal };
        inv->TransformPoints(tmpC, 1);
        centerLocal = tmpC[0];
        // compute radius by transforming a point at (center + r, center) and measuring distance
        PointF rightPt(centerPt.X + r, centerPt.Y);
        PointF rightLocal[1] = { rightPt };
        inv->TransformPoints(rightLocal, 1);
        float dxr = rightLocal[0].X - centerLocal.X;
        float dyr = rightLocal[0].Y - centerLocal.Y;
        evalRadius = std::sqrt(dxr*dxr + dyr*dyr);
    }
    if (evalRadius <= 0.0f) {
        // fallback: use max distance from centerLocal to sampled boundary points
        float maxd = 0.0f;
        for (int pi = 0; pi < pointCount; ++pi) {
            float dxp = pts[pi].X - centerLocal.X;
            float dyp = pts[pi].Y - centerLocal.Y;
            float dist = std::sqrt(dxp*dxp + dyp*dyp);
            if (dist > maxd) maxd = dist;
        }
        evalRadius = (maxd > 0.0f) ? maxd : 1.0f;
    }

    for (int pi = 0; pi < pointCount; ++pi) {
        float dxp = pts[pi].X - centerLocal.X;
        float dyp = pts[pi].Y - centerLocal.Y;
        float dist = std::sqrt(dxp*dxp + dyp*dyp);
        float t = std::clamp(dist / evalRadius, 0.0f, 1.0f);
        surround[pi] = evalColorAt(t);
        // Avoid fully-transparent surround colors which can reveal unrelated
        // underlying geometry and look like a stray blue patch. If alpha==0,
        // promote to opaque (preserve RGB) to approximate intended radial fill.
        if (surround[pi].GetA() == 0) {
            surround[pi] = Color(255, surround[pi].GetR(), surround[pi].GetG(), surround[pi].GetB());
        }
        // diagnostic
        Color sc = surround[pi];
        std::cout << "    surround[" << pi << "] t=" << t << " rgba=(" << (int)sc.GetR() << "," << (int)sc.GetG() << "," << (int)sc.GetB() << "," << (int)sc.GetA() << ")\n";
    }
    brush->SetSurroundColors(surround.data(), &pointCount);

    // Instead of returning the PathGradientBrush (which can produce artifacts
    // for multi-stop SVG gradients), rasterize the radial gradient into a
    // texture by rendering many narrow LinearGradientBrush strips. Then use a
    // TextureBrush mapped to the ellipse bounds and apply the SVG gradientTransform.

    path.GetBounds(&pathBounds);
    int texW = 32;
    {
        int cw = static_cast<int>(std::ceil(pathBounds.Width));
        if (cw > texW) texW = cw;
        if (texW > 2048) texW = 2048;
    }
    int texH = 32;
    {
        int ch = static_cast<int>(std::ceil(pathBounds.Height));
        if (ch > texH) texH = ch;
        if (texH > 2048) texH = 2048;
    }

    // Create a premultiplied ARGB bitmap and rasterize per-pixel using evalColorAt.
    // Using LockBits + PixelFormat32bppPARGB gives correct premultiplied storage for GDI+.
    Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(texW, texH, PixelFormat32bppPARGB);
    {
        Gdiplus::Rect lockRect(0, 0, texW, texH);
        Gdiplus::BitmapData data;
        if (bmp->LockBits(&lockRect, ImageLockModeWrite, PixelFormat32bppPARGB, &data) == Ok) {
            unsigned char* scan0 = static_cast<unsigned char*>(data.Scan0);
            int stride = data.Stride;
            int denomInt = texW + texH - 2;
            if (denomInt < 1) denomInt = 1;
            const float denom = float(denomInt);
            // Precompute pixel->path scale
            float invTexW = (texW > 0) ? (1.0f / texW) : 1.0f;
            float invTexH = (texH > 0) ? (1.0f / texH) : 1.0f;
            for (int y = 0; y < texH; ++y) {
                unsigned char* row = scan0 + y * stride;
                for (int x = 0; x < texW; ++x) {
                    // Map pixel center to path bounds (device space)
                    float px = pathBounds.X + ( (x + 0.5f) * (pathBounds.Width * invTexW) );
                    float py = pathBounds.Y + ( (y + 0.5f) * (pathBounds.Height * invTexH) );

                    // Transform into gradient-local coordinates if we have an inverse
                    PointF samplePt(px, py);
                    if (inv) {
                        PointF tmpArr[1] = { samplePt };
                        inv->TransformPoints(tmpArr, 1);
                        samplePt = tmpArr[0];
                    }

                    // Distance from gradient center (in gradient-local space)
                    float dx = samplePt.X - centerLocal.X;
                    float dy = samplePt.Y - centerLocal.Y;
                    float dist = std::sqrt(dx*dx + dy*dy);
                    float t = (evalRadius > 0.0f) ? (dist / evalRadius) : 0.0f;
                    t = std::clamp(t, 0.0f, 1.0f);

                    Color c = evalColorAt(t);
                    BYTE a = c.GetA();
                    // premultiply
                    BYTE r = static_cast<BYTE>((int(c.GetR()) * int(a) + 127) / 255);
                    BYTE g = static_cast<BYTE>((int(c.GetG()) * int(a) + 127) / 255);
                    BYTE b = static_cast<BYTE>((int(c.GetB()) * int(a) + 127) / 255);
                    // PixelFormat32bppPARGB in memory is B G R A
                    int idx = x * 4;
                    row[idx + 0] = b;
                    row[idx + 1] = g;
                    row[idx + 2] = r;
                    row[idx + 3] = a;
                }
            }
            bmp->UnlockBits(&data);
        } else {
            // fallback: clear bitmap via Graphics if LockBits failed
            Gdiplus::Graphics g(bmp);
            g.Clear(Gdiplus::Color(0,0,0,0));
        }
    }

    // Create TextureBrush from bitmap and map it to the path bounding box.
    Gdiplus::TextureBrush* tb = new Gdiplus::TextureBrush(bmp);
    Gdiplus::Matrix mapM;
    // Scale texture to path bounds size then translate to path origin
    mapM.Translate(pathBounds.X, pathBounds.Y, Gdiplus::MatrixOrderAppend);
    if (texW > 0 && texH > 0) {
        mapM.Scale(pathBounds.Width / (REAL)texW, pathBounds.Height / (REAL)texH, Gdiplus::MatrixOrderAppend);
    }
    tb->SetTransform(&mapM);

    // Apply SVG gradientTransform to the brush so the texture follows the SVG
    // gradient's transform semantics.
    if (mg.transform) {
        tb->MultiplyTransform(mg.transform.get(), Gdiplus::MatrixOrderAppend);
        float elems[6] = {0};
        mg.transform->GetElements(elems);
        std::cout << "  applied transform to texture brush: [" << elems[0] << "," << elems[1] << "," << elems[2] << "," << elems[3] << "," << elems[4] << "," << elems[5] << "]\n";
    }

    // spreadMethod -> WrapMode
    if (!mg.spreadMethod.empty()) {
        if (mg.spreadMethod == "reflect") tb->SetWrapMode(WrapModeTileFlipXY);
        else if (mg.spreadMethod == "repeat") tb->SetWrapMode(WrapModeTile);
        else tb->SetWrapMode(WrapModeClamp);
    }

    // Return the TextureBrush (TextureBrush takes ownership of the bitmap in practice
    // for our usage pattern; the bitmap pointer must remain valid for the brush lifetime).
    return std::unique_ptr<Gdiplus::Brush>(tb);
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
