#ifndef GEOMETRYRELATION_H
#define GEOMETRYRELATION_H

#include "Point.h"
#include "LineString.h"
#include "Polygon.h"

// 空间关系类
class geometryRelation
{
public:
	// 判断点是否与另一个点大致重合
	static bool IsOnPoint(const Point& point1, const Point& point2);
	// 判断点是否在线上
	static bool IsOnLine(const Point& point, const LineString& line);
	// 判断点是否在闭合环内
	// 注：该函数假设输入的线是首尾相连的闭合环
	static bool IsInLineString(const Point& point, const LineString& line);
	// 判断点是否在多边形内
	static bool IsInPolygon(const Point& point, const myPolygon& polygon);
};

#endif