/************************************************************
FileName: OverlayAnalysis.h
Author: 谢宇瀚
Version : 1.9
Date: 2024-07-28
Description: 叠加分析类，用于执行要素的叠加分析操作
Function List:
1. performOverlay - 执行叠加分析，根据指定的操作类型对两个输入图层进行叠加操作
2. createGeometryFromStore - 从Store对象创建GEOS几何对象
3. createStoreFromGeometry - 从GEOS几何对象创建Store对象
***********************************************************/

#ifndef OVERLAYANALYSIS_H
#define OVERLAYANALYSIS_H

#include "Store.h"
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/overlay/OverlayNodeFactory.h>

class OverlayAnalysis {
public:
	enum OperationType {
		Difference,  // 交集取反
		SymDifference, // 擦除
		Union,      // 更新
		Intersection, // 相交
	};

	// 执行叠加分析，根据指定的操作类型对两个输入图层进行叠加操作
	static Store performOverlay(const Store& input1, const Store& input2, OperationType operationType);

private:
	static std::unique_ptr<geos::geom::Geometry> createGeometryFromStore(const Store& store); // 从Store对象创建GEOS几何对象
	static Store createStoreFromGeometry(const geos::geom::Geometry* geometry); // 从GEOS几何对象创建Store对象
};

#endif
