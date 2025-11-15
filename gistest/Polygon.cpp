#include "Polygon.h"
#include "Point.h"
#include <string>
#include <vector>
#include <cmath>

// 构造函数
myPolygon::myPolygon(LineString vExteriorRing, std::vector<LineString> vInteriorRings)
{
	mExteriorRing = vExteriorRing;
	mvInteriorRings = vInteriorRings;
	mnID = SnPolygonID;
	SnPolygonID++;
	if (!mExteriorRing.isEmpty())
	{
		updateBoundary();
	}
}
// 拷贝构造函数
myPolygon::myPolygon(const myPolygon& other)
{
	mExteriorRing = other.getExteriorRing();
	mvInteriorRings = other.getInteriorRings();
	mnID = SnPolygonID;
	SnPolygonID++;
	mdMaxX = other.getMaxX();
	mdMinX = other.getMinX();
	mdMaxY = other.getMaxY();
	mdMinY = other.getMinY();
}
// 清空
void myPolygon::clear() { mExteriorRing.clear(); mvInteriorRings.clear(); }
// 判断是否为空
bool myPolygon::isEmpty() const { return mExteriorRing.isEmpty(); }
// 更新边界
void myPolygon::updateBoundary()
{
	mdMaxX = mExteriorRing.getMaxX();
	mdMinX = mExteriorRing.getMinX();
	mdMaxY = mExteriorRing.getMaxY();
	mdMinY = mExteriorRing.getMinY();
}
// 添加外部环端点（读取文件时使用）
void myPolygon::addExteriorPoint(Point point)
{
	mExteriorRing.addPoint(point);
	updateBoundary();
}
// 插入式添加外部环端点，不破坏首尾连接
void myPolygon::insertExteriorPoint(Point point, int nIndex)
{
	if (nIndex == 0)
	{
		mExteriorRing.removePoint(mExteriorRing.getPoints().size() - 1);
		mExteriorRing.insertPoint(point, 0);
		mExteriorRing.addPoint(point);
	}
	else
		mExteriorRing.insertPoint(point, nIndex);
}
// 删除外部环端点
void myPolygon::removeExteriorPoint(Point point)
{
	mExteriorRing.removePoint(point);
	updateBoundary();
}
void myPolygon::removeExteriorPoint(int nIndex)
{
	if (nIndex == 0 || nIndex == mExteriorRing.getPoints().size() - 1) {
		mExteriorRing.removePoint(mExteriorRing.getPoints().size() - 1);
		mExteriorRing.removePoint(0);
		mExteriorRing.addPoint(mExteriorRing.getPoints()[0]);
	}
	else
		mExteriorRing.removePoint(nIndex);
	updateBoundary();
}
// 添加内部环
void myPolygon::addInteriorRing(LineString lineString)
{
	mvInteriorRings.push_back(lineString);
}
// 删除内部环
void myPolygon::removeInteriorRing(LineString lineString)
{
	auto it = std::find(mvInteriorRings.begin(), mvInteriorRings.end(), lineString);
	if (it != mvInteriorRings.end())
	{
		mvInteriorRings.erase(it);
	}
}
// 向指定内部环添加端点
void myPolygon::addInteriorPoint(Point point, int nIndex)
{
	mvInteriorRings[nIndex].addPoint(point);
}
// 删除指定内部环端点
void myPolygon::removeInteriorPoint(Point point, int nIndex)
{
	mvInteriorRings[nIndex].removePoint(point);
}
// 获取ID
int myPolygon::getID() const { return mnID; }
// 获取外部环
LineString myPolygon::getExteriorRing() const { return mExteriorRing; }
// 获取内环集
std::vector<LineString> myPolygon::getInteriorRings() const { return mvInteriorRings; }
// 获取x, y范围
double myPolygon::getMinX() const { return mdMinX; }
double myPolygon::getMaxX() const { return mdMaxX; }
double myPolygon::getMinY() const { return mdMinY; }
double myPolygon::getMaxY() const { return mdMaxY; }
// 重载运算符"=="
bool myPolygon::operator==(const myPolygon& other) const
{
	return (mnID == other.getID());
}

// 获取面积
double myPolygon::getArea() const {
	auto area = [](const LineString& ring) -> double {
		const auto& points = ring.getPoints();
		double sum = 0.0;
		const double R_MAJOR = 6378137.0;
		const double R_MINOR = 6356752.3142;
		for (size_t i = 0; i < points.size(); ++i) {
			const auto& p1 = points[i];
			const auto& p2 = points[(i + 1) % points.size()];
			sum += (R_MAJOR * qDegreesToRadians(p1.getX()) * R_MAJOR * log(tan(M_PI_4 + qDegreesToRadians(p2.getY()) / 2.0)) - (R_MAJOR * qDegreesToRadians(p2.getX()) * R_MAJOR * log(tan(M_PI_4 + qDegreesToRadians(p1.getY()) / 2.0))));
		}
		return std::abs(sum) / 2.0;
		};

	double totalArea = area(mExteriorRing);
	for (const auto& ring : mvInteriorRings) {
		totalArea -= area(ring);
	}
	return totalArea;
}

// 获取周长
double myPolygon::getPerimeter() const {
	auto perimeter = [](const LineString& ring) -> double {
		const auto& points = ring.getPoints();
		double sum = 0.0;
		for (size_t i = 0; i < points.size(); ++i) {
			const auto& p1 = points[i];
			const auto& p2 = points[(i + 1) % points.size()];
			sum += p1.geoDistance(p2);
		}
		return sum;
		};

	double totalPerimeter = perimeter(mExteriorRing);
	for (const auto& ring : mvInteriorRings) {
		totalPerimeter += perimeter(ring);
	}
	return totalPerimeter;
}

// 获取要素面积
double myPolygon::calculateArea() const {
	double area = 0.0;
	const std::vector<Point>& points = getExteriorRing().getPoints();
	int numPoints = points.size();

	const double R_MAJOR = 6378137.0;
	const double R_MINOR = 6356752.3142;
	for (size_t i = 0; i < points.size(); ++i) {
		const auto& p1 = points[i];
		const auto& p2 = points[(i + 1) % points.size()];
		area += (R_MAJOR * qDegreesToRadians(p1.getX()) * R_MAJOR * log(tan(M_PI_4 + qDegreesToRadians(p2.getY()) / 2.0)) - (R_MAJOR * qDegreesToRadians(p2.getX()) * R_MAJOR * log(tan(M_PI_4 + qDegreesToRadians(p1.getY()) / 2.0))));
	}

	area = std::abs(area) / 2.0;
	return area;
}

// 获取要素周长
double myPolygon::calculatePerimeter() const {
	double perimeter = 0.0;
	const std::vector<Point>& points = getExteriorRing().getPoints();

	for (size_t i = 0; i < points.size(); ++i) {
		const auto& p1 = points[i];
		const auto& p2 = points[(i + 1) % points.size()];
		perimeter += p1.geoDistance(p2);
	}
	return perimeter;
}