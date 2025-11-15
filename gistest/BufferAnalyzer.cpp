#include "BufferAnalyzer.h"
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/GeometryCollection.h>

BufferAnalyzer::BufferAnalyzer() {}

// 计算缓冲区
// 传入的Store包含点、线、面对象，根据指定的半径计算它们的缓冲区，并返回一个新的Store对象
// 如果merge为true，将合并重叠的缓冲区
Store BufferAnalyzer::computeBuffer(const Store& inputStore, double radius, bool merge) {
	Store bufferStore;

	// 为所有点创建缓冲区
	for (auto* point : inputStore.getStorePoints()) {
		bufferStore.addPolygon(createCircularBuffer(point, radius));
	}

	// 为所有线创建缓冲区
	for (auto* line : inputStore.getStoreLines()) {
		bufferStore.addPolygon(createLineBuffer(line, radius));
	}

	// 为所有面创建缓冲区
	for (auto* polygon : inputStore.getStorePolygons()) {
		auto polygons = createPolygonBuffer(polygon, radius);
		for (auto* poly : polygons) {
			bufferStore.addPolygon(poly);
		}
	}

	// 如果需要合并重叠的缓冲区，则执行合并操作
	if (merge) {
		bufferStore = mergeOverlappingBuffers(bufferStore);
	}

	return bufferStore;
}

// 为点创建圆形缓冲区
// 使用GEOS库生成缓冲区并转换回自定义的myPolygon格式
myPolygon* BufferAnalyzer::createCircularBuffer(Point* point, double radius) {
	// 将点的WGS84坐标转换为EPSG3857坐标
	QPointF epsgPoint = myGraphicsView::wgs84ToEPSG3857(point->getX(), point->getY());
	const geos::geom::GeometryFactory* factory = geos::geom::GeometryFactory::getDefaultInstance();
	geos::geom::Coordinate coord(epsgPoint.x(), epsgPoint.y());
	std::unique_ptr<geos::geom::Point> geosPoint(factory->createPoint(coord));
	// 使用指定的半径生成缓冲区
	std::unique_ptr<geos::geom::Geometry> buffer(geosPoint->buffer(radius));
	buffer = buffer->buffer(0);

	// 获取缓冲区的坐标序列
	std::unique_ptr<geos::geom::CoordinateSequence> coordSeq(buffer->getCoordinates());
	std::vector<Point> points;
	for (std::size_t i = 0; i < coordSeq->size(); ++i) {
		const geos::geom::Coordinate& c = coordSeq->getAt(i);
		// 将EPSG3857坐标转换回WGS84坐标
		Point geoPoint = myGraphicsView::epsg3857ToWGS84(c.x, c.y);
		points.emplace_back(geoPoint.getX(), geoPoint.getY());
	}

	// 创建并返回新的myPolygon对象
	return new myPolygon(LineString(points));
}

// 为线创建缓冲区
myPolygon* BufferAnalyzer::createLineBuffer(LineString* line, double radius) {
	const geos::geom::GeometryFactory* factory = geos::geom::GeometryFactory::getDefaultInstance();
	std::vector<geos::geom::Coordinate> coords;

	// 将线的所有点转换为EPSG3857坐标
	for (const auto& point : line->getPoints()) {
		QPointF epsgPoint = myGraphicsView::wgs84ToEPSG3857(point.getX(), point.getY());
		coords.emplace_back(epsgPoint.x(), epsgPoint.y());
	}

	// 创建GEOS线字符串
	std::unique_ptr<geos::geom::CoordinateArraySequence> coordSeq(new geos::geom::CoordinateArraySequence(coords.size(), 2));
	for (std::size_t i = 0; i < coords.size(); ++i) {
		coordSeq->setAt(coords[i], i);
	}
	std::unique_ptr<geos::geom::LineString> geosLineString(factory->createLineString(std::move(coordSeq)));

	// 生成缓冲区
	std::unique_ptr<geos::geom::Geometry> buffer(geosLineString->buffer(radius));

	// 获取缓冲区的坐标序列并转换回WGS84
	std::unique_ptr<geos::geom::CoordinateSequence> bufferCoordSeq(buffer->getCoordinates());
	std::vector<Point> points;
	for (std::size_t i = 0; i < bufferCoordSeq->size(); ++i) {
		const geos::geom::Coordinate& c = bufferCoordSeq->getAt(i);
		Point geoPoint = myGraphicsView::epsg3857ToWGS84(c.x, c.y);
		points.emplace_back(geoPoint.getX(), geoPoint.getY());
	}

	// 创建并返回新的myPolygon对象
	return new myPolygon(LineString(points));
}

// 为面创建缓冲区
std::vector<myPolygon*> BufferAnalyzer::createPolygonBuffer(myPolygon* polygon, double radius) {
	const geos::geom::GeometryFactory* factory = geos::geom::GeometryFactory::getDefaultInstance();
	std::vector<geos::geom::Coordinate> coords;

	// 将面外环的所有点转换为EPSG3857坐标
	for (const auto& point : polygon->getExteriorRing().getPoints()) {
		QPointF epsgPoint = myGraphicsView::wgs84ToEPSG3857(point.getX(), point.getY());
		coords.emplace_back(epsgPoint.x(), epsgPoint.y());
	}

	// 创建GEOS线性环并生成面缓冲区
	std::unique_ptr<geos::geom::CoordinateArraySequence> coordSeq(new geos::geom::CoordinateArraySequence(coords.size(), 2));
	for (std::size_t i = 0; i < coords.size(); ++i) {
		coordSeq->setAt(coords[i], i);
	}
	std::unique_ptr<geos::geom::LinearRing> ring(factory->createLinearRing(std::move(coordSeq)));
	std::unique_ptr<geos::geom::Polygon> geosPolygon(factory->createPolygon(std::move(ring), {}));
	std::unique_ptr<geos::geom::Geometry> buffer(geosPolygon->buffer(radius));

	std::vector<myPolygon*> resultPolygons;

	// 检查生成的几何类型并处理
	if (buffer->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
		// 单一面缓冲区
		resultPolygons.push_back(createMyPolygonFromGeometry(buffer.get()));
	}
	else if (buffer->getGeometryTypeId() == geos::geom::GEOS_GEOMETRYCOLLECTION || buffer->getGeometryTypeId() == geos::geom::GEOS_MULTIPOLYGON) {
		// 多个面或几何集合缓冲区
		const geos::geom::GeometryCollection* collection = dynamic_cast<const geos::geom::GeometryCollection*>(buffer.get());
		for (std::size_t i = 0; i < collection->getNumGeometries(); ++i) {
			const geos::geom::Geometry* geom = collection->getGeometryN(i);
			if (geom->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
				resultPolygons.push_back(createMyPolygonFromGeometry(geom));
			}
		}
	}

	return resultPolygons;
}

// 从GEOS几何对象创建自定义的myPolygon对象
myPolygon* BufferAnalyzer::createMyPolygonFromGeometry(const geos::geom::Geometry* geometry) {
	const geos::geom::Polygon* polygon = dynamic_cast<const geos::geom::Polygon*>(geometry);
	if (!polygon) return nullptr;

	// 获取外环坐标并转换回WGS84坐标
	std::vector<Point> exteriorPoints;
	std::unique_ptr<geos::geom::CoordinateSequence> exteriorCoords(polygon->getExteriorRing()->getCoordinates());
	for (std::size_t i = 0; i < exteriorCoords->size(); ++i) {
		const geos::geom::Coordinate& c = exteriorCoords->getAt(i);
		Point geoPoint = myGraphicsView::epsg3857ToWGS84(c.x, c.y);
		exteriorPoints.emplace_back(geoPoint.getX(), geoPoint.getY());
	}
	LineString exteriorRing(exteriorPoints);

	// 获取所有内环并转换回WGS84坐标
	std::vector<LineString> interiorRings;
	for (std::size_t i = 0; i < polygon->getNumInteriorRing(); ++i) {
		std::vector<Point> interiorPoints;
		std::unique_ptr<geos::geom::CoordinateSequence> interiorCoords(polygon->getInteriorRingN(i)->getCoordinates());
		for (std::size_t j = 0; j < interiorCoords->size(); ++j) {
			const geos::geom::Coordinate& c = interiorCoords->getAt(j);
			Point geoPoint = myGraphicsView::epsg3857ToWGS84(c.x, c.y);
			interiorPoints.emplace_back(geoPoint.getX(), geoPoint.getY());
		}
		interiorRings.emplace_back(interiorPoints);
	}

	// 创建并返回新的myPolygon对象，包含外环和内环
	return new myPolygon(exteriorRing, interiorRings);
}

// 合并重叠的缓冲区
// 将多个可能重叠的缓冲区合并为一个或多个非重叠的几何对象
Store BufferAnalyzer::mergeOverlappingBuffers(const Store& bufferStore) {
	const geos::geom::GeometryFactory* factory = geos::geom::GeometryFactory::getDefaultInstance();
	std::vector<std::unique_ptr<geos::geom::Geometry>> geometries;

	// 遍历存储中的面，将其转换为GEOS几何对象
	for (const auto& polygon : bufferStore.getStorePolygons()) {
		std::vector<geos::geom::Coordinate> coords;
		for (const auto& point : polygon->getExteriorRing().getPoints()) {
			QPointF epsgPoint = myGraphicsView::wgs84ToEPSG3857(point.getX(), point.getY());
			coords.emplace_back(epsgPoint.x(), epsgPoint.y());
		}

		// 创建GEOS的线性环和面对象
		auto coordSeq = std::make_unique<geos::geom::CoordinateArraySequence>(coords.size(), 2);
		for (std::size_t i = 0; i < coords.size(); ++i) {
			coordSeq->setAt(coords[i], i);
		}
		auto ring = factory->createLinearRing(std::move(coordSeq));
		auto geosPolygon = factory->createPolygon(std::move(ring), {});
		geometries.push_back(std::move(geosPolygon));
	}

	// 将所有几何对象集合创建为一个几何集合
	std::vector<std::unique_ptr<geos::geom::Geometry>> geometryPtrs;
	for (auto& geom : geometries) {
		geometryPtrs.push_back(std::move(geom));
	}
	auto geometryCollection = factory->createGeometryCollection(std::move(geometryPtrs));
	// 使用GEOS的Union操作合并所有几何对象
	auto mergedGeometry = std::unique_ptr<geos::geom::Geometry>(geometryCollection->Union());

	Store mergedStore;
	// 检查合并后的几何类型
	if (mergedGeometry->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
		// 如果是单个面，则直接添加到Store中
		mergedStore.addPolygon(createMyPolygonFromGeometry(mergedGeometry.get()));
	}
	else if (mergedGeometry->getGeometryTypeId() == geos::geom::GEOS_GEOMETRYCOLLECTION || mergedGeometry->getGeometryTypeId() == geos::geom::GEOS_MULTIPOLYGON) {
		// 如果是几何集合或多面，则遍历每个面并添加到Store中
		const geos::geom::GeometryCollection* collection = dynamic_cast<const geos::geom::GeometryCollection*>(mergedGeometry.get());
		for (std::size_t i = 0; i < collection->getNumGeometries(); ++i) {
			const geos::geom::Geometry* geom = collection->getGeometryN(i);
			if (geom->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
				mergedStore.addPolygon(createMyPolygonFromGeometry(geom));
			}
		}
	}

	// 返回包含合并结果的Store对象
	return mergedStore;
}