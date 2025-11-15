#ifndef IMAGECUT_H
#define IMAGECUT_H

#include "gdal_priv.h"
#include <iostream>
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"
#include "gdalwarper.h"

using namespace std;

class ImageCut {
public:
	ImageCut();
	~ImageCut();
	/*设置影像输入路径*/
	void setSrcImageFilename(char* pszSrcFile);
	/*设置裁剪后影像输出的路径*/
	void setDstImageFilename(char* pszDstFile);
	/*设置shape文件的路径*/
	void setShapeFilename(char* pszShapeName);
	/*设置输出影像的格式*/
	void setImageFormat(char* pszFormat);
	/*开始影像AOI裁剪*/
	int ImageCutByAOI();
private:
	char* mpcSzSrcFile = nullptr;
	char* mpcSzDstFile = nullptr;
	char* mpcSzShapeName = nullptr;
	char* mpcSzFormat = nullptr;
	/*输入影像的指针*/
	GDALDataset* mppoTrSrcImage;
	/*投影坐标转行列号*/
	bool Projection2ImageRowCol(double* adfGeoTransform, double dProjX, double dProjY, int& iCol, int& iRow);
	/*行列号转投影坐标*/
	bool ImageRowCol2Projection(double* adfGeoTransform, int iCol, int iRow, double& dProjX, double& dProjY);
	/*根据shape文件和输入的影像输出多边形WKT文件*/
	int outputPolygonWkt(string& pszAOIWKT);
	/*影像裁剪的代码*/
	int ImageCutting(char* pszAOIWKT);

};

#endif
