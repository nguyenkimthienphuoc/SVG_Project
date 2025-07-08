#pragma once
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
#include <string>

std::string readSVGFile(const std::string& filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();  // Trả về nội dung file SVG dưới dạng string
}
//class SVGParser