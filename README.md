# CUGIS

## Project Overview

CUGIS is a desktop Geographic Information System (GIS) application developed based on the Qt framework, specifically designed for geospatial data processing and analysis. This project was developed by students from China University of Geosciences and integrates various GIS functionalities including vector data management, raster data analysis, and spatial analysis.

## Features

### 🗺️ Data Management
- **Vector Data Support**: Reading, displaying, and editing vector features such as points, lines, and polygons
- **Raster Data Support**: Reading and displaying raster formats like TIFF
- **Layer Management**: Multi-layer overlay display with layer order adjustment and visibility control
- **Project Files**: Support for saving and loading project files

### 📊 Spatial Analysis
- **Convex Hull Calculation**: Automatic calculation of convex hull for point sets
- **Buffer Analysis**: Creating buffers of specified distance for features
- **Overlay Analysis**: Spatial overlay operations between layers
- **Delaunay Triangulation**: Building Delaunay triangulation networks
- **Voronoi Diagram**: Generating Voronoi polygons

### 🛠️ Editing Tools
- **Feature Creation**: Interactive creation of points, lines, and polygons
- **Feature Editing**: Support for vertex addition, deletion, and movement operations
- **Attribute Editing**: Editing of feature attribute data
- **Undo/Redo**: Complete edit history functionality

### 📈 Statistical Analysis
- **Feature Statistics**: Counting features in layers
- **Geometric Calculations**: Calculating geometric properties like area, perimeter, and length
- **Raster Statistics**: Statistical analysis of raster data

### 🌄 Raster Analysis
- **Grayscale Histogram**: Displaying grayscale distribution of raster data
- **Color Composition**: Support for true color and false color image generation
- **Neighborhood Analysis**: Neighborhood statistics like mean, maximum, and minimum
- **Terrain Analysis**: Calculating terrain factors like slope and aspect
- **Flow Direction Analysis**: D8+ raster flow direction analysis

### 🎨 Visualization & Interface
- **Coordinate System**: Support for WGS84 (EPSG:4326) and Web Mercator (EPSG:3857) coordinate display
- **Symbol System**: Customizable display styles for points, lines, and polygons
- **Theme Switching**: Support for light and dark theme switching
- **Zoom Navigation**: Map zooming, panning, and other navigation operations

## Technical Architecture

### Development Environment
- **Programming Language**: C++
- **GUI Framework**: Qt 6.6.1
- **Development Tool**: Visual Studio 2022
- **Build System**: MSBuild

### Core Dependencies
- **GEOS Library**: Geometry engine for spatial analysis operations
- **GDAL/OGR**: Geographic data abstraction library supporting multiple data formats
- **Exiv2**: Image metadata processing library
- **RapidJSON**: JSON parsing library

### Project Structure
```
CUGISv1.5/
├── gistest/                 # Main project directory
│   ├── *.cpp               # Source files
│   ├── *.h                 # Header files
│   ├── gistest.vcxproj     # Visual Studio project file
│   ├── gistest.ui          # Qt Designer interface file
│   ├── icon/               # Icon resources
│   └── thirdlib/           # Third-party libraries
├── gistest.sln             # Visual Studio solution file
└── README.md              # Project documentation
```

## Quick Start

### Environment Requirements
- Windows 10/11 64-bit
- Visual Studio 2022
- Qt 6.6.1 (MSVC 2019 64-bit)

### Compilation Steps
1. Clone the project locally
2. Open `gistest.sln` with Visual Studio 2022
3. Configure Qt environment variables, ensuring correct Qt installation path
4. Select x64 platform and Release configuration
5. Compile the project

### Running the Program
After successful compilation, find the executable file in the `x64/Release/` directory.

## User Guide

### Basic Operations
1. **Open Data**: Open vector or raster data through the menu bar
2. **Layer Management**: Manage layer display and order in the layer tree
3. **Map Navigation**: Use mouse wheel for zooming, drag for panning
4. **Coordinate Viewing**: Status bar displays current mouse position geographic coordinates

### Spatial Analysis
1. Select the layer to analyze
2. Choose the corresponding analysis function from the analysis menu
3. Set analysis parameters
4. View analysis results

### Data Editing
1. Enter edit mode
2. Select features to edit or create new features
3. Perform editing operations
4. Save editing results

## Development Team

- **Zeng Yifan** - Main Developer
- **Xie Yuhan** - Main Developer

## License

This project is for learning and research purposes only.

## Changelog

### v2.3 (2024-08-17)
- Added raster terrain analysis functionality
- Optimized coordinate display system
- Improved user interface experience
- Fixed known issues

### v2.0
- Refactored project architecture
- Added multiple spatial analysis functions
- Improved data management mechanisms

## Contribution Guidelines

Welcome to submit Issues and Pull Requests to improve the project.

## Contact

If you have questions or suggestions, please contact us through GitHub Issues.
