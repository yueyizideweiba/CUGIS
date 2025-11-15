/************************************************************
FileName: BufferAnalyzer.h
Author: 谢宇瀚
Version : 1.9
Date: 2024-07-28
Description: 缓冲区分析类，用于计算要素缓冲区
Function List:
1. BufferAnalyzer - 构造函数
2. computeBuffer - 计算指定要素的缓冲区
3. createCircularBuffer - 创建点的圆形缓冲区
4. createLineBuffer - 创建线要素的缓冲区
5. createPolygonBuffer - 创建多边形要素的缓冲区
6. createMyPolygonFromGeometry - 从GEOS几何对象创建myPolygon对象
7. mergeOverlappingBuffers - 合并重叠的缓冲区
***********************************************************/

#ifndef BUFFERANALYZER_H
#define BUFFERANALYZER_H

#include "Store.h"
#include "Polygon.h"
#include "myGraphicsView.h"
#include <QString>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>

class BufferAnalyzer {
public:
	BufferAnalyzer();
	Store computeBuffer(const Store& inputStore, double radius, bool merge); // 计算缓冲区

private:
	myPolygon* createCircularBuffer(Point* point, double radius); // 创建点的圆形缓冲区
	myPolygon* createLineBuffer(LineString* line, double radius); // 创建线的缓冲区
	std::vector<myPolygon*> createPolygonBuffer(myPolygon* polygon, double radius); // 创建多边形的缓冲区
	myPolygon* createMyPolygonFromGeometry(const geos::geom::Geometry* geometry); // 从GEOS对象创建myPolygon
	Store mergeOverlappingBuffers(const Store& bufferStore); // 合并重叠的缓冲区
};

#endif
