#include "OverlayAnalysis.h"
#include "Store.h"
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>

// 将 Store 转换为 GEOS Geometry
std::unique_ptr<geos::geom::Geometry> OverlayAnalysis::createGeometryFromStore(const Store& store) {
	geos::geom::GeometryFactory::Ptr factory = geos::geom::GeometryFactory::create();
	std::vector<std::unique_ptr<geos::geom::Geometry>> geometries;

	// 转换点集合
	for (auto point : store.getStorePoints()) {
		geos::geom::Coordinate coord(point->getX(), point->getY());
		auto geomPoint = factory->createPoint(coord);
		geometries.emplace_back(std::move(geomPoint));
	}

	// 转换线集合
	for (auto line : store.getStoreLines()) {
		std::vector<geos::geom::Coordinate> coords;
		for (const auto& p : line->getPoints()) {
			coords.emplace_back(p.getX(), p.getY());
		}
		// 创建 CoordinateArraySequence 对象
		std::unique_ptr<geos::geom::CoordinateArraySequence> coordSeq(new geos::geom::CoordinateArraySequence(coords.size(), 2));
		for (std::size_t i = 0; i < coords.size(); ++i) {
			coordSeq->setAt(coords[i], i);
		}

		auto geomLine = factory->createLineString(std::move(coordSeq));
		geometries.emplace_back(std::move(geomLine));
	}

	// 转换多边形集合
	for (auto polygon : store.getStorePolygons()) {
		std::vector<geos::geom::Coordinate> coords;
		for (const auto& p : polygon->getExteriorRing().getPoints()) {
			coords.emplace_back(p.getX(), p.getY());
		}
		// 创建 CoordinateArraySequence 对象
		std::unique_ptr<geos::geom::CoordinateArraySequence> coordSeq(new geos::geom::CoordinateArraySequence(coords.size(), 2));
		for (std::size_t i = 0; i < coords.size(); ++i) {
			coordSeq->setAt(coords[i], i);
		}
		auto shell = factory->createLinearRing(std::move(coordSeq));

		std::vector<std::unique_ptr<geos::geom::LinearRing>> holes;
		for (const auto& ring : polygon->getInteriorRings()) {
			std::vector<geos::geom::Coordinate> holeCoords;
			for (const auto& p : ring.getPoints()) {
				holeCoords.emplace_back(p.getX(), p.getY());
			}
			// 创建 CoordinateArraySequence 对象
			std::unique_ptr<geos::geom::CoordinateArraySequence> holeSeq(new geos::geom::CoordinateArraySequence(holeCoords.size(), 2));
			for (std::size_t i = 0; i < holeCoords.size(); ++i) {
				holeSeq->setAt(holeCoords[i], i);
			}
			holes.push_back(factory->createLinearRing(std::move(holeSeq)));
		}
		// 创建 Polygon
		geometries.push_back(factory->createPolygon(std::move(shell), std::move(holes)));
	}

	// 使用智能指针创建 GeometryCollection
	std::vector<std::unique_ptr<geos::geom::Geometry>> geomVector;
	for (auto& geom : geometries) {
		geomVector.push_back(std::move(geom));
	}
	auto geomCollection = factory->createGeometryCollection(std::move(geomVector));

	// 返回智能指针
	return geomCollection;
}

// 将 GEOS Geometry 转换为 Store
Store OverlayAnalysis::createStoreFromGeometry(const geos::geom::Geometry* geometry) {
	std::vector<Point*> points;
	std::vector<LineString*> lines;
	std::vector<myPolygon*> polygons;

	for (size_t i = 0; i < geometry->getNumGeometries(); ++i) {
		const geos::geom::Geometry* geom = geometry->getGeometryN(i);

		switch (geom->getGeometryTypeId()) {
		case geos::geom::GEOS_POINT: {
			const geos::geom::Point* point = dynamic_cast<const geos::geom::Point*>(geom);
			points.push_back(new Point(point->getX(), point->getY())); // 保持指针
			break;
		}
		case geos::geom::GEOS_LINESTRING: {
			const geos::geom::LineString* line = dynamic_cast<const geos::geom::LineString*>(geom);
			std::vector<Point> linePoints;
			for (size_t j = 0; j < line->getNumPoints(); ++j) {
				const geos::geom::Coordinate& coord = line->getCoordinateN(j);
				linePoints.push_back(Point(coord.x, coord.y)); // 创建 Point 对象
			}
			lines.push_back(new LineString(linePoints)); // 保持指针
			break;
		}
		case geos::geom::GEOS_POLYGON: {
			const geos::geom::Polygon* polygon = dynamic_cast<const geos::geom::Polygon*>(geom);
			std::vector<Point> exteriorPoints;
			const geos::geom::LinearRing* shell = polygon->getExteriorRing();
			for (size_t j = 0; j < shell->getNumPoints(); ++j) {
				const geos::geom::Coordinate& coord = shell->getCoordinateN(j);
				exteriorPoints.push_back(Point(coord.x, coord.y)); // 创建 Point 对象
			}
			std::vector<LineString> interiorRings;
			for (size_t j = 0; j < polygon->getNumInteriorRing(); ++j) {
				const geos::geom::LineString* interior = polygon->getInteriorRingN(j);
				std::vector<Point> interiorPoints;
				for (size_t k = 0; k < interior->getNumPoints(); ++k) {
					const geos::geom::Coordinate& coord = interior->getCoordinateN(k);
					interiorPoints.push_back(Point(coord.x, coord.y)); // 创建 Point 对象
				}
				interiorRings.push_back(LineString(interiorPoints)); // 创建 LineString 对象
			}
			LineString* exteriorLine = new LineString(exteriorPoints);
			polygons.push_back(new myPolygon(*exteriorLine, interiorRings)); // 保持指针
			break;
		}
		}
	}


	return Store(points, lines, polygons);
}

// 执行叠加操作
Store OverlayAnalysis::performOverlay(const Store& input1, const Store& input2, OperationType operationType) {
	auto geom1 = createGeometryFromStore(input1);
	auto geom2 = createGeometryFromStore(input2);
	if (geom1 == nullptr || geom2 == nullptr) {
		throw std::runtime_error("不存在地理要素");
	}
	std::unique_ptr<geos::geom::Geometry> resultGeom;

	switch (operationType) {
	case Difference:
		resultGeom = geom1->difference(geom2.get());
		break;
	case SymDifference:
		resultGeom = geom1->symDifference(geom2.get());
		break;
	case Union:
		resultGeom = geom1->Union(geom2.get());
		break;
	case Intersection:
		resultGeom = geom1->intersection(geom2.get());
		break;
	}

	Store resultStore = createStoreFromGeometry(resultGeom.get());

	return resultStore;
}
