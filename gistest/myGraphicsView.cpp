#include "myGraphicsView.h"
#include "Point.h"
#include <QWheelEvent>
#include <QScrollBar>
#include <QDebug>
#include <QMenu>

// 构造函数，初始化自定义的QGraphicsView
myGraphicsView::myGraphicsView(QGraphicsScene* scene, MainWindow* mainWindow, QWidget* parent) :
	QGraphicsView(scene, parent),
	mnEditingMode(0),  // 初始编辑模式为0
	mbInteractionEnabled(false),  // 初始时交互模式禁用
	mpMainWindow(mainWindow),  // 保存主窗口的指针
	mbDisplayGeoCoordinates(true)  // 初始化为显示地理坐标
{
	// 设置视图的样式表，移除内边距和边框
	setStyleSheet("padding: 0px; border: 0px;");
	// 禁用滚动条
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

// 析构函数
myGraphicsView::~myGraphicsView() {

}

// 鼠标滚轮事件处理，用于缩放和旋转视图
void myGraphicsView::wheelEvent(QWheelEvent* event) {
	qreal scaleFactor = 1.15;  // 缩放因子
	if (event->modifiers() & Qt::ControlModifier) {  // 如果按下了Ctrl键
		qreal angleDelta = event->angleDelta().y();
		qreal rotationAngle = angleDelta * 0.1;  // 计算旋转角度
		rotate(rotationAngle);  // 旋转视图
	}
	else {  // 否则进行缩放
		if (event->angleDelta().y() > 0) {
			scale(scaleFactor, scaleFactor);  // 放大
		}
		else {
			scale(1 / scaleFactor, 1 / scaleFactor);  // 缩小
		}
		event->accept();
	}
	adjustItemWidths(); // 调整图形项的宽度
	updateStatusBar();  // 更新状态栏
}

// 调整场景中图形项的宽度，以适应当前缩放比例
void myGraphicsView::adjustItemWidths() {
	qreal currentZoom = transform().m11();  // 获取当前缩放比例
	qreal adjustedWidth = 1.0 / currentZoom * 2;  // 计算调整后的宽度

	// 遍历场景中的所有图形项
	for (QGraphicsItem* item : scene()->items()) {
		QPen pen;
		// 根据图形项的类型调整其宽度
		if (auto* lineItem = dynamic_cast<QGraphicsLineItem*>(item)) {
			pen = lineItem->pen();
			pen.setWidthF(adjustedWidth);
			lineItem->setPen(pen);
		}
		else if (auto* pathItem = dynamic_cast<QGraphicsPathItem*>(item)) {
			pen = pathItem->pen();
			pen.setWidthF(adjustedWidth);
			pathItem->setPen(pen);
		}
		else if (auto* polygonItem = dynamic_cast<QGraphicsPolygonItem*>(item)) {
			pen = polygonItem->pen();
			pen.setWidthF(adjustedWidth);
			polygonItem->setPen(pen);
		}
		else if (auto* ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(item)) {
			pen = ellipseItem->pen();
			pen.setWidthF(adjustedWidth);
			ellipseItem->setPen(pen);
			// 调整点要素的大小
			qreal adjustedSize = 10.0 / currentZoom;
			ellipseItem->setRect(ellipseItem->rect().x() + ellipseItem->rect().width() / 2 - adjustedSize / 2,
				ellipseItem->rect().y() + ellipseItem->rect().height() / 2 - adjustedSize / 2,
				adjustedSize, adjustedSize);
		}
	}
}

// 鼠标移动事件处理，更新状态栏信息，并在编辑模式下处理顶点移动
void myGraphicsView::mouseMoveEvent(QMouseEvent* event) {
	QGraphicsView::mouseMoveEvent(event);
	updateStatusBar();  // 更新状态栏
	if (mnEditingMode == 6) {  // 编辑模式为6时，允许移动顶点
		int nIndex = mpMainWindow->getEditPointIndex();
		if (nIndex > -1) {  // 如果有选中的点
			// 获取当前鼠标位置并转换为场景坐标
			QPointF mousePos = mapToScene(mapFromGlobal(QCursor::pos()));
			// 将像素坐标转换为地理坐标
			Point mousePoint = mpMainWindow->getDraw().pixelToGeo(mousePos.x(), mousePos.y(),
				mpMainWindow->getMinLon(),
				mpMainWindow->getMaxLon(),
				mpMainWindow->getMinLat(),
				mpMainWindow->getMaxLat(),
				mpMainWindow->getViewW(),
				mpMainWindow->getViewH());
			// 更新选中点的坐标
			mpMainWindow->getEditedStore()->getStorePoints()[nIndex]->setX(mousePoint.getX());
			mpMainWindow->getEditedStore()->getStorePoints()[nIndex]->setY(mousePoint.getY());
			if (mnEditingObjectType == 1) {  // 编辑对象类型为1（线）
				mpMainWindow->getEditLineString()->insertPoint(mousePoint, nIndex);
				mpMainWindow->getEditLineString()->removePoint(nIndex + 1);
			}
			else if (mnEditingObjectType == 2) {  // 编辑对象类型为2（面）
				mpMainWindow->getEditPolygon()->insertExteriorPoint(mousePoint, nIndex);
				mpMainWindow->getEditPolygon()->removeExteriorPoint(nIndex + 1);
			}
			mpMainWindow->drawEditLayer();  // 重绘编辑图层
		}
	}
}

// 右键菜单事件处理，提供切换坐标系的选项
void myGraphicsView::contextMenuEvent(QContextMenuEvent* event) {
	QMenu contextMenu(this);  // 创建右键菜单
	QAction action1("切换为EPSG:4326坐标", this);
	QAction action2("切换为EPSG:3857坐标", this);

	// 触发显示地理坐标
	connect(&action1, &QAction::triggered, this, [this]() {
		mbDisplayGeoCoordinates = true;
		updateStatusBar();
		});

	// 触发显示投影坐标
	connect(&action2, &QAction::triggered, this, [this]() {
		mbDisplayGeoCoordinates = false;
		updateStatusBar();
		});

	// 将选项添加到右键菜单中并显示
	contextMenu.addAction(&action1);
	contextMenu.addAction(&action2);
	contextMenu.exec(event->globalPos());
}

// 将WGS84坐标转换为EPSG:3857坐标（Web Mercator投影）
QPointF myGraphicsView::wgs84ToEPSG3857(double lon, double lat) {
	const double R_MAJOR = 6378137.0;
	const double R_MINOR = 6356752.3142;
	double x = R_MAJOR * qDegreesToRadians(lon);  // 经度转换为x坐标
	double y = R_MAJOR * log(tan(M_PI_4 + qDegreesToRadians(lat) / 2.0));  // 纬度转换为y坐标
	return QPointF(x, y);
}

// 将EPSG:3857坐标转换为WGS84坐标
Point myGraphicsView::epsg3857ToWGS84(double x, double y) {
	const double R_MAJOR = 6378137.0;
	const double R_MINOR = 6356752.3142;
	double lon = qRadiansToDegrees(x / R_MAJOR);  // x坐标转换为经度
	double lat = qRadiansToDegrees(atan(sinh(y / R_MAJOR)));  // y坐标转换为纬度
	return Point(lon, lat);
}

// 将屏幕坐标转换为WGS84坐标
Point myGraphicsView::pixelToGeo(double dx, double dy, double minLon, double maxLon, double minLat, double maxLat, int viewWidth, int viewHeight) {
	QPointF epsg3857Min = wgs84ToEPSG3857(minLon, minLat);
	QPointF epsg3857Max = wgs84ToEPSG3857(maxLon, maxLat);

	double lonRange = epsg3857Max.x() - epsg3857Min.x();  // 经度范围
	double latRange = epsg3857Max.y() - epsg3857Min.y();  // 纬度范围
	double x = epsg3857Min.x() + (dx / viewWidth) * lonRange;  // 计算对应的x坐标
	double y = epsg3857Max.y() - (dy / viewHeight) * latRange;  // 计算对应的y坐标

	// 将EPSG:3857坐标转换为WGS84坐标并返回
	return epsg3857ToWGS84(x, y);
}

// 更新状态栏信息，显示当前鼠标位置的坐标、缩放比例和旋转角度
void myGraphicsView::updateStatusBar() {
	// 获取当前鼠标位置并转换为场景坐标
	QPointF mousePos = mapToScene(mapFromGlobal(QCursor::pos()));
	double minLon = mpMainWindow->getMinLon();
	double maxLon = mpMainWindow->getMaxLon();
	double minLat = mpMainWindow->getMinLat();
	double maxLat = mpMainWindow->getMaxLat();
	int viewWidth = mpMainWindow->getViewW();
	int viewHeight = mpMainWindow->getViewH();

	QString mousePosText;
	// 将像素坐标转换为WGS84地理坐标
	Point geoCoord = pixelToGeo(mousePos.x(), mousePos.y(), minLon, maxLon, minLat, maxLat, viewWidth, viewHeight);

	// 根据当前显示的坐标格式显示地理坐标或投影坐标
	if (mbDisplayGeoCoordinates) {
		// 显示WGS84地理坐标，并附加东西南北方向
		QString lonDirection = geoCoord.getX() >= 0 ? "E" : "W";
		QString latDirection = geoCoord.getY() >= 0 ? "N" : "S";
		mousePosText = QString("坐标: (%1° %2, %3° %4)")
			.arg(std::abs(geoCoord.getX()), 0, 'f', 6).arg(lonDirection)
			.arg(std::abs(geoCoord.getY()), 0, 'f', 6).arg(latDirection);
	}
	else {
		// 显示EPSG:3857投影坐标
		QPointF epsgCoord = wgs84ToEPSG3857(geoCoord.getX(), geoCoord.getY());
		mousePosText = QString("坐标: (%1m, %2m)")
			.arg(epsgCoord.x(), 0, 'f', 2).arg(epsgCoord.y(), 0, 'f', 2);
	}

	// 获取当前的缩放比例
	qreal currentZoom = transform().m11();
	QString zoomText = QString("缩放比例: %1").arg(currentZoom);

	// 计算当前的旋转角度
	qreal rotationAngle = std::atan2(transform().m21(), transform().m11()) * 180 / M_PI;
	QString rotationText = QString("旋转角度: %1").arg(rotationAngle);

	// 发送状态栏更新信号，更新主窗口的状态栏显示
	emit statusBarUpdated(mousePosText, zoomText, rotationText);
}