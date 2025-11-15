#include "drawVec.h"

// 地理坐标转屏幕坐标
QPointF drawVec::geoToPixel(double lon, double lat, double minLon, double maxLon, double minLat, double maxLat, int viewWidth, int viewHeight) {
	double lonRange = maxLon - minLon;
	double latRange = maxLat - minLat;

	// 使用相同的比例因子
	double scale = std::min(viewWidth / lonRange, viewHeight / latRange);

	double x = (lon - minLon) * scale;
	double y = (maxLat - lat) * scale;

	return QPointF(x, y);
}

// 屏幕坐标转换为地理坐标
Point drawVec::pixelToGeo(double dx, double dy, double minLon, double maxLon, double minLat, double maxLat, int viewWidth, int viewHeight) {
	double lonRange = maxLon - minLon;
	double latRange = maxLat - minLat;

	// 使用相同的比例因子
	double scale = std::min(viewWidth / lonRange, viewHeight / latRange);

	double lon = minLon + (dx / scale);
	double lat = maxLat - (dy / scale);

	return Point(lon, lat);
}

// 进行绘制可视化store中所有对象
QGraphicsItemGroup* drawVec::showVec(Store& store, QGraphicsScene& scene, double minLon, double maxLon, double minLat, double maxLat, int viewWidth, int viewHeight, QPen pen, QBrush brush) {
	QGraphicsItemGroup* group = new QGraphicsItemGroup();
	mCurrentLinePen = pen;

	// 绘制点
	for (const Point* point : store.getStorePoints()) {
		QPointF pixelCoord = geoToPixel(point->getX(), point->getY(), minLon, maxLon, minLat, maxLat, viewWidth, viewHeight);
		if (mUseCustomPointImage) {
			QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(mCustomPointImage);
			pixmapItem->setPos(pixelCoord.x(), pixelCoord.y());
			pixmapItem->setTransformationMode(Qt::SmoothTransformation);
			group->addToGroup(pixmapItem);
		}
		else {
			QGraphicsEllipseItem* ellipse = scene.addEllipse(pixelCoord.x() - 2.5, pixelCoord.y() - 2.5, 5, 5, pen, brush);
			group->addToGroup(ellipse);
		}
	}

	// 绘制线
	for (const LineString* lineString : store.getStoreLines()) {
		QPointF lastPoint;
		bool firstPoint = true;
		QGraphicsLineItem* line;
		for (const Point& point : lineString->getPoints()) {
			QPointF currentPoint = geoToPixel(point.getX(), point.getY(), minLon, maxLon, minLat, maxLat, viewWidth, viewHeight);
			if (!firstPoint) {
				if (mUseCustomLineImage) {
					QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(mCustomLineImage);
					// 计算线段的长度和角度，设置 pixmapItem 的大小和旋转
					pixmapItem->setPos((lastPoint + currentPoint) / 2);
					pixmapItem->setTransformationMode(Qt::SmoothTransformation);
					group->addToGroup(pixmapItem);
				}
				else {
					line = scene.addLine(lastPoint.x(), lastPoint.y(), currentPoint.x(), currentPoint.y(), mCurrentLinePen);
					group->addToGroup(line);
				}
			}
			lastPoint = currentPoint;
			firstPoint = false;
		}
	}

	// 绘制面
	for (const myPolygon* polygon : store.getStorePolygons()) {
		QVector<QPointF> exteriorPoints;
		for (const Point& point : polygon->getExteriorRing().getPoints()) {
			QPointF pixelCoord = geoToPixel(point.getX(), point.getY(), minLon, maxLon, minLat, maxLat, viewWidth, viewHeight);
			exteriorPoints.push_back(pixelCoord);
		}
		QGraphicsPolygonItem* exteriorPolygon = scene.addPolygon(QPolygonF(exteriorPoints), pen, brush);
		group->addToGroup(exteriorPolygon);

		for (const LineString& interiorRing : polygon->getInteriorRings()) {
			QVector<QPointF> interiorPoints;
			for (const Point& point : interiorRing.getPoints()) {
				QPointF pixelCoord = geoToPixel(point.getX(), point.getY(), minLon, maxLon, minLat, maxLat, viewWidth, viewHeight);
				interiorPoints.push_back(pixelCoord);
			}
			QGraphicsPolygonItem* interiorPolygon = scene.addPolygon(QPolygonF(interiorPoints), pen, brush);
			group->addToGroup(interiorPolygon);
		}
	}

	scene.addItem(group);

	return 0;
}