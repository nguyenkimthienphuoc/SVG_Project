// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files
#include <windows.h>
#include <objidl.h>     // For IStream
#include <ole2.h>       // For COM and OLE interfaces

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// GDI+ Graphics
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")  // Link with GDI+ library
using namespace Gdiplus;

// External libraries
#include "rapidxml.hpp" 

// STL
#include <fstream>  
#include <sstream>
#include <string>
#include <vector>

// Project headers
#include "SVGBasics.h"
#include "SVGElement.h"
#include "SVGCircle.h"
#include "SVGRect.h"
#include "SVGLine.h"
#include "SVGText.h"
#include "SVGEllipse.h"
#include "SVGPath.h"
#include "SVGParser.h"
#include "SVGPolygon.h"
#include "SVGPolyline.h"
#include "SVGPath.h"
