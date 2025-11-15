//ImageCut.cpp
//read
#include "ImageCut.h"
#include<QDebug>
ImageCut::ImageCut() {
	GDALAllRegister();
	OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	CPLSetConfigOption("SHAPE_ENCODING", "");
}

ImageCut::~ImageCut() {
	GDALClose(mppoTrSrcImage);//关闭源影像
}
//加载源影像
void ImageCut::setSrcImageFilename(char* pszSrcFile) {
	this->mpcSzSrcFile = pszSrcFile;
}
//输出AOI的tif文件
void ImageCut::setDstImageFilename(char* pszDstFile) {
	this->mpcSzDstFile = pszDstFile;
}
//输出AOI的wkt文件
void ImageCut::setShapeFilename(char* pszShapeName) {
	this->mpcSzShapeName = pszShapeName;
}
//设置输出格式
void ImageCut::setImageFormat(char* pszFormat) {
	this->mpcSzFormat = pszFormat;
}
//计算函数
int ImageCut::ImageCutByAOI() {
	string pszAOIWKT = "";
	int isoutput = outputPolygonWkt(pszAOIWKT);
	if (isoutput == 0)
	{
		cout << "output polygon wkt false" << endl;
		return 0;
	}
	std::vector<char>_temp;

	for (char c : pszAOIWKT) {
		if (c != '\n') {
			_temp.push_back(c);
		}
	};
	char* pszAOIWKT1 = new char[_temp.size()];
	for (int i = 0; i < _temp.size(); i++)
	{
		pszAOIWKT1[i] = _temp[i];
	}
	int iscut = ImageCutting(pszAOIWKT1);
	if (iscut != 1)
	{
		cout << "Image Cut false" << endl;
		return 0;
	}
	return 1;
}
//输出AOI的wkt文件
int ImageCut::outputPolygonWkt(string& pszAOIWKT) {
	mppoTrSrcImage = (GDALDataset*)GDALOpen(mpcSzSrcFile, GA_ReadOnly);
	if (mppoTrSrcImage == NULL)
	{
		cout << "read image false" << endl;
		return 0;
	}
	double SrcGeoTransform[6] = { 0 };
	mppoTrSrcImage->GetGeoTransform(SrcGeoTransform);
	//读取shape数据--------------
	
	const char* _temo = mpcSzShapeName;
	GDALDataset* ptrShape = (GDALDataset*)GDALOpenEx(_temo, GDAL_OF_VECTOR, NULL, NULL, NULL);

	if (ptrShape == NULL)
	{
		cout << "read shape false" << endl;
		return 0;
	}
	int numLayer = ptrShape->GetLayerCount();//获取图层个数
	if (numLayer != 1)
	{
		cout << "the method only support one layer" << endl;
		return 0;
	}
	OGRLayer* ptrLayer = ptrShape->GetLayer(0);//这里只支持一个图层
	if (ptrLayer == NULL)
	{
		cout << "open the layer false" << endl;
		return 0;
	}
	qDebug() << "line 86" << "pszDstFile" << mpcSzDstFile;
	ptrLayer->ResetReading();//将图层重置一下，标号从0开始
	OGRFeature* poFeature = NULL;//定义包含几何和属性的要素指针
	//统计一下图层内多边形个数
	int count_ = 0, cout_1(0);
	while ((poFeature = ptrLayer->GetNextFeature()) != NULL) {
		count_++;
	}
	poFeature = NULL;
	ptrLayer->ResetReading();
	pszAOIWKT.clear();
	if (count_ == 1)
	{
		pszAOIWKT = "POLYGON ((";
	}
	else
	{
		pszAOIWKT = "MULTIPOLYGON (((";
	}
	while ((poFeature = ptrLayer->GetNextFeature()) != NULL)
	{
		OGRGeometry* ptrGeometry = poFeature->GetGeometryRef();//获得要素的几何属性
		OGRwkbGeometryType geomType = wkbFlatten(ptrGeometry->getGeometryType());
		cout << "Geometry type: " << geomType << endl;
		if (wkbFlatten(ptrGeometry->getGeometryType()) != wkbPolygon)
		{
			cout << "Only polygon feature cutting images are supported(wkbPolygon)" << endl;
			continue;
		}
		OGRPolygon* ptrPolygon = (OGRPolygon*)ptrGeometry->clone();//将几何要素指针强制转化为多边形要素指针
		OGRLinearRing* ptrOGRLinearRing = ptrPolygon->getExteriorRing();//获取多边形的外环点的指针
		int numExterRing = ptrOGRLinearRing->getNumPoints();//外环点数
		int numInteriorRing = ptrPolygon->getNumInteriorRings();//内环数(暂时不加入考虑)
		double x, y;//存储临时的xy值
		int row_, col_;//存储临时的行列值
		for (int i = 0; i < numExterRing; i++)
		{
			if (i != 0) {
				pszAOIWKT += ",";
			}
			x = ptrOGRLinearRing->getX(i);
			y = ptrOGRLinearRing->getY(i);
			row_, col_;
			Projection2ImageRowCol(SrcGeoTransform, x, y, row_, col_);
			pszAOIWKT += (to_string(row_) + " " + to_string(col_));
		}
		cout_1++;
		if (count_ == 1)
		{
			pszAOIWKT += "))";
		}
		else if (cout_1 == count_)
		{
			pszAOIWKT += ")))";
		}
		else
		{
			pszAOIWKT += ")),((";
		}
		OGRFeature::DestroyFeature(poFeature);
		qDebug() << "line 154" << "pszDstFile" << mpcSzDstFile;
	}
	GDALClose((GDALDatasetH)ptrShape);
}
//根据传入的AOI的WKT，进行图像裁剪
int ImageCut::ImageCutting(char* pszAOIWKT) {
	// 打开原始图像并计算图像信息
	GDALDataType eDT = mppoTrSrcImage->GetRasterBand(1)->GetRasterDataType();

	int iBandCount = mppoTrSrcImage->GetRasterCount();
	int iSrcWidth = mppoTrSrcImage->GetRasterXSize();
	int iSrcHeight = mppoTrSrcImage->GetRasterYSize();

	double pSrcGeoTransform[6] = { 0 };
	double pDstGeoTransform[6] = { 0 };
	mppoTrSrcImage->GetGeoTransform(pSrcGeoTransform);			//图像的仿射变换信息
	memcpy(pDstGeoTransform, pSrcGeoTransform, sizeof(double) * 6);

	// 将传入的AOI的WKT处理为一个OGRGeometry类型，用于后续处理
	char* pszWKT = (char*)pszAOIWKT;
	OGRGeometry* pAOIGeometry = NULL;
	//cout << *pszWKT << endl;
	if (*pszWKT == 'P')
	{
		pAOIGeometry = OGRGeometryFactory::createGeometry(wkbPolygon);
	}
	else
	{
		pAOIGeometry = OGRGeometryFactory::createGeometry(wkbMultiPolygon);
	}

	pAOIGeometry->importFromWkt(&pszWKT);

	OGREnvelope eRect;
	pAOIGeometry->getEnvelope(&eRect);

	// 设置输出图像的左上角坐标
	GDALApplyGeoTransform(pSrcGeoTransform, eRect.MinX, eRect.MinY, (&pDstGeoTransform[0]), &(pDstGeoTransform[3]));
	// 根据裁切范围确定裁切后的图像宽高
	int iDstWidth = static_cast<int>(eRect.MaxX - eRect.MinX);
	int iDstHeight = static_cast<int>(eRect.MaxY - eRect.MinY);

	// 创建输出图像
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(mpcSzFormat);

	GDALDataset* pDstDS = poDriver->Create(mpcSzDstFile, iDstWidth, iDstHeight, iBandCount, eDT, NULL);

	qDebug() << "line 203" << "pszFormat:" << mpcSzFormat;
	qDebug() << "line 204" << "pszDstFile" << mpcSzDstFile;
	pDstDS->SetGeoTransform(pDstGeoTransform);
	pDstDS->SetProjection(mppoTrSrcImage->GetProjectionRef());

	// 构造坐标转换关系
	void* hTransformArg = GDALCreateGenImgProjTransformer2((GDALDatasetH)mppoTrSrcImage, (GDALDatasetH)pDstDS, NULL);
	GDALTransformerFunc pfnTransformer = GDALGenImgProjTransform;

	// 构造GDALWarp的变换选项
	GDALWarpOptions* psWO = GDALCreateWarpOptions();

	psWO->papszWarpOptions = CSLDuplicate(NULL);
	psWO->eWorkingDataType = eDT;
	psWO->eResampleAlg = GRA_NearestNeighbour;

	psWO->hSrcDS = (GDALDatasetH)mppoTrSrcImage;
	psWO->hDstDS = (GDALDatasetH)pDstDS;

	psWO->pfnTransformer = pfnTransformer;
	psWO->pTransformerArg = hTransformArg;

	psWO->nBandCount = iBandCount;
	psWO->panSrcBands = (int*)CPLMalloc(psWO->nBandCount * sizeof(int));
	psWO->panDstBands = (int*)CPLMalloc(psWO->nBandCount * sizeof(int));
	for (int i = 0; i < iBandCount; i++)
	{
		psWO->panSrcBands[i] = i + 1;
		psWO->panDstBands[i] = i + 1;
	}

	// 设置裁切AOI，AOI中的坐标必须是图像的行列号坐标，否则不能进行裁切
	psWO->hCutline = (void*)pAOIGeometry;
	// 设置上面的hCutline的值和使用下面的CUTLINE配置项的效果一样，两个选择一个即可
	psWO->papszWarpOptions = CSLSetNameValue(psWO->papszWarpOptions, "CUTLINE", pszAOIWKT);

	// 创建GDALWarp执行对象，并使用GDALWarpOptions来进行初始化
	GDALWarpOperation oWO;
	oWO.Initialize(psWO);

	// 执行处理
	oWO.ChunkAndWarpImage(0, 0, iDstWidth, iDstHeight);

	// 释放资源和关闭文件
	GDALDestroyGenImgProjTransformer(psWO->pTransformerArg);
	GDALDestroyWarpOptions(psWO);

	GDALClose((GDALDatasetH)pDstDS);

	return 1;
}
bool ImageCut::Projection2ImageRowCol(double* adfGeoTransform, double dProjX, double dProjY, int& iCol, int& iRow)
{
	try
	{
		double dTemp = adfGeoTransform[1] * adfGeoTransform[5] - adfGeoTransform[2] * adfGeoTransform[4];
		double dCol = 0.0, dRow = 0.0;
		dCol = (adfGeoTransform[5] * (dProjX - adfGeoTransform[0]) -
			adfGeoTransform[2] * (dProjY - adfGeoTransform[3])) / dTemp + 0.5;
		dRow = (adfGeoTransform[1] * (dProjY - adfGeoTransform[3]) -
			adfGeoTransform[4] * (dProjX - adfGeoTransform[0])) / dTemp + 0.5;
		iCol = static_cast<int>(dCol);
		iRow = static_cast<int>(dRow);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool ImageCut::ImageRowCol2Projection(double* adfGeoTransform, int iCol, int iRow, double& dProjX, double& dProjY)
{
	try
	{
		dProjX = adfGeoTransform[0] + adfGeoTransform[1] * iCol + adfGeoTransform[2] * iRow;
		dProjY = adfGeoTransform[3] + adfGeoTransform[4] * iCol + adfGeoTransform[5] * iRow;
		return true;
	}
	catch (...)
	{
		return false;
	}
}