#pragma once
#include <string>
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

class SVGElement { // Base class, các class khác sẽ kế thừa từ class này
public:
    virtual void draw(Graphics* graphics) = 0;
    virtual ~SVGElement() = default;
};