#include "stdafx.h"
#include <algorithm>
#include <iostream>
#include <cmath>

using namespace std;
using namespace Gdiplus;

// Function helps double dispatch
void SVGPath::accept(SVGVisitor* visitor){
	visitor->visit(this);
}

SVGPath::SVGPath(const std::string& data, const PaintStyle& style) : pathData(data) {
    this->style = style;
}
void SVGPath::draw(Graphics* graphics) const {
    if (!graphics) return;

    // Save graphics state
    GraphicsState state = graphics->Save();
    
    // Apply transform
    graphics->MultiplyTransform(&getTransform());

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
        case 'A': { // Arc
            float rx, ry, xAxisRotation, largeArcFlag, sweepFlag;
            ss >> rx >> ry >> xAxisRotation >> largeArcFlag >> sweepFlag >> x >> y;
            
            end = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x, y);
            
            // Convert SVG arc to GDI+ using proper arc implementation
            if (rx > 0 && ry > 0 && (currentPoint.X != end.X || currentPoint.Y != end.Y)) {
                // Convert rotation angle to radians
                float phi = xAxisRotation * 3.14159f / 180.0f;
                
                // Calculate center point and angles using SVG arc equations
                float dx = (currentPoint.X - end.X) / 2.0f;
                float dy = (currentPoint.Y - end.Y) / 2.0f;
                
                // Rotate by -phi
                float x1_prime = cos(phi) * dx + sin(phi) * dy;
                float y1_prime = -sin(phi) * dx + cos(phi) * dy;
                
                // Ensure rx, ry are large enough
                float lambda = (x1_prime * x1_prime) / (rx * rx) + (y1_prime * y1_prime) / (ry * ry);
                if (lambda > 1) {
                    rx *= sqrt(lambda);
                    ry *= sqrt(lambda);
                }
                
                // Calculate center
                float sign = (largeArcFlag == sweepFlag) ? -1.0f : 1.0f;
                float sq = max(0.0f, (rx * rx * ry * ry - rx * rx * y1_prime * y1_prime - ry * ry * x1_prime * x1_prime) / 
                                     (rx * rx * y1_prime * y1_prime + ry * ry * x1_prime * x1_prime));
                float coeff = sign * sqrt(sq);
                
                float cx_prime = coeff * (rx * y1_prime / ry);
                float cy_prime = coeff * -(ry * x1_prime / rx);
                
                // Transform back to original coordinate system
                float cx = cos(phi) * cx_prime - sin(phi) * cy_prime + (currentPoint.X + end.X) / 2.0f;
                float cy = sin(phi) * cx_prime + cos(phi) * cy_prime + (currentPoint.Y + end.Y) / 2.0f;
                
                // For GDI+, use AddArc with bounding rectangle
                float left = cx - rx;
                float top = cy - ry;
                float width = 2 * rx;
                float height = 2 * ry;
                
                // Calculate start and sweep angles
                float startAngle = atan2((currentPoint.Y - cy) / ry, (currentPoint.X - cx) / rx) * 180.0f / 3.14159f;
                float endAngle = atan2((end.Y - cy) / ry, (end.X - cx) / rx) * 180.0f / 3.14159f;
                
                float sweepAngle = endAngle - startAngle;
                if (sweepFlag == 0 && sweepAngle > 0) sweepAngle -= 360;
                if (sweepFlag == 1 && sweepAngle < 0) sweepAngle += 360;
                
                // Add arc to path
                gp.AddArc(left, top, width, height, startAngle, sweepAngle);
            } else {
                // Fallback to line if arc parameters are invalid
                gp.AddLine(currentPoint, end);
            }
            
            currentPoint = end;
            prevCommand = upperCmd;
            break;
        }
        
        // Update prevCommand for next iteration if not set by specific command
        if (prevCommand != upperCmd) {
            prevCommand = upperCmd;
        }
    }

    // Fill and stroke with proper namespace
    Gdiplus::SolidBrush fillBrush(Gdiplus::Color(
        static_cast<BYTE>(style.fillColor.GetA()),
        style.fillColor.GetR(), style.fillColor.GetG(), style.fillColor.GetB()));

    Gdiplus::Pen strokePen(Gdiplus::Color(
        static_cast<BYTE>(style.strokeColor.GetA()),
        style.strokeColor.GetR(), style.strokeColor.GetG(), style.strokeColor.GetB()),
        style.strokeWidth);

    // Use alpha channel to check if fill/stroke should be rendered
    if (style.fillColor.GetA() > 0) {
        graphics->FillPath(&fillBrush, &gp);
    }
    if (style.strokeColor.GetA() > 0 && style.strokeWidth > 0) {
        graphics->DrawPath(&strokePen, &gp);
    }
    
    // Restore graphics state
    graphics->Restore(state);
    }
}