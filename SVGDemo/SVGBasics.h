#pragma once
#include <string>
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

struct PaintStyle {
    // Màu trực tiếp (ARGB: alpha = độ trong suốt)
    Color strokeColor = Color(255, 0, 0, 0);  // black, fully opaque
    Color fillColor = Color(255, 0, 0, 0);  // black, fully opaque
    float strokeWidth = 1.0f;
    float strokeOpacity = 1.0f;
    float fillOpacity = 1.0f;
};