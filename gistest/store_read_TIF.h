#pragma warning(disable : 4996)
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS

#include<gdal.h>
#include<gdal_priv.h>
#include<vector>
#include<Qstring>//qt环境
#include<QFileDialog.h>
#include<Qstring>
#include<QDebug>
#include <cstring>
#include <iostream>
#include <cpl_conv.h>
#include "myGraphicsView.h"
#include<QGraphicsPixmapItem>
#include<QTimer>
#include<QCoreApplication>
#include <omp.h>
#include <chrono>
#include <emmintrin.h> // 包含 SSE2 头文件
#include<utility>
#include"Coordinate.h"
enum { MINMUM = 0, MAXIMUM = 1, AVERAGE = 2, NOT_EXIST = -100 };
#define ALIGN16 __attribute__((aligned(16)))
enum D8_Directions {
	D8_N = 1,     // 0° - 北
	D8_NE = 2,    // 45° - 东北
	D8_E = 4,     // 90° - 东
	D8_SE = 8,    // 135° - 东南
	D8_S = 16,    // 180° - 南
	D8_SW = 32,   // 225° - 西南
	D8_W = 64,    // 270° - 西
	D8_NW = 128   // 315° - 西北
};
// 前置声明
//必须要写，不知道为什么，但是不写的话，编译器会报错
class myGraphicsView;

//template<class T>
class gdal_engine
{
public:
	char* mpcQPath = nullptr;//栅格路径
	int mnNums_Band;//该栅格路径下的波段数
	int mnwidth;//该栅格的宽度
	int mnheight;//该栅格的高度
	char* mpcTypeofBand = nullptr;//后续使用模板编程进行处理，前期使用int作为类型进行顶替
	std::vector< GDALRasterBand*> mpvBand;//各个波段的指针
	int mpnBandsChosen[3];//选择波段
	float mdSCale;
	float  mfX;
	float mfY;
	Coordinate_System mCoordinate;//坐标系统
	double mdViewWidth;
	double mdViewHeight;
	double mdGlobalMinLon;
	double mdGlobalMaxLon;
	double mdGlobalMinLat;
	double mdGlobalMaxLat;

	std::vector<float> mvGetposition();
	//选择波段
	bool BandChose(int nR = 1, int nG = 1, int nB = 1);
	//转化QString为char*
	void my_strcpy(char* dest, const char* src);

	//内部更新tif信息
	void update_info(QString Path);
	//构造与析构函数
	void update_coordinate();

	gdal_engine(QString Path, double mdWidth, double mdHeight, double GlobalMinLon, double GlobalMaxLon, double GlobalMinLat, double GlobalMaxLat);

	gdal_engine() {  };

	~gdal_engine() { qDebug() << "正在析构"; delete[] mpcQPath; delete[] mpcTypeofBand; qDebug() << "析构完成"; };

	void setCoordinate(double dScale, float fy, float fx) { mdSCale = dScale; mfX = fx; mfY = fy; };//设置坐标

	bool readTIFOneBand(int Band, std::vector<unsigned char>& tifStore);//读取一个波段

	void tiffShow(myGraphicsView* view, QGraphicsScene* sencor);//显示

	//波段快速显示
	void quickTifShow(myGraphicsView* view, QGraphicsScene* scene, const float FSCALE, bool multiband, int tileSize = 1000);//快速显示

	void  saveTif(QString path);//保存tif,没有最终实现。

	void tifValueProcess(myGraphicsView* view, QGraphicsScene* scene, int band, int TyPe, int xoff = 0, int yoff = 0);//波段值处理

	double getMinX() const;

	double getMaxX() const;

	double getMinY() const;

	double getMaxY() const;

	void fillHoles(myGraphicsView* view);

	// 计算流向的函数
	int CalculateD8FlowDirection(int x, int y, const std::vector<float>& elevation, int width, int height);

	//D8算法
	void D8FlowDirectionAnalysis(myGraphicsView* view);

	//计算坡向
	float CalculateAspect(int x, int y, const std::vector<float>& elevation, int width, int height, float dx, float dy);

	//计算坡向
	void AspectAnalysis(myGraphicsView* view);

	//计算坡度
	float CalculateSlope(int x, int y, const std::vector<float>& elevation, int width, int height, float dx, float dy);

	//计算坡度
	void SlopeAnalysis(myGraphicsView* view, float dX, float dY);
};

#endif