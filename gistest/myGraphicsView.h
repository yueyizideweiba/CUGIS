/************************************************************
FileName: myGraphicsView.h
Author: 曾亦凡 谢宇瀚
Version : 2.3
Date: 2024-08-17
Description: 自定义QGraphicsView类，用于处理地图视图的交互
Function List:
1. myGraphicsView - 构造函数，初始化视图和相关对象
2. ~myGraphicsView - 析构函数
3. wheelEvent - 重写鼠标滚轮事件处理函数，用于缩放视图
4. mouseMoveEvent - 重写鼠标移动事件处理函数，用于更新状态栏信息
5. updateStatusBar - 更新状态栏，显示鼠标位置、缩放级别、旋转角度等信息
6. contextMenuEvent - 重写右键菜单事件处理函数
7. wgs84ToEPSG3857 - 将WGS84坐标转换为EPSG3857坐标
8. epsg3857ToWGS84 - 将EPSG3857坐标转换为WGS84坐标
9. pixelToGeo - 将像素坐标转换为地理坐标
10. setEditingEnabled - 启用或禁用编辑模式
11. setInteractionMode - 启用或禁用交互模式
12. setEditingMode - 设置编辑模式
13. getEditingMode - 获取当前的编辑模式
14. setEditingObjectType - 设置编辑对象类型（线或面）
15. getEditingObjectType - 获取当前的编辑对象类型
16. adjustItemWidths - 调整图形项的宽度
17. mousePressEvent - 重写鼠标按下事件处理函数，用于处理场景点击
18. sceneClicked - 信号，场景被点击时发射
19. statusBarUpdated - 信号，状态栏更新时发射
***********************************************************/

#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <proj.h>
#include "mainwindow.h"

// 前置声明
class MainWindow;

class myGraphicsView : public QGraphicsView
{
	Q_OBJECT
public:
	myGraphicsView(QGraphicsScene* scene, MainWindow* mainWindow, QWidget* parent = nullptr);

	~myGraphicsView();

	void wheelEvent(QWheelEvent* event) override; // 重写鼠标滚轮事件处理函数
	void mouseMoveEvent(QMouseEvent* event) override; // 重写鼠标移动事件处理函数
	void updateStatusBar(); // 更新状态栏信息
	void contextMenuEvent(QContextMenuEvent* event) override; // 重写右键菜单事件处理函数

	static QPointF wgs84ToEPSG3857(double lon, double lat); // 将WGS84坐标转换为EPSG3857坐标
	static Point epsg3857ToWGS84(double x, double y); // 将EPSG3857坐标转换为WGS84坐标
	static Point pixelToGeo(double dx, double dy, double minLon, double maxLon, double minLat, double maxLat, int viewWidth, int viewHeight); // 将像素坐标转换为地理坐标

	void setEditingEnabled(bool enabled) { mbEditingEnabled = enabled; } // 设置编辑模式是否启用

	void setInteractionMode(bool enabled) { mbInteractionEnabled = enabled; } // 设置交互模式是否启用

	void setEditingMode(int mode) { mnEditingMode = mode; } // 设置编辑模式

	int getEditingMode() const { return mnEditingMode; } // 获取当前编辑模式

	void setEditingObjectType(int type) { mnEditingObjectType = type; } // 设置编辑对象类型

	int getEditingObjectType() const { return mnEditingObjectType; } // 获取当前编辑对象类型

	void adjustItemWidths(); // 调整项目宽度

protected:
	void mousePressEvent(QMouseEvent* event) override {
		// 重写鼠标按下事件处理函数
		if (mbInteractionEnabled && event->button() == Qt::LeftButton) {
			QPointF scenePos = mapToScene(event->pos()); // 将鼠标位置转换为场景坐标
			emit sceneClicked(scenePos); // 发射场景点击信号
		}
		QGraphicsView::mousePressEvent(event); // 调用基类的鼠标按下事件处理函数
	}

signals:
	void sceneClicked(const QPointF& pos); // 场景点击信号
	void statusBarUpdated(const QString& mousePos, const QString& zoom, const QString& rotation); // 状态栏更新信号

private:
	bool mbInteractionEnabled; // 交互模式是否启用
	bool mbEditingEnabled; // 编辑模式是否启用
	int mnEditingMode;  // 编辑模式，0 选择，1 创建点，2 创建线，3 创建面，4 选择折点，5 添加折点，6 移动折点
	int mnEditingObjectType; // 编辑对象类型，0 无，1 线，2 面
	MainWindow* mpMainWindow; // 指向主窗口的指针

	bool mbDisplayGeoCoordinates; // 用于跟踪当前显示的坐标格式
};

#endif