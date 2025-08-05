#ifndef SVGPARSER_H
#define SVGPARSER_H

#include "SVGBasics.h"
#include "SVGElement.h"
#include "SVGCircle.h"
#include "SVGRect.h"
#include "SVGLine.h"
#include "SVGText.h"
#include "SVGEllipse.h"
#include "SVGPath.h"
#include "SVGPolygon.h"
#include "SVGPolyline.h"
#include "../rapidxml/rapidxml.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <gdiplus.h>
#include <cstring>
#include <stack>

using namespace Gdiplus;

std::string readSVGFile(const std::string& filePath);

std::string extractAttr(rapidxml::xml_node<>* node, const std::string& attrName);
std::wstring extractText(rapidxml::xml_node<>* node);

// class SVGParser
class SVGParser
{
private:
   std::string SVG_Raw_Data;
   std::string heightSVG, widthSVG;
   std::vector<SVGElement *> elements;
   rapidxml::xml_document<> doc;
   char* xmlData; // Store XML data for RapidXML
   std::stack<PaintStyle> paintStyleStack;

public:
   // Constructor and Destructor
   explicit SVGParser(const std::string &filePath);
   ~SVGParser();

   // Methods
   void parseSVG();
   void parseHeader();
   void parseNode(rapidxml::xml_node<>* node);
   Color parseColor(const std::string& colorStr);
   PaintStyle parsePaintStyle(rapidxml::xml_node<>* node);
   TextPaintStyle parseTextStyle(rapidxml::xml_node<>* node);
   std::vector<PointF> parsePoints(const std::string& pointStr);
   void parseTransform(SVGElement* element, const std::string& transformStr);
   SVGElement *createElementFromNode(rapidxml::xml_node<>* node);
   SVGElement* createElementFromNodeWithStyle(rapidxml::xml_node<>* node, const PaintStyle& inheritedStyle);

   // Getters
   std::vector<SVGElement*> getElements() const;
   std::string getHeight() const;
   std::string getWidth() const;
   std::string getRawData() const;
};
#endif