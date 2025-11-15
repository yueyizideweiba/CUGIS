#ifndef WRITEVEC_H
#define WRITEVEC_H

#include "Store.h"
#include "gdal_priv.h"
/************************************************************
FileName: writeVec.h
Author: 曾亦凡
Version : 2.3
Date: 2024-08-10
Description: 矢量数据导出类，支持导出图层为shp文件
Function List:
1. writeShapefile - 导出图层为shp文件
***********************************************************/

#include "ogr_api.h"
#include"gdal.h"
#include "ogr_geometry.h"
#include "ogrsf_frmts.h"
#include "ogr_feature.h"
#include <iostream>
#include <string>

class vecWriter
{
public:
	// 写入shp文件
	void writeShapefile(std::string strFilePath, Store& store);
};

#endif

