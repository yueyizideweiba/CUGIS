/************************************************************
FileName: vecReader.h
Author: 谢宇瀚 曾亦凡 叶昱彤
Version : 1.0
Date: 2024-06-29
Description: 矢量数据读取类，支持读取WKT、GeoJSON和Shapefile文件格式
Function List:
1. readWKT - 读取WKT格式文件并将数据存储到Store对象中
2. readGeojson - 读取GeoJSON格式文件并将数据存储到Store对象中
3. readShapefile - 读取Shapefile格式文件并将数据存储到Store对象中

Private Classes:
1. readFileShapefileFeature - 处理Shapefile文件的读取和解析
   - readSHP - 读取Shapefile文件
   - readPolygon - 解析多边形要素
2. readFileWKTFeature - 处理WKT文件的读取和解析
   - ADDpoint - 解析点要素
   - ADDLine - 解析线要素
   - ADDPloyon - 解析多边形要素
   - multiPolyon - 解析多多边形要素
   - VectorStole - 解析矢量数据并存储到Store对象中
3. readFileGeoJsonFeature - 处理GeoJSON文件的读取和解析
   - parseGeojson - 解析GeoJSON数据
   - parseFeature - 解析GeoJSON特征
   - parseGeometry - 解析GeoJSON几何
***********************************************************/

#ifndef READ_VEC_H
#define READ_VEC_H

#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
#include <iostream>
#include <vector>
#include "gdal.h"
#include "ogrsf_frmts.h"
#include "Store.h"
#include <fstream>

class vecReader {
public:
	// 读取WKT文件
	void readWKT(std::string strPath, Store& store);

	// 读取GEOJSON文件
	void readGeojson(std::string strFilePath, Store& store);

	// 读取shp文件
	void readShapefile(const char* shapefilePath, Store& store);

private:
	// 读取shp文件要素
	class readFileShapefileFeature {
	public:
		void readSHP(const char* shapefilePath, Store& store);
		void readPolygon(OGRGeometry* poGeometry, Store& store, const std::map<QString, QString>& attributes);
	};
	// 读取WKT文件要素
	class readFileWKTFeature {
	public:
		void ADDpoint(OGRGeometry* geometry, Store& Storen);
		void ADDLine(OGRGeometry* geometry, Store& Storen);
		void ADDPloyon(OGRGeometry* geometry, Store& Storen);
		void multiPolyon(OGRGeometry* geometry, Store& Storen);
		int VectorStole(std::string Path, Store& Storen);
	};
	// 读取geojson文件要素
	class readFileGeoJsonFeature {
	public:
		void parseGeojson(const std::string& strGeojson, Store& store);
		void parseFeature(const rapidjson::Value& feature, Store& store);
		void parseGeometry(const rapidjson::Value& geometry, Store& store, const std::map<QString, QString>& attributes);
	};
};

#endif