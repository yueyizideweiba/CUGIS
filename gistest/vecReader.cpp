#include "vecReader.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "Store.h"
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
#include "gdal.h"
#include "GISlog.h"

// 读取矢量数据的接口函数
void vecReader::readWKT(std::string strPath, Store& store) {
	readFileWKTFeature read;
	read.VectorStole(strPath, store);
	Logger::getInstance().logFileOpenSuccess(strPath);
}

void vecReader::readGeojson(std::string strFilePath, Store& store) {
	std::ifstream ifs(strFilePath);
	std::string strGeojson((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	readFileGeoJsonFeature read;
	read.parseGeojson(strGeojson, store);
	Logger::getInstance().logFileOpenSuccess(strGeojson);
}

void vecReader::readShapefile(const char* shapefilePath, Store& store) {
	readFileShapefileFeature read;
	read.readSHP(shapefilePath, store);
	Logger::getInstance().logFileOpenSuccess(shapefilePath);
}

// 读取矢量数据的实现函数
void vecReader::readFileShapefileFeature::readSHP(const char* shapefilePath, Store& store) {
	GDALAllRegister();

	GDALDataset* poDS = (GDALDataset*)GDALOpenEx(shapefilePath, GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (poDS == NULL) {
		std::cerr << "打开文件失败，可能因为路径输入不正确，也可能因为路径中有中文。" << std::endl;
		Logger::getInstance().logFileOpenError(shapefilePath);
		return;
	}

	OGRLayer* poLayer = poDS->GetLayer(0); // 获取第一个图层
	if (poLayer == nullptr) {
		std::cerr << "获取图层失败。" << std::endl;
		Logger::getInstance().logFunctionFail("Failed to get layer");
		GDALClose(poDS);
		return;
	}

	OGRFeature* poFeature;
	poLayer->ResetReading(); // 重置图层的读取

	// 逐个读取要素
	while ((poFeature = poLayer->GetNextFeature()) != NULL) {
		OGRGeometry* poGeometry = poFeature->GetGeometryRef();
		if (poGeometry != NULL) {
			OGRwkbGeometryType geomType = wkbFlatten(poGeometry->getGeometryType());
			std::map<QString, QString> attributes;
			OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
			// 获取要素的属性
			for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
				OGRFieldDefn* poFieldDefn = poFDefn->GetFieldDefn(i);
				attributes[poFieldDefn->GetNameRef()] = poFeature->GetFieldAsString(i);
			}

			// 根据几何类型将几何对象存储到store中
			if (geomType == wkbPoint) {
				OGRPoint* point = dynamic_cast<OGRPoint*>(poGeometry);
				if (point != nullptr) {
					store.addPoint(new Point(point->getX(), point->getY()), attributes);
				}
			}
			else if (geomType == wkbLineString) {
				OGRLineString* Line = dynamic_cast<OGRLineString*>(poGeometry);
				std::vector<Point> vInputLine;
				if (Line != nullptr) {
					for (int i = 0; i < Line->getNumPoints(); ++i) {
						OGRPoint point;
						Line->getPoint(i, &point);
						vInputLine.push_back(Point(point.getX(), point.getY()));
					}
					store.addLine(new LineString(vInputLine), attributes);
				}
			}
			else if (geomType == wkbPolygon || geomType == wkbMultiPolygon) {
				if (geomType == wkbPolygon) {
					OGRPolygon* poPolygon = (OGRPolygon*)poGeometry;
					readPolygon(poPolygon, store, attributes);
				}
				else if (geomType == wkbMultiPolygon) {
					OGRMultiPolygon* poMultiPolygon = (OGRMultiPolygon*)poGeometry;
					for (int i = 0; i < poMultiPolygon->getNumGeometries(); ++i) {
						OGRPolygon* poPolygon = (OGRPolygon*)poMultiPolygon->getGeometryRef(i);
						readPolygon(poPolygon, store, attributes);
					}
				}
			}
		}
		OGRFeature::DestroyFeature(poFeature);
	}

	GDALClose(poDS);
}

// 读取解析面对象
void vecReader::readFileShapefileFeature::readPolygon(OGRGeometry* poGeometry, Store& store, const std::map<QString, QString>& attributes) {
	OGRPolygon* _OGRPoly = dynamic_cast<OGRPolygon*>(poGeometry);
	OGRLinearRing* poExteriorRing = _OGRPoly->getExteriorRing();
	std::vector<Point> _vInnerLine;
	std::vector<LineString> _vOuterLine;
	if (poExteriorRing != nullptr) {
		for (int i = 0; i < poExteriorRing->getNumPoints(); ++i) {
			_vInnerLine.push_back(Point(poExteriorRing->getX(i), poExteriorRing->getY(i)));
		}
	}
	for (int i = 0; i < _OGRPoly->getNumInteriorRings(); ++i) {
		OGRLinearRing* poInteriorRing = _OGRPoly->getInteriorRing(i);
		std::vector<Point> vInteriorRing;
		for (int j = 0; j < poInteriorRing->getNumPoints(); ++j) {
			vInteriorRing.push_back(Point(poInteriorRing->getX(j), poInteriorRing->getY(j)));
		}
		LineString linestring1(vInteriorRing);
		_vOuterLine.push_back(linestring1);
	}
	LineString linestring2(_vInnerLine);
	store.addPolygon(new myPolygon(linestring2, _vOuterLine), attributes);
}

// 读取WKT格式中的点对象并存储到store中
void vecReader::readFileWKTFeature::ADDpoint(OGRGeometry* geometry, Store& store) {
	OGRPoint* point = dynamic_cast<OGRPoint*>(geometry);
	if (point != nullptr) {
		store.addPoint(new Point(point->getX(), point->getY()));
	}
}

// 读取WKT格式中的线对象并存储到store中
void vecReader::readFileWKTFeature::ADDLine(OGRGeometry* geometry, Store& store) {
	OGRLineString* Line = dynamic_cast<OGRLineString*>(geometry);
	std::vector<Point> vInputLine;
	if (Line != nullptr) {
		for (int i = 0; i < Line->getNumPoints(); ++i) {
			OGRPoint point;
			Line->getPoint(i, &point);
			vInputLine.push_back(Point(point.getX(), point.getY()));
		}
		store.addLine(new LineString(vInputLine));
	}
}

// 读取WKT格式中的面对象并存储到store中
void vecReader::readFileWKTFeature::ADDPloyon(OGRGeometry* geometry, Store& store) {
	OGRPolygon* _OGRPoly = dynamic_cast<OGRPolygon*>(geometry);
	OGRLinearRing* poExteriorRing = _OGRPoly->getExteriorRing();
	std::vector<Point> _vInnerLine;
	std::vector<LineString> _vOuterLine;
	if (poExteriorRing != nullptr) {
		for (int i = 0; i < poExteriorRing->getNumPoints(); ++i) {
			_vInnerLine.push_back(Point(poExteriorRing->getX(i), poExteriorRing->getY(i)));
		}
	}
	for (int i = 0; i < _OGRPoly->getNumInteriorRings(); ++i) {
		OGRLinearRing* poInteriorRing = _OGRPoly->getInteriorRing(i);
		std::vector<Point> vInteriorRing;
		for (int j = 0; j < poInteriorRing->getNumPoints(); ++j) {
			vInteriorRing.push_back(Point(poInteriorRing->getX(j), poInteriorRing->getY(j)));
		}
		LineString linestring1(vInteriorRing);
		_vOuterLine.push_back(linestring1);
	}
	LineString linestring2(_vInnerLine);
	store.addPolygon(new myPolygon(linestring2, _vOuterLine));
}

// 读取WKT格式中的多面对象并存储到store中
void vecReader::readFileWKTFeature::multiPolyon(OGRGeometry* geometry, Store& store) {
	OGRMultiPolygon* poMultiPolygon = dynamic_cast<OGRMultiPolygon*>(geometry);
	for (int i = 0; i < poMultiPolygon->getNumGeometries(); ++i) {
		OGRPolygon* poPolygon = dynamic_cast<OGRPolygon*>(poMultiPolygon->getGeometryRef(i));
		ADDPloyon(poPolygon, store);
	}
}

// 读取WKT格式中所有要素并储存
int vecReader::readFileWKTFeature::VectorStole(std::string Path, Store& store) {
	std::ifstream file(Path);

	// 检查文件是否成功打开
	if (!file.is_open()) {
		std::cerr << "无法打开文件: " << Path << std::endl; // 日志写入处
		return 1;
	}

	std::string line;
	// 逐行读取 WKT 文件
	while (std::getline(file, line)) { // 日志写入处
		const std::string inputline = line;
		const char* pointString = inputline.c_str();  // 声明一个指向字符串常量的指针
		char* pointerToString = const_cast<char*>(pointString);  // 强制类型转换，将 const char* 转换为 char*
		char** A1 = &pointerToString;
		GDALAllRegister();
		OGRGeometry* geometry;
		OGRGeometryFactory::createFromWkt(A1, nullptr, &geometry);
		if (geometry != nullptr) {
			if (geometry->getGeometryType() == wkbPoint) {
				ADDpoint(geometry, store);
			}
			if (geometry->getGeometryType() == wkbLineString) {
				ADDLine(geometry, store);
			}
			if (geometry->getGeometryType() == wkbPolygon) {
				ADDPloyon(geometry, store);
			}
			if (geometry->getGeometryType() == wkbMultiPolygon) {
				multiPolyon(geometry, store);
			}
			// 释放几何对象
			OGRGeometryFactory::destroyGeometry(geometry);
		}
		else {
			std::cerr << "解析 WKT 字符串失败: " << line << std::endl; // 日志写入处
		}
	}

	// 关闭文件
	file.close();
	return 0;
}

// 读取 GeoJSON 数据的实现函数
void vecReader::readFileGeoJsonFeature::parseGeojson(const std::string& strGeojson, Store& store) {
	// 解析 GeoJSON 字符串为 DOM 对象
	rapidjson::Document doc;
	doc.Parse(strGeojson.c_str());
	// 判断是否解析成功
	if (doc.HasParseError()) {
		std::cout << "JSON parse error: " << doc.GetParseError() << std::endl;
	}
	std::string strType = doc["type"].GetString();
	// 判断类型
	if (strType == "FeatureCollection") {
		for (auto& feature : doc["features"].GetArray()) {
			parseFeature(feature, store);
		}
	}
	else {
		parseFeature(doc, store);
	}
}

// 解析GeoJSON特征对象并将几何信息存储到Store中
void vecReader::readFileGeoJsonFeature::parseFeature(const rapidjson::Value& feature, Store& store) {
	const rapidjson::Value& geometry = feature["geometry"];  // 获取几何对象
	std::map<QString, QString> attributes;  // 初始化属性映射表
	parseGeometry(geometry, store, attributes);  // 解析几何并存储
}

// 解析GeoJSON几何对象，并根据其类型将其存储到Store中
void vecReader::readFileGeoJsonFeature::parseGeometry(const rapidjson::Value& geometry, Store& store, const std::map<QString, QString>& attributes) {
	std::string strType = geometry["type"].GetString();  // 获取几何类型

	if (strType == "Point") {  // 处理点类型
		store.addPoint(new Point(geometry["coordinates"][0].GetDouble(), geometry["coordinates"][1].GetDouble()), attributes);
	}
	else if (strType == "LineString") {  // 处理线类型
		std::vector<Point> vPoints;
		for (auto& point : geometry["coordinates"].GetArray()) {  // 遍历线的所有点
			vPoints.push_back(Point(point[0].GetDouble(), point[1].GetDouble()));
		}
		store.addLine(new LineString(vPoints), attributes);
	}
	else if (strType == "Polygon") {  // 处理多边形类型
		myPolygon polygon;
		// 添加外环点
		for (auto& point : geometry["coordinates"].GetArray()[0].GetArray()) {
			polygon.addExteriorPoint(Point(point[0].GetDouble(), point[1].GetDouble()));
		}
		// 添加内环（如果有）
		for (int i = 1; i < geometry["coordinates"].GetArray().Size(); i++) {
			LineString interiorRing;
			for (auto& point : geometry["coordinates"].GetArray()[i].GetArray()) {
				interiorRing.addPoint(Point(point[0].GetDouble(), point[1].GetDouble()));
			}
			polygon.addInteriorRing(interiorRing);
		}
		store.addPolygon(new myPolygon(polygon), attributes);
	}
	else if (strType == "MultiPoint") {  // 处理多点类型
		for (auto& point : geometry["coordinates"].GetArray()) {
			store.addPoint(new Point(point[0].GetDouble(), point[1].GetDouble()), attributes);
		}
	}
	else if (strType == "MultiLineString") {  // 处理多线类型
		for (auto& line : geometry["coordinates"].GetArray()) {
			std::vector<Point> _vPoints;
			for (auto& point : line.GetArray()) {  // 遍历每条线的所有点
				_vPoints.push_back(Point(point[0].GetDouble(), point[1].GetDouble()));
			}
			store.addLine(new LineString(_vPoints), attributes);
		}
	}
	else if (strType == "MultiPolygon") {  // 处理多多边形类型
		for (auto& polygon : geometry["coordinates"].GetArray()) {
			myPolygon _polygon;
			// 添加外环点
			for (auto& point : polygon.GetArray()[0].GetArray()) {
				_polygon.addExteriorPoint(Point(point[0].GetDouble(), point[1].GetDouble()));
			}
			// 添加内环（如果有）
			for (int i = 1; i < polygon.GetArray().Size(); i++) {
				LineString interiorRing;
				for (auto& point : polygon.GetArray()[i].GetArray()) {
					interiorRing.addPoint(Point(point[0].GetDouble(), point[1].GetDouble()));
				}
				_polygon.addInteriorRing(interiorRing);
			}
			store.addPolygon(new myPolygon(_polygon), attributes);
		}
	}
	else if (strType == "GeometryCollection") {  // 处理几何集合类型
		for (auto& geom : geometry["geometries"].GetArray()) {  // 遍历集合中的每个几何对象
			parseGeometry(geom, store, attributes);  // 递归解析几何对象
		}
	}
}
