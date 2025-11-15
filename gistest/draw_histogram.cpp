#include "draw_histogram.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QDialog>
#include <vector>
#include <algorithm>
#include <iostream>

//画直方图
void displayImage(const QImage& qImg, const QString& title) {
	
	//创建一个QGraphicsScene并添加QImage
	QGraphicsScene* scene = new QGraphicsScene();
	scene->addPixmap(QPixmap::fromImage(qImg));

	//创建一个QGraphicsView，设置场景，并在对话框中显示
	QGraphicsView* view = new QGraphicsView(scene);
	QDialog* dialog = new QDialog();
	dialog->setWindowTitle(title);
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(view);
	dialog->setLayout(layout);
	dialog->exec();
	delete scene;
	delete view;
}
//计算直方图
std::vector<GUIntBig> computeHistogram(const std::vector<unsigned char>& imageData, int nBuckets) {
	std::vector<GUIntBig> histogram(nBuckets, 0);
	for (unsigned char value : imageData) {
		histogram[value]++;
	}
	return histogram;
}
//直方图均衡化
std::vector<unsigned char> equalizeHistogram(const std::vector<unsigned char>& imageData, const std::vector<GUIntBig>& histogram) {
	int totalPixels = imageData.size();
	std::vector<float> cdf(histogram.size(), 0.0f); // 累积分布函数

	// 计算累积分布函数（CDF）
	cdf[0] = static_cast<float>(histogram[0]) / totalPixels;
	for (size_t i = 1; i < histogram.size(); ++i) {
		cdf[i] = cdf[i - 1] + static_cast<float>(histogram[i]) / totalPixels;
	}

	// 创建映射表
	std::vector<unsigned char> lookupTable(256);
	for (size_t i = 0; i < cdf.size(); ++i) {
		lookupTable[i] = static_cast<unsigned char>(255 * cdf[i]);
	}

	// 应用映射表进行直方图均衡化
	std::vector<unsigned char> equalizedImageData(imageData.size());
	for (size_t i = 0; i < imageData.size(); ++i) {
		equalizedImageData[i] = lookupTable[imageData[i]];
	}
	return equalizedImageData;
}