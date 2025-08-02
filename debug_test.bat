@echo off
echo Testing SVG Renderer with debug info
echo ====================================

echo.
echo Available SVG files in svg_samples:
dir /b svg_samples\*.svg

echo.
echo Testing with test.svg...
echo.

if exist "x64\Debug\SVGDemo.exe" (
    "x64\Debug\SVGDemo.exe" "test.svg"
    echo.
    echo Exit code: %errorlevel%
) else (
    echo Error: SVGDemo.exe not found in x64\Debug\
    echo Please build the project first.
)

echo.
pause
