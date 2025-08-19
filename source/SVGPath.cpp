#include "stdafx.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <array>
#include <cmath>
#include <cctype>


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

    // Tokenize path data robustly (commands + numbers including exponents)
    std::string s = pathData;
    std::vector<std::string> tokens;
    auto isCommandLetter = [](char c) {
        return std::string("MmLlHhVvCcSsQqTtAaZz").find(c) != std::string::npos;
    };

    size_t i = 0;
    while (i < s.size()) {
        char c = s[i];
        if (isCommandLetter(c)) {
            tokens.emplace_back(1, c);
            ++i;
        } else if (std::isspace((unsigned char)c) || c == ',') {
            ++i;
        } else {
            // parse number: [+-]?(
            //    (\n+            //      digits(.digits)? | .digits
            //    )([eE][+-]?digits)?
            size_t start = i;
            // sign
            if (s[i] == '+' || s[i] == '-') ++i;
            bool hasDigits = false;
            while (i < s.size() && std::isdigit((unsigned char)s[i])) { ++i; hasDigits = true; }
            if (i < s.size() && s[i] == '.') {
                ++i;
                while (i < s.size() && std::isdigit((unsigned char)s[i])) { ++i; hasDigits = true; }
            }
            if (!hasDigits && start < s.size() && s[start] == '.') {
                // .5 style
                i = start + 1;
                while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
            }
            // exponent
            if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
                size_t epos = i;
                ++i;
                if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
                bool expDigits = false;
                while (i < s.size() && std::isdigit((unsigned char)s[i])) { ++i; expDigits = true; }
                if (!expDigits) i = epos; // rollback if malformed
            }
            tokens.emplace_back(s.substr(start, i - start));
        }
    }

    std::cout << "Processed tokens: " << tokens.size() << std::endl;

    Gdiplus::GraphicsPath gp;
    Gdiplus::PointF currentPoint(0, 0);
    Gdiplus::PointF startPoint(0, 0);

    char currentCommand = 0;
    char lastCommand = 0;
    // Track last control points for S/T handling
    Gdiplus::PointF lastCubicCtrl(0,0);
    Gdiplus::PointF lastQuadCtrl(0,0);

    auto reflect = [](const Gdiplus::PointF& origin, const Gdiplus::PointF& point) -> Gdiplus::PointF {
        return Gdiplus::PointF(2*origin.X - point.X, 2*origin.Y - point.Y);
    };

    const double PI = 3.14159265358979323846;

    auto quadToCubic = [&](const Gdiplus::PointF& p0, const Gdiplus::PointF& qc, const Gdiplus::PointF& p1)->std::array<Gdiplus::PointF,3>{
        // convert quadratic (p0, qc, p1) to cubic control points
        float c1x = p0.X + 2.0f/3.0f*(qc.X - p0.X);
        float c1y = p0.Y + 2.0f/3.0f*(qc.Y - p0.Y);
        float c2x = p1.X + 2.0f/3.0f*(qc.X - p1.X);
        float c2y = p1.Y + 2.0f/3.0f*(qc.Y - p1.Y);
        return std::array<Gdiplus::PointF,3>{ Gdiplus::PointF(c1x,c1y), Gdiplus::PointF(c2x,c2y), p1 };
    };

    // arc -> cubic bezier conversion (endpoint to center parameterization + split)
    auto arcToBeziers = [&](double x1, double y1, double rx, double ry, double phiDeg, int largeArcFlag, int sweepFlag, double x2, double y2){
        std::vector<std::array<Gdiplus::PointF,3>> out;
        if (rx == 0.0 || ry == 0.0) {
            out.push_back(std::array<Gdiplus::PointF,3>{ Gdiplus::PointF((float)x1,(float)y1), Gdiplus::PointF((float)x2,(float)y2), Gdiplus::PointF((float)x2,(float)y2) });
            return out;
        }
        double phi = phiDeg * M_PI / 180.0;
        double cosPhi = cos(phi), sinPhi = sin(phi);
        // Step1: compute (x1', y1')
        double dx2 = (x1 - x2) / 2.0;
        double dy2 = (y1 - y2) / 2.0;
        double x1p = cosPhi * dx2 + sinPhi * dy2;
        double y1p = -sinPhi * dx2 + cosPhi * dy2;

        // Ensure radii are large enough
        double rx2 = rx*rx, ry2 = ry*ry, x1p2 = x1p*x1p, y1p2 = y1p*y1p;
        double lam = x1p2/rx2 + y1p2/ry2;
        if (lam > 1.0) {
            double s = sqrt(lam);
            rx *= s; ry *= s; rx2 = rx*rx; ry2 = ry*ry;
        }

        double sign = (largeArcFlag == sweepFlag) ? -1.0 : 1.0;
        double num = rx2*ry2 - rx2*y1p2 - ry2*x1p2;
        double denom = rx2*y1p2 + ry2*x1p2;
        double cc = 0.0;
        if (denom != 0.0) {
            double val = num/denom;
            if (val < 0.0) val = 0.0;
            cc = sign * sqrt(val);
        }
        double cxp = cc * (rx * y1p / ry);
        double cyp = cc * (-ry * x1p / rx);

        // center
        double cx = cosPhi * cxp - sinPhi * cyp + (x1 + x2)/2.0;
        double cy = sinPhi * cxp + cosPhi * cyp + (y1 + y2)/2.0;

        // angles
        auto angle = [](double ux, double uy, double vx, double vy)->double{
            double dot = ux*vx + uy*vy;
            double len = sqrt((ux*ux+uy*uy)*(vx*vx+vy*vy));
            double v = std::clamp(dot/len, -1.0, 1.0);
            double a = acos(v);
            if (ux*vy - uy*vx < 0) a = -a;
            return a;
        };

        double ux = (x1p - cxp)/rx;
        double uy = (y1p - cyp)/ry;
        double vx = (-x1p - cxp)/rx;
        double vy = (-y1p - cyp)/ry;
        double theta1 = angle(1.0, 0.0, ux, uy);
        double delta = angle(ux, uy, vx, vy);
        if (!sweepFlag && delta > 0) delta -= 2*M_PI;
        else if (sweepFlag && delta < 0) delta += 2*M_PI;

        // break into segments <= pi/2
    int segments = (int)ceil(fabs(delta) / (PI/2.0));
    if (segments < 1) segments = 1;
        double deltaPer = delta / segments;

        for (int iSeg = 0; iSeg < segments; ++iSeg) {
            double th1 = theta1 + iSeg * deltaPer;
            double th2 = th1 + deltaPer;
            double t = (4.0/3.0) * tan((th2 - th1)/4.0);

            double cosTh1 = cos(th1), sinTh1 = sin(th1);
            double cosTh2 = cos(th2), sinTh2 = sin(th2);

            // endpoints in original coordinate
            double xk1 = cx + rx * (cosPhi * cosTh1 - sinPhi * sinTh1);
            double yk1 = cy + ry * (sinPhi * cosTh1 + cosPhi * sinTh1);
            double xk2 = cx + rx * (cosPhi * cosTh2 - sinPhi * sinTh2);
            double yk2 = cy + ry * (sinPhi * cosTh2 + cosPhi * sinTh2);

            // control points
            double dx1 = -t * (rx * (cosPhi * sinTh1 + sinPhi * cosTh1));
            double dy1 = -t * (ry * (sinPhi * sinTh1 - cosPhi * cosTh1));
            double dx2 = t * (rx * (cosPhi * sinTh2 + sinPhi * cosTh2));
            double dy2 = t * (ry * (sinPhi * sinTh2 - cosPhi * cosTh2));

            Gdiplus::PointF c1((float)(xk1 + dx1), (float)(yk1 + dy1));
            Gdiplus::PointF c2((float)(xk2 + dx2), (float)(yk2 + dy2));
            Gdiplus::PointF e((float)xk2, (float)yk2);
            out.push_back({ c1, c2, e });
        }

        return out;
    };

    size_t idx = 0;
    while (idx < tokens.size()) {
        std::string tk = tokens[idx++];
        if (tk.size() == 1 && isCommandLetter(tk[0])) {
            currentCommand = tk[0];
        } else {
            // If token is a number but we have no currentCommand, continue
            if (currentCommand == 0) {
                std::cout << "Unexpected parameter without command: " << tk << std::endl;
                continue;
            }
            // put back one
            --idx;
        }

        bool isRelative = std::islower((unsigned char)currentCommand);
        char uc = std::toupper((unsigned char)currentCommand);

        try {
            if (uc == 'M') {
                // MoveTo: pairs of coordinates
                while (idx + 1 < tokens.size() && tokens[idx].size()>0 && tokens[idx+1].size()>0 && !isCommandLetter(tokens[idx][0])) {
                    float x = std::stof(tokens[idx++]);
                    float y = std::stof(tokens[idx++]);
                    Gdiplus::PointF np = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x,y);
                    currentPoint = np; startPoint = np; gp.StartFigure();
                    // After first pair, subsequent are treated as LineTo per spec
                    currentCommand = isRelative ? 'l' : 'L'; uc = std::toupper((unsigned char)currentCommand);
                    break; // leave possible additional coordinate pairs to the L/L handling below
                }
            }

            if (uc == 'L') {
                while (idx + 1 < tokens.size() && !isCommandLetter(tokens[idx][0])) {
                    float x = std::stof(tokens[idx++]);
                    float y = std::stof(tokens[idx++]);
                    Gdiplus::PointF np = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x,y);
                    gp.AddLine(currentPoint, np);
                    currentPoint = np;
                }
            } else if (uc == 'H') {
                while (idx < tokens.size() && !isCommandLetter(tokens[idx][0])) {
                    float x = std::stof(tokens[idx++]);
                    Gdiplus::PointF np = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y) : Gdiplus::PointF(x, currentPoint.Y);
                    gp.AddLine(currentPoint, np); currentPoint = np;
                }
            } else if (uc == 'V') {
                while (idx < tokens.size() && !isCommandLetter(tokens[idx][0])) {
                    float y = std::stof(tokens[idx++]);
                    Gdiplus::PointF np = isRelative ? Gdiplus::PointF(currentPoint.X, currentPoint.Y + y) : Gdiplus::PointF(currentPoint.X, y);
                    gp.AddLine(currentPoint, np); currentPoint = np;
                }
            } else if (uc == 'C') {
                while (idx + 5 < tokens.size() && !isCommandLetter(tokens[idx][0])) {
                    float x1 = std::stof(tokens[idx++]);
                    float y1 = std::stof(tokens[idx++]);
                    float x2 = std::stof(tokens[idx++]);
                    float y2 = std::stof(tokens[idx++]);
                    float x = std::stof(tokens[idx++]);
                    float y = std::stof(tokens[idx++]);
                    Gdiplus::PointF p1 = isRelative ? Gdiplus::PointF(currentPoint.X + x1, currentPoint.Y + y1) : Gdiplus::PointF(x1,y1);
                    Gdiplus::PointF p2 = isRelative ? Gdiplus::PointF(currentPoint.X + x2, currentPoint.Y + y2) : Gdiplus::PointF(x2,y2);
                    Gdiplus::PointF e  = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x,y);
                    gp.AddBezier(currentPoint, p1, p2, e);
                    lastCubicCtrl = p2; // last control is second control
                    currentPoint = e; lastCommand = uc;
                }
            } else if (uc == 'S') {
                while (idx + 3 < tokens.size() && !isCommandLetter(tokens[idx][0])) {
                    float x2 = std::stof(tokens[idx++]);
                    float y2 = std::stof(tokens[idx++]);
                    float x = std::stof(tokens[idx++]);
                    float y = std::stof(tokens[idx++]);
                    Gdiplus::PointF ctrl1;
                    if (lastCommand == 'C' || lastCommand == 'S') ctrl1 = reflect(currentPoint, lastCubicCtrl);
                    else ctrl1 = currentPoint;
                    Gdiplus::PointF p2 = isRelative ? Gdiplus::PointF(currentPoint.X + x2, currentPoint.Y + y2) : Gdiplus::PointF(x2,y2);
                    Gdiplus::PointF e  = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x,y);
                    gp.AddBezier(currentPoint, ctrl1, p2, e);
                    lastCubicCtrl = p2; currentPoint = e; lastCommand = uc;
                }
            } else if (uc == 'Q') {
                while (idx + 3 < tokens.size() && !isCommandLetter(tokens[idx][0])) {
                    float qx = std::stof(tokens[idx++]);
                    float qy = std::stof(tokens[idx++]);
                    float x = std::stof(tokens[idx++]);
                    float y = std::stof(tokens[idx++]);
                    Gdiplus::PointF qc = isRelative ? Gdiplus::PointF(currentPoint.X + qx, currentPoint.Y + qy) : Gdiplus::PointF(qx,qy);
                    Gdiplus::PointF e  = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x,y);
                    auto cub = quadToCubic(currentPoint, qc, e);
                    gp.AddBezier(currentPoint, cub[0], cub[1], cub[2]);
                    lastQuadCtrl = qc; currentPoint = e; lastCommand = uc;
                }
            } else if (uc == 'T') {
                while (idx + 1 < tokens.size() && !isCommandLetter(tokens[idx][0])) {
                    float x = std::stof(tokens[idx++]);
                    float y = std::stof(tokens[idx++]);
                    Gdiplus::PointF qc;
                    if (lastCommand == 'Q' || lastCommand == 'T') qc = reflect(currentPoint, lastQuadCtrl);
                    else qc = currentPoint;
                    Gdiplus::PointF e = isRelative ? Gdiplus::PointF(currentPoint.X + x, currentPoint.Y + y) : Gdiplus::PointF(x,y);
                    auto cub = quadToCubic(currentPoint, qc, e);
                    gp.AddBezier(currentPoint, cub[0], cub[1], cub[2]);
                    lastQuadCtrl = qc; currentPoint = e; lastCommand = uc;
                }
            } else if (uc == 'A') {
                while (idx + 6 < tokens.size() && !isCommandLetter(tokens[idx][0])) {
                    double rx = std::stod(tokens[idx++]);
                    double ry = std::stod(tokens[idx++]);
                    double phi = std::stod(tokens[idx++]);
                    int laf = (int)std::stoi(tokens[idx++]);
                    int sf  = (int)std::stoi(tokens[idx++]);
                    double x = std::stod(tokens[idx++]);
                    double y = std::stod(tokens[idx++]);
                    double x1 = currentPoint.X, y1 = currentPoint.Y;
                    double x2 = isRelative ? currentPoint.X + x : x;
                    double y2 = isRelative ? currentPoint.Y + y : y;
                    auto beziers = arcToBeziers(x1,y1, rx, ry, phi, laf, sf, x2,y2);
                    for (auto &b : beziers) {
                        // b = {c1, c2, end}
                        gp.AddBezier(currentPoint, b[0], b[1], b[2]);
                        currentPoint = b[2];
                    }
                    lastCommand = uc;
                }
            } else if (uc == 'Z') {
                gp.CloseFigure(); currentPoint = startPoint; lastCommand = uc;
            }
        } catch (...) {
            std::cout << "Failed to parse path token sequence at idx=" << idx << std::endl;
            break;
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