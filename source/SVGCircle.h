#pragma once
#include "SVGElement.h"
#include <string>
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

class SVGCircle : public SVGElement {
public:
	Gdiplus::PointF center;
	float radius = 0;

	SVGCircle(Gdiplus::PointF center, float radius);
	void draw(Graphics* graphics) const override;
};