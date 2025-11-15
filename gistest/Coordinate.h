#ifndef COORDINATE_H
#define COORDINATE_H

#include<gdal.h>
#include<gdal_priv.h>
#include<vector>
#include<Qstring>//qt环境
#include<QFileDialog.h>
#include<QDebug>
#include <cstring>
#include <iostream>
#include <cpl_conv.h>
#include<QGraphicsPixmapItem>
#include<QTimer>
#include<QCoreApplication>
#include <omp.h>
#include <chrono>
#include <emmintrin.h> // 包含 SSE2 头文件
#include<utility>
//enum { MINMUM = 0, MAXIMUM = 1, AVERAGE = 2, NOT_EXIST = -100 };
class Coordinate_System
{
public:
	double mdminLon;
	double mdminLat;
	double  mdScale;
	double  md0Lat;
	double  md0Lon;
	double md0X;
	double md0Y;
	int mnWidth;
	int mnHeight;

	std::pair<double, double> TransLateToX_Y(double _dlon, double _dlat);//经纬度转像素坐标

	std::pair<double, double> TransLateToLon_Lat(double _dx, double _dy);//像素坐标转经纬度

	void update(double _dlon, double _dlat, double _dminLon, double _dminLat, int width, int height);//更新坐标

	std::pair<int, int> GetNumsOfBand(double _dx, double _dy);//获取像素坐标对应的行列号

	std::pair<double, double>GetLonLatByPixel(int nums);//获取像素坐标对应的经纬度

	Coordinate_System(double _dminLon, double _dminLat, double _dlon, double _dlat, int width, int height);

	Coordinate_System() {};
};

#endif // !