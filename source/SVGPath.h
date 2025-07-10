#ifndef SVGPATH_H
#define SVGPATH_H
#include <string>
#include <vector>
#include "SVGElement.h"

using namespace std;

class SVGPath : public SVGElement
{
private:
    std::string pathData;

public:
    SVGPath() = default;
    SVGPath(const std::string& data, const PaintStyle& style);

    /*void moveTo(float x, float y);
    void lineTo(float x, float y);
    void curveTo(float x1, float y1, float x2, float y2, float x, float y);
    void closePath();*/
    void setRawData(const std::string& raw) { pathData = raw; }
    const std::string& getPathData() const { return pathData; };
    void draw(Graphics *graphics) const override;
};
#endif