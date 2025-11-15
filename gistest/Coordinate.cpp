#include "Coordinate.h"

std::pair<double, double>Coordinate_System::TransLateToX_Y(double _dlon, double _dlat)//地理坐标转投影坐标
{
	const double R_MAJOR = 6378137.0;
	const double R_MINOR = 6356752.3142;
	double x = R_MAJOR * qDegreesToRadians(_dlon);
	double y = R_MAJOR * log(tan(M_PI_4 + qDegreesToRadians(_dlat) / 2.0));
	std::pair<double, double> xy(x, y);
	return xy;
};

std::pair<double, double>Coordinate_System::TransLateToLon_Lat(double _dx, double _dy)//坐标转化
{
	double R_MAJOR = 6378137.0;
	double R_MINOR = 6356752.3142;
	double lon = qRadiansToDegrees(_dx / R_MAJOR);
	double lat = qRadiansToDegrees(atan(sinh(_dy / R_MAJOR)));
	std::pair<double, double> lonlat(lon, lat);
	return lonlat;
};

void Coordinate_System::update(double _dlon, double _dlat, double _dminLon, double _dminLat, int width, int height)//函数参数更新
{
	mdScale = _dminLon;
	qDebug() << "scale:" << mdScale;
	mdminLat = _dminLat;
	mdminLon = _dminLon;
	md0X = _dlon;
	md0Y = _dlat;
	qDebug() << "md0X:" << md0X << "md0Y:" << md0Y;
	md0Lat = _dlat;
	md0Lon = _dlon;
	mnWidth = width;
	mnHeight = height;
};

std::pair<int, int> Coordinate_System::GetNumsOfBand(double _dx, double _dy)//获取像素坐标
{
	int gridX = static_cast<int>((_dx - md0X) / mdScale);
	int gridY = static_cast<int>((abs(_dy - md0Y)) / mdScale);
	return std::make_pair(gridX, gridY);
};

Coordinate_System::Coordinate_System(double _dminLon, double _dminLat, double _dlon, double _dlat, int width, int height)//构造函数
{
	update(_dlon, _dlat, _dminLon, _dminLat, width, height);
};

std::pair<double, double>Coordinate_System::GetLonLatByPixel(int nums)//获取像素坐标对应的经纬度
{
	int y = nums / mnWidth;
	int x = nums % mnWidth;
	double lon = md0Lon + mdScale * x;
	double lat = md0Lat - mdScale * y;
	return std::make_pair(lon, lat);
};