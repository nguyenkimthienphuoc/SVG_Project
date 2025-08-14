#include "stdafx.h"
#include "SVGTranslate.h"
#include "SVGGroup.h"

void SVGTranslate::visit(SVGCircle *circle)
{
    // Apply translation using matrix transform
    Gdiplus::Matrix translateMatrix;
    translateMatrix.Translate(tx, ty);
    
    // Apply the translation transform to the element
    circle->applyTransform(translateMatrix);
    
    std::cout << "Applied matrix translate(" << tx << ", " << ty << ") to circle" << std::endl;
}

void SVGTranslate::visit(SVGRect *rectangle)
{
    // Apply translation using matrix transform
    Gdiplus::Matrix translateMatrix;
    translateMatrix.Translate(tx, ty);
    
    // Apply the translation transform to the element
    rectangle->applyTransform(translateMatrix);
    
    std::cout << "Applied matrix translate(" << tx << ", " << ty << ") to rectangle" << std::endl;
}

void SVGTranslate::visit(SVGLine *line)
{
    // Apply translation using matrix transform
    Gdiplus::Matrix translateMatrix;
    translateMatrix.Translate(tx, ty);
    
    // Apply the translation transform to the element
    line->applyTransform(translateMatrix);
    
    std::cout << "Applied matrix translate(" << tx << ", " << ty << ") to line" << std::endl;
}

void SVGTranslate::visit(SVGText *text)
{
    // Apply translation using matrix transform
    Gdiplus::Matrix translateMatrix;
    translateMatrix.Translate(tx, ty);
    
    // Apply the translation transform to the element
    text->applyTransform(translateMatrix);
    
    std::cout << "Applied matrix translate(" << tx << ", " << ty << ") to text" << std::endl;
}

void SVGTranslate::visit(SVGEllipse *ellipse)
{
    // Apply translation using matrix transform
    Gdiplus::Matrix translateMatrix;
    translateMatrix.Translate(tx, ty);
    
    // Apply the translation transform to the element
    ellipse->applyTransform(translateMatrix);
    
    std::cout << "Applied matrix translate(" << tx << ", " << ty << ") to ellipse" << std::endl;
}

void SVGTranslate::visit(SVGPath *path)
{
    // Apply translation using matrix transform
    Gdiplus::Matrix translateMatrix;
    translateMatrix.Translate(tx, ty);
    
    // Apply the translation transform to the element
    path->applyTransform(translateMatrix);
    
    std::cout << "Applied matrix translate(" << tx << ", " << ty << ") to path" << std::endl;
}

void SVGTranslate::visit(SVGPolygon *polygon)
{
    // Apply translation using matrix transform
    Gdiplus::Matrix translateMatrix;
    translateMatrix.Translate(tx, ty);
    
    // Apply the translation transform to the element
    polygon->applyTransform(translateMatrix);
    
    std::cout << "Applied matrix translate(" << tx << ", " << ty << ") to polygon" << std::endl;
}

void SVGTranslate::visit(SVGPolyline *polyline)
{
    // Apply translation using matrix transform
    Gdiplus::Matrix translateMatrix;
    translateMatrix.Translate(tx, ty);
    
    // Apply the translation transform to the element
    polyline->applyTransform(translateMatrix);
    
    std::cout << "Applied matrix translate(" << tx << ", " << ty << ") to polyline" << std::endl;
}

void SVGTranslate::visit(SVGGroup *group)
{   
    // Apply translation using matrix transform
    Gdiplus::Matrix translateMatrix;
    translateMatrix.Translate(tx, ty);

    // Apply the translation transform to the element
    group->applyTransform(translateMatrix);

    //// Apply translation to all children in the group
    //for (auto child : group->getChildren()) {
    //    child->accept(this);
    //}
}
