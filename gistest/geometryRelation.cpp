#include "GeometryRelation.h"

// 空间关系类
// 判断点是否与另一个点大致重合
bool geometryRelation::IsOnPoint(const Point& point1, const Point& point2)
{
	double dEPS = 1e-2;
	return (point1.distance(point2) < dEPS);
}
// 判断点是否在线上
bool geometryRelation::IsOnLine(const Point& point, const LineString& line)
{
	double dEPS = 1e-2;
	return (line.distanceToLineString(point) < dEPS);
}
// 判断点是否在闭合环内
// 注：该函数假设输入的线是首尾相连的闭合环
bool geometryRelation::IsInLineString(const Point& point, const LineString& line)
{
	// 射线法：
	int nCrossings = 0;
	std::vector<Point> vPoints = line.getPoints();
	// 遍历每条边
	for (int i = 0; i < vPoints.size(); i++) {
		const Point& p1 = vPoints[i];
		const Point& p2 = vPoints[(i + 1) % vPoints.size()];

		// 检查射线是否与边相交
		if (((p1.getY() <= point.getY() && point.getY() < p2.getY()) || (p2.getY() <= point.getY() && point.getY() < p1.getY())) &&
			(point.getX() < (p2.getX() - p1.getX()) * (point.getY() - p1.getY()) / (p2.getY() - p1.getY()) + p1.getX())) {
			nCrossings++;
		}
	}
	// 如果交点数为奇数，点在多边形内部
	return (nCrossings % 2 == 1);
}
// 判断点是否在多边形内
bool geometryRelation::IsInPolygon(const Point& point, const myPolygon& polygon)
{
	if (IsInLineString(point, polygon.getExteriorRing()))
	{
		for (int i = 0; i < polygon.getInteriorRings().size(); i++)
		{
			if (IsInLineString(point, polygon.getInteriorRings()[i]))
				return false;
		}
		return true;
	}
	return false;
}