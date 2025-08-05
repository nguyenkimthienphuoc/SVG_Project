#pragma once
#include "SVGBasics.h"
#include <string>
#include <windows.h>
#include <gdiplus.h>

// Abstract base class
class SVGElement {
protected:
	Gdiplus::Matrix transformMatrix;
public:
	PaintStyle style;

	SVGElement() = default;
	virtual ~SVGElement() = default; // Must have 

	virtual void draw(Graphics* graphics) const = 0;

    // Áp dụng transform
    virtual void applyTransform(const Gdiplus::Matrix& m) {
        transformMatrix.Multiply(&m, Gdiplus::MatrixOrderAppend);
    }

    // Áp dụng phép tịnh tiến
    void setTranslate(float tx, float ty) {
        transformMatrix.Translate(tx, ty, Gdiplus::MatrixOrderAppend);
    }

    // Áp dụng phép xoay quanh gốc (0,0)
    void setRotate(float angle) {
        transformMatrix.Rotate(angle, Gdiplus::MatrixOrderAppend);
    }

    // Áp dụng thay đổi tỉ lệ
    void setScale(float sx, float sy) {
        transformMatrix.Scale(sx, sy, Gdiplus::MatrixOrderAppend);
    }

    // Reset tất cả transform
    void resetTransform() {
        transformMatrix.Reset();
    }

    const Gdiplus::Matrix& getTransform() const {
        return transformMatrix;
    }
};