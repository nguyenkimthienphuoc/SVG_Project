#ifndef SVGROTATE_H
#define SVGROTATE_H
#include "SVGVisitor.h"
#include "SVGCircle.h"
#include "SVGRect.h"
#include "SVGLine.h"
#include "SVGText.h"
#include "SVGEllipse.h"
#include "SVGPath.h"
#include "SVGPolygon.h"
#include "SVGPolyline.h"
#include "SVGElement.h"
#include "SVGBasics.h"
#include <cmath>
class SVGRotate : public SVGVisitor
{
private:
    float degree;

public:
    SVGRotate(float degree) : degree(degree) {}

    void visit(SVGCircle *circle) override;
    void visit(SVGRect *rectangle) override;
    void visit(SVGLine *line) override;
    void visit(SVGText *text) override;
    void visit(SVGEllipse *ellipse) override;
    // void visit(SVGPath *path) override;
    void visit(SVGPolygon *polygon) override;
    void visit(SVGPolyline *polyline) override;
};
#endif