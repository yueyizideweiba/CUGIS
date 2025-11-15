#include "LineString.h"
#include "Point.h"
#include <vector>
#include <string>

// 构造函数
LineString::LineString(std::vector<Point> vPoints)
{
	mvPoints = vPoints;
	mnID = SnLineStringID;
	SnLineStringID++;
	if (!mvPoints.empty())
	{
		mdMaxX = vPoints[0].getX();
		mdMaxY = vPoints[0].getY();
		mdMinX = vPoints[0].getX();
		mdMinY = vPoints[0].getY();
		updateMaxY();
		updateMinY();
		updateMaxX();
		updateMinX();
	}
}
// 拷贝构造函数
LineString::LineString(const LineString& other)
{
	mvPoints = other.getPoints();
	mnID = SnLineStringID;
	SnLineStringID++;
	mdMaxX = other.getMaxX();
	mdMaxY = other.getMaxY();
	mdMinX = other.getMinX();
	mdMinY = other.getMinY();
}
// 清空
void LineString::clear() { mvPoints.clear(); };
// 判断是否为空
bool LineString::isEmpty() const
{
	return mvPoints.empty();
}
// 更新上边界
void LineString::updateMaxY()
{
	for (int i = 0; i < mvPoints.size(); i++)
	{
		mdMaxY = std::max(mdMaxY, mvPoints[i].getY());
	}
}
// 更新下边界
void LineString::updateMinY()
{
	for (int i = 0; i < mvPoints.size(); i++)
	{
		mdMinY = std::min(mdMinY, mvPoints[i].getY());
	}
}
// 更新左边界
void LineString::updateMinX()
{
	for (int i = 0; i < mvPoints.size(); i++)
	{
		mdMinX = std::min(mdMinX, mvPoints[i].getX());
	}
}
// 更新右边界
void LineString::updateMaxX()
{
	for (int i = 0; i < mvPoints.size(); i++)
	{
		mdMaxX = std::max(mdMaxX, mvPoints[i].getX());
	}
}
// 添加端点
void LineString::addPoint(Point point)
{
	if (mvPoints.empty())
	{
		mdMaxX = point.getX();
		mdMaxY = point.getY();
		mdMinX = point.getX();
		mdMinY = point.getY();
	}
	else
	{
		mdMaxX = std::max(mdMaxX, point.getX());
		mdMaxY = std::max(mdMaxY, point.getY());
		mdMinX = std::min(mdMinX, point.getX());
		mdMinY = std::min(mdMinY, point.getY());
	}
	mvPoints.push_back(point);
}
// 插入端点
void LineString::insertPoint(Point point, int nIndex)
{
	mvPoints.insert(mvPoints.begin() + nIndex, point);
}
// 删除端点
void LineString::removePoint(Point point)
{
	auto it = std::find(mvPoints.begin(), mvPoints.end(), point);
	if (it != mvPoints.end())
	{
		mvPoints.erase(it);
	}
	if (point.getX() == mdMinX) { updateMinX(); }
	if (point.getX() == mdMaxX) { updateMaxX(); }
	if (point.getY() == mdMinY) { updateMinY(); }
	if (point.getY() == mdMaxY) { updateMaxY(); }
}
void LineString::removePoint(int nIndex)
{
	if (nIndex >= 0 && nIndex < mvPoints.size())
	{
		if (mvPoints[nIndex].getX() == mdMinX) { updateMinX(); }
		if (mvPoints[nIndex].getX() == mdMaxX) { updateMaxX(); }
		if (mvPoints[nIndex].getY() == mdMinY) { updateMinY(); }
		if (mvPoints[nIndex].getY() == mdMaxY) { updateMaxY(); }
		mvPoints.erase(mvPoints.begin() + nIndex);
	}
}
// 获取ID
int LineString::getID() const { return mnID; }
// 获取端点集
std::vector<Point> LineString::getPoints() const { return mvPoints; }
// 获取x, y范围
double LineString::getMinX() const { return mdMinX; }
double LineString::getMaxX() const { return mdMaxX; }
double LineString::getMinY() const { return mdMinY; }
double LineString::getMaxY() const { return mdMaxY; }
// 重载运算符"=="
bool LineString::operator==(const LineString& other) const
{
	return (mnID == other.getID());
}

// 获取长度
double LineString::getLength() const {
	double length = 0.0;
	for (size_t i = 0; i < mvPoints.size() - 1; ++i) {
		length += mvPoints[i].geoDistance(mvPoints[i + 1]);
	}
	return length;
}

double LineString::calculateLength() const {
	double totalLength = 0.0;
	const std::vector<Point>& points = getPoints();

	if (points.size() < 2) {
		return totalLength; // 如果点数少于2，长度为0
	}

	for (size_t i = 1; i < points.size(); ++i) {
		// 计算连续两个点之间的欧几里得距离，并将其累加
		double deltaX = points[i].getX() - points[i - 1].getX();
		double deltaY = points[i].getY() - points[i - 1].getY();
		totalLength += std::sqrt(deltaX * deltaX + deltaY * deltaY);
	}

	return totalLength;
}

// 计算点到折线的最短距离
double LineString::distanceToLineString(Point point) const {
	double dMinDistance = point.distanceToSegment(mvPoints[0], mvPoints[1]);
	for (int i = 1; i < mvPoints.size() - 1; i++)
	{
		if (point.distanceToSegment(mvPoints[i], mvPoints[i + 1]) < dMinDistance)
		{
			dMinDistance = point.distanceToSegment(mvPoints[i], mvPoints[i + 1]);
		}
	}
	return dMinDistance;
}