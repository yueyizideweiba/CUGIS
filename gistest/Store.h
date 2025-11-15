#ifndef STORE_H
#define STORE_H

#include "Point.h"
#include "LineString.h"
#include "Polygon.h"
#include <vector>
#include <string>
#include <algorithm>
#include <ogr_spatialref.h>
#include <ogrsf_frmts.h>

static int SnStoreID = 0;  // 静态变量，用于自动生成ID
class Store
{
private:
	std::vector<Point*> mvStorePoints;      // 点集
	std::vector<LineString*> mvStoreLines;  // 线集
	std::vector<myPolygon*> mvStorePolygons;// 面集
	int mnID;                               // 唯一标识符
	double mdMinX = 0, mdMaxX = 0, mdMinY = 0, mdMaxY = 0;
	OGRSpatialReference mSpatialRef;        // 添加参考坐标系

	// 属性表数据
	std::map<Point*, std::map<QString, QString>> mmapPointAttributes;
	std::map<LineString*, std::map<QString, QString>> mmapLineAttributes;
	std::map<myPolygon*, std::map<QString, QString>> mmapPolygonAttributes;


public:
	// 构造函数
	Store(std::vector<Point*> vStorePoints = {}, std::vector<LineString*> vStoreLines = {}, std::vector<myPolygon*> vStorePolygons = {});

	// 拷贝构造函数
	Store(const Store& other);

	// 更新边界
	void updateMaxY();
	void updateMinY();
	void updateMinX();
	void updateMaxX();

	// 清空
	void clear();

	// 判断 Store 是否为空
	bool isEmpty() const;

	// 添加几何对象
	void addPoint(Point* point, const std::map<QString, QString>& attributes = {}, int nIndex = -1);
	void addLine(LineString* line, const std::map<QString, QString>& attributes = {}, int nIndex = -1);
	void addPolygon(myPolygon* polygon, const std::map<QString, QString>& attributes = {}, int nIndex = -1);

	// 删除几何对象
	void removePoint(Point* point = nullptr);
	void removePoint(int nIndex);
	void removeLineString(LineString* line = nullptr);
	void removeLineString(int nIndex);
	void removePolygon(myPolygon* polygon = nullptr);
	void removePolygon(int nIndex);

	// 获取ID
	int getID() const;

	// 获取几何对象
	std::vector<Point*> getStorePoints() const;
	std::vector<LineString*> getStoreLines() const;
	std::vector<myPolygon*> getStorePolygons() const;

	// 获取外部环数量
	int getExteriorPolygonCount() const;

	// 获取边界
	double getMinX() const;
	double getMaxX() const;
	double getMinY() const;
	double getMaxY() const;

	// 设置和获取参考坐标系
	void setSpatialReference(const OGRSpatialReference& spatialRef);
	OGRSpatialReference getSpatialReference() const;

	// 重载运算符
	bool operator==(const Store& other) const;

	// 获取面积、周长、长度
	double getTotalArea() const;
	double getTotalPerimeter() const;
	double getTotalLength() const;

	// 判断是否包含几何对象
	bool containsPoint(const Point& point) const;
	bool containsLineString(const LineString& line) const;
	bool containsPolygon(const myPolygon& polygon) const;

	// 存储属性表
	std::map<QString, QString> getAttributes(Point* point) const;
	std::map<QString, QString> getAttributes(LineString* line) const;
	std::map<QString, QString> getAttributes(myPolygon* polygon) const;
	// 添加属性
	void addPointAttribute(Point* point, QString strkey, QString strvalue);
};

#endif
