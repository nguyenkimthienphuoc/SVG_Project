@echo off
echo SVG Renderer - Simple Command Line Tool
echo ========================================

if "%1"=="" (
    echo No filename provided.
    echo.
    echo Usage: run_svg.bat filename.svg
    echo Example: run_svg.bat logo.svg
    echo.
    echo The file will be loaded from the svg_samples folder.
    echo.
    pause
    
    if exist "x64\Debug\SVGDemo.exe" (
        "x64\Debug\SVGDemo.exe"
    ) else (
        echo Error: SVGDemo.exe not found in x64\Debug\
        echo Please build the project first.
        pause
        exit /b 1
    )
) else (
    echo Loading: %1 from svg_samples folder
    echo.
    
    if exist "x64\Debug\SVGDemo.exe" (
        "x64\Debug\SVGDemo.exe" "%1"
    ) else (
        echo Error: SVGDemo.exe not found in x64\Debug\
        echo Please build the project first.
        pause
        exit /b 1
    )
)
