#pragma once
#include <string>
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

struct PaintStyle {
    // Màu trực tiếp (ARGB: alpha = độ trong suốt)
    Gdiplus::Color strokeColor = Gdiplus::Color(255, 0, 0, 0);  // black, fully opaque
    Gdiplus::Color fillColor = Gdiplus::Color(255, 0, 0, 0);  // black, fully opaque

    float strokeWidth = 1.0f;
    float strokeOpacity = 1.0f;
    float fillOpacity = 1.0f;
};
struct TextPaintStyle {
    // Màu chữ
    Gdiplus::Color fillColor = Gdiplus::Color(255, 0, 0, 0);      // mặc định: đen, không trong suốt
    float fillOpacity = 1.0f;

    //// Viền chữ (ít dùng nhưng SVG vẫn hỗ trợ)
    //Color strokeColor = Color(0, 0, 0, 0);      // mặc định: không viền
    //float strokeWidth = 0.0f;
    //float strokeOpacity = 1.0f;

    // Font
    std::string fontFamily = "Arial";          // font mặc định
    std::string fontWeight = "normal";         // normal | bold | lighter | 100–900
    std::string fontStyle = "normal";          // normal | italic | oblique

    // Căn lề văn bản
    std::string textAnchor = "start";          // start | middle | end

    // Tùy chọn khác
    float opacity = 1.0f;                      // độ trong suốt tổng thể
};
