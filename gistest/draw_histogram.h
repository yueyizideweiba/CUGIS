#ifndef DRAW_HISTOGRAM_H
#define DRAW_HISTOGRAM_H

#include "store_read_TIF.h"
#include <QImage>
#include <QString>
#include <vector>

void displayImage(const QImage& qImg, const QString& title);//显示图片
std::vector<GUIntBig> computeHistogram(const std::vector<unsigned char>& imageData, int nBuckets);//计算直方图
std::vector<unsigned char> equalizeHistogram(const std::vector<unsigned char>& imageData, const std::vector<GUIntBig>& histogram);//直方图均衡化

#endif