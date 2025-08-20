#pragma once
#include <string>
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

struct PaintStyle {
    Gdiplus::Color strokeColor = Gdiplus::Color(0, 0, 0, 0);
    Gdiplus::Color fillColor = Gdiplus::Color(255, 0, 0, 0);

    float strokeWidth = 1.0f;
    float strokeOpacity = 1.0f;
    float fillOpacity = 1.0f;

    // NEW: hỗ trợ paint server
    bool  fillNone = false;
    bool  strokeNone = false;
    std::string fillUrlId;   // "c" nếu fill="url(#c)"
    std::string strokeUrlId; // hiện chưa dùng stroke gradient, nhưng để sẵn
};

struct TextPaintStyle {
    // Màu chữ
    Gdiplus::Color fillColor = Gdiplus::Color(255, 0, 0, 0);      // mặc định: đen, không trong suốt
    float fillOpacity = 1.0f;

    // Viền chữ (ít dùng nhưng SVG vẫn hỗ trợ)
    Gdiplus::Color strokeColor = Color(0, 0, 0, 0);       //mặc định: không viền
    float strokeWidth = 0.0f;
    float strokeOpacity = 1.0f;

    // Font
    std::string fontFamily = "Arial";          // font mặc định
    std::string fontWeight = "normal";         // normal | bold | lighter | 100–900
    std::string fontStyle = "normal";          // normal | italic | oblique
    float fontSize = 16.0f;                    // default font size

    // Căn lề văn bản
    std::string textAnchor = "start";          // start | middle | end

    // Text positioning offsets
    float dx = 0.0f;                           // horizontal offset
    float dy = 0.0f;                           // vertical offset

    // Tùy chọn khác
    float opacity = 1.0f;                      // độ trong suốt tổng thể
};
