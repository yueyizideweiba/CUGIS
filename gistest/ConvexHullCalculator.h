/************************************************************
FileName: ConvexHullCalculator.h
Author: 谢宇瀚
Version : 2.3
Date: 2024-08-08
Description: 凸包计算器类，用于计算要素凸包
Function List:
1. ConvexHullCalculator - 构造函数
2. compute - 计算输入要素集合的凸包
3. polarCompare - 极角比较函数，用于点的排序
4. crossProduct - 计算三点间的向量叉积，用于判断点的转向关系
***********************************************************/

#ifndef CONVEXHULLCALCULATOR_H
#define CONVEXHULLCALCULATOR_H

#include "Point.h"
#include "Store.h"
#include <vector>
#include <QString>

class ConvexHullCalculator {
public:
	ConvexHullCalculator();
	Store compute(const Store& inputStore); // 计算输入要素的凸包

private:
	static bool polarCompare(const Point* p1, const Point* p2, const Point* origin); // 极角比较，用于排序点
	static double crossProduct(const Point* p1, const Point* p2, const Point* p3); // 计算向量叉积，用于判断转向
};

#endif