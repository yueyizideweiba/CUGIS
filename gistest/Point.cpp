#include "Point.h"
#include <string>
#include <cmath>

// 构造函数
Point::Point(double dx, double dy)
{
	mdX = dx;
	mdY = dy;
	mnID = SnPointID;
	SnPointID++;
	mmapFieldValue = {};
}
// 拷贝构造函数
Point::Point(const Point& other)
{
	mdX = other.getX();
	mdY = other.getY();
	mnID = SnPointID;
	SnPointID++;
	mmapFieldValue = other.mmapFieldValue;
}
// 获取ID
int Point::getID() const { return mnID; }
// 获取x坐标
double Point::getX() const { return mdX; }
// 获取y坐标
double Point::getY() const { return mdY; }
// 设置x坐标
double Point::setX(double dx) { return mdX = dx; }
// 设置y坐标
double Point::setY(double dy) { return mdY = dy; }
// 计算两个点的几何距离
double Point::distance(const Point& other) const
{
	return std::sqrt((mdX - other.getX()) * (mdX - other.getX()) + (mdY - other.getY()) * (mdY - other.getY()));
}
// 计算两个地理坐标的实际距离
double Point::geoDistance(const Point& other) const
{
	const double R_MAJOR = 6378137.0;
	const double R_MINOR = 6356752.3142;
	return std::sqrt((R_MAJOR * qDegreesToRadians(mdX) - R_MAJOR * qDegreesToRadians(other.getX())) * (R_MAJOR * qDegreesToRadians(mdX) - R_MAJOR * qDegreesToRadians(other.getX())) + (R_MAJOR * log(tan(M_PI_4 + qDegreesToRadians(mdY) / 2.0)) - R_MAJOR * log(tan(M_PI_4 + qDegreesToRadians(other.getY()) / 2.0))) * (R_MAJOR * log(tan(M_PI_4 + qDegreesToRadians(mdY) / 2.0)) - R_MAJOR * log(tan(M_PI_4 + qDegreesToRadians(other.getY()) / 2.0))));
}
// 计算点到线段的距离
double Point::distanceToSegment(const Point& p1, const Point& p2) const
{
	// 计算向量
	double dx1 = p2.getX() - p1.getX();
	double dy1 = p2.getY() - p1.getY();
	double dx2 = mdX - p1.getX();
	double dy2 = mdY - p1.getY();
	if (dx1 * dx2 + dy1 * dy2 == 0) {
		return distance(p1);
	}
	// 计算投影点
	double t = (dx2 * dx1 + dy2 * dy1) / (dx1 * dx1 + dy1 * dy1);
	if (t <= 0)
		return distance(p1);
	else if (t >= 1)
		return distance(p2);
	else
		return distance(Point(p1.getX() + t * dx1, p1.getY() + t * dy1));
}
// 重载运算符"=="
bool Point::operator==(const Point& other) const
{
	return (mnID == other.getID());
}

// 设置属性数据
void Point::setFieldValue(const QString& key, const QString& value)
{
	mmapFieldValue.insert(key, value);
}
// 获取属性数据
QString Point::getFieldValue(const QString& key) const
{
	return mmapFieldValue.value(key);
}
// 获取属性表
QHash<QString, QString> Point::getFieldValueMap() const
{
	return mmapFieldValue;
}