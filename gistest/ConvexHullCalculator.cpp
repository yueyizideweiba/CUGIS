#include "ConvexHullCalculator.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <vector>

ConvexHullCalculator::ConvexHullCalculator() {}

// 极角比较函数
bool ConvexHullCalculator::polarCompare(const Point* p1, const Point* p2, const Point* origin) {
	double dx1 = p1->getX() - origin->getX();
	double dy1 = p1->getY() - origin->getY();
	double dx2 = p2->getX() - origin->getX();
	double dy2 = p2->getY() - origin->getY();
	return atan2(dy1, dx1) < atan2(dy2, dx2);
}

// 计算向量叉积，确定点的转向关系
double ConvexHullCalculator::crossProduct(const Point* p1, const Point* p2, const Point* p3) {
	return (p2->getX() - p1->getX()) * (p3->getY() - p1->getY()) -
		(p2->getY() - p1->getY()) * (p3->getX() - p1->getX());
}

// 构建凸包
Store ConvexHullCalculator::compute(const Store& inputStore) {
	std::vector<Point*> allPoints;

	// 收集点要素
	for (const auto& point : inputStore.getStorePoints()) {
		allPoints.push_back(point);
	}

	// 收集线要素的端点
	for (const auto& line : inputStore.getStoreLines()) {
		for (const Point& p : line->getPoints()) {
			allPoints.push_back(new Point(p));
		}
	}

	// 收集面要素的外环和内环的所有点
	for (const auto& polygon : inputStore.getStorePolygons()) {
		for (const Point& p : polygon->getExteriorRing().getPoints()) {
			allPoints.push_back(new Point(p));
		}
		for (const auto& ring : polygon->getInteriorRings()) {
			for (const Point& p : ring.getPoints()) {
				allPoints.push_back(new Point(p));
			}
		}
	}

	if (allPoints.size() < 3) {
		throw std::invalid_argument("点的数量不足以计算凸包");
	}

	// 寻找最低且最左边的点作为起点
	Point* origin = *std::min_element(allPoints.begin(), allPoints.end(), [](const Point* p1, const Point* p2) {
		return p1->getY() < p2->getY() || (p1->getY() == p2->getY() && p1->getX() < p2->getX());
		});

	// 根据极角排序
	std::sort(allPoints.begin(), allPoints.end(), [&origin](const Point* p1, const Point* p2) {
		return polarCompare(p1, p2, origin);
		});

	// 构建凸包
	std::vector<Point*> hull;
	for (auto& point : allPoints) {
		while (hull.size() >= 2 && crossProduct(hull[hull.size() - 2], hull[hull.size() - 1], point) <= 0) {
			hull.pop_back();  // 移除非凸点
		}
		hull.push_back(point);
	}

	// 创建新的图层存储并添加凸包多边形
	Store convexHullStore;
	if (hull.size() > 2) {
		std::vector<Point> polygonPoints;
		for (Point* p : hull) {
			polygonPoints.push_back(*p);
		}
		// 关闭环路
		polygonPoints.push_back(*hull.front());
		LineString exteriorRing(polygonPoints);
		myPolygon* convexHullPolygon = new myPolygon(exteriorRing);
		convexHullStore.addPolygon(convexHullPolygon);
	}

	return convexHullStore;
}
