#include "stdafx.h"
#include "SVGRect.h"

// Function helps double dispatch
void SVGRect::accept(SVGVisitor* visitor){
    visitor->visit(this);
}

Gdiplus::RectF SVGRect::localBounds() const {
    return Gdiplus::RectF(topLeft.X, topLeft.Y, width, height);
}

//Constructors
SVGRect::SVGRect(PointF topLeft, REAL width, REAL height, const PaintStyle &s)
{
	this->topLeft = topLeft;
	this->width = width;
	this->height = height;
	if (width < 0 || height < 0) {
		this->width = 0; // Đặt chiều rộng về 0 nếu âm
		this->height = 0; // Đặt chiều cao về 0 nếu âm
	}
	this->style = s;
}

void SVGRect::draw(Graphics* graphics) const {
    if (!graphics) return;
    auto state = graphics->Save();
    graphics->MultiplyTransform(&getTransform());

    const RectF bb = localBounds();

    // FILL
    if (!style.fillNone && style.fillOpacity > 0.0f) {
        if (gradientRegistry && !style.fillUrlId.empty()) {
            auto brush = gradientRegistry->makeBrush(style.fillUrlId, bb);
            if (brush) {
                graphics->FillRectangle(brush.get(), bb);
            }
        }
        else {
            BYTE alpha = static_cast<BYTE>(style.fillOpacity * style.fillColor.GetA());
            Color fillColor(alpha, style.fillColor.GetR(), style.fillColor.GetG(), style.fillColor.GetB());
            SolidBrush fillBrush(fillColor);
            graphics->FillRectangle(&fillBrush, bb);
        }
    }

    // STROKE (giữ như cũ – có thể mở rộng url tương tự nếu cần)
    if (!style.strokeNone && style.strokeWidth > 0.0f && style.strokeOpacity > 0.0f) {
        BYTE alpha = static_cast<BYTE>(style.strokeOpacity * style.strokeColor.GetA());
        Color strokeColor(alpha, style.strokeColor.GetR(), style.strokeColor.GetG(), style.strokeColor.GetB());
        Pen pen(strokeColor, static_cast<REAL>(style.strokeWidth));
        graphics->DrawRectangle(&pen, bb);
    }
    graphics->Restore(state);
}
