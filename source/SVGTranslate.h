#ifndef SVGTRANSLATE_H
#define SVGTRANSLATE_H

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

class SVGTranslate : public SVGVisitor
{
private:
    float tx, ty;

public:
    SVGTranslate(float tx, float ty) : tx(tx), ty(ty) {}

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
