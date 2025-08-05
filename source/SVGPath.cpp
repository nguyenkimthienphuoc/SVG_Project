#include "stdafx.h"
#include <algorithm>
#include <iostream>

// Function helps double dispatch
void SVGPath::accept(SVGVisitor* visitor){
	visitor->visit(this);
}

SVGPath::SVGPath(const std::string& data, const PaintStyle& style) : pathData(data) {
    this->style = style;
}
void SVGPath::draw(Graphics* graphics) const {
    if (!graphics) return;

    // Preprocess path data to handle SVG format properly
    std::string processedData = pathData;
    
    // Add spaces around commands for easier parsing
    for (char c : "MmLlHhVvCcSsQqTtAaZz") {
        std::string target(1, c);
        std::string replacement = std::string(" ") + c + " ";
        
        size_t pos = 0;
        while ((pos = processedData.find(target, pos)) != std::string::npos) {
            processedData.replace(pos, 1, replacement);
            pos += replacement.length();
        }
    }
    
    // Replace commas with spaces
    std::replace(processedData.begin(), processedData.end(), ',', ' ');
    

    Gdiplus::GraphicsPath gp;
    std::istringstream ss(processedData);
    char cmd;
    float x, y, x1, y1, x2, y2 = 0;
    Gdiplus::PointF currentPoint(0, 0);
    Gdiplus::PointF nextPoint(0, 0);

    Gdiplus::PointF p1(0, 0), p2(0, 0), end(0, 0);

    Gdiplus::PointF startPoint(0, 0);  // used for Z/z
    Gdiplus::PointF prevControlPoint(0, 0); // needed for Curve
    char prevCommand = 0;

    while (ss >> cmd) {
        bool isRelative = islower(cmd);
        char upperCmd = toupper(cmd);
        switch (upperCmd) {
        case 'M': // Move to
            ss >> x >> y;
            currentPoint = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x, y);
            startPoint = currentPoint;
            gp.StartFigure();
            break;

        case 'Z': //Close path
            currentPoint = startPoint;
            gp.CloseFigure();
            break;

        case 'L': //Line to
            ss >> x >> y;
            nextPoint = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x, y);
            gp.AddLine(currentPoint, nextPoint);
            currentPoint = nextPoint;
            break;
        case 'H': //Horizontal line to
            ss >> x;
            nextPoint = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y) : Gdiplus::PointF(x, currentPoint.Y);
            gp.AddLine(currentPoint, nextPoint);
            currentPoint = nextPoint;
            break;
        case 'V': //Vertical line to
            ss >> y;
            nextPoint = isRelative ? Gdiplus::PointF(currentPoint.X, currentPoint.Y + y) : Gdiplus::PointF(currentPoint.X, y);
            gp.AddLine(currentPoint, nextPoint);
            currentPoint = nextPoint;
            break;
        case 'C': // Cubic Bézier curve
            ss >> x1 >> y1 >> x2 >> y2 >> x >> y;
            p1 = isRelative ? Gdiplus::PointF(currentPoint.X + x1, currentPoint.Y + y1) : Gdiplus::PointF(x1, y1);
            p2 = isRelative ? Gdiplus::PointF(currentPoint.X + x2, currentPoint.Y + y2) : Gdiplus::PointF(x2, y2);
            end = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x, y);

            gp.AddBezier(currentPoint, p1, p2, end);
            prevControlPoint = p2;
            currentPoint = end;
            prevCommand = upperCmd;
            break;
        case 'S': {// Smooth cubic Bézier curve
            ss >> x2 >> y2 >> x >> y;

            p2 = isRelative ? Gdiplus::PointF(currentPoint.X + x2, currentPoint.Y + y2) : Gdiplus::PointF(x2, y2);
            end = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x, y);

            Gdiplus::PointF control1;
            if (prevCommand == 'C' || prevCommand == 'S') {
                control1.X = 2 * currentPoint.X - prevControlPoint.X;
                control1.Y = 2 * currentPoint.Y - prevControlPoint.Y;
            }
            else {
                control1 = currentPoint;
            }

            gp.AddBezier(currentPoint, control1, p2, end);
            prevControlPoint = p2;
            currentPoint = end;
            prevCommand = upperCmd;
            break;
        }
        case 'Q': {
            ss >> x1 >> y1 >> x >> y;
            p1 = isRelative ? Gdiplus::PointF(currentPoint.X + x1, currentPoint.Y + y1) : Gdiplus::PointF(x1, y1);
            end = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x, y);

            // Convert to cubic Bézier
            gp.AddBezier(
                currentPoint,
                Gdiplus::PointF(currentPoint.X + (2.0f / 3.0f) * (p1.X - currentPoint.X),
                    currentPoint.Y + (2.0f / 3.0f) * (p1.Y - currentPoint.Y)),
                Gdiplus::PointF(end.X + (2.0f / 3.0f) * (p1.X - end.X),
                    end.Y + (2.0f / 3.0f) * (p1.Y - end.Y)),
                end
            );

            prevControlPoint = p1;
            currentPoint = end;
            prevCommand = upperCmd;
            break;
        }
        case 'T': {
            ss >> x >> y;
            end = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x, y);

            if (prevCommand == 'Q' || prevCommand == 'T') {
                p1.X = 2 * currentPoint.X - prevControlPoint.X;
                p1.Y = 2 * currentPoint.Y - prevControlPoint.Y;
            }
            else {
                p1 = currentPoint;
            }

            gp.AddBezier(
                currentPoint,
                Gdiplus::PointF(currentPoint.X + (2.0f / 3.0f) * (p1.X - currentPoint.X),
                    currentPoint.Y + (2.0f / 3.0f) * (p1.Y - currentPoint.Y)),
                Gdiplus::PointF(end.X + (2.0f / 3.0f) * (p1.X - end.X),
                    end.Y + (2.0f / 3.0f) * (p1.Y - end.Y)),
                end
            );

            prevControlPoint = p1;
            currentPoint = end;
            prevCommand = upperCmd;
            break;
        }
    }
}

    // Fill and stroke
    SolidBrush fillBrush(Color(
        static_cast<BYTE>(style.fillOpacity * style.fillColor.GetA()),
        style.fillColor.GetR(), style.fillColor.GetG(), style.fillColor.GetB()));

    Pen strokePen(Color(
        static_cast<BYTE>(style.strokeOpacity * style.strokeColor.GetA()),
        style.strokeColor.GetR(), style.strokeColor.GetG(), style.strokeColor.GetB()),
        style.strokeWidth);

    if (style.fillOpacity > 0)
        graphics->FillPath(&fillBrush, &gp);
    if (style.strokeOpacity > 0 && style.strokeWidth > 0)
        graphics->DrawPath(&strokePen, &gp);
}