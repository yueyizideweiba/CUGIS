#include"writeVec.h"

// 写入shp文件
void vecWriter::writeShapefile(std::string strFilePath, Store& store) {
	// 注册所有GDAL驱动
	GDALAllRegister();
	OGRRegisterAll();
	// 创建一个新的Shapefile文件
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
	if (driver == nullptr) {
		std::cerr << "驱动不可用!" << std::endl;
		return;
	}
	GDALDataset* poDS = driver->Create(strFilePath.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
	if (poDS == nullptr) {
		std::cerr << "创建输出文件失败." << std::endl;
		return;
	}
	if (store.getStorePoints().size() > 0) {
		// 创建一个用于存储点的图层
		OGRLayer* pointLayer = poDS->CreateLayer("Points", nullptr, wkbPoint, nullptr);
		// 向点图层添加点
		std::vector<Point*> points = store.getStorePoints();
		for (Point* point : points) {
			OGRPoint* ogrPoint = new OGRPoint;
			ogrPoint->setX(point->getX());
			ogrPoint->setY(point->getY());
			OGRFeature* feature = new OGRFeature(pointLayer->GetLayerDefn());
			feature->SetGeometry(ogrPoint);
			if (pointLayer->CreateFeature(feature) != OGRERR_NONE) {
				std::cerr << "在点图层创建要素失败." << std::endl;
				OGRFeature::DestroyFeature(feature);
				GDALClose(poDS);
				return;
			}
			OGRFeature::DestroyFeature(feature);
		}
	}
	else if (store.getStoreLines().size() > 0) {
		// 创建一个用于存储线的图层
		OGRLayer* lineLayer = poDS->CreateLayer("Lines", nullptr, wkbLineString, nullptr);
		// 向线图层添加线
		std::vector<LineString*> lines = store.getStoreLines();
		for (LineString* line : lines) {
			OGRLineString* ogrLineString = new OGRLineString();
			std::vector<Point> points = line->getPoints();
			for (const Point& point : points) {
				ogrLineString->addPoint(point.getX(), point.getY());
			}
			OGRFeature* feature = new OGRFeature(lineLayer->GetLayerDefn());
			feature->SetGeometry(ogrLineString);
			if (lineLayer->CreateFeature(feature) != OGRERR_NONE) {
				std::cerr << "在线图层创建要素失败." << std::endl;
				OGRFeature::DestroyFeature(feature);
				GDALClose(poDS);
				return;
			}
			OGRFeature::DestroyFeature(feature);
		}
	}
	else if (store.getStorePolygons().size() > 0) {
		// 创建一个用于存储多边形的图层
		OGRLayer* polygonLayer = poDS->CreateLayer("Polygons", nullptr, wkbPolygon, nullptr);
		// 向多边形图层添加多边形
		std::vector<myPolygon*> polygons = store.getStorePolygons();
		for (myPolygon* polygon : polygons) {
			OGRPolygon* ogrPolygon = new OGRPolygon();
			OGRLinearRing* ogrLout = new OGRLinearRing();
			std::vector<Point> points = polygon->getExteriorRing().getPoints();
			for (const Point& point : points)
			{
				ogrLout->addPoint(point.getX(), point.getY());
			}
			ogrPolygon->addRing(ogrLout);
			for (const LineString& innerRing : polygon->getInteriorRings())
			{
				OGRLinearRing* ogrLin = new OGRLinearRing();
				for (const Point& point : innerRing.getPoints())
				{
					ogrLin->addPoint(point.getX(), point.getY());
				}
				ogrPolygon->addRing(ogrLin);
			}
			OGRFeature* feature = new OGRFeature(polygonLayer->GetLayerDefn());
			feature->SetGeometry(ogrPolygon);
			if (polygonLayer->CreateFeature(feature) != OGRERR_NONE) {
				std::cerr << "在多边形图层创建要素失败." << std::endl;
				OGRFeature::DestroyFeature(feature);
				GDALClose(poDS);
				return;
			}
			OGRFeature::DestroyFeature(feature);
		}
	}
	// 清理
	GDALClose(poDS);
}