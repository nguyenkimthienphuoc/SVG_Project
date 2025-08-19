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

    std::cout << "Processing path: " << pathData << std::endl;

    // Better preprocessing to handle negative numbers
    std::string data = pathData;
    
    // First, add spaces around commands
    for (char c : "MmLlHhVvCcSsQqTtAaZz") {
        for (size_t pos = 0; pos < data.length(); pos++) {
            if (data[pos] == c) {
                // Insert space before if not already there
                if (pos > 0 && data[pos-1] != ' ') {
                    data.insert(pos, " ");
                    pos++;
                }
                // Insert space after if not already there
                if (pos + 1 < data.length() && data[pos+1] != ' ') {
                    data.insert(pos + 1, " ");
                    pos++;
                }
            }
        }
    }
    
    // Handle negative numbers by adding spaces before '-' when it's not at start of number
    for (size_t pos = 1; pos < data.length(); pos++) {
        if (data[pos] == '-' && pos > 0) {
            char prev = data[pos-1];
            // Add space before '-' if previous char is a digit (not space, not start of string)
            if (isdigit(prev)) {
                data.insert(pos, " ");
                pos++; // Skip the space we just inserted
            }
        }
    }
    
    // Replace commas with spaces
    std::replace(data.begin(), data.end(), ',', ' ');
    
    std::cout << "Processed data: '" << data << "'" << std::endl;

    Gdiplus::GraphicsPath gp;
    std::istringstream ss(data);
    std::string token;
    
    Gdiplus::PointF currentPoint(0, 0);
    Gdiplus::PointF startPoint(0, 0);
    char currentCommand = 0;
    
    // Parse tokens sequentially
    while (ss >> token) {
        std::cout << "Token: '" << token << "'" << std::endl;
        
        // Check if it's a command
        if (token.length() == 1 && std::string("MmLlHhVvCcSsQqTtAaZz").find(token[0]) != std::string::npos) {
            currentCommand = token[0];
            std::cout << "Command: " << currentCommand << std::endl;
            continue;
        }
        
        // Parse coordinates based on current command
        try {
            bool isRelative = islower(currentCommand);
            char upperCmd = toupper(currentCommand);
            
            switch (upperCmd) {
                case 'M': { // MoveTo
                    float x = std::stof(token);
                    std::string nextToken;
                    if (ss >> nextToken) {
                        float y = std::stof(nextToken);
                        currentPoint = isRelative ? 
                            Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : 
                            Gdiplus::PointF(x, y);
                        startPoint = currentPoint;
                        gp.StartFigure();
                        std::cout << "Move to: (" << currentPoint.X << ", " << currentPoint.Y << ")" << std::endl;
                        
                        // After MoveTo, subsequent pairs are LineTo
                        currentCommand = isRelative ? 'l' : 'L';
                    }
                    break;
                }
                
                case 'L': { // LineTo
                    float x = std::stof(token);
                    std::string nextToken;
                    if (ss >> nextToken) {
                        float y = std::stof(nextToken);
                        Gdiplus::PointF nextPoint = isRelative ? 
                            Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : 
                            Gdiplus::PointF(x, y);
                        gp.AddLine(currentPoint, nextPoint);
                        currentPoint = nextPoint;
                        std::cout << "Line to: (" << currentPoint.X << ", " << currentPoint.Y << ")" << std::endl;
                    }
                    break;
                }
                
                case 'H': { // Horizontal LineTo
                    float x = std::stof(token);
                    Gdiplus::PointF nextPoint = isRelative ? 
                        Gdiplus::PointF(currentPoint.X + x, currentPoint.Y) : 
                        Gdiplus::PointF(x, currentPoint.Y);
                    gp.AddLine(currentPoint, nextPoint);
                    currentPoint = nextPoint;
                    std::cout << "Horizontal line to: (" << currentPoint.X << ", " << currentPoint.Y << ")" << std::endl;
                    break;
                }
                
                case 'V': { // Vertical LineTo
                    float y = std::stof(token);
                    Gdiplus::PointF nextPoint = isRelative ? 
                        Gdiplus::PointF(currentPoint.X, currentPoint.Y + y) : 
                        Gdiplus::PointF(currentPoint.X, y);
                    gp.AddLine(currentPoint, nextPoint);
                    currentPoint = nextPoint;
                    std::cout << "Vertical line to: (" << currentPoint.X << ", " << currentPoint.Y << ")" << std::endl;
                    break;
                }
                
                case 'C': { // Cubic Bezier
                    float x1 = std::stof(token);
                    std::string t2, t3, t4, t5, t6;
                    if (ss >> t2 >> t3 >> t4 >> t5 >> t6) {
                        float y1 = std::stof(t2);
                        float x2 = std::stof(t3);
                        float y2 = std::stof(t4);
                        float x = std::stof(t5);
                        float y = std::stof(t6);
                        
                        Gdiplus::PointF p1 = isRelative ? 
                            Gdiplus::PointF(currentPoint.X + x1, currentPoint.Y + y1) : 
                            Gdiplus::PointF(x1, y1);
                        Gdiplus::PointF p2 = isRelative ? 
                            Gdiplus::PointF(currentPoint.X + x2, currentPoint.Y + y2) : 
                            Gdiplus::PointF(x2, y2);
                        Gdiplus::PointF end = isRelative ? 
                            Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : 
                            Gdiplus::PointF(x, y);

                        gp.AddBezier(currentPoint, p1, p2, end);
                        currentPoint = end;
                        std::cout << "Cubic bezier to: (" << currentPoint.X << ", " << currentPoint.Y << ")" << std::endl;
                    }
                    break;
                }
                
                case 'Z': { // ClosePath
                    gp.CloseFigure();
                    currentPoint = startPoint;
                    std::cout << "Close path" << std::endl;
                    break;
                }
            }
        } catch (...) {
            std::cout << "Failed to parse: " << token << std::endl;
        }
    }

    auto state = graphics->Save();
    graphics->MultiplyTransform(&getTransform());

    // Bounds theo local coords (trước transform vì transform đã áp vào Graphics)
    Gdiplus::RectF bb;
    gp.GetBounds(&bb);

    // FILL
    if (!style.fillNone && style.fillOpacity > 0.0f) {
        if (gradientRegistry && !style.fillUrlId.empty()) {
            auto brush = gradientRegistry->makeBrush(style.fillUrlId, bb);
            if (brush) {
                graphics->FillPath(brush.get(), &gp);
            }
        }
        else if (style.fillColor.GetA() > 0) {
            SolidBrush fillBrush(Color(
                static_cast<BYTE>(style.fillOpacity * style.fillColor.GetA()),
                style.fillColor.GetR(), style.fillColor.GetG(), style.fillColor.GetB()));
            graphics->FillPath(&fillBrush, &gp);
        }
    }

    // STROKE giữ nguyên
    if (!style.strokeNone && style.strokeColor.GetA() > 0 && style.strokeOpacity > 0 && style.strokeWidth > 0) {
        Pen strokePen(Color(
            static_cast<BYTE>(style.strokeOpacity * style.strokeColor.GetA()),
            style.strokeColor.GetR(), style.strokeColor.GetG(), style.strokeColor.GetB()),
            style.strokeWidth);
        graphics->DrawPath(&strokePen, &gp);
    }

    graphics->Restore(state);

    
    std::cout << "Path rendering completed" << std::endl;
}