#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// Include thứ tự đúng cho GDI+
#include <windows.h>
#include <gdiplus.h>

struct SVGGradientStop {
    float offset = 0.0f; // 0..1
    Gdiplus::Color color = Gdiplus::Color(255, 0, 0, 0);
};

struct SVGLinearGradientDef {
    std::string id;
    bool userSpaceOnUse = false;
    // x1,y1,x2,y2: nếu !userSpaceOnUse thì hiểu là theo bbox (0..1)
    float x1 = 0.0f, y1 = 0.0f, x2 = 1.0f, y2 = 0.0f;
    std::unique_ptr<Gdiplus::Matrix> transform; // gradientTransform
    std::vector<SVGGradientStop> stops;
    std::string href; // xlink:href
};

struct SVGRadialGradientDef {
    std::string id;
    bool userSpaceOnUse = false;
    // nếu !userSpaceOnUse thì các giá trị theo bbox (0..1)
    float cx = 0.5f, cy = 0.5f, r = 0.5f;
    float fx = -1.0f, fy = -1.0f; // -1 = “không đặt”, sẽ dùng cx,cy
    std::unique_ptr<Gdiplus::Matrix> transform; // gradientTransform
    std::vector<SVGGradientStop> stops;
    std::string href; // xlink:href
};

class SVGGradientRegistry {
public:
    std::unordered_map<std::string, SVGLinearGradientDef> linear;
    std::unordered_map<std::string, SVGRadialGradientDef> radial;

    // tạo Brush cho fill: trả về unique_ptr<Gdiplus::Brush> đã set transform/stops
    std::unique_ptr<Gdiplus::Brush> makeBrush(
        const std::string& id,
        const Gdiplus::RectF& elementLocalBounds) const;

private:
    static Gdiplus::Color applyOpacity(const Gdiplus::Color& c, float mult);
    static void sortStops(std::vector<SVGGradientStop>& stops);

    void resolveInheritanceLinear(SVGLinearGradientDef& g) const;
    void resolveInheritanceRadial(SVGRadialGradientDef& g) const;

    std::unique_ptr<Gdiplus::Brush> makeLinearBrush(
        const SVGLinearGradientDef& g, const Gdiplus::RectF& bb) const;

    std::unique_ptr<Gdiplus::Brush> makeRadialBrush(
        const SVGRadialGradientDef& g, const Gdiplus::RectF& bb) const;
};
