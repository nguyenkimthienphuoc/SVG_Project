#ifndef SVGSCALE_H
#define SVGSCALE_H
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
class SVGScaleByTimes : public SVGVisitor
{
private:
    float times;

public:
    SVGScaleByTimes (float times) : times(times){}

    void visit(SVGCircle *circle) override;
    void visit(SVGRect *rectangle) override;
    void visit(SVGLine *line) override;
    void visit(SVGText *text) override;
    void visit(SVGEllipse *ellipse) override;
    void visit(SVGPath *path) override;
    void visit(SVGPolygon *polygon) override;
    void visit(SVGPolyline *polyline) override;
    void visit(SVGGroup *group) override;
};

class SVGScaleByXY : public SVGVisitor
{
private:
    float x;
    float y;

public:
    SVGScaleByXY (float x, float y) : x(x), y(y){}

    void visit(SVGCircle *circle) override;
    void visit(SVGRect *rectangle) override;
    void visit(SVGLine *line) override;
    void visit(SVGText *text) override;
    void visit(SVGEllipse *ellipse) override;
    void visit(SVGPath *path) override;
    void visit(SVGPolygon *polygon) override;
    void visit(SVGPolyline *polyline) override;
    void visit(SVGGroup *group) override;
};

#endif