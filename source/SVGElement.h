#pragma once
#include "SVGBasics.h"
#include <string>
#include <windows.h>
#include <gdiplus.h>

// Abstract base class
class SVGElement {
public:
	PaintStyle style;

	SVGElement() = default;
	virtual ~SVGElement() = default; // Must have 

	virtual void draw(Graphics* graphics) const = 0;
};