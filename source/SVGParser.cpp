#include "stdafx.h"
#include <unordered_map>
#include <limits>

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
		parseDefs();
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

	std::string tag = node->name();
	std::cout << "Processing node: " << tag << std::endl;
	
	if (tag == "g") {
		PaintStyle groupStyle = parsePaintStyle(node);
		TextPaintStyle groupTextStyle = parseTextStyle(node);
		SVGGroup* group = new SVGGroup(groupStyle);

		// Apply transform to group if it exists
		std::string transformStr = extractAttr(node, "transform");
		if (!transformStr.empty()) {
			parseTransform(group, transformStr);
			std::cout << "Applied transform to group: " << transformStr << std::endl;
		}

		std::cout << "Processing group with " << rapidxml::count_children(node) << " children" << std::endl;

		// Lưu stack
		paintStyleStack.push(groupStyle);
		textStyleStack.push(groupTextStyle);
		
		int childCount = 0;
		for (rapidxml::xml_node<>* child = node->first_node(); child; child = child->next_sibling()) {
			std::string childTag = child->name();
			std::cout << "Processing group child: " << childTag << std::endl;
			SVGElement* childElement = nullptr;
			
			if (childTag == "text") {
				// For text elements, use text style inheritance
				childElement = createTextFromNodeWithStyle(child, groupTextStyle);
			} else if (childTag == "g") {
				// Nested group - create a temporary parser state
				SVGGroup* nestedGroup = nullptr;
				
				// Parse nested group as a separate element
				PaintStyle nestedGroupStyle = parsePaintStyle(child);
				TextPaintStyle nestedGroupTextStyle = parseTextStyle(child);
				nestedGroup = new SVGGroup(nestedGroupStyle);
				
				// Apply transform to nested group if it exists
				std::string nestedTransformStr = extractAttr(child, "transform");
				if (!nestedTransformStr.empty()) {
					parseTransform(nestedGroup, nestedTransformStr);
					std::cout << "Applied transform to nested group: " << nestedTransformStr << std::endl;
				}
				
				// Process children of nested group
				for (rapidxml::xml_node<>* grandChild = child->first_node(); grandChild; grandChild = grandChild->next_sibling()) {
					std::string grandChildTag = grandChild->name();
					SVGElement* grandChildElement = nullptr;
					
					if (grandChildTag == "text") {
						grandChildElement = createTextFromNodeWithStyle(grandChild, nestedGroupTextStyle);
					} else {
						grandChildElement = createElementFromNodeWithStyle(grandChild, nestedGroupStyle);
					}
					
					if (grandChildElement) {
						nestedGroup->addChild(grandChildElement);
						std::cout << "Added grandchild element to nested group" << std::endl;
					}
				}
				
				childElement = nestedGroup;
			} else {
				// For other elements, use regular style inheritance
				childElement = createElementFromNodeWithStyle(child, groupStyle);
			}
			
			if (childElement) {
				group->addChild(childElement);
				childCount++;
				std::cout << "Added child element to group" << std::endl;
			}
		}
		
		textStyleStack.pop();
		paintStyleStack.pop();

		std::cout << "Group processed with " << childCount << " children" << std::endl;
		elements.push_back(group);
	}
	else {
		SVGElement* element = createElementFromNode(node);
		if (element) {
			elements.push_back(element);
		}

		// Process children for non-group elements if any
		for (rapidxml::xml_node<>* child = node->first_node(); child; child = child->next_sibling()) {
			parseNode(child);
		}
	}
}

void SVGParser::parseDefs() {
	auto* svgRoot = doc.first_node("svg");
	if (!svgRoot) return;
	auto* defs = svgRoot->first_node("defs");
	if (!defs) return;

	for (auto* n = defs->first_node(); n; n = n->next_sibling()) {
		std::string tag = n->name();
		if (tag == "linearGradient") {
			parseLinearGradient(n);
		}
		else if (tag == "radialGradient") {
			parseRadialGradient(n);
		}
	}
}

void SVGParser::parseLinearGradient(rapidxml::xml_node<>* node) {
	SVGLinearGradientDef g;
	g.id = extractAttr(node, "id");
	if (g.id.empty()) return;

	std::string units = extractAttr(node, "gradientUnits");
	if (!units.empty()) { g.unitsSpecified = true; g.userSpaceOnUse = (units == "userSpaceOnUse"); }

	// default SVG: x1=0%, y1=0%, x2=100%, y2=0%
	auto parseCoord = [](const std::string &s, float fallback) -> float {
		if (s.empty()) return fallback;
		if (s.back() == '%') {
			return std::stof(s.substr(0, s.size()-1)) / 100.0f;
		}
		return std::stof(s);
	};
	auto x1s = extractAttr(node, "x1"); if (!x1s.empty()) { g.x1 = parseCoord(x1s, 0.0f); g.x1Specified = true; }
	auto y1s = extractAttr(node, "y1"); if (!y1s.empty()) { g.y1 = parseCoord(y1s, 0.0f); g.y1Specified = true; }
	auto x2s = extractAttr(node, "x2"); if (!x2s.empty()) { g.x2 = parseCoord(x2s, 1.0f); g.x2Specified = true; }
	auto y2s = extractAttr(node, "y2"); if (!y2s.empty()) { g.y2 = parseCoord(y2s, 0.0f); g.y2Specified = true; }

	// Accept either xlink:href or href
	std::string href = extractAttr(node, "xlink:href");
	if (href.empty()) href = extractAttr(node, "href");
	if (href.rfind("#", 0) == 0) href = href.substr(1);
	g.href = href;

	// spreadMethod (pad | reflect | repeat)
	g.spreadMethod = extractAttr(node, "spreadMethod");
	if (g.spreadMethod.empty()) g.spreadMethod = "pad";

	// gradientTransform
	std::string t = extractAttr(node, "gradientTransform");
	if (!t.empty()) {
		g.transform = parseTransformToMatrix(t);
		g.transformSpecified = true;
	}

	// stops
	for (auto* st = node->first_node("stop"); st; st = st->next_sibling("stop")) {
		SVGGradientStop s;
		std::string off = extractAttr(st, "offset");
		if (off.find('%') != std::string::npos) {
			s.offset = std::stof(off) / 100.0f;
		}
		else {
			// use NaN to mark "unspecified" offsets so we can interpolate later
			if (off.empty()) s.offset = std::numeric_limits<float>::quiet_NaN();
			else s.offset = std::stof(off);
		}
		std::string sc = extractAttr(st, "stop-color");
		std::string so = extractAttr(st, "stop-opacity");
		if (so.empty()) so = "1";
		// parse màu đơn giản #RRGGBB hoặc tên cơ bản, bạn có thể tái dụng parseColor(...)
		// ở đây thêm opacity vào alpha
		Gdiplus::Color base = parseColor(sc);
		BYTE a = static_cast<BYTE>(base.GetA() * std::stof(so));
		s.color = Gdiplus::Color(a, base.GetR(), base.GetG(), base.GetB());
		g.stops.push_back(s);
	}
	gradientRegistry.linear.emplace(g.id, std::move(g));

}

void SVGParser::parseRadialGradient(rapidxml::xml_node<>* node) {
	SVGRadialGradientDef g;
	g.id = extractAttr(node, "id");
	if (g.id.empty()) return;

	std::string units = extractAttr(node, "gradientUnits");
	if (!units.empty()) { g.unitsSpecified = true; g.userSpaceOnUse = (units == "userSpaceOnUse"); }

	// support percentages for coords
	auto parseCoord = [](const std::string &s, float fallback) -> float {
		if (s.empty()) return fallback;
		if (s.back() == '%') return std::stof(s.substr(0, s.size()-1)) / 100.0f;
		return std::stof(s);
	};
	auto cxs = extractAttr(node, "cx"); if (!cxs.empty()) { g.cx = parseCoord(cxs, 0.5f); g.cxSpecified = true; }
	auto cys = extractAttr(node, "cy"); if (!cys.empty()) { g.cy = parseCoord(cys, 0.5f); g.cySpecified = true; }
	auto rs = extractAttr(node, "r");  if (!rs.empty()) { g.r = parseCoord(rs, 0.5f); g.rSpecified = true; }

	auto fxs = extractAttr(node, "fx"); if (!fxs.empty()) { g.fx = parseCoord(fxs, -1.0f); g.fxSpecified = true; }
	auto fys = extractAttr(node, "fy"); if (!fys.empty()) { g.fy = parseCoord(fys, -1.0f); g.fySpecified = true; }

	// Accept either xlink:href or href
	std::string href = extractAttr(node, "xlink:href");
	if (href.empty()) href = extractAttr(node, "href");
	if (href.rfind("#", 0) == 0) href = href.substr(1);
	g.href = href;

	// spreadMethod
	g.spreadMethod = extractAttr(node, "spreadMethod");
	if (g.spreadMethod.empty()) g.spreadMethod = "pad";

	std::string t = extractAttr(node, "gradientTransform");
	if (!t.empty()) {
		g.transform = parseTransformToMatrix(t);
		g.transformSpecified = true;
	}

	for (auto* st = node->first_node("stop"); st; st = st->next_sibling("stop")) {
		SVGGradientStop s;
		std::string off = extractAttr(st, "offset");
		if (off.find('%') != std::string::npos) s.offset = std::stof(off) / 100.0f;
		else {
			if (off.empty()) s.offset = std::numeric_limits<float>::quiet_NaN();
			else s.offset = std::stof(off);
		}

		std::string sc = extractAttr(st, "stop-color");
		std::string so = extractAttr(st, "stop-opacity");
		if (so.empty()) so = "1";
		Gdiplus::Color base = parseColor(sc);
		BYTE a = static_cast<BYTE>(base.GetA() * std::stof(so));
		s.color = Gdiplus::Color(a, base.GetR(), base.GetG(), base.GetB());
		g.stops.push_back(s);
	}
	gradientRegistry.radial.emplace(g.id, std::move(g));
}

std::unique_ptr<Gdiplus::Matrix>
SVGParser::parseTransformToMatrix(const std::string& transformStr) {
	using namespace Gdiplus;
	auto mat = std::make_unique<Matrix>(); // identity

	std::string s = transformStr;
	size_t pos = 0;
	while (pos < s.size()) {
		size_t open = s.find('(', pos);
		if (open == std::string::npos) break;

		std::string name = s.substr(pos, open - pos);
		// trim spaces
		while (!name.empty() && std::isspace((unsigned char)name.front())) name.erase(name.begin());
		while (!name.empty() && std::isspace((unsigned char)name.back()))  name.pop_back();

		size_t close = s.find(')', open);
		if (close == std::string::npos) break;
		std::string content = s.substr(open + 1, close - open - 1);

		// replace commas with spaces
		for (char& c : content) if (c == ',') c = ' ';
		std::istringstream ss(content);

		if (name == "matrix") {
			float a, b, c, d, e, f;
			if (ss >> a >> b >> c >> d >> e >> f) {
				Matrix mm(a, b, c, d, e, f);
				mat->Multiply(&mm, MatrixOrderAppend);
			}
		}
		else if (name == "translate") {
			float tx = 0, ty = 0; ss >> tx; if (!(ss >> ty)) ty = 0;
			Matrix mm; mm.Translate(tx, ty);
			mat->Multiply(&mm, MatrixOrderAppend);
		}
		else if (name == "scale") {
			float sx = 1, sy = 1; ss >> sx; if (!(ss >> sy)) sy = sx;
			Matrix mm; mm.Scale(sx, sy);
			mat->Multiply(&mm, MatrixOrderAppend);
		}
		else if (name == "rotate") {
			float deg = 0; ss >> deg;
			Matrix mm; mm.Rotate(deg);
			mat->Multiply(&mm, MatrixOrderAppend);
		}

		pos = close + 1;
	}

	return mat; // trả về unique_ptr (KHÔNG copy Matrix)
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

	// 2. Check for hex color format (#RRGGBB or #RGB)
	if (colorStr.length() > 1 && colorStr[0] == '#') {
		std::string hex = colorStr.substr(1);
		if (hex.length() == 6) {
			// #RRGGBB format
			int r = std::stoi(hex.substr(0, 2), nullptr, 16);
			int g = std::stoi(hex.substr(2, 2), nullptr, 16);
			int b = std::stoi(hex.substr(4, 2), nullptr, 16);
			return Color(255, r, g, b);
		}
		else if (hex.length() == 3) {
			// #RGB format - expand to #RRGGBB
			int r = std::stoi(std::string(2, hex[0]), nullptr, 16);
			int g = std::stoi(std::string(2, hex[1]), nullptr, 16);
			int b = std::stoi(std::string(2, hex[2]), nullptr, 16);
			return Color(255, r, g, b);
		}
	}

	// 3. Check for "none" value first
	if (colorStr == "none")
		return Gdiplus::Color(0, 0, 0, 0); // Transparent

	// 4. Check for named colors
	if (colorStr == "red")
		return Gdiplus::Color(255, 255, 0, 0);
	if (colorStr == "green")
		return Gdiplus::Color(255, 0, 128, 0);
	if (colorStr == "blue")
		return Gdiplus::Color(255, 0, 0, 255);
	if (colorStr == "yellow")
		return Gdiplus::Color(255, 255, 255, 0);
	if (colorStr == "orange")
		return Gdiplus::Color(255, 255, 165, 0);
	if (colorStr == "purple")
		return Gdiplus::Color(255, 128, 0, 128);
	if (colorStr == "pink")
		return Gdiplus::Color(255, 255, 192, 203);
	if (colorStr == "cyan")
		return Gdiplus::Color(255, 0, 255, 255);
	if (colorStr == "magenta")
		return Gdiplus::Color(255, 255, 0, 255);
	if (colorStr == "lime")
		return Gdiplus::Color(255, 0, 255, 0);
	if (colorStr == "brown")
		return Gdiplus::Color(255, 165, 42, 42);
	if (colorStr == "gray" || colorStr == "grey")
		return Gdiplus::Color(255, 128, 128, 128);
	if (colorStr == "black")
		return Gdiplus::Color(255, 0, 0, 0);
	if (colorStr == "white")
		return Gdiplus::Color(255, 255, 255, 255);
	if (colorStr == "darkred")
		return Gdiplus::Color(255, 139, 0, 0);
	if (colorStr == "darkgreen")
		return Gdiplus::Color(255, 0, 100, 0);
	if (colorStr == "darkblue")
		return Gdiplus::Color(255, 0, 0, 139);
	if (colorStr == "lightred")
		return Gdiplus::Color(255, 255, 182, 193);
	if (colorStr == "lightgreen")
		return Gdiplus::Color(255, 144, 238, 144);
	if (colorStr == "lightblue")
		return Gdiplus::Color(255, 173, 216, 230);
	if (colorStr == "gold")
		return Gdiplus::Color(255, 255, 215, 0);
	if (colorStr == "silver")
		return Gdiplus::Color(255, 192, 192, 192);

	// Default to black if color not recognized
	std::cout << "Warning: Unknown color '" << colorStr << "', using black" << std::endl;
	return Gdiplus::Color(255, 0, 0, 0);
}

PaintStyle SVGParser::parsePaintStyle(rapidxml::xml_node<>* node)
{
	PaintStyle s;

	// Support style="key:val; key2:val2" inline CSS style attribute
	std::string styleAttr = extractAttr(node, "style");
	std::unordered_map<std::string, std::string> styleMap;
	if (!styleAttr.empty()) {
		// split by ';'
		std::stringstream ss(styleAttr);
		std::string decl;
		while (std::getline(ss, decl, ';')) {
			auto pos = decl.find(':');
			if (pos == std::string::npos) continue;
			std::string key = decl.substr(0, pos);
			std::string val = decl.substr(pos+1);
			// trim spaces
			auto trim = [](std::string &str) {
				while (!str.empty() && isspace((unsigned char)str.front())) str.erase(str.begin());
				while (!str.empty() && isspace((unsigned char)str.back())) str.pop_back();
			};
			trim(key); trim(val);
			styleMap[key] = val;
		}
	}

	std::string stroke_str = extractAttr(node, "stroke");
	if (stroke_str.empty() && styleMap.count("stroke")) stroke_str = styleMap["stroke"];
	if (!stroke_str.empty()) {
		if (stroke_str == "none") {
			s.strokeNone = true;
			s.strokeOpacity = 0.0f;
			s.strokeWidth = 0.0f;
		}
		else if (stroke_str.rfind("url(", 0) == 0) {
			auto start = stroke_str.find('#');
			auto end = stroke_str.find(')', start);
			if (start != std::string::npos && end != std::string::npos && end > start) {
				s.strokeUrlId = stroke_str.substr(start + 1, end - start - 1);
			}
		}
		else {
			s.strokeColor = parseColor(stroke_str);
		}
	}

	std::string fill_str = extractAttr(node, "fill");
	if (fill_str.empty() && styleMap.count("fill")) fill_str = styleMap["fill"];
	if (!fill_str.empty()) {
		if (fill_str == "none") {
			s.fillNone = true;
			s.fillOpacity = 0.0f;
		}
		else if (fill_str.rfind("url(", 0) == 0) {
			// url(#id)
			auto start = fill_str.find('#');
			auto end = fill_str.find(')', start);
			if (start != std::string::npos && end != std::string::npos && end > start) {
				s.fillUrlId = fill_str.substr(start + 1, end - start - 1);
			}
		}
		else {
			s.fillColor = parseColor(fill_str);
		}
	}

	// opacity and specific opacities: accept both attribute and style map
	std::string opacity_str = extractAttr(node, "opacity"); if (opacity_str.empty() && styleMap.count("opacity")) opacity_str = styleMap["opacity"];
	if (!opacity_str.empty()) {
		s.strokeOpacity *= std::stof(opacity_str);
		s.fillOpacity *= std::stof(opacity_str);
	}

	std::string fillO = extractAttr(node, "fill-opacity"); if (fillO.empty() && styleMap.count("fill-opacity")) fillO = styleMap["fill-opacity"];
	if (!fillO.empty()) {
		s.fillOpacity = std::stof(fillO);
	}

	std::string strokeO = extractAttr(node, "stroke-opacity"); if (strokeO.empty() && styleMap.count("stroke-opacity")) strokeO = styleMap["stroke-opacity"];
	if (!strokeO.empty()) {
		s.strokeOpacity = std::stof(strokeO);
	}

	std::string strokeW = extractAttr(node, "stroke-width"); if (strokeW.empty() && styleMap.count("stroke-width")) strokeW = styleMap["stroke-width"];
	if (!strokeW.empty()) {
		s.strokeWidth = std::stof(strokeW);
	}

	return s;
}

PaintStyle mergePaintStyle(const PaintStyle& parent, const PaintStyle& child) {
	PaintStyle result = child;

	// opacity & width kế thừa như trước
	if (child.fillOpacity == 1.0f) result.fillOpacity *= parent.fillOpacity;
	if (child.strokeOpacity == 1.0f) result.strokeOpacity *= parent.strokeOpacity;
	if (child.strokeWidth == 1.0f) result.strokeWidth = parent.strokeWidth;

	// màu hoặc url
	if (child.fillUrlId.empty()) {
		if (child.fillColor.GetValue() == Gdiplus::Color(255, 0, 0, 0).GetValue())
			result.fillColor = parent.fillColor;
		// nếu parent có url thì kế thừa
		if (parent.fillUrlId.size() && result.fillUrlId.empty())
			result.fillUrlId = parent.fillUrlId;
		if (child.fillNone) result.fillUrlId.clear();
	}
	if (child.strokeUrlId.empty()) {
		if (child.strokeColor.GetValue() == Gdiplus::Color(255, 0, 0, 0).GetValue())
			result.strokeColor = parent.strokeColor;
		if (parent.strokeUrlId.size() && result.strokeUrlId.empty())
			result.strokeUrlId = parent.strokeUrlId;
		if (child.strokeNone) result.strokeUrlId.clear();
	}
	result.fillNone = child.fillNone || parent.fillNone;
	result.strokeNone = child.strokeNone || parent.strokeNone;
	return result;
}


TextPaintStyle mergeTextStyle(const TextPaintStyle& parent, const TextPaintStyle& child) {
	TextPaintStyle result = child;

	// Inherit from parent if child doesn't have specific values
	if (child.fontSize == 16.0f) result.fontSize = parent.fontSize; // Use parent font size if child is default
	if (child.fontFamily == "Arial") result.fontFamily = parent.fontFamily;
	if (child.fontWeight == "normal") result.fontWeight = parent.fontWeight;
	if (child.fontStyle == "normal") result.fontStyle = parent.fontStyle;
	if (child.textAnchor == "start") result.textAnchor = parent.textAnchor;
	if (child.dx == 0.0f) result.dx = parent.dx;
	if (child.dy == 0.0f) result.dy = parent.dy;

	// Merge colors if child is default
	if (child.fillColor.GetValue() == Gdiplus::Color(255, 0, 0, 0).GetValue())
		result.fillColor = parent.fillColor;
	if (child.strokeColor.GetValue() == Gdiplus::Color(0, 0, 0, 0).GetValue())
		result.strokeColor = parent.strokeColor;

	// Opacity inheritance
	if (child.fillOpacity == 1.0f) result.fillOpacity *= parent.fillOpacity;
	if (child.strokeOpacity == 1.0f) result.strokeOpacity *= parent.strokeOpacity;
	if (child.strokeWidth == 0.0f) result.strokeWidth = parent.strokeWidth;

	return result;
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

	if (std::string fontSize_str = (extractAttr(node, "font-size")); !fontSize_str.empty())
	{
		t.fontSize = std::stof(fontSize_str);
	}

	if (std::string textA = (extractAttr(node, "text-anchor")); !textA.empty())
	{
		t.textAnchor = textA;
	}

	if (std::string dx_str = (extractAttr(node, "dx")); !dx_str.empty())
	{
		t.dx = std::stof(dx_str);
	}

	if (std::string dy_str = (extractAttr(node, "dy")); !dy_str.empty())
	{
		t.dy = std::stof(dy_str);
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
	std::cout << "Parsing transform: " << transformStr << std::endl;
	
	// Split transforms by looking for transform functions
	std::string str = transformStr;
	size_t pos = 0;
	
	while (pos < str.length()) {
		// Find the next transform function
		size_t translatePos = str.find("translate(", pos);
		size_t rotatePos = str.find("rotate(", pos);
		size_t scalePos = str.find("scale(", pos);
		
		// Find the earliest position
		size_t nextPos = std::string::npos;
		std::string transformType = "";
		
		if (translatePos != std::string::npos) {
			nextPos = translatePos;
			transformType = "translate";
		}
		if (rotatePos != std::string::npos && (nextPos == std::string::npos || rotatePos < nextPos)) {
			nextPos = rotatePos;
			transformType = "rotate";
		}
		if (scalePos != std::string::npos && (nextPos == std::string::npos || scalePos < nextPos)) {
			nextPos = scalePos;
			transformType = "scale";
		}
		
		if (nextPos == std::string::npos) break;
		
		// Find the closing parenthesis
		size_t openParen = str.find('(', nextPos);
		size_t closeParen = str.find(')', openParen);
		
		if (openParen == std::string::npos || closeParen == std::string::npos) break;
		
		// Extract the content between parentheses
		std::string content = str.substr(openParen + 1, closeParen - openParen - 1);
		
		if (transformType == "translate") {
			float tx = 0, ty = 0;
			
			// Replace commas with spaces to handle both formats
			for (char& c : content) {
				if (c == ',') c = ' ';
			}
			
			std::stringstream vals(content);
			std::string xStr, yStr;

			if (vals >> xStr >> yStr) {
				tx = std::stof(xStr);
				ty = std::stof(yStr);
			}
			else if (vals >> xStr) {
				tx = std::stof(xStr);
			}

			// Apply translation using visitor pattern
			SVGTranslate translateVisitor(tx, ty);
			element->accept(&translateVisitor);
			std::cout << "Applied translate(" << tx << ", " << ty << ")" << std::endl;
		}
		else if (transformType == "rotate") {
			float degree = 0;
			
			std::stringstream vals(content);
			std::string angleStr;
			if (vals >> angleStr) {
				degree = std::stof(angleStr);
			}
			
			// Apply rotation using visitor pattern
			SVGRotate rotateVisitor(degree);
			element->accept(&rotateVisitor);
			std::cout << "Applied rotate(" << degree << ")" << std::endl;
		}

		else if (transformType == "scale") {
			std::string s = content;
			bool hadComma = (s.find(',') != std::string::npos);
			for (char& c : s) if (c == ',') c = ' ';

			std::istringstream ss(s);

			float sx;
			if (!(ss >> sx)) {
				std::cout << "scale(): missing sx in '" << content << "'\n";
				return;
			}

			float sy;
			// 2 parameters
			if (ss >> sy) {
				SVGScaleByXY v(sx, sy);
				element->accept(&v);
				std::cout << "Applied scale(" << sx << ", " << sy << ")\n";
			}
			else { // 1 parameters
				// had 1 para, but have ',' -> invalid
				if (hadComma) {
					std::cout << "scale(): comma found but missing sy in '" << content << "'\n";
					return;
				}
				
				SVGScaleByTimes v(sx);
				element->accept(&v);
				std::cout << "Applied scale(" << sx << ")\n";
			}

			// Cảnh báo nếu còn rác phía sau (vd: "scale(2x)")
			std::string leftover;
			if (ss >> leftover) {
				std::cout << "scale(): extra tokens ignored: '" << leftover
					<< "' in '" << content << "'\n";
			}
		}
		
		// Move to the next position after this transform
		pos = closeParen + 1;
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
			rect->setGradientRegistry(&gradientRegistry);

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
			circle->setGradientRegistry(&gradientRegistry);
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
			line->setGradientRegistry(&gradientRegistry);
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
			ellipse->setGradientRegistry(&gradientRegistry);

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
			polygon->setGradientRegistry(&gradientRegistry);

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
			polyline->setGradientRegistry(&gradientRegistry);

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
			path->setGradientRegistry(&gradientRegistry);

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
				text->setGradientRegistry(&gradientRegistry);

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

SVGElement* SVGParser::createElementFromNodeWithStyle(rapidxml::xml_node<>* node, const PaintStyle& inheritedStyle)
{
	if (!node) return nullptr;

	// Parse style riêng nếu có
	PaintStyle localStyle = parsePaintStyle(node);
	PaintStyle finalStyle = mergePaintStyle(inheritedStyle, localStyle);

	std::string nodeName = node->name();
	std::cout << "Creating element for: " << nodeName << std::endl;

	// rectangle
	if (nodeName == "rect")
	{
		/*PaintStyle s = parsePaintStyle(node);*/
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

			SVGRect* rect = new SVGRect(topLeft, width, height, finalStyle);
			rect->setGradientRegistry(&gradientRegistry);

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
			/*PaintStyle s = parsePaintStyle(node);*/

			SVGCircle* circle = new SVGCircle(topLeft, radius, finalStyle);
			circle->setGradientRegistry(&gradientRegistry);

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
			/*PaintStyle s = parsePaintStyle(node);*/

			SVGLine* line = new SVGLine(x1, x2, y1, y2, finalStyle);
			line->setGradientRegistry(&gradientRegistry);

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
			/*PaintStyle s = parsePaintStyle(node);*/

			SVGEllipse* ellipse = new SVGEllipse(cx, cy, rx, ry, finalStyle);
			ellipse->setGradientRegistry(&gradientRegistry);

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

			/*PaintStyle s = parsePaintStyle(node);*/

			SVGPolygon* polygon = new SVGPolygon(points, finalStyle);
			polygon->setGradientRegistry(&gradientRegistry);

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

			/*PaintStyle s = parsePaintStyle(node);*/

			SVGPolyline* polyline = new SVGPolyline(points, finalStyle);
			polyline->setGradientRegistry(&gradientRegistry);

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
			/*PaintStyle style = parsePaintStyle(node);*/
			SVGPath* path = new SVGPath(data, finalStyle);
			path->setGradientRegistry(&gradientRegistry);

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
				text->setGradientRegistry(&gradientRegistry);

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

SVGElement* SVGParser::createTextFromNodeWithStyle(rapidxml::xml_node<>* node, const TextPaintStyle& inheritedTextStyle)
{
	if (!node) return nullptr;

	std::string nodeName = node->name();
	std::cout << "Creating text element for: " << nodeName << std::endl;

	if (nodeName == "text") {
		std::wstring content = extractText(node);
		std::cout << "Text content length: " << content.length() << std::endl;

		if (!content.empty()) {
			std::string xStr = extractAttr(node, "x");
			std::string yStr = extractAttr(node, "y");
			
			std::cout << "Text attributes - x:" << xStr << " y:" << yStr << " inherited-font-size:" << inheritedTextStyle.fontSize << std::endl;
			
			if (!xStr.empty() && !yStr.empty()) {
				float x = stof(xStr);
				float y = stof(yStr);
				PointF startPoint{ x, y };
				
				// Parse local text style and merge with inherited
				TextPaintStyle localTextStyle = parseTextStyle(node);
				
				// Check if local element has font-size attribute
				std::string localFontSizeStr = extractAttr(node, "font-size");
				float fontSize = localFontSizeStr.empty() ? inheritedTextStyle.fontSize : std::stof(localFontSizeStr);
				
				std::cout << "Local font-size attribute: '" << localFontSizeStr << "'" << std::endl;
				std::cout << "Final fontSize will be: " << fontSize << std::endl;
				
				// Merge text styles (but font size is handled separately above)
				TextPaintStyle finalTextStyle = mergeTextStyle(inheritedTextStyle, localTextStyle);
				finalTextStyle.fontSize = fontSize; // Override with correct font size

				SVGText* text = new SVGText(content, startPoint, finalTextStyle, fontSize);
				text->setGradientRegistry(&gradientRegistry);

				std::string transformStr = extractAttr(node, "transform");
				if (!transformStr.empty()) {
					parseTransform(text, transformStr);
				}

				std::cout << "Created text successfully with font-size: " << fontSize << std::endl;
				return text;
			}
		}
	}

	std::cout << "No text element created for: " << nodeName << std::endl;
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