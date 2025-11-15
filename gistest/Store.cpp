#include "Store.h"
#include <vector>
#include <string>
#include <ogrsf_frmts.h>
#include <sstream>

// 构造函数
Store::Store(std::vector<Point*> vStorePoints, std::vector<LineString*> vStoreLines, std::vector<myPolygon*> vStorePolygons)
{
	mvStorePoints = vStorePoints;
	mvStoreLines = vStoreLines;
	mvStorePolygons = vStorePolygons;
	mnID = SnStoreID;
	SnStoreID++;
	// 计算边界
	if (!isEmpty())
	{
		updateMaxX();
		updateMaxY();
		updateMinX();
		updateMinY();
	}
}
// 拷贝构造函数
Store::Store(const Store& other)
{
	mvStorePoints = other.getStorePoints();
	mvStoreLines = other.getStoreLines();
	mvStorePolygons = other.getStorePolygons();
	mnID = SnStoreID;
	SnStoreID++;
	mdMaxX = other.getMaxX();
	mdMaxY = other.getMaxY();
	mdMinX = other.getMinX();
	mdMinY = other.getMinY();
}
// 更新上边界
void Store::updateMaxY()
{
	for (int i = 0; i < mvStorePoints.size(); i++)
	{
		mdMaxY = std::max(mdMaxY, mvStorePoints[i]->getY());
	}
	for (int i = 0; i < mvStoreLines.size(); i++)
	{
		mdMaxY = std::max(mdMaxY, mvStoreLines[i]->getMaxY());
	}
	for (int i = 0; i < mvStorePolygons.size(); i++)
	{
		mdMaxY = std::max(mdMaxY, mvStorePolygons[i]->getMaxY());
	}
}
// 更新下边界
void Store::updateMinY()
{
	for (int i = 0; i < mvStorePoints.size(); i++)
	{
		mdMinY = std::min(mdMinY, mvStorePoints[i]->getY());
	}
	for (int i = 0; i < mvStoreLines.size(); i++)
	{
		mdMinY = std::min(mdMinY, mvStoreLines[i]->getMinY());
	}
	for (int i = 0; i < mvStorePolygons.size(); i++)
	{
		mdMinY = std::min(mdMinY, mvStorePolygons[i]->getMinY());
	}
}
// 更新左边界
void Store::updateMinX()
{
	for (int i = 0; i < mvStorePoints.size(); i++)
	{
		mdMinX = std::min(mdMinX, mvStorePoints[i]->getX());
	}
	for (int i = 0; i < mvStoreLines.size(); i++)
	{
		mdMinX = std::min(mdMinX, mvStoreLines[i]->getMinX());
	}
	for (int i = 0; i < mvStorePolygons.size(); i++)
	{
		mdMinX = std::min(mdMinX, mvStorePolygons[i]->getMinX());
	}
}
// 更新右边界
void Store::updateMaxX()
{
	for (int i = 0; i < mvStorePoints.size(); i++)
	{
		mdMaxX = std::max(mdMaxX, mvStorePoints[i]->getX());
	}
	for (int i = 0; i < mvStoreLines.size(); i++)
	{
		mdMaxX = std::max(mdMaxX, mvStoreLines[i]->getMaxX());
	}
	for (int i = 0; i < mvStorePolygons.size(); i++)
	{
		mdMaxX = std::max(mdMaxX, mvStorePolygons[i]->getMaxX());
	}
}
// 清空
void Store::clear()
{
	mvStorePoints.clear();
	mvStoreLines.clear();
	mvStorePolygons.clear();
}
// 判断 Store 是否为空
bool Store::isEmpty() const
{
	return mvStorePoints.empty() && mvStoreLines.empty() && mvStorePolygons.empty();
}
// 添加点
void Store::addPoint(Point* point, const std::map<QString, QString>& attributes, int nIndex)
{
	if (isEmpty())
	{
		mdMaxX = point->getX();
		mdMaxY = point->getY();
		mdMinX = point->getX();
		mdMinY = point->getY();
	}
	else
	{
		mdMaxX = std::max(mdMaxX, point->getX());
		mdMaxY = std::max(mdMaxY, point->getY());
		mdMinX = std::min(mdMinX, point->getX());
		mdMinY = std::min(mdMinY, point->getY());
	}
	if (nIndex == -1)
		mvStorePoints.push_back(point);
	else
		mvStorePoints.insert(mvStorePoints.begin() + nIndex, point);
	for (auto& attribute : attributes) {
		mmapPointAttributes[point][attribute.first] = attribute.second;
		qDebug() << attribute.first << ":" << mmapPointAttributes[point][attribute.first];
	}
	if(mmapPointAttributes.empty())
		qDebug() << "attributes is empty";
}
// 添加线
void Store::addLine(LineString* line, const std::map<QString, QString>& attributes, int nIndex)
{
	if (isEmpty())
	{
		mdMaxX = line->getMaxX();
		mdMaxY = line->getMaxY();
		mdMinX = line->getMinX();
		mdMinY = line->getMinY();
	}
	else
	{
		mdMaxX = std::max(mdMaxX, line->getMaxX());
		mdMaxY = std::max(mdMaxY, line->getMaxY());
		mdMinX = std::min(mdMinX, line->getMinX());
		mdMinY = std::min(mdMinY, line->getMinY());
	}
	if (nIndex == -1)
		mvStoreLines.push_back(line);
	else
		mvStoreLines.insert(mvStoreLines.begin() + nIndex, line);
	if (!attributes.empty()) {
		mmapLineAttributes[line] = attributes;
	}
}
// 添加面
void Store::addPolygon(myPolygon* polygon, const std::map<QString, QString>& attributes, int nIndex)
{
	if (isEmpty())
	{
		mdMaxX = polygon->getMaxX();
		mdMaxY = polygon->getMaxY();
		mdMinX = polygon->getMinX();
		mdMinY = polygon->getMinY();
	}
	else
	{
		mdMaxX = std::max(mdMaxX, polygon->getMaxX());
		mdMaxY = std::max(mdMaxY, polygon->getMaxY());
		mdMinX = std::min(mdMinX, polygon->getMinX());
		mdMinY = std::min(mdMinY, polygon->getMinY());
	}
	if (nIndex == -1)
		mvStorePolygons.push_back(polygon);
	else
		mvStorePolygons.insert(mvStorePolygons.begin() + nIndex, polygon);
	if (!attributes.empty()) {
		mmapPolygonAttributes[polygon] = attributes;
	}
}

// 删除点
void Store::removePoint(Point* point)
{
	// 如果没有输入参数，则删除最后一个对象
	if (point == nullptr) {
		mvStorePoints.pop_back();
		return;
	}
	auto it = std::find(mvStorePoints.begin(), mvStorePoints.end(), point);
	if (it != mvStorePoints.end())
	{
		mvStorePoints.erase(it);
	}
	if (point->getX() == mdMinX) { updateMinX(); }
	if (point->getX() == mdMaxX) { updateMaxX(); }
	if (point->getY() == mdMinY) { updateMinY(); }
	if (point->getY() == mdMaxY) { updateMaxY(); }
}

void Store::removePoint(int nIndex)
{
	if (nIndex >= 0 && nIndex < mvStorePoints.size())
	{
		mvStorePoints.erase(mvStorePoints.begin() + nIndex);
	}
}
// 删除线
void Store::removeLineString(LineString* line)
{
	// 如果没有任何输入参数，则删除最后一个对象
	if (line == nullptr) {
		mvStoreLines.pop_back();
		return;
	}
	auto it = std::find(mvStoreLines.begin(), mvStoreLines.end(), line);
	if (it != mvStoreLines.end())
	{
		mvStoreLines.erase(it);
	}
	if (line->getMinX() == mdMinX) { updateMinX(); }
	if (line->getMaxX() == mdMaxX) { updateMaxX(); }
	if (line->getMinY() == mdMinY) { updateMinY(); }
	if (line->getMaxY() == mdMaxY) { updateMaxY(); }
}

void Store::removeLineString(int nIndex)
{
	if (nIndex >= 0 && nIndex < mvStoreLines.size())
	{
		mvStoreLines.erase(mvStoreLines.begin() + nIndex);
	}
}
// 删除面
void Store::removePolygon(myPolygon* polygon)
{
	// 如果没有任何输入参数，则删除最后一个对象
	if (polygon == nullptr) {
		mvStorePolygons.pop_back();
		return;
	}
	auto it = std::find(mvStorePolygons.begin(), mvStorePolygons.end(), polygon);
	if (it != mvStorePolygons.end())
	{
		mvStorePolygons.erase(it);
	}
	if (polygon->getMinX() == mdMinX) { updateMinX(); }
	if (polygon->getMaxX() == mdMaxX) { updateMaxX(); }
	if (polygon->getMinY() == mdMinY) { updateMinY(); }
	if (polygon->getMaxY() == mdMaxY) { updateMaxY(); }
}

void Store::removePolygon(int nIndex)
{
	if (nIndex >= 0 && nIndex < mvStorePolygons.size())
	{
		mvStorePolygons.erase(mvStorePolygons.begin() + nIndex);
	}
}
// 获取ID
int Store::getID() const { return mnID; }
// 获取点集
std::vector<Point*> Store::getStorePoints() const { return mvStorePoints; }
// 获取线集
std::vector<LineString*> Store::getStoreLines() const { return mvStoreLines; }
// 获取面集
std::vector<myPolygon*> Store::getStorePolygons() const { return mvStorePolygons; }
// 获取属性
std::map<QString, QString> Store::getAttributes(Point* point) const {
	auto it = mmapPointAttributes.find(point);
	if (it != mmapPointAttributes.end()) {
		qDebug() << "Point already has attributes";
		return it->second;
	}
	return {};
}
std::map<QString, QString> Store::getAttributes(LineString* line) const {
	auto it = mmapLineAttributes.find(line);
	if (it != mmapLineAttributes.end()) {
		return it->second;
	}
	return {};
}
std::map<QString, QString> Store::getAttributes(myPolygon* polygon) const {
	auto it = mmapPolygonAttributes.find(polygon);
	if (it != mmapPolygonAttributes.end()) {
		return it->second;
	}
	return {};
}
// 添加属性
void Store::addPointAttribute(Point* point, QString strkey, QString strvalue) {
	auto it = mmapPointAttributes.find(point);
	if (it != mmapPointAttributes.end()) {
		qDebug() << "Point already has attributes";
		it->second[strkey] = strvalue;
	}
}

// 获取x, y范围
double Store::getMinX() const { return mdMinX; }
double Store::getMinY() const { return mdMinY; }
double Store::getMaxX() const { return mdMaxX; }
double Store::getMaxY() const { return mdMaxY; }
// 重载运算符"=="
bool Store::operator==(const Store& other) const
{
	return (mnID == other.getID());
}

// 获取总面积
double Store::getTotalArea() const {
	double totalArea = 0.0;
	for (const auto& polygon : mvStorePolygons) {
		totalArea += polygon->getArea();
	}
	return totalArea;
}

// 获取总周长
double Store::getTotalPerimeter() const {
	double totalPerimeter = 0.0;
	for (const auto& polygon : mvStorePolygons) {
		totalPerimeter += polygon->getPerimeter();
	}
	return totalPerimeter;
}

// 获取总长度
double Store::getTotalLength() const {
	double totalLength = 0.0;
	for (const auto& line : mvStoreLines) {
		totalLength += line->getLength();
	}
	return totalLength;
}

// 获取外部环数量
int Store::getExteriorPolygonCount() const {
	int count = 0;
	for (const auto& polygon : mvStorePolygons) {
		if (!polygon->getExteriorRing().isEmpty()) {
			++count;
		}
	}
	return count;
}

// 判断 Store 是否包含某点
bool Store::containsPoint(const Point& point) const
{
	for (auto& p : mvStorePoints)
	{
		if (*p == point) { return true; }
	}
	return false;
}
// 判断 Store 是否包含某线
bool Store::containsLineString(const LineString& line) const
{
	for (auto& l : mvStoreLines)
	{
		if (*l == line) { return true; }
	}
	return false;
}
// 判断 Store 是否包含某面
bool Store::containsPolygon(const myPolygon& polygon) const
{
	for (auto& p : mvStorePolygons)
	{
		if (*p == polygon) { return true; }
	}
	return false;
}

// 获取坐标系
void Store::setSpatialReference(const OGRSpatialReference& spatialRef) {
	mSpatialRef = spatialRef;
}

OGRSpatialReference Store::getSpatialReference() const {
	return mSpatialRef;
}
