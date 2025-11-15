#ifndef Polygon_H
#define Polygon_H

#include "Point.h"
#include "LineString.h"
#include <string>
#include <vector>
#include <QGraphicsView>

static int SnPolygonID = 0;  // 静态变量，用于自动生成ID
// 多边形类
class myPolygon
{
private:
	LineString mExteriorRing;                 // 外部环
	std::vector<LineString> mvInteriorRings;   // 内部环集合
	int mnID;                                  // 唯一标识符
	// x, y范围
	double mdMinX = 0, mdMaxX = 0, mdMinY = 0, mdMaxY = 0;
public:
	// 构造函数
	myPolygon(LineString vExteriorRing = {}, std::vector<LineString> vInteriorRings = {});
	// 拷贝构造函数
	myPolygon(const myPolygon& other);
	// 清空
	void clear();
	// 判断是否为空
	bool isEmpty() const;
	// 更新边界
	void updateBoundary();
	// 添加外部环端点
	void addExteriorPoint(Point point);
	// 插入式添加外部环端点，不破坏首尾连接
	void insertExteriorPoint(Point point, int nIndex);
	// 删除外部环端点
	void removeExteriorPoint(Point point);
	void removeExteriorPoint(int nIndex);
	// 添加内部环
	void addInteriorRing(LineString lineString);
	// 删除内部环
	void removeInteriorRing(LineString lineString);
	// 向指定内部环添加端点
	void addInteriorPoint(Point point, int nIndex);
	// 删除指定内部环端点
	void removeInteriorPoint(Point point, int nIndex);
	// 获取ID
	int getID() const;
	// 获取外部环
	LineString getExteriorRing() const;
	// 获取内部环集合
	std::vector<LineString> getInteriorRings() const;
	// 获取x, y范围
	double getMinX() const;
	double getMaxX() const;
	double getMinY() const;
	double getMaxY() const;
	// 重载运算符"=="
	bool operator==(const myPolygon& other) const;
	// 获取面积
	double getArea() const;
	// 获取周长
	double getPerimeter() const;
	double calculateArea() const;
	double calculatePerimeter() const;
};

#endif