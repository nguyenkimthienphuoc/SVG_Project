﻿#include "stdafx.h"
#include "SVGParser.h"
#include "SVGRect.h"
#include "SVGCircle.h"
#include "SVGLine.h"
#include "SVGBasics.h"
#include "SVGEllipse.h"
#include "SVGText.h"
#include "SVGPolygon.h"
#include "SVGPolyline.h"
#include <gdiplus.h>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstring>
#include <iostream>
#include <vector>

std::string readSVGFile(const std::string &filePath)
{
	std::ifstream file(filePath);
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str(); // Trả về nội dung file SVG dưới dạng string
}

std::string extractAttr(const std::string& tag, const std::string& attrName) {
	// Ghép regex động dạng: " attrName="..."
	std::string pattern = "(?:\\s|^)" + attrName + "=\"([^\"]+)\"";
	std::regex attrRegex(pattern);
	std::smatch match;
	if (std::regex_search(tag, match, attrRegex))
	{
		return match[1].str();
	}
	return "";
}
std::wstring extractInnerText(const std::string& tag) {
	std::regex contentRegex(R"(>([^<]+)<)");
	std::smatch match;

	if (std::regex_search(tag, match, contentRegex)) {
		std::string raw = match[1].str();

		// Chuyển từ string thành wstring
		std::wstring wideValue(raw.begin(), raw.end());

		return wideValue;
	}
	return L"";
}

SVGParser::SVGParser(const std::string& filePath)
{
	std::string data = readSVGFile(filePath);
	this->SVG_Raw_Data = data;

	if (data.empty())
	{
		std::cerr << "Failed to read SVG file." << std::endl;
		exit(1);
	}
	parseHeader();

}

SVGParser::~SVGParser()
{
	for (int i = 0; i < elements.size(); i++)
	{
		delete elements[i];
	}
	elements.clear();
}

void SVGParser::parseSVG()
{
	// 1. Lấy tất cả thẻ <text>...</text>
	std::regex textTagRegex(R"(<text\b[^>]*>[\s\S]*?<\/text>)", std::regex::icase);
	std::smatch match;
	std::string textSearch = SVG_Raw_Data;

	while (std::regex_search(textSearch, match, textTagRegex)) {
		std::string fullTextTag = match[0];
		parseElements(fullTextTag);
		textSearch = match.suffix().str();
	}

	// 2. Xử lý các tag đơn còn lại
	std::regex simpleTagRegex(R"(<(\w+)([^>]*)/?>)");
	std::smatch match2;
	std::string simpleSearch = SVG_Raw_Data;

	while (std::regex_search(simpleSearch, match2, simpleTagRegex)) {
		std::string tag = match2[0];

		// Bỏ qua text (đã xử lý)
		if (tag.find("<text") != 0 && tag.find("</text>") == std::string::npos) {
			parseElements(tag);
		}

		simpleSearch = match2.suffix().str();
	}
}

void SVGParser::parseHeader()
{
	std::regex SVG_Tag_Regex(R"(<svg\s+[^>]+>)");
	std::smatch match;

	if (std::regex_search(SVG_Raw_Data, match, SVG_Tag_Regex))
	{
		std::string SVG_Tag = match.str();

		std::regex widthRegex("width=\"([^\"]+)\"");
		std::regex heightRegex("height=\"([^\"]+)\"");

		std::smatch widthMatch;
		if (std::regex_search(SVG_Raw_Data, widthMatch, widthRegex))
		{
			this->widthSVG = widthMatch[1];
		}

		std::smatch heightMatch;
		if (std::regex_search(SVG_Raw_Data, heightMatch, heightRegex))
		{
			this->heightSVG = heightMatch[1];
		}
	}
}

Color SVGParser::parseColor(const std::string &colorStr)
{
	// 1. Check for rgb(r,g,b) format
	std::regex rgbRegex(R"(rgb\((\d+),\s*(\d+),\s*(\d+)\))");
	std::smatch match;

	if (std::regex_match(colorStr, match, rgbRegex))
	{
		int r = std::stoi(match[1]);
		int g = std::stoi(match[2]);
		int b = std::stoi(match[3]);
		return Color(255, r, g, b); // default alpha = 255
	}

	// 2. Check for simple color format
	if (colorStr == "red")
		return Color(255, 255, 0, 0);
	if (colorStr == "blue")
		return Color(255, 0, 0, 255);
	if (colorStr == "green")
		return Color(255, 0, 128, 0);
	if (colorStr == "black")
		return Color(255, 0, 0, 0);
	if (colorStr == "white")
		return Color(255, 255, 255, 255);

	return Color(255, 0, 0, 0);
}

PaintStyle SVGParser::parsePaintStyle(const std::string &tag)
{
	PaintStyle s;

	if (std::string stroke_str = extractAttr(tag, "stroke"); !stroke_str.empty())
	{
		s.strokeColor = parseColor(stroke_str);
	}

	if (std::string fill_str = extractAttr(tag, "fill"); !fill_str.empty())
	{
		s.fillColor = parseColor(fill_str);
	}

	if (std::string opacity_str = extractAttr(tag, "opacity"); !opacity_str.empty())
	{
		s.strokeOpacity *= std::stof(opacity_str);
		s.fillOpacity *= std::stof(opacity_str);
	}

	if (std::string fillO = extractAttr(tag, "fill-opacity"); !fillO.empty())
	{
		s.fillOpacity = std::stof(fillO);
	}

	if (std::string strokeO = extractAttr(tag, "stroke-opacity"); !strokeO.empty())
	{
		s.strokeOpacity = std::stof(strokeO);
	}

	if (std::string strokeW = extractAttr(tag, "stroke-width"); !strokeW.empty())
	{
		s.strokeWidth = std::stof(strokeW);
	}

	return s;
}

TextPaintStyle SVGParser::parseTextStyle(const std::string &tag)
{
	TextPaintStyle t;
	if (std::string fill_str = (extractAttr(tag, "fill")); !fill_str.empty())
	{
		t.fillColor = parseColor(fill_str);
	}

	if (std::string fillO = (extractAttr(tag, "fill-opacity")); !fillO.empty())
	{
		t.fillOpacity = std::stof(fillO);
	}

	if (std::string stroke_str = (extractAttr(tag, "stroke")); !stroke_str.empty())
	{
		t.strokeColor = parseColor(stroke_str);
	}

	if (std::string strokeW = (extractAttr(tag, "stroke-width")); !strokeW.empty())
	{
		t.strokeWidth = std::stof(strokeW);
	}

	if (std::string strokeO = (extractAttr(tag, "stroke-opacity")); !strokeO.empty())
	{
		t.strokeWidth = std::stof(strokeO);
	}

	if (std::string fontFam = (extractAttr(tag, "font-family")); !fontFam.empty())
	{
		t.fontFamily = fontFam;
	}

	if (std::string fontW = (extractAttr(tag, "font-weight")); !fontW.empty())
	{
		t.fontWeight = fontW;
	}

	if (std::string textA = (extractAttr(tag, "text-anchor")); !textA.empty())
	{
		t.textAnchor = textA;
	}

	if (std::string opacity_str = extractAttr(tag, "opacity"); !opacity_str.empty())
	{
		t.strokeOpacity *= std::stof(opacity_str);
		t.fillOpacity *= std::stof(opacity_str);
	}

	return t;
}

std::vector<PointF> SVGParser::parsePoints(const std::string &pointStr)
{
	std::vector<PointF> points;
	std::regex coordRegex(R"(([0-9.+-]+)[, ]+([0-9.+-]+))");
	auto begin = std::sregex_iterator(pointStr.begin(), pointStr.end(), coordRegex);
	auto end = std::sregex_iterator();

	for (auto it = begin; it != end; ++it)
	{
		float x = std::stof((*it)[1]);
		float y = std::stof((*it)[2]);
		points.emplace_back(x, y);
	}
	return points;
}

void SVGParser::parseElements(const std::string &tag)
{
	SVGElement *element = createElementFromTag(tag);
	if (element)
	{
		this->elements.push_back(element);
	}
}

SVGElement *SVGParser::createElementFromTag(const std::string &tag)
{
	// rectangle
	if (tag.find("<rect") == 0)
	{
		PaintStyle s = parsePaintStyle(tag);
		REAL x = static_cast<REAL>(std::stof(extractAttr(tag, "x")));
		REAL y = static_cast<REAL>(std::stof(extractAttr(tag, "y")));
		REAL width = static_cast<REAL>(std::stof(extractAttr(tag, "width")));
		REAL height = static_cast<REAL>(std::stof(extractAttr(tag, "height")));
		PointF topLeft(x, y);
		return new SVGRect(topLeft, width, height, s);
	}

	// circle
	else if (tag.find("<circle") == 0)
	{
		float x = std::stof(extractAttr(tag, "cx"));
		float y = std::stof(extractAttr(tag, "cy"));
		float radius = std::stof(extractAttr(tag, "r"));
		PointF topLeft{x, y};
		PaintStyle s = parsePaintStyle(tag);
		return new SVGCircle(topLeft, radius, s);
	}

	// line
	else if (tag.find("<line") == 0)
	{
		float x1 = std::stof(extractAttr(tag, "x1"));
		float y1 = std::stof(extractAttr(tag, "y1"));
		float x2 = std::stof(extractAttr(tag, "x2"));
		float y2 = std::stof(extractAttr(tag, "y2"));
		PaintStyle s = parsePaintStyle(tag);
		return new SVGLine(x1, x2, y1, y2, s);
	}

	// ellipse
	else if (tag.find("<ellipse") == 0)
	{
		float cx = std::stof(extractAttr(tag, "cx"));
		float cy = std::stof(extractAttr(tag, "cy"));
		float rx = std::stof(extractAttr(tag, "rx"));
		float ry = std::stof(extractAttr(tag, "ry"));
		PaintStyle s = parsePaintStyle(tag);
		return new SVGEllipse(cx, cy, rx, ry, s);
	}

	// polygon
	else if (tag.find("<polygon") == 0)
	{
		std::string pointStr = extractAttr(tag, "points");
		std::vector<PointF> points = parsePoints(pointStr);
		PaintStyle s = parsePaintStyle(tag);
		return new SVGPolygon(points, s);
	}

	// pollyline
	else if (tag.find("<polyline") == 0)
	{
		std::string pointStr = extractAttr(tag, "points");
		std::vector<PointF> points = parsePoints(pointStr);
		PaintStyle s = parsePaintStyle(tag);
		return new SVGPolyline(points, s);
	}

	// path
	else if (tag.find("<path") == 0)
	{
		std::string data = extractAttr(tag, "d");
		if (!data.empty())
		{
			PaintStyle style = parsePaintStyle(tag);
			return new SVGPath(data, style);
		}
	}

	// text
	else if (tag.find("<text") == 0)
	{
        std::wstring content = extractInnerText(tag);

		if (!content.empty())
		{
			float x = stof(extractAttr(tag, "x"));
			float y = stof(extractAttr(tag, "y"));
			PointF startPoint{x, y};
			TextPaintStyle t = parseTextStyle(tag);
			float size = stof(extractAttr(tag, "font-size"));
			return new SVGText(content, startPoint, t, size);
		}
	}

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