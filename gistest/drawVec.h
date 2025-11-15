/************************************************************
FileName: drawVec.h
Author: 谢宇瀚
Version : 1.5
Date: 2024-07-20
Description: 矢量绘制类，用于可视化矢量数据
Function List:
1. showVec - 在场景中显示矢量数据
2. setCustomPointImage - 设置自定义点图像
3. setDefaultPointStyle - 设置默认点样式
4. setCustomLineImage - 设置自定义线图像
5. setDefaultLineStyle - 设置默认线样式
6. setCurrentLineStyle - 设置当前线样式
7. setPen - 设置绘图笔
8. setBrush - 设置画刷
9. pixelToGeo - 将像素坐标转换为地理坐标
10. geoToPixel - 将地理坐标转换为像素坐标
***********************************************************/

#ifndef DRAWVEC_H
#define DRAWVEC_H

#include <QApplication>
#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QPen>
#include <QBrush>
#include <QGraphicsItemGroup>
#include <proj.h>
#include <vector>
#include "Store.h"
#include "vecReader.h"

class drawVec {
public:
	QGraphicsItemGroup* showVec(Store& store, QGraphicsScene& scene, double minLon, double maxLon, double minLat, double maxLat, int viewWidth, int viewHeight, QPen pen, QBrush brush);

	void setCustomPointImage(const QPixmap& image) { mCustomPointImage = image; mUseCustomPointImage = true; } // 设置自定义点图像
	void setDefaultPointStyle(const QPen& pen) { mPen = pen; mUseCustomPointImage = false; } // 设置默认点样式
	void setCustomLineImage(const QPixmap& image) { mCustomLineImage = image; mUseCustomLineImage = true; } // 设置自定义线图像
	void setDefaultLineStyle(const QPen& pen) { mPen = pen; mUseCustomLineImage = false; } // 设置默认线样式
	void setCurrentLineStyle(QPen pen) { mCurrentLinePen = pen; }  // 设置当前的线样式

	void setPen(const QPen& pen) { this->mPen = pen; } // 设置绘图笔
	void setBrush(const QBrush& brush) { this->mBrush = brush; } // 设置画刷

	static Point pixelToGeo(double dx, double dy, double minLon, double maxLon, double minLat, double maxLat, int viewWidth, int viewHeight); // 将像素坐标转换为地理坐标
	static QPointF geoToPixel(double lon, double lat, double minLon, double maxLon, double minLat, double maxLat, int viewWidth, int viewHeight); // 将地理坐标转换为像素坐标

private:
	QPen mPen;
	QBrush mBrush;

	QPixmap mCustomPointImage; // 自定义点图像
	QPixmap mCustomLineImage; // 自定义线图像

	bool mUseCustomPointImage = false; // 是否使用自定义点图像
	bool mUseCustomLineImage = false; // 是否使用自定义线图像
	QPen mCurrentLinePen; // 当前的线样式
};

#endif