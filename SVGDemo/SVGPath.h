#ifndef SVGPATH_H
#define SVGPATH_H
#include <string>
#include <vector>
#include "SVGElement.h"

using namespace std;

class SVGPath : public SVGElement
{
private:
    string path;

public:
    SVGPath() = default;

    void moveTo(float x, float y);
    void lineTo(float x, float y);
    void curveTo(float x1, float y1, float x2, float y2, float x, float y);
    void closePath();
    void setRawData(const std::string &raw);
    const std::string &getPathData() const;
    void draw(Graphics *graphics) const override;
};
#endif