#ifndef POINT_H
#define POINT_H

#include <QGraphicsView>

static int SnPointID = 0;  // 静态变量，用于自动生成ID
// 点类
class Point
{
private:
	double mdX, mdY;  // x, y 坐标
	int mnID;         // 唯一标识符
	QHash<QString, QString> mmapFieldValue;	// 属性表

public:
	// 构造函数
	Point(double dx = 0, double dy = 0);
	// 拷贝构造函数
	Point(const Point& other);
	// 获取ID
	int getID() const;
	// 获取x坐标
	double getX() const;
	// 获取y坐标
	double getY() const;
	// 设置x坐标
	double setX(double dx);
	// 设置y坐标
	double setY(double dy);
	// 计算两个点的几何距离
	double distance(const Point& other) const;
	// 计算两个地理坐标的实际距离
	double geoDistance(const Point& other) const;
	// 计算点到线段的距离
	double distanceToSegment(const Point& p1, const Point& p2) const;
	// 重载运算符"=="
	bool operator==(const Point& other) const;

	// 设置属性数据
	void setFieldValue(const QString& key, const QString& value);
	// 获取属性数据
	QString getFieldValue(const QString& key) const;
	// 获取属性表
	QHash<QString, QString> getFieldValueMap() const;
};

#endif