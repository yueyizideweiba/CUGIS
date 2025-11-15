#include "store_read_TIF.h"
#include "gdal_priv.h"
#include "cpl_conv.h"
#include <thread>
#include <chrono>
#include <ogr_spatialref.h>
#include<QMessageBox>
#include <omp.h>
#include<QFileDialog>
// 设置proj.db搜索路径（这里指定根目录，也可以指定为其它相对路径）
#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
gdal_engine::gdal_engine(QString Path, double mdWidth, double mdHeight, double GlobalMinLon, double GlobalMaxLon, double GlobalMinLat, double GlobalMaxLat)
{
	mdViewWidth = mdWidth;
	mdViewHeight = mdHeight;
	mdGlobalMinLon = GlobalMinLon;
	mdGlobalMaxLon = GlobalMaxLon;
	mdGlobalMinLat = GlobalMinLat;
	mdGlobalMaxLat = GlobalMaxLat;
	update_info(Path);
	update_coordinate();

};
//获取位置信息
std::vector<float> gdal_engine::mvGetposition()
{
	std::vector<float> scale;
	double Scale = mCoordinate.mdScale;
	scale.push_back(mCoordinate.md0Lat - Scale * mnheight);
	scale.push_back(mCoordinate.md0Lon);
	scale.push_back(mCoordinate.md0Lat);
	scale.push_back(mCoordinate.md0Lon + Scale * mnwidth);
	qDebug() << "导入栅格极值" << scale[0] << "  " << scale[1] << "  " << scale[2] << "  " << scale[3];
	return scale;
};

//读取栅格波段
bool gdal_engine::BandChose(int nR, int nG, int nB)
{
	if (nR >= mpvBand.size() || nG >= mpvBand.size() || nB >= mpvBand.size())
	{
		qDebug() << "cannot find the band";
		return 0;
	};
	mpnBandsChosen[0] = nR;
	mpnBandsChosen[1] = nG;
	mpnBandsChosen[2] = nB;
	return 1;
};

//数据转化
void gdal_engine::my_strcpy(char* dest, const char* src) {
	while ((*dest++ = *src++) != '\0');

};

//读取栅格数据
void gdal_engine::update_info(QString Path)
{
	//转换格式
	const char* charStr = Path.toUtf8().constData();//UTF8格式编码
	size_t length = std::strlen(charStr) + 1; // 加 1 是为了包括结尾的空字符
	mpcQPath = new char[length]; // 动态分配内存
	my_strcpy(mpcQPath, charStr); // 复制字符串内容
	GDALAllRegister();
	GDALDataset* dataset = (GDALDataset*)GDALOpen(mpcQPath, GA_ReadOnly);
	qDebug() << mpcQPath;

	//初始化其余成员变量
	if (dataset == nullptr) {
		qDebug() << "cannot inital tif file: " << Path;
		qDebug() << charStr;
		qDebug() << mpcQPath;
		return;
	}
	else
	{
		mnwidth = dataset->GetRasterXSize();
		mnheight = dataset->GetRasterYSize();
		mnNums_Band = dataset->GetRasterCount();
		const char* driverName = dataset->GetDriver()->GetDescription();
		size_t length = std::strlen(driverName) + 1; // 加 1 是为了包括结尾的空字符
		mpcTypeofBand = new char[length]; // 动态分配内存
		strcpy(mpcTypeofBand, driverName); // 复制字符串内容
		QString qPath(mpcQPath);
		QString typeOfBand(mpcTypeofBand);
		for (int tagBand = 1; tagBand <= mnNums_Band; tagBand++)
		{
			mpvBand.push_back(dataset->GetRasterBand(tagBand));
		};
		qDebug() << "open" << qPath << " dataType is" << typeOfBand << "   bands" << mnNums_Band;
		qDebug() << "width  " << mnwidth << "  hright  " << mnheight;
	};
	//初始化波段
	mpnBandsChosen[0] = 0;
	mpnBandsChosen[1] = 0;
	mpnBandsChosen[2] = 0;
};

//更新坐标
void gdal_engine::update_coordinate()
{
	GDALDataset* dataset = (GDALDataset*)GDALOpen(mpcQPath, GA_ReadOnly);
	// 获取投影信息
	OGRSpatialReference SRS;
	const char* pszProjection;
	pszProjection = dataset->GetProjectionRef();
	// 设置坐标系
	if (SRS.importFromWkt((char**)&pszProjection) == OGRERR_NONE) {
		char* pszPrettyWkt = NULL;
		SRS.exportToPrettyWkt(&pszPrettyWkt, false);
		qDebug() << "坐标系信息:";
		qDebug().noquote() << pszPrettyWkt;
		CPLFree(pszPrettyWkt);
	}
	else {
		printf("无法获取坐标系信息\n");
	}
	double adfGeoTransform[6];
	if (dataset->GetGeoTransform(adfGeoTransform) == CE_None) {
		double x = adfGeoTransform[0]; // 左上角x坐标
		double y = adfGeoTransform[3]; // 左上角y坐标
		double width = adfGeoTransform[1]; // x方向分辨率
		double height = adfGeoTransform[5]; // y方向分辨率
		qDebug() << "左上角坐标 (x, y):" << x << "," << y;
		qDebug() << "分辨率 (width, height):" << width << "," << height;
	}
	else {
		qDebug() << "无法获取地理变换信息";
	}
	mCoordinate.update(adfGeoTransform[0], adfGeoTransform[3], adfGeoTransform[1], adfGeoTransform[5], mnwidth, mnheight);

	qDebug() << "左上角坐标 (x, y):" << mCoordinate.md0Lon << "," << mCoordinate.md0Lat;
	mdSCale = adfGeoTransform[1]; mfX = adfGeoTransform[0]; mfY = adfGeoTransform[3];
	qDebug() << "左上角坐标 (x, y):" << mCoordinate.md0Lon << "," << mCoordinate.md0Lat;

	mdSCale = std::abs(std::min(mdViewWidth / (mdGlobalMaxLon - mdGlobalMinLon) * adfGeoTransform[1], mdViewHeight / (mdGlobalMaxLat - mdGlobalMinLat) * adfGeoTransform[1]));
	//mfX = adfGeoTransform[0]; mfY = adfGeoTransform[3];

	qDebug() << "视图范围" << mdViewWidth << "  " << mdViewHeight;
	qDebug() << adfGeoTransform[1] * mnwidth << "  " << adfGeoTransform[1] * mnheight;
	qDebug() << "缩放比例" << mdSCale;
	qDebug() << "历史极值" << mdGlobalMinLon << " " << mdGlobalMaxLat;

	mfX = (adfGeoTransform[0] - mdGlobalMinLon) * mdSCale / adfGeoTransform[1];
	//mfY = (mdGlobalMaxLat-adfGeoTransform[3])*mdSCale/ adfGeoTransform[1];
	mfY = (mdGlobalMaxLat - mCoordinate.md0Lat) * mdSCale / adfGeoTransform[1];
	if (mdGlobalMaxLon == 0 && mdGlobalMinLon == 0)
	{
		mdSCale = std::abs(std::min(mdViewWidth / (mnwidth), mdViewHeight / (mnheight)));
		mfX = adfGeoTransform[0]; mfY = adfGeoTransform[3];
	}
	qDebug() << "mfy" << mfY << " " << mdGlobalMaxLat << " " << mCoordinate.md0Lat << " " << adfGeoTransform[1];
};


//读取一个波段
bool gdal_engine::readTIFOneBand(int Band, std::vector<unsigned char>& tifStore)
{
	tifStore.clear();
	tifStore.resize(mnheight * mnwidth);
	qDebug() << "begain";
	mpvBand[Band - 1]->RasterIO(GF_Read, 0, 0, mnwidth, mnheight, tifStore.data(), mnwidth, mnheight, GDT_Byte, 0, 0);
	qDebug() << "end";
	return 1;
};


//栅格快速显示
void gdal_engine::quickTifShow(myGraphicsView* view, QGraphicsScene* scene, const float FSCALE, bool multiband, int tileSize)
{
	auto start = std::chrono::steady_clock::now();
	int numTilesX = (mnwidth + tileSize - 1) / tileSize;  // Ceiling division
	int numTilesY = (mnheight + tileSize - 1) / tileSize;
	const float FX = mfX;
	const float FY = mfY;
	for (int tileY = 0; tileY < numTilesY; ++tileY) {
		for (int tileX = 0; tileX < numTilesX; ++tileX) {
			int xoff = tileX * tileSize;
			int yoff = tileY * tileSize;
			int xsize = std::min(tileSize, mnwidth - xoff);
			int ysize = std::min(tileSize, mnheight - yoff);
			//内存分配
			unsigned char* pafScanlineR; // 模板R
			pafScanlineR = (unsigned char*)CPLMalloc(sizeof(unsigned char) * xsize * ysize);
			unsigned char* pafScanlineG; // 模板G
			pafScanlineG = (unsigned char*)CPLMalloc(sizeof(unsigned char) * xsize * ysize);
			unsigned char* pafScanlineB; // 模板B
			pafScanlineB = (unsigned char*)CPLMalloc(sizeof(unsigned char) * xsize * ysize);
			if (multiband == 0)
				// Read the raster data
			{
				mpvBand[0]->RasterIO(GF_Read, xoff, yoff, xsize, ysize,
					pafScanlineR, xsize, ysize, GDT_Byte, 0, 0);
				mpvBand[0]->RasterIO(GF_Read, xoff, yoff, xsize, ysize,
					pafScanlineG, xsize, ysize, GDT_Byte, 0, 0);
				mpvBand[0]->RasterIO(GF_Read, xoff, yoff, xsize, ysize,
					pafScanlineB, xsize, ysize, GDT_Byte, 0, 0);
			}
			else
			{
				mpvBand[mpnBandsChosen[0]]->RasterIO(GF_Read, xoff, yoff, xsize, ysize,
					pafScanlineR, xsize, ysize, GDT_Byte, 0, 0);
				mpvBand[mpnBandsChosen[1]]->RasterIO(GF_Read, xoff, yoff, xsize, ysize,
					pafScanlineG, xsize, ysize, GDT_Byte, 0, 0);
				mpvBand[mpnBandsChosen[2]]->RasterIO(GF_Read, xoff, yoff, xsize, ysize,
					pafScanlineB, xsize, ysize, GDT_Byte, 0, 0);
			}
			CPLErr errR = mpvBand[0]->RasterIO(GF_Read, xoff, yoff, xsize, ysize,
				pafScanlineR, xsize, ysize, GDT_Byte, 0, 0);
			//qDebug() << "read";
			QImage img(xsize, ysize, QImage::Format_RGB32);
			//qDebug() << "img";
			QRgb* line = reinterpret_cast<QRgb*>(img.bits());
			//qDebug() << "line";
			int bytesPerPixel = sizeof(unsigned char); // 假设每个像素占用 unsigned 类型的字节数
			//int stride = xsize * bytesPerPixel;    // 计算每行像素数据的步幅
			for (int y = 0; y < ysize; ++y) {
				for (int x = 0; x < xsize; ++x) {
					int index = y * xsize + x;
					line[y * xsize + x] = qRgb(pafScanlineR[index], pafScanlineG[index], pafScanlineB[index]);
				}
			};
			QImage imgCopy = img.copy(0, 0, xsize, ysize);
			QPixmap pixmap = QPixmap::fromImage(imgCopy);
			view->setScene(scene);
			QGraphicsPixmapItem* item = scene->addPixmap(pixmap);
			item->setPos(float(FX + xoff * mdSCale), float((FY + yoff * mdSCale)));
			qreal _tempScal = qreal(FSCALE);
			item->setScale(_tempScal);
			view->show();
			QRectF sceneRect(FX, FY, mnwidth * FSCALE, mnheight * FSCALE);
			view->centerOn(item);

			// 将视图调整为适应场景矩形
			view->fitInView(sceneRect, Qt::KeepAspectRatio);
			CPLFree(pafScanlineR);
			CPLFree(pafScanlineG);
			CPLFree(pafScanlineB);
			QCoreApplication::processEvents();
		};
	};
	auto end = std::chrono::steady_clock::now();  // 记录结束时间
	// 计算执行时间并打印
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	qDebug() << "Elapsed time: " << duration.count() << " milliseconds\n";
};

//栅格普通显示
void gdal_engine::tiffShow(myGraphicsView* view, QGraphicsScene* scene)
{
	GDALDataset* poDataset;
	GDALAllRegister();
	poDataset = (GDALDataset*)GDALOpen(mpcQPath, GA_ReadOnly);
	if (poDataset == NULL) {

		return;
	}
	int nBands = poDataset->GetRasterCount();
	int width = poDataset->GetRasterXSize();
	int height = poDataset->GetRasterYSize();
	GDALRasterBand* uniqueBand = mpvBand[0];
	QImage img(width, height, QImage::Format_RGB32);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			double value;
			uniqueBand->RasterIO(GF_Read, x, y, 1, 1, &value, 1, 1, GDT_Float64, 0, 0);
			QRgb colorValue = qRgb(value, value, value);
			img.setPixel(x, y, colorValue);
		};
	};
	QImage imgCopy = img.copy(0, 0, width, height);
	QPixmap pixmap = QPixmap::fromImage(imgCopy);
	qDebug() << "pixmap";
	QGraphicsPixmapItem* item = scene->addPixmap(pixmap);
	scene->addItem(item);
	item->setPos(mfX, mfY);
	item->setScale(mdSCale);
	view->setScene(scene);
	view->show();
	GDALClose(poDataset);
}

//栅格分析
void gdal_engine::tifValueProcess(myGraphicsView* view, QGraphicsScene* scene, int band, int TyPe, int xoff, int yoff)
{
	GDALDataset* poDataset;
	GDALAllRegister();
	poDataset = (GDALDataset*)GDALOpen(mpcQPath, GA_ReadOnly);
	if (poDataset == NULL) {
		return;
	}
	//获取栅格数据信息
	int nBands = poDataset->GetRasterCount();
	int width = poDataset->GetRasterXSize();
	int height = poDataset->GetRasterYSize();
	//分配内存以读取栅格波段
	GDALRasterBand* uniqueBand = mpvBand[band];
	QImage img(width, height, QImage::Format_RGB32);
	//使用单独的缓冲区存储计算值
	std::vector<double> buffer(width * height);
	std::vector<float> imageData(width * height);
	uniqueBand->RasterIO(GF_Read, 0, 0, width, height, imageData.data(), width, height, GDT_Float32, 0, 0);
	//使用OpenMP并行处理
#pragma omp parallel for
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			int xMin = std::max(0, x - 1);
			int xMax = std::min(width - 1, x + 1);
			int yMin = std::max(0, y - 1);
			int yMax = std::min(height - 1, y + 1);

			std::vector<double> windowValues;
			for (int yy = yMin; yy <= yMax; ++yy) {
				for (int xx = xMin; xx <= xMax; ++xx) {
					float value;
					value = imageData[yy * width + xx];
					windowValues.push_back(value);
				}
			}

			double Value = 0;
			if (TyPe == MINMUM) {
				Value = *std::min_element(windowValues.begin(), windowValues.end());
			}
			else if (TyPe == MAXIMUM) {
				Value = *std::max_element(windowValues.begin(), windowValues.end());
			}
			else if (TyPe == AVERAGE) {
				Value = std::accumulate(windowValues.begin(), windowValues.end(), 0.0) / windowValues.size();
			}
			else {
				qDebug() << "Error: Invalid type specified";
			}
			buffer[y * width + x] = Value;
		}
	}
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			double Value = buffer[y * width + x];
			QRgb colorValue = qRgb(Value, Value, Value);
			img.setPixel(x, y, colorValue);
		}
	}
	QImage imgCopy = img.copy(0, 0, width, height);
	QPixmap pixmap = QPixmap::fromImage(imgCopy);
	qDebug() << "pixmap";
	view->setScene(scene);
	scene->clear();
	std::vector<unsigned char> byteBuffer(width * height);
	double minVal = *std::min_element(buffer.begin(), buffer.end());
	double maxVal = *std::max_element(buffer.begin(), buffer.end());
	for (int i = 0; i < buffer.size(); ++i) {
		// Map float values to the range [0, 255]
		byteBuffer[i] = static_cast<unsigned char>(
			255 * (buffer[i] - minVal) / (maxVal - minVal)
			);
	}
	QGraphicsPixmapItem* item = scene->addPixmap(pixmap);
	item->setPos(mfX, mfY);
	item->setScale(mdSCale);
	view->show();
	QString fileName = QFileDialog::getSaveFileName(
		view,
		"Save Image",
		QString(),
		"TIFF Images (*.tif *.tiff)"
	);
	if (fileName.isEmpty()) {
		GDALClose(poDataset);
		return;
	}
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (poDriver == NULL) {
		GDALClose(poDataset);
		return;
	}
	GDALDataset* poNewDataset = poDriver->Create(fileName.toStdString().c_str(), width, height, 1, GDT_Byte, NULL);
	if (poNewDataset == NULL) {
		GDALClose(poDataset);
		return;
	}
	double adfGeoTransform[6];
	poDataset->GetGeoTransform(adfGeoTransform);
	poNewDataset->SetProjection(poDataset->GetProjectionRef());
	poNewDataset->SetGeoTransform(adfGeoTransform);
	GDALRasterBand* poNewBand = poNewDataset->GetRasterBand(1);
	poNewBand->RasterIO(GF_Write, 0, 0, width, height, byteBuffer.data(), width, height, GDT_Byte, 0, 0);
	GDALClose(poDataset);
	GDALClose(poNewDataset);
}

double gdal_engine::getMinX() const {
	return mfX;
}

double gdal_engine::getMaxX() const {
	return mfX + mnwidth * mdSCale;
}

double gdal_engine::getMinY() const {
	return mfY - mnheight * mdSCale;
}

double gdal_engine::getMaxY() const {
	return mfY;
};
void gdal_engine::fillHoles(myGraphicsView* view) {
	// 打开栅格数据集
	GDALDataset* dataset = (GDALDataset*)GDALOpen(this->mpcQPath, GA_Update);
	if (dataset == nullptr) {
		std::cerr << "Failed to open file." << std::endl;
		return;
	}

	// 假设我们处理第一个波段
	GDALRasterBand* band = dataset->GetRasterBand(1);
	if (band == nullptr) {
		std::cerr << "Failed to get raster band." << std::endl;
		GDALClose(dataset);
		return;
	}

	// 获取栅格的尺寸
	int width = band->GetXSize();
	int height = band->GetYSize();

	// 创建缓冲区读取栅格数据
	std::vector<int> buffer(width * height);
	band->RasterIO(GF_Read, 0, 0, width, height, buffer.data(), width, height, GDT_Int32, 0, 0);

	bool modified;
	do {
		modified = false;
		for (int y = 1; y < height - 1; ++y) {
			for (int x = 1; x < width - 1; ++x) {
				int centerIdx = y * width + x;
				int centerValue = buffer[centerIdx];

				// 收集3x3窗口中的值
				std::vector<int> neighbors;
				for (int dy = -1; dy <= 1; ++dy) {
					for (int dx = -1; dx <= 1; ++dx) {
						if (dy == 0 && dx == 0) continue;
						int idx = (y + dy) * width + (x + dx);
						neighbors.push_back(buffer[idx]);
					}
				}

				int maxNeighbor = *std::max_element(neighbors.begin(), neighbors.end());
				int minNeighbor = *std::min_element(neighbors.begin(), neighbors.end());

				if (centerValue > maxNeighbor) {
					buffer[centerIdx] = maxNeighbor;
					modified = true;
				}
				else if (centerValue < minNeighbor) {
					buffer[centerIdx] = minNeighbor;
					modified = true;
				}
			}
		}
	} while (modified);

	// 将修改后的数据写回栅格
   // band->RasterIO(GF_Write, 0, 0, width, height, buffer.data(), width, height, GDT_Int32, 0, 0);
	QString fileName = QFileDialog::getSaveFileName(
		view,
		"Save Image",
		QString(),
		"TIFF Images (*.tif *.tiff)"
	);

	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");

	GDALDataset* poNewDataset = poDriver->Create(fileName.toStdString().c_str(), width, height, 1, GDT_Int32, NULL);

	double adfGeoTransform[6];
	dataset->GetGeoTransform(adfGeoTransform);
	poNewDataset->SetProjection(dataset->GetProjectionRef());
	poNewDataset->SetGeoTransform(adfGeoTransform);
	GDALRasterBand* poNewBand = poNewDataset->GetRasterBand(1);
	poNewBand->RasterIO(GF_Write, 0, 0, width, height, buffer.data(), width, height, GDT_Int32, 0, 0);
	//GDALClose(poDataset);
	GDALClose(poNewDataset);
	// 关闭数据集
	GDALClose(dataset);
};
int gdal_engine::CalculateD8FlowDirection(int x, int y, const std::vector<float>& elevation, int width, int height) {
	static const int offsets[8][2] = {
		{ 0, -1}, { 1, -1}, { 1, 0}, { 1, 1},
		{ 0,  1}, {-1, 1}, {-1, 0}, {-1, -1}
	};

	static const int directions[8] = {
		D8_N, D8_NE, D8_E, D8_SE,
		D8_S, D8_SW, D8_W, D8_NW
	};

	float min_slope = std::numeric_limits<float>::max();
	int flow_direction = 0;

	float center_elevation = elevation[y * width + x];

	for (int i = 0; i < 8; ++i) {
		int new_x = x + offsets[i][0];
		int new_y = y + offsets[i][1];

		if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height) {
			float neighbor_elevation = elevation[new_y * width + new_x];
			float slope = center_elevation - neighbor_elevation;
			if (slope < min_slope) {
				min_slope = slope;
				flow_direction = directions[i];
			}
		}
	}
	return flow_direction;
}


void gdal_engine::D8FlowDirectionAnalysis(myGraphicsView* view) {
	QString fileName = QFileDialog::getSaveFileName(
		view,
		"Save Image",
		QString(),
		"TIFF Images (*.tif *.tiff)"
	);

	if (fileName.isEmpty()) {
		std::cerr << "No output file selected." << std::endl;
		return;
	}

	GDALDataset* dataset = (GDALDataset*)GDALOpen(this->mpcQPath, GA_ReadOnly);
	if (dataset == nullptr) {
		std::cerr << "Failed to open input file." << std::endl;
		return;
	}

	GDALRasterBand* band = dataset->GetRasterBand(1);
	if (band == nullptr) {
		std::cerr << "Failed to get raster band." << std::endl;
		GDALClose(dataset);
		return;
	}

	int width = band->GetXSize();
	int height = band->GetYSize();
	qDebug() << " create before";

	// Use 1D vector for elevation and flow_direction
	std::vector<float> elevation(width * height);
	std::vector<int> flow_direction(width * height, 0);

	qDebug() << "create after";

	// Read elevation data
	band->RasterIO(GF_Read, 0, 0, width, height, elevation.data(), width, height, GDT_Float32, 0, 0);

	// Calculate D8 flow directions
	qDebug() << "Calculate D8 flow directions";
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			flow_direction[y * width + x] = CalculateD8FlowDirection(x, y, elevation, width, height);

		}
	}

	// Create output dataset
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (driver == nullptr) {
		std::cerr << "Failed to get GTiff driver." << std::endl;
		GDALClose(dataset);
		return;
	}

	GDALDataset* outDataset = driver->Create(fileName.toStdString().c_str(), width, height, 1, GDT_Int32, nullptr);
	if (outDataset == nullptr) {
		std::cerr << "Failed to create output file." << std::endl;
		GDALClose(dataset);
		return;
	}

	GDALRasterBand* outBand = outDataset->GetRasterBand(1);
	if (outBand == nullptr) {
		std::cerr << "Failed to get output raster band." << std::endl;
		GDALClose(outDataset);
		GDALClose(dataset);
		return;
	}

	// Copy geotransform and projection
	double adfGeoTransform[6];
	dataset->GetGeoTransform(adfGeoTransform);
	outDataset->SetProjection(dataset->GetProjectionRef());
	outDataset->SetGeoTransform(adfGeoTransform);

	// Write flow direction data
	outBand->RasterIO(GF_Write, 0, 0, width, height, flow_direction.data(), width, height, GDT_Int32, 0, 0);

	// Close datasets
	GDALClose(outDataset);
	GDALClose(dataset);
};

float gdal_engine::CalculateAspect(int x, int y, const std::vector<float>& elevation, int width, int height, float dx, float dy) {
	static const int offsets[8][2] = {
		{ 0, -1}, { 1, -1}, { 1, 0}, { 1, 1},
		{ 0,  1}, {-1, 1}, {-1, 0}, {-1, -1}
	};

	float dzdx = 0.0f;
	float dzdy = 0.0f;
	float center_elevation = elevation[y * width + x];

	// 计算x方向和y方向的梯度
	for (int i = 0; i < 8; ++i) {
		int new_x = x + offsets[i][0];
		int new_y = y + offsets[i][1];

		if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height) {
			float neighbor_elevation = elevation[new_y * width + new_x];
			float dx_offset = offsets[i][0] * dx;
			float dy_offset = offsets[i][1] * dy;

			dzdx += (neighbor_elevation - center_elevation) * dx_offset;

			dzdy += (neighbor_elevation - center_elevation) * dy_offset;

		}
	}

	// 计算坡向
	float aspect = atan2(dzdy, -dzdx) * (180.0 / M_PI);
	if (aspect < 0) {
		aspect += 360.0;
	}

	return aspect;
};
//计算坡向

void gdal_engine::AspectAnalysis(myGraphicsView* view) {
	QString fileName = QFileDialog::getSaveFileName(
		view,
		"Save Aspect Image",
		QString(),
		"TIFF Images (*.tif *.tiff)"
	);

	if (fileName.isEmpty()) {
		std::cerr << "No output file selected." << std::endl;
		return;
	}

	GDALDataset* dataset = (GDALDataset*)GDALOpen(this->mpcQPath, GA_ReadOnly);
	if (dataset == nullptr) {
		std::cerr << "Failed to open input file." << std::endl;
		return;
	}

	GDALRasterBand* band = dataset->GetRasterBand(1);
	if (band == nullptr) {
		std::cerr << "Failed to get raster band." << std::endl;
		GDALClose(dataset);
		return;
	}

	int width = band->GetXSize();
	int height = band->GetYSize();

	// 获取像素单位长度
	double adfGeoTransform[6];
	dataset->GetGeoTransform(adfGeoTransform);
	float dx = fabs(adfGeoTransform[1]); // 像素宽度
	float dy = fabs(adfGeoTransform[5]); // 像素高度

	// 使用一维数组存储高程和坡向数据
	std::vector<float> elevation(width * height);
	std::vector<float> aspect(width * height, 0.0f);

	// 读取高程数据
	band->RasterIO(GF_Read, 0, 0, width, height, elevation.data(), width, height, GDT_Float32, 0, 0);

	// 计算坡向数据
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			aspect[y * width + x] = CalculateAspect(x, y, elevation, width, height, dx, dy);
		}
	}

	// 创建输出数据集
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (driver == nullptr) {
		std::cerr << "Failed to get GTiff driver." << std::endl;
		GDALClose(dataset);
		return;
	}

	GDALDataset* outDataset = driver->Create(fileName.toStdString().c_str(), width, height, 1, GDT_Float32, nullptr);
	if (outDataset == nullptr) {
		std::cerr << "Failed to create output file." << std::endl;
		GDALClose(dataset);
		return;
	}

	GDALRasterBand* outBand = outDataset->GetRasterBand(1);
	if (outBand == nullptr) {
		std::cerr << "Failed to get output raster band." << std::endl;
		GDALClose(outDataset);
		GDALClose(dataset);
		return;
	}

	// 复制地理变换和投影
	outDataset->SetProjection(dataset->GetProjectionRef());
	outDataset->SetGeoTransform(adfGeoTransform);

	// 写入坡向数据
	outBand->RasterIO(GF_Write, 0, 0, width, height, aspect.data(), width, height, GDT_Float32, 0, 0);

	// 关闭数据集
	GDALClose(outDataset);
	GDALClose(dataset);
};


float gdal_engine::CalculateSlope(int x, int y, const std::vector<float>& elevation, int width, int height, float dx, float dy) {
	// 提取周围8个点的高程
	float a = (x > 0 && y > 0) ? elevation[(y - 1) * width + (x - 1)] : 0.0f;
	float b = (y > 0) ? elevation[(y - 1) * width + x] : 0.0f;
	float c = (x < width - 1 && y > 0) ? elevation[(y - 1) * width + (x + 1)] : 0.0f;
	float d = (x > 0) ? elevation[y * width + (x - 1)] : 0.0f;
	float e = elevation[y * width + x];
	float f = (x < width - 1) ? elevation[y * width + (x + 1)] : 0.0f;
	float g = (x > 0 && y < height - 1) ? elevation[(y + 1) * width + (x - 1)] : 0.0f;
	float h = (y < height - 1) ? elevation[(y + 1) * width + x] : 0.0f;
	float i = (x < width - 1 && y < height - 1) ? elevation[(y + 1) * width + (x + 1)] : 0.0f;

	// 计算dz/dx
	float dzdx = ((c + 2 * f + i) * 4 / 4 - (a + 2 * d + g) * 4 / 4) / (8 * dx);

	// 计算dz/dy
	float dzdy = ((g + 2 * h + i) * 4 / 4 - (a + 2 * b + c) * 4 / 4) / (8 * dy);

	// 计算坡向
	float aspect = atan(sqrt(dzdy * dzdy + dzdx * dzdx)) * (180.0 / M_PI);

	return aspect;
};

void gdal_engine::SlopeAnalysis(myGraphicsView* view, float dX, float dY) {
	QString fileName = QFileDialog::getSaveFileName(
		view,
		"Save Slope Image",
		QString(),
		"TIFF Images (*.tif *.tiff)"
	);

	if (fileName.isEmpty()) {
		std::cerr << "No output file selected." << std::endl;
		return;
	}

	GDALDataset* dataset = (GDALDataset*)GDALOpen(this->mpcQPath, GA_ReadOnly);
	if (dataset == nullptr) {
		std::cerr << "Failed to open input file." << std::endl;
		return;
	}

	GDALRasterBand* band = dataset->GetRasterBand(1);
	if (band == nullptr) {
		std::cerr << "Failed to get raster band." << std::endl;
		GDALClose(dataset);
		return;
	}

	int width = band->GetXSize();
	int height = band->GetYSize();

	// 手动输入栅格的单位长度（单位：米）
	float dx = dX, dy = dY;
	// 使用一维数组存储高程和坡度数据
	std::vector<float> elevation(width * height);
	std::vector<float> slope(width * height, 0.0f);

	// 读取高程数据
	band->RasterIO(GF_Read, 0, 0, width, height, elevation.data(), width, height, GDT_Float32, 0, 0);

	// 计算坡度数据
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			slope[y * width + x] = CalculateSlope(x, y, elevation, width, height, dx, dy);
			//	qDebug() << "Calculate Slope";
		}
	}

	// 创建输出数据集
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (driver == nullptr) {
		std::cerr << "Failed to get GTiff driver." << std::endl;
		GDALClose(dataset);
		return;
	}

	GDALDataset* outDataset = driver->Create(fileName.toStdString().c_str(), width, height, 1, GDT_Float32, nullptr);
	if (outDataset == nullptr) {
		std::cerr << "Failed to create output file." << std::endl;
		GDALClose(dataset);
		return;
	}

	GDALRasterBand* outBand = outDataset->GetRasterBand(1);
	if (outBand == nullptr) {
		std::cerr << "Failed to get output raster band." << std::endl;
		GDALClose(outDataset);
		GDALClose(dataset);
		return;
	}
	double adfGeoTransform[6];
	dataset->GetGeoTransform(adfGeoTransform);


	// 复制地理变换和投影
	outDataset->SetProjection(dataset->GetProjectionRef());
	outDataset->SetGeoTransform(adfGeoTransform);

	// 写入坡度数据
	outBand->RasterIO(GF_Write, 0, 0, width, height, slope.data(), width, height, GDT_Float32, 0, 0);

	// 关闭数据集
	GDALClose(outDataset);
	GDALClose(dataset);
};
