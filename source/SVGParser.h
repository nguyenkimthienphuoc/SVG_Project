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

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <gdiplus.h>
#include <regex>

using namespace Gdiplus;

std::string readSVGFile(const std::string& filePath);

std::string extractAttr(const std::string& tag, const std::string& attrName);

// class SVGParser
class SVGParser
{
private:
   std::string SVG_Raw_Data;
   std::string heightSVG, widthSVG;
   std::vector<SVGElement *> elements;

public:
   // Constructor and Destructor
   explicit SVGParser(const std::string &filePath);
   ~SVGParser();

   // Methods
   void parseSVG();
   void parseHeader();
   Color parseColor(const std::string& colorStr);
   PaintStyle parsePaintStyle(const std::string &tag);
   TextPaintStyle parseTextStyle (const std::string& tag);
   std::vector<PointF> parsePoints(const std::string& pointStr);
   void parseElements(const std::string &tag);
   SVGElement *createElementFromTag(const std::string &tag);

   // Getters
   std::vector<SVGElement*> getElements() const;
   std::string getHeight() const;
   std::string getWidth() const;
   std::string getRawData() const;
};
#endif