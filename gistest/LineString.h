#ifndef LINESTRING_H
#define LINESTRING_H

#include "Point.h"
#include <string>
#include <vector>
#include <algorithm>

static int SnLineStringID = 0;  // 静态变量，用于自动生成ID
// 线类
class LineString
{
private:
	std::vector<Point> mvPoints; // 点集合
	int mnID;                    // 唯一标识符
	// x, y范围
	double mdMinX, mdMaxX, mdMinY, mdMaxY;
public:
	// 构造函数
	LineString(std::vector<Point> vPoints = {});
	// 拷贝构造函数
	LineString(const LineString& other);
	// 清空
	void clear();
	// 判断是否为空
	bool isEmpty() const;
	// 更新上边界
	void updateMaxY();
	// 更新下边界
	void updateMinY();
	// 更新左边界
	void updateMinX();
	// 更新右边界
	void updateMaxX();
	// 添加端点
	void addPoint(Point point);
	// 插入端点
	void insertPoint(Point point, int nIndex);
	// 删除端点
	void removePoint(Point point);
	void removePoint(int nIndex);
	// 获取ID
	int getID() const;
	// 获取端点集
	std::vector<Point> getPoints() const;
	// 获取x, y范围
	double getMinX() const;
	double getMaxX() const;
	double getMinY() const;
	double getMaxY() const;
	// 重载运算符"=="
	bool operator==(const LineString& other) const;
	// 获取长度
	double getLength() const;
	double calculateLength() const;
	// 计算点到折线的最短距离
	double distanceToLineString(Point point) const;
};

#endif