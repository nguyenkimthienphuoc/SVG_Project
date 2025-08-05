#ifndef SVGVISITOR_H
#define SVGVISITOR_H

class SVGCircle;
class SVGRect;
class SVGLine;
class SVGText;
class SVGEllipse;
class SVGPath;
class SVGPolygon;
class SVGPolyline;
class SVGGroup;

class SVGVisitor
{
public:
    virtual void visit(SVGCircle *circle) = 0;
    virtual void visit(SVGRect *rectangle) = 0;
    virtual void visit(SVGLine *line) = 0;
    virtual void visit(SVGText *text) = 0;
    virtual void visit(SVGEllipse *ellipse) = 0;
    virtual void visit(SVGPath *path) = 0;
    virtual void visit(SVGPolygon* polygon) = 0;
    virtual void visit(SVGPolyline* polyline) = 0;
	virtual void visit(SVGGroup* group) = 0;
};

#endif