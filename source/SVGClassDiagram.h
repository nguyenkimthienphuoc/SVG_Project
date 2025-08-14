#pragma once
// =============================================================================
// SVG PROJECT - CONSOLIDATED CLASS DEFINITIONS FOR CLASS DIAGRAM
// =============================================================================
// This file contains all class definitions from the SVG project
// for the purpose of creating class diagrams
// =============================================================================

#include <string>
#include <vector>
#include <stack>
#include <windows.h>
#include <gdiplus.h>
#include "../rapidxml/rapidxml.hpp"

using namespace Gdiplus;

// =============================================================================
// 1. BASIC STRUCTURES AND STYLES
// =============================================================================

struct PaintStyle {
    Gdiplus::Color strokeColor = Gdiplus::Color(255, 0, 0, 0);  // black, fully opaque
    Gdiplus::Color fillColor = Gdiplus::Color(255, 0, 0, 0);    // black, fully opaque
    float strokeWidth = 1.0f;
    float strokeOpacity = 1.0f;
    float fillOpacity = 1.0f;
};

struct TextPaintStyle {
    // Text color
    Gdiplus::Color fillColor = Gdiplus::Color(255, 0, 0, 0);    // default: black, opaque
    float fillOpacity = 1.0f;

    // Text stroke (rarely used but SVG supports it)
    Gdiplus::Color strokeColor = Color(0, 0, 0, 0);             // default: no stroke
    float strokeWidth = 0.0f;
    float strokeOpacity = 1.0f;

    // Font properties
    std::string fontFamily = "Arial";          // default font
    std::string fontWeight = "normal";         // normal | bold | lighter | 100–900
    std::string fontStyle = "normal";          // normal | italic | oblique
    float fontSize = 16.0f;                    // default font size

    // Text alignment
    std::string textAnchor = "start";          // start | middle | end

    // Text positioning offsets
    float dx = 0.0f;                           // horizontal offset
    float dy = 0.0f;                           // vertical offset

    // Other options
    float opacity = 1.0f;                      // overall transparency
};

// =============================================================================
// 2. VISITOR PATTERN - ABSTRACT VISITOR
// =============================================================================

// Forward declarations for visitor pattern
class SVGCircle;
class SVGRect;
class SVGLine;
class SVGText;
class SVGEllipse;
class SVGPath;
class SVGPolygon;
class SVGPolyline;
class SVGGroup;

class SVGVisitor {
public:
    virtual ~SVGVisitor() = default;
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

// =============================================================================
// 3. ABSTRACT BASE CLASS - SVG ELEMENT
// =============================================================================

class SVGElement {
protected:
    Gdiplus::Matrix transformMatrix;
public:
    PaintStyle style;

    SVGElement() = default;
    virtual ~SVGElement() = default;

    // Pure virtual methods
    virtual void draw(Graphics* graphics) const = 0;
    virtual void accept(SVGVisitor* visitor) = 0;

    // Transform operations
    virtual void applyTransform(const Gdiplus::Matrix& m) {
        transformMatrix.Multiply(&m, Gdiplus::MatrixOrderAppend);
    }

    void resetTransform() {
        transformMatrix.Reset();
    }

    const Gdiplus::Matrix& getTransform() const {
        return transformMatrix;
    }
};

// =============================================================================
// 4. CONCRETE SVG SHAPE CLASSES
// =============================================================================

class SVGCircle : public SVGElement {
public:
    PointF center;
    float radius = 0;

    SVGCircle(PointF center, float radius, const PaintStyle &s);
    void draw(Graphics* graphics) const override;
    void accept(SVGVisitor* visitor) override;
   
    // Getters and Setters
    PointF getCenter() const { return center; }
    float getRadius() const { return radius; }
    void setCenter(PointF newCenter) { center = newCenter; }
    void setRadius(float newRadius) { radius = newRadius; }
};

class SVGRect : public SVGElement {
public:
    PointF topLeft;
    REAL width;
    REAL height;

    SVGRect(PointF topLeft, REAL width, REAL height, const PaintStyle& s);
    void draw(Graphics* graphics) const override;
    void accept(SVGVisitor* visitor) override;
    
    // Getters and Setters
    PointF getTopLeft() const { return topLeft; }
    REAL getWidth() const { return width; }
    REAL getHeight() const { return height; }
    void setTopLeft(PointF newTopLeft) { topLeft = newTopLeft; }
    void setWidth(REAL newWidth) { width = newWidth; }
    void setHeight(REAL newHeight) { height = newHeight; }
};

class SVGLine : public SVGElement {
private:
    float x1, x2, y1, y2;

public:
    SVGLine(float x1, float x2, float y1, float y2, const PaintStyle &s);
    void draw(Gdiplus::Graphics* graphics) const override;
    void accept(SVGVisitor* visitor) override;

    // Getters and Setters
    float getter_x1() const { return x1; }
    float getter_x2() const { return x2; }
    float getter_y1() const { return y1; }
    float getter_y2() const { return y2; }
    void setStart(float x, float y) { x1 = x; y1 = y; }
    void setEnd(float x, float y) { x2 = x; y2 = y; }
};

class SVGEllipse : public SVGElement {
private:
    float cx, cy;   // Ellipse center
    float rx, ry;   // X and Y axis radii

public:
    SVGEllipse(float cx, float cy, float rx, float ry, const PaintStyle& style);
    void draw(Graphics* graphics) const override;
    void accept(SVGVisitor* visitor) override;

    // Getters and Setters
    void setCenter(float cx, float cy);
    void setRadius(float rx, float ry);
    float getCx() const;
    float getCy() const;
    float getRx() const;
    float getRy() const;
};

class SVGPath : public SVGElement {
private:
    std::string pathData;

public:
    SVGPath() = default;
    SVGPath(const std::string& data, const PaintStyle& style);
    void draw(Graphics *graphics) const override;
    void accept(SVGVisitor* visitor) override;

    // Getters and Setters
    void setRawData(const std::string& raw) { pathData = raw; }
    const std::string& getPathData() const { return pathData; }
};

class SVGPolygon : public SVGElement {
private:
    std::vector<PointF> points;
public:
    SVGPolygon() = default;
    SVGPolygon(const std::vector<PointF>& pts, const PaintStyle& style);
    void draw(Graphics* graphics) const override;
    void accept(SVGVisitor* visitor) override;

    // Getters and Setters
    void addPoint(const PointF& point);
    const std::vector<PointF>& getPoints() const { return points; }
    void setPoints(std::vector<PointF> points) { this->points = points; }
};

class SVGPolyline : public SVGElement {
private:
    std::vector<PointF> points;
public:
    SVGPolyline() = default;
    SVGPolyline(const std::vector<PointF>& pts, const PaintStyle& style);
    void draw(Graphics* graphics) const override;
    void accept(SVGVisitor* visitor) override;

    // Getters and Setters
    void addPoint(const PointF& point);
    const std::vector<PointF>& getPoints() const { return points; }
    void setPoints(std::vector<PointF> points) { this->points = points; }
};

class SVGText : public SVGElement {
private:
    std::wstring textContent;   // Text content
    Gdiplus::PointF startPoint; // Start Point
    float fontSize;             // Font size
    TextPaintStyle textStyle;   // Text style

public:
    SVGText(const std::wstring& textContent, Gdiplus::PointF startPoint, 
            const TextPaintStyle& style, float fontSize);
    void draw(Graphics* graphics) const override;
    void accept(SVGVisitor* visitor) override;

    // Getters and Setters
    void setPosition(float x, float y);
    void setText(const std::wstring& content);
    void setFontSize(float size);
    std::wstring getText() const;
    Gdiplus::PointF getStartPoint() const { return startPoint; }
    float getSize() const { return fontSize; }
    void setStart(Gdiplus::PointF newStartPoint) { startPoint = newStartPoint; }
};

class SVGGroup : public SVGElement {
private:
    std::vector<SVGElement*> children;

public:
    SVGGroup(const PaintStyle& style);
    ~SVGGroup();
    void draw(Graphics* graphics) const override;
    void accept(SVGVisitor* visitor) override;

    // Group operations
    void addChild(SVGElement* child);
    const std::vector<SVGElement*>& getChildren() const;
};

// =============================================================================
// 5. CONCRETE VISITOR CLASSES (TRANSFORMATION OPERATIONS)
// =============================================================================

class SVGTranslate : public SVGVisitor {
private:
    float tx, ty;   // Translation values

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

class SVGRotate : public SVGVisitor {
private:
    float degree;   // Rotation angle in degrees

public:
    SVGRotate(float degree) : degree(degree) {}

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

class SVGScaleByTimes : public SVGVisitor {
private:
    float times;    // Uniform scaling factor

public:
    SVGScaleByTimes(float times) : times(times) {}

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

class SVGScaleByXY : public SVGVisitor {
private:
    float x, y;     // Scaling factors for X and Y axes

public:
    SVGScaleByXY(float x, float y) : x(x), y(y) {}

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

// =============================================================================
// 6. SVG PARSER CLASS
// =============================================================================

class SVGParser {
private:
    std::string SVG_Raw_Data;
    std::string heightSVG, widthSVG;
    std::vector<SVGElement*> elements;
    rapidxml::xml_document<> doc;
    char* xmlData; // Store XML data for RapidXML
    std::stack<PaintStyle> paintStyleStack;
    std::stack<TextPaintStyle> textStyleStack;

public:
    // Constructor and Destructor
    explicit SVGParser(const std::string &filePath);
    ~SVGParser();

    // Main parsing methods
    void parseSVG();
    void parseHeader();
    void parseNode(rapidxml::xml_node<>* node);

    // Style and attribute parsing
    Color parseColor(const std::string& colorStr);
    PaintStyle parsePaintStyle(rapidxml::xml_node<>* node);
    TextPaintStyle parseTextStyle(rapidxml::xml_node<>* node);
    std::vector<PointF> parsePoints(const std::string& pointStr);
    void parseTransform(SVGElement* element, const std::string& transformStr);

    // Element creation methods
    SVGElement* createElementFromNode(rapidxml::xml_node<>* node);
    SVGElement* createElementFromNodeWithStyle(rapidxml::xml_node<>* node, 
                                              const PaintStyle& inheritedStyle);
    SVGElement* createTextFromNodeWithStyle(rapidxml::xml_node<>* node, 
                                           const TextPaintStyle& inheritedTextStyle);

    // Getters
    std::vector<SVGElement*> getElements() const;
    std::string getHeight() const;
    std::string getWidth() const;
    std::string getRawData() const;
};

// =============================================================================
// 7. UTILITY FUNCTIONS
// =============================================================================

// File operations
std::string readSVGFile(const std::string& filePath);

// XML attribute extraction
std::string extractAttr(rapidxml::xml_node<>* node, const std::string& attrName);
std::wstring extractText(rapidxml::xml_node<>* node);

// Style merging functions
PaintStyle mergePaintStyle(const PaintStyle& parent, const PaintStyle& child);
TextPaintStyle mergeTextStyle(const TextPaintStyle& parent, const TextPaintStyle& child);

// =============================================================================
// CLASS HIERARCHY SUMMARY FOR DIAGRAM CREATION:
// =============================================================================
/*
1. INHERITANCE HIERARCHY:
   - SVGElement (Abstract Base Class)
     ├── SVGCircle
     ├── SVGRect
     ├── SVGLine
     ├── SVGEllipse
     ├── SVGPath
     ├── SVGPolygon
     ├── SVGPolyline
     ├── SVGText
     └── SVGGroup

2. VISITOR PATTERN:
   - SVGVisitor (Abstract Visitor)
     ├── SVGTranslate
     ├── SVGRotate
     ├── SVGScaleByTimes
     └── SVGScaleByXY

3. COMPOSITION RELATIONSHIPS:
   - SVGElement HAS-A PaintStyle
   - SVGText HAS-A TextPaintStyle
   - SVGGroup HAS-A vector<SVGElement*> (children)
   - SVGParser HAS-A vector<SVGElement*> (elements)
   - SVGParser HAS-A stack<PaintStyle> (paintStyleStack)
   - SVGParser HAS-A stack<TextPaintStyle> (textStyleStack)

4. KEY DESIGN PATTERNS USED:
   - Visitor Pattern: For transformation operations
   - Composite Pattern: SVGGroup can contain other SVGElements
   - Factory Pattern: SVGParser creates different types of SVGElements
   - Template Method: SVGElement defines the interface, concrete classes implement draw()

5. DEPENDENCIES:
   - All SVGElement subclasses depend on SVGVisitor for accept() method
   - SVGParser depends on rapidxml for XML parsing
   - All classes depend on Gdiplus for graphics operations
*/
