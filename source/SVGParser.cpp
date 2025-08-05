#include "stdafx.h"
#include "SVGParser.h"
#include "SVGRect.h"
#include "SVGCircle.h"
#include "SVGLine.h"
#include "SVGBasics.h"
#include "SVGEllipse.h"
#include "SVGText.h"
#include "SVGPolygon.h"
#include "SVGPolyline.h"
#include "SVGPath.h"
#include "../rapidxml/rapidxml.hpp"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>

std::string readSVGFile(const std::string &filePath)
{
	std::ifstream file(filePath);
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str(); // Trả về nội dung file SVG dưới dạng string
}

std::string extractAttr(rapidxml::xml_node<>* node, const std::string& attrName) {
	rapidxml::xml_attribute<>* attr = node->first_attribute(attrName.c_str());
	if (attr) {
		return std::string(attr->value());
	}
	return "";
}

std::wstring extractText(rapidxml::xml_node<>* node) {
	if (node->value()) {
		std::string raw(node->value());
		// Chuyển từ string thành wstring
		std::wstring wideValue(raw.begin(), raw.end());
		return wideValue;
	}
	return L"";
}

SVGParser::SVGParser(const std::string& filePath) : xmlData(nullptr)
{
	std::string data = readSVGFile(filePath);
	this->SVG_Raw_Data = data;

	if (data.empty())
	{
		std::cerr << "Failed to read SVG file." << std::endl;
		exit(1);
	}

	// Parse XML using RapidXML
	try {
		// Create a copy for RapidXML (it modifies the string)
		xmlData = new char[data.size() + 1];
		strcpy_s(xmlData, data.size() + 1, data.c_str());
		
		doc.parse<0>(xmlData);
		parseHeader();
	}
	catch (const rapidxml::parse_error& e) {
		std::cerr << "XML Parse error: " << e.what() << std::endl;
		if (xmlData) {
			delete[] xmlData;
			xmlData = nullptr;
		}
		exit(1);
	}
}

SVGParser::~SVGParser()
{
	for (int i = 0; i < elements.size(); i++)
	{
		delete elements[i];
	}
	elements.clear();
	
	if (xmlData) {
		delete[] xmlData;
		xmlData = nullptr;
	}
}

void SVGParser::parseSVG()
{
	std::cout << "Starting SVG parsing..." << std::endl;
	rapidxml::xml_node<>* svgRoot = doc.first_node("svg");
	if (svgRoot) {
		std::cout << "Found SVG root node" << std::endl;
		// Duyệt qua tất cả các child nodes của SVG
		int nodeCount = 0;
		for (rapidxml::xml_node<>* child = svgRoot->first_node(); child; child = child->next_sibling()) {
			std::cout << "Processing node: " << child->name() << std::endl;
			parseNode(child);
			nodeCount++;
		}
		std::cout << "Processed " << nodeCount << " nodes" << std::endl;
	} else {
		std::cout << "Error: No SVG root node found!" << std::endl;
	}
	std::cout << "Total elements created: " << elements.size() << std::endl;
}

void SVGParser::parseNode(rapidxml::xml_node<>* node)
{
	if (!node) return;
	
	std::cout << "Parsing node: " << node->name() << std::endl;
	
	// Tạo element từ node hiện tại
	SVGElement* element = createElementFromNode(node);
	if (element) {
		this->elements.push_back(element);
		std::cout << "Successfully created element for: " << node->name() << std::endl;
	} else {
		std::cout << "Failed to create element for: " << node->name() << std::endl;
	}
	
	// Đệ quy xử lý các child nodes (hierarchical structure)
	for (rapidxml::xml_node<>* child = node->first_node(); child; child = child->next_sibling()) {
		parseNode(child);
	}
}

void SVGParser::parseHeader()
{
	rapidxml::xml_node<>* svgRoot = doc.first_node("svg");
	if (svgRoot) {
		this->widthSVG = extractAttr(svgRoot, "width");
		this->heightSVG = extractAttr(svgRoot, "height");
	}
}

Color SVGParser::parseColor(const std::string &colorStr)
{
	// 1. Check for rgb(r,g,b) format
	if (colorStr.find("rgb(") == 0) {
		size_t start = colorStr.find('(') + 1;
		size_t end = colorStr.find(')');
		if (start != std::string::npos && end != std::string::npos) {
			std::string rgbValues = colorStr.substr(start, end - start);
			
			// Parse r,g,b values
			std::vector<int> values;
			std::stringstream ss(rgbValues);
			std::string item;
			
			while (std::getline(ss, item, ',')) {
				// Remove spaces
				item.erase(0, item.find_first_not_of(" \t"));
				item.erase(item.find_last_not_of(" \t") + 1);
				values.push_back(std::stoi(item));
			}
			
			if (values.size() == 3) {
				return Color(255, values[0], values[1], values[2]);
			}
		}
	}

	// 2. Check for simple color format
	if (colorStr == "red")
		return Gdiplus::Color(255, 255, 0, 0);
	if (colorStr == "blue")
		return Gdiplus::Color(255, 0, 0, 255);
	if (colorStr == "green")
		return Gdiplus::Color(255, 0, 128, 0);
	if (colorStr == "black")
		return Gdiplus::Color(255, 0, 0, 0);
	if (colorStr == "white")
		return Gdiplus::Color(255, 255, 255, 255);

	return Gdiplus::Color(255, 0, 0, 0);
}

PaintStyle SVGParser::parsePaintStyle(rapidxml::xml_node<>* node)
{
	PaintStyle s;

	if (std::string stroke_str = extractAttr(node, "stroke"); !stroke_str.empty())
	{
		s.strokeColor = parseColor(stroke_str);
	}

	if (std::string fill_str = extractAttr(node, "fill"); !fill_str.empty())
	{
		s.fillColor = parseColor(fill_str);
	}

	if (std::string opacity_str = extractAttr(node, "opacity"); !opacity_str.empty())
	{
		s.strokeOpacity *= std::stof(opacity_str);
		s.fillOpacity *= std::stof(opacity_str);
	}

	if (std::string fillO = extractAttr(node, "fill-opacity"); !fillO.empty())
	{
		s.fillOpacity = std::stof(fillO);
	}

	if (std::string strokeO = extractAttr(node, "stroke-opacity"); !strokeO.empty())
	{
		s.strokeOpacity = std::stof(strokeO);
	}

	if (std::string strokeW = extractAttr(node, "stroke-width"); !strokeW.empty())
	{
		s.strokeWidth = std::stof(strokeW);
	}

	return s;
}

TextPaintStyle SVGParser::parseTextStyle(rapidxml::xml_node<>* node)
{
	TextPaintStyle t;
	if (std::string fill_str = (extractAttr(node, "fill")); !fill_str.empty())
	{
		t.fillColor = parseColor(fill_str);
	}

	if (std::string fillO = (extractAttr(node, "fill-opacity")); !fillO.empty())
	{
		t.fillOpacity = std::stof(fillO);
	}

	if (std::string stroke_str = (extractAttr(node, "stroke")); !stroke_str.empty())
	{
		t.strokeColor = parseColor(stroke_str);
	}

	if (std::string strokeW = (extractAttr(node, "stroke-width")); !strokeW.empty())
	{
		t.strokeWidth = std::stof(strokeW);
	}

	if (std::string strokeO = (extractAttr(node, "stroke-opacity")); !strokeO.empty())
	{
		t.strokeWidth = std::stof(strokeO);
	}

	if (std::string fontFam = (extractAttr(node, "font-family")); !fontFam.empty())
	{
		t.fontFamily = fontFam;
	}

	if (std::string fontW = (extractAttr(node, "font-weight")); !fontW.empty())
	{
		t.fontWeight = fontW;
	}

	if (std::string textA = (extractAttr(node, "text-anchor")); !textA.empty())
	{
		t.textAnchor = textA;
	}

	if (std::string opacity_str = extractAttr(node, "opacity"); !opacity_str.empty())
	{
		t.strokeOpacity *= std::stof(opacity_str);
		t.fillOpacity *= std::stof(opacity_str);
	}

	return t;
}

std::vector<PointF> SVGParser::parsePoints(const std::string &pointStr)
{
	std::cout << "Parsing points string: '" << pointStr << "'" << std::endl;
	std::vector<PointF> points;
	std::vector<float> coords;
	
	// Replace commas with spaces first
	std::string processedStr = pointStr;
	for (char& c : processedStr) {
		if (c == ',') {
			c = ' ';
		}
	}
	
	std::cout << "After comma replacement: '" << processedStr << "'" << std::endl;
	
	std::stringstream ss(processedStr);
	std::string token;
	
	// Parse coordinates separated by spaces
	while (ss >> token) {
		if (!token.empty()) {
			std::cout << "Found coordinate: " << token << std::endl;
			coords.push_back(std::stof(token));
		}
	}
	
	std::cout << "Total coordinates found: " << coords.size() << std::endl;
	
	// Create points from pairs of coordinates
	for (size_t i = 0; i + 1 < coords.size(); i += 2) {
		PointF point(coords[i], coords[i + 1]);
		points.push_back(point);
		std::cout << "Created point: (" << coords[i] << ", " << coords[i + 1] << ")" << std::endl;
	}
	
	std::cout << "Total points created: " << points.size() << std::endl;
	return points;
}


void SVGParser::parseTransform(SVGElement* element, const std::string& transformStr) {
	std::stringstream ss(transformStr);
	std::string token;

	while (std::getline(ss, token, ')')) {
		if (token.find("translate(") != std::string::npos) {
			float tx = 0, ty = 0;
			std::string content = token.substr(token.find('(') + 1);
			std::stringstream vals(content);
			std::string xStr, yStr;

			if (std::getline(vals, xStr, ',') && std::getline(vals, yStr, ',')) {
				tx = std::stof(xStr);
				ty = std::stof(yStr);
			}
			else if (!xStr.empty()) {
				tx = std::stof(xStr);
			}

			Gdiplus::Matrix m;
			m.Translate(tx, ty);
			element->applyTransform(m);
		}
		// TO DO 'scale' and 'rotate'
	}
}

SVGElement *SVGParser::createElementFromNode(rapidxml::xml_node<>* node)
{
	if (!node) return nullptr;
	
	std::string nodeName = node->name();
	std::cout << "Creating element for: " << nodeName << std::endl;
	
	// rectangle
	if (nodeName == "rect")
	{
		PaintStyle s = parsePaintStyle(node);
		std::string xStr = extractAttr(node, "x");
		std::string yStr = extractAttr(node, "y");
		std::string widthStr = extractAttr(node, "width");
		std::string heightStr = extractAttr(node, "height");

		std::cout << "Rect attributes - x:" << xStr << " y:" << yStr
			<< " width:" << widthStr << " height:" << heightStr << std::endl;

		if (!xStr.empty() && !yStr.empty() && !widthStr.empty() && !heightStr.empty()) {
			REAL x = static_cast<REAL>(std::stof(xStr));
			REAL y = static_cast<REAL>(std::stof(yStr));
			REAL width = static_cast<REAL>(std::stof(widthStr));
			REAL height = static_cast<REAL>(std::stof(heightStr));
			PointF topLeft(x, y);

			SVGRect* rect = new SVGRect(topLeft, width, height, s);

			std::string transformStr = extractAttr(node, "transform");
			if (!transformStr.empty()) {
				parseTransform(rect, transformStr);
			}

			std::cout << "Created rect successfully" << std::endl;
			return rect;
		}
	}

	// circle
	else if (nodeName == "circle")
	{
		std::string cxStr = extractAttr(node, "cx");
		std::string cyStr = extractAttr(node, "cy");
		std::string rStr = extractAttr(node, "r");
		
		std::cout << "Circle attributes - cx:" << cxStr << " cy:" << cyStr << " r:" << rStr << std::endl;
		
		if (!cxStr.empty() && !cyStr.empty() && !rStr.empty()) {
			float x = std::stof(cxStr);
			float y = std::stof(cyStr);
			float radius = std::stof(rStr);
			PointF topLeft{ x, y };
			PaintStyle s = parsePaintStyle(node);

			SVGCircle* circle = new SVGCircle(topLeft, radius, s);

			// Áp dụng transform nếu có
			std::string transformStr = extractAttr(node, "transform");
			if (!transformStr.empty()) {
				parseTransform(circle, transformStr);
			}

			std::cout << "Created circle successfully" << std::endl;
			return circle;
		}
	}

	// line
	else if (nodeName == "line")
	{
		std::string x1Str = extractAttr(node, "x1");
		std::string y1Str = extractAttr(node, "y1");
		std::string x2Str = extractAttr(node, "x2");
		std::string y2Str = extractAttr(node, "y2");
		
		std::cout << "Line attributes - x1:" << x1Str << " y1:" << y1Str << " x2:" << x2Str << " y2:" << y2Str << std::endl;
		
		if (!x1Str.empty() && !y1Str.empty() && !x2Str.empty() && !y2Str.empty()) {
			float x1 = std::stof(x1Str);
			float y1 = std::stof(y1Str);
			float x2 = std::stof(x2Str);
			float y2 = std::stof(y2Str);
			PaintStyle s = parsePaintStyle(node);

			SVGLine* line = new SVGLine(x1, x2, y1, y2, s);

			// Áp dụng transform nếu có
			std::string transformStr = extractAttr(node, "transform");
			if (!transformStr.empty()) {
				parseTransform(line, transformStr);
			}

			std::cout << "Created line successfully" << std::endl;
			return line;
		}
	}

	// ellipse
	else if (nodeName == "ellipse")
	{
		std::string cxStr = extractAttr(node, "cx");
		std::string cyStr = extractAttr(node, "cy");
		std::string rxStr = extractAttr(node, "rx");
		std::string ryStr = extractAttr(node, "ry");
		
		std::cout << "Ellipse attributes - cx:" << cxStr << " cy:" << cyStr << " rx:" << rxStr << " ry:" << ryStr << std::endl;
		
		if (!cxStr.empty() && !cyStr.empty() && !rxStr.empty() && !ryStr.empty()) {
			float cx = std::stof(cxStr);
			float cy = std::stof(cyStr);
			float rx = std::stof(rxStr);
			float ry = std::stof(ryStr);
			PaintStyle s = parsePaintStyle(node);

			SVGEllipse* ellipse = new SVGEllipse(cx, cy, rx, ry, s);

			// Áp dụng transform nếu có
			std::string transformStr = extractAttr(node, "transform");
			if (!transformStr.empty()) {
				parseTransform(ellipse, transformStr);
			}

			std::cout << "Created ellipse successfully" << std::endl;
			return ellipse;
		}
	}

	// polygon
	else if (nodeName == "polygon")
	{
		std::string pointStr = extractAttr(node, "points");
		std::cout << "Polygon points: " << pointStr << std::endl;

		if (!pointStr.empty()) {
			std::vector<PointF> points = parsePoints(pointStr);
			std::cout << "Parsed " << points.size() << " points for polygon" << std::endl;

			PaintStyle s = parsePaintStyle(node);

			SVGPolygon* polygon = new SVGPolygon(points, s);

			// Áp dụng transform nếu có
			std::string transformStr = extractAttr(node, "transform");
			if (!transformStr.empty()) {
				parseTransform(polygon, transformStr);
			}

			std::cout << "Created polygon successfully" << std::endl;
			return polygon;
		}
	}

	// polyline
	else if (nodeName == "polyline")
	{
		std::string pointStr = extractAttr(node, "points");
		std::cout << "Polyline points: " << pointStr << std::endl;
		if (!pointStr.empty()) {
			std::vector<PointF> points = parsePoints(pointStr);
			std::cout << "Parsed " << points.size() << " points for polyline" << std::endl;

			PaintStyle s = parsePaintStyle(node);

			SVGPolyline* polyline = new SVGPolyline(points, s);

			// Áp dụng transform nếu có
			std::string transformStr = extractAttr(node, "transform");
			if (!transformStr.empty()) {
				parseTransform(polyline, transformStr);
			}

			std::cout << "Created polyline successfully" << std::endl;
			return polyline;
		}
	}

	// path
	else if (nodeName == "path")
	{
		std::string data = extractAttr(node, "d");
		std::cout << "Path data: " << data << std::endl;

		if (!data.empty())
		{
			PaintStyle style = parsePaintStyle(node);
			SVGPath* path = new SVGPath(data, style);

			std::string transformStr = extractAttr(node, "transform");
			if (!transformStr.empty()) {
				parseTransform(path, transformStr);
			}

			std::cout << "Created path successfully" << std::endl;
			return path;
		}
	}

	// text
	else if (nodeName == "text")
	{
        std::wstring content = extractText(node);
		std::cout << "Text content length: " << content.length() << std::endl;

		if (!content.empty())
		{
			std::string xStr = extractAttr(node, "x");
			std::string yStr = extractAttr(node, "y");
			std::string sizeStr = extractAttr(node, "font-size");
			
			std::cout << "Text attributes - x:" << xStr << " y:" << yStr << " font-size:" << sizeStr << std::endl;
			
			if (!xStr.empty() && !yStr.empty() && !sizeStr.empty()) {
				float x = stof(xStr);
				float y = stof(yStr);
				PointF startPoint{ x, y };
				TextPaintStyle t = parseTextStyle(node);
				float size = stof(sizeStr);

				SVGText* text = new SVGText(content, startPoint, t, size);

				std::string transformStr = extractAttr(node, "transform");
				if (!transformStr.empty()) {
					parseTransform(text, transformStr);
				}

				std::cout << "Created text successfully" << std::endl;
				return text;
			}
		}
	}

	std::cout << "No element created for: " << nodeName << std::endl;
	return nullptr;
}

std::vector<SVGElement *> SVGParser::getElements() const
{
	return elements;
}

std::string SVGParser::getHeight() const
{
	return heightSVG;
}

std::string SVGParser::getWidth() const
{
	return widthSVG;
}

std::string SVGParser::getRawData() const
{
	return SVG_Raw_Data;
}