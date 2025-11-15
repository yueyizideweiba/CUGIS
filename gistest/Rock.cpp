#include"Rock.h"
// RockMainWindow.cpp

#include <QMenuBar>
#include <QMenu>
#include <QFile>
#include <QPixmap>
#include<QGraphicsPixmapItem>
#include<QMessageBox>
#include"myDialog.h"
RockMainWindow::RockMainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	QMenuBar* menuBar = new QMenuBar(this);
	QMenu* fileMenu = menuBar->addMenu(tr("&File"));

	// 创建打开图像的动作
	openAction = new QAction(tr("&Open Image"), this);
	connect(openAction, &QAction::triggered, this, &RockMainWindow::onOpenImage);
	fileMenu->addAction(openAction);

	// 创建保存图像的动作
	saveAction = new QAction(tr("&Save Image"), this);
	connect(saveAction, &QAction::triggered, this, &RockMainWindow::onSaveImage);
	fileMenu->addAction(saveAction);

	//QMenuBar* funBar = new QMenuBar(this);
	QMenu* fileMenu1 = menuBar->addMenu(tr("&影像处理"));

	GaosiAxtion = new QAction(tr("&高斯二值化"), this);
	connect(GaosiAxtion, &QAction::triggered, this, &RockMainWindow::GaosiImage);
	fileMenu1->addAction(GaosiAxtion);

	// 创建保存图像的动作
	SelfDefineAction = new QAction(tr("&全局均值二值化"), this);
	connect(SelfDefineAction, &QAction::triggered, this, &RockMainWindow::SelfDefineImage);
	fileMenu1->addAction(SelfDefineAction);

	OtsuAxtion = new QAction(tr("&Ostus全局二值化"), this);
	connect(OtsuAxtion, &QAction::triggered, this, &RockMainWindow::OtsuImage);
	fileMenu1->addAction(OtsuAxtion);

	// 创建保存图像的动作
	BlockAction = new QAction(tr("&分块均值二值化"), this);
	connect(BlockAction, &QAction::triggered, this, &RockMainWindow::BlockImage);
	fileMenu1->addAction(BlockAction);

	BlockOtsuAction = new QAction(tr("&分块Ostus二值化"), this);
	connect(BlockOtsuAction, &QAction::triggered, this, &RockMainWindow::BlockOtsuImage);
	fileMenu1->addAction(BlockOtsuAction);

	// 创建保存图像的动作
	DetailEnhanceAction = new QAction(tr("&降噪增纹理"), this);
	connect(DetailEnhanceAction, &QAction::triggered, this, &RockMainWindow::DetailEnhanceImage);
	fileMenu1->addAction(DetailEnhanceAction);

	RockAction = new QAction(tr("&岩石纹理增强"), this);
	connect(RockAction, &QAction::triggered, this, &RockMainWindow::RockImage);
	fileMenu1->addAction(RockAction);



	// 设置菜单栏
	setMenuBar(menuBar);

	// 创建四个图形场景
	mainScene = new QGraphicsScene(this);
	auxScene1 = new QGraphicsScene(this);
	auxScene2 = new QGraphicsScene(this);
	auxScene3 = new QGraphicsScene(this);

	// 创建四个图形视图


	mainView = new QGraphicsView(mainScene, this);
	auxView1 = new QGraphicsView(auxScene1, this);
	auxView2 = new  QGraphicsView(auxScene2, this);
	auxView3 = new QGraphicsView(auxScene3, this);

	// 设置图形视图的一些属性
	mainView->setRenderHint(QPainter::Antialiasing);
	auxView1->setRenderHint(QPainter::Antialiasing);
	auxView2->setRenderHint(QPainter::Antialiasing);
	auxView3->setRenderHint(QPainter::Antialiasing);

	// 创建一个水平分割器，用于三个辅助视图
	splitterAux = new QSplitter(Qt::Vertical, this);
	splitterAux->addWidget(auxView1);
	splitterAux->addWidget(auxView2);
	splitterAux->addWidget(auxView3);
	splitterAux->setFixedWidth(150);  // 设置固定宽度为 200 像素

	// 设置分隔条的宽度
	splitterAux->setHandleWidth(10);  // 设置分隔条宽度为 10 像素

	// 设置分隔器的尺寸策略
	splitterAux->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	// 创建图层管理的 Dock Widget
	layerDock = new QDockWidget("图层管理", this);
	layerDock->setWindowIcon(QIcon("./icon/layerman.png"));
	layerTreeWidget = new QTreeWidget(layerDock);
	layerTreeWidget->setHeaderLabels(QStringList() << "图层");
	layerDock->setWidget(layerTreeWidget);
	addDockWidget(Qt::LeftDockWidgetArea, layerDock);

	// 创建一个垂直布局，用于放置主视图和辅助视图
	QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->addWidget(mainView);
	mainLayout->addWidget(splitterAux);

	// 创建一个中心小部件，并设置布局
	QWidget* centralWidget = new QWidget(this);
	centralWidget->setLayout(mainLayout);

	// 设置中央小部件
	setCentralWidget(centralWidget);

	// 设置窗口标题
	setWindowTitle("Rock Recognition Window");
}
void RockMainWindow::GaosiImage()
{
	myDialog* dialog = new myDialog(this);
	dialog->setWindowTitle("高斯二值化");
	dialog->label1->setText("请输入高斯核大小");
	dialog->label2->setText("请输入高斯阈值");

	dialog->exec();
	if (dialog->result() == QDialog::Accepted) {
		mainmap = adaptiveGaussianThresholding(mainmap, dialog->FristBotton, dialog->SecondBotton);
	}
	mainScene->clear();
	mainScene->addPixmap(mainmap);
};

void RockMainWindow::SelfDefineImage()
{
	myDialog* dialog = new myDialog(this);
	dialog->setWindowTitle("全局均值二值化");
	dialog->label1->setText("不需要输入任何内容");
	dialog->label2->setText("不需要输入任何内容");
	mainmap = adaptiveMeanThresholding(mainmap, 1);
	dialog->exec();
	mainScene->clear();
	mainScene->addPixmap(mainmap);
};


void RockMainWindow::BlockImage()
{
	myDialog* dialog = new myDialog(this);
	dialog->setWindowTitle("局域均值二值化");
	dialog->label1->setText("请输入分块数量");
	dialog->label2->setText("不需要输入任何内容");
	dialog->exec();
	if (dialog->result() == QDialog::Accepted) {
		mainmap = adaptiveMeanThresholding(mainmap, dialog->FristBotton);
	};
	mainScene->clear();
	mainScene->addPixmap(mainmap);
};

void RockMainWindow::BlockOtsuImage()
{
	myDialog* dialog = new myDialog(this);
	dialog->setWindowTitle("局域OSTU二值化");
	dialog->label1->setText("请输入分块数量");
	dialog->label2->setText("不需要输入任何内容");
	dialog->exec();
	if (dialog->result() == QDialog::Accepted) {
		mainmap = adaptiveOtsuThresholding(mainmap, dialog->FristBotton);
	};
	mainScene->clear();
	mainScene->addPixmap(mainmap);
};

void RockMainWindow::DetailEnhanceImage()
{
	myDialog* dialog = new myDialog(this);
	dialog->setWindowTitle("细节增强");
	dialog->label1->setText("请输入增强核半径,若降噪，请填1");
	dialog->label2->setText("请输入可支配填充数，若降噪，请填-1");
	dialog->exec();
	if (dialog->result() == QDialog::Accepted) {
		mainmap = denoisePixmap(mainmap, dialog->FristBotton, dialog->SecondBotton);
	};
	mainScene->clear();
	mainScene->addPixmap(mainmap);
};


void  RockMainWindow::OtsuImage()
{
	myDialog* dialog = new myDialog(this);
	dialog->setWindowTitle("Ostus");
	dialog->label1->setText("您不需要输入任何内容！");
	dialog->label2->setText("您不需要输入任何内容！");
	dialog->exec();
	if (dialog->result() == QDialog::Accepted) {
		mainmap = adaptiveOtsuThresholding(mainmap, 1);
	};
	mainScene->clear();
	mainScene->addPixmap(mainmap);
};

void RockMainWindow::RockImage()
{
	myDialog* dialog = new myDialog(this);
	dialog->setWindowTitle("岩石纹理增强");
	dialog->label1->setText("您不需要输入任何内容！");
	dialog->label2->setText("您不需要输入任何内容！");
	dialog->exec();
	if (dialog->result() == QDialog::Accepted) {
		QPixmap newMap = RockMainWindow::adaptiveGaussianThresholding(mainmap, 11, 2);
		QPixmap newMap1 = denoisePixmap(newMap, 1, -1);
		QPixmap newMap2 = denoisePixmap(newMap, 5, 3);
		mainmap = newMap2;
	};
	mainScene->clear();
	mainScene->addPixmap(mainmap);
};

RockMainWindow::~RockMainWindow()
{
	delete mainScene;
	delete auxScene1;
	delete auxScene2;
	delete auxScene3;
	delete mainView;
	delete auxView1;
	delete auxView2;
	delete auxView3;
	delete layerTreeWidget;
}

void RockMainWindow::onOpenImage()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Images (*.png *.jpg *.bmp)"));
	if (!fileName.isEmpty()) {
		QPixmap pixmap(fileName);
		mainmap = pixmap;
		mainScene->clear();
		mainScene->addPixmap(pixmap);
		drawHueHistogram(auxScene1, pixmap);
		drawGrayscaleHistogram(auxScene2, pixmap);
		// 获取图像的宽度和高度
	//QPixmap newMap = 	RockMainWindow::adaptiveGaussianThresholding(pixmap, 11, 2);
	//QPixmap newMap1 = denoisePixmap(newMap,1,-1);
	//QPixmap newMap2 = denoisePixmap(newMap, 5, 3);

		mainScene->clear();
		//mainScene->addPixmap(newMap);
		int width = pixmap.width();
		int height = pixmap.height();
		int value = std::max(width, height);
		//QPixmap NewIMap = adaptiveMeanThresholding(pixmap, value /10);
		mainScene->addPixmap(pixmap);

	};
};

void RockMainWindow::drawHueHistogram(QGraphicsScene* scene, QPixmap& pixmap)
{
	int width = pixmap.width();
	int height = pixmap.height();

	// 创建直方图数据结构
	QVector<int> histogram(720, 0);  // 720个单位，每0.5度一个单位

	// 遍历每个像素点并计算色相值
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			QRgb pixel = pixmap.toImage().pixel(x, y);
			// 使用 QColor 的 hue 方法来获取色相值
			QColor color(pixel);
			float hue = color.hue();  // 色相值，范围 [0, 360)

			// 映射到 0 到 720 的范围内，每 0.5 度一个单位
			int hueIndex = static_cast<int>(hue * 2);  // 乘以2是因为我们想要每0.5度一个单位
			histogram[hueIndex]++;  // 如果超出范围，它会自动被截断
		}
	}

	// 下采样至180个单位
	QVector<int> downSampledHistogram(180, 0);
	for (int i = 0; i < 180; ++i) {
		for (int j = 0; j < 4; ++j) {  // 每4个单位合并为1个单位
			downSampledHistogram[i] += histogram[i * 4 + j];
		}
	}

	// 清除旧的直方图
	auxScene1->clear();

	int barWidth = 4;  // 直方图柱的宽度
	int barSpacing = 0;  // 直方图柱之间的间距
	int barHeightScale = 1;  // 缩放因子，根据需要调整
	int maxCount = *std::max_element(downSampledHistogram.begin(), downSampledHistogram.end());  // 找到最大计数值

	// 设置场景的大小
	auxScene1->setSceneRect(0, 0, downSampledHistogram.size() * (barWidth + barSpacing), maxCount * barHeightScale);

	// 绘制直方图柱
	for (int i = 0; i < downSampledHistogram.size(); ++i) {
		int count = sqrt(downSampledHistogram[i]);
		//double sqrtCount = sqrt(count);
		QColor color = QColor::fromHsv(0, 255, 255);
		if (i != 0)
		{
			color = QColor::fromHsv(i * 2 - 1, 255, 255);  // 使用HSV颜色模型
		}

		QRectF rect(i * (barWidth + barSpacing), auxScene1->height() - count * barHeightScale / 2, barWidth, count * barHeightScale);
		auxScene1->addRect(rect, Qt::NoPen, QBrush(color));  // 添加直方图柱
	}

	// 添加色标
	int colorBarHeight = 20;
	int colorBarWidth = 20;
	int colorBarStartY = auxScene1->height() + 10;  // 在直方图下面开始绘制
	for (int i = 0; i < 180; i += 30) {
		QColor color = QColor::fromHsv(i, 255, 255);  // 使用HSV颜色模型
		QRectF colorBar(i * (barWidth + barSpacing), colorBarStartY, colorBarWidth, colorBarHeight);
		//	auxScene1->addRect(colorBar, Qt::NoPen, QBrush(color));
	}

	auxView1->setScene(auxScene1);

	// 设置辅助视图的缩放比例
	//auxView1->fitInView(auxScene1->sceneRect(), Qt::KeepAspectRatio);
};
void RockMainWindow::drawGrayscaleHistogram(QGraphicsScene* scene, QPixmap& pixmap)
{
	// 获取图像的宽度和高度
	int width = pixmap.width();
	int height = pixmap.height();

	// 创建直方图数据结构
	QVector<int> histogram(256, 0);  // 256个单位，代表0-255的灰度级

	// 遍历每个像素点并计算灰度值
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			QRgb pixel = pixmap.toImage().pixel(x, y);
			// 计算灰度值
			int grayValue = qGray(pixel);
			histogram[grayValue]++;  // 增加该灰度值的计数
		}
	}

	// 清除旧的直方图
	scene->clear();

	int barWidth = 2;  // 直方图柱的宽度
	int barSpacing = 0;  // 直方图柱之间的间距
	int barHeightScale = 1;  // 缩放因子，根据需要调整
	int maxCount = *std::max_element(histogram.begin(), histogram.end());  // 找到最大计数值

	// 设置场景的大小
	scene->setSceneRect(0, 0, histogram.size() * (barWidth + barSpacing), maxCount * barHeightScale);

	// 绘制直方图柱
	for (int i = 0; i < histogram.size(); ++i) {
		int count = histogram[i];
		double sqrtCount = sqrt(count);

		// 定义颜色（通常使用灰色）
		QColor color = Qt::gray;

		QRectF rect(i * (barWidth + barSpacing), scene->height() - sqrtCount * barHeightScale, barWidth, sqrtCount * barHeightScale);
		scene->addRect(rect, Qt::NoPen, QBrush(color));  // 添加直方图柱
	}
}
QPixmap RockMainWindow::adaptiveGaussianThresholding(QPixmap& pixmap, int blockSize, int C)
{
	// 获取原始图像的宽度和高度
	int width = pixmap.width();
	int height = pixmap.height();

	// 创建一个新的图像用于保存二值化后的结果
	QImage binaryImage(width, height, QImage::Format_Grayscale8);

	// 计算高斯核的半径
	int radius = blockSize / 2;

	// 预计算高斯核
	std::vector<float> gaussianKernel(blockSize * blockSize);
	float sigma = 1.0f;  // 高斯核的标准差
	float kernelSum = 0.0f;

	for (int dy = -radius; dy <= radius; dy++) {
		for (int dx = -radius; dx <= radius; dx++) {
			// 高斯函数计算权重
			float weight = exp(-((dx * dx + dy * dy) / (2.0f * sigma * sigma)));
			gaussianKernel[(dy + radius) * blockSize + (dx + radius)] = weight;
			kernelSum += weight;
		}
	}

	// 归一化高斯核
	for (auto& weight : gaussianKernel) {
		weight /= kernelSum;
	}
	//#pragma omp parallel for
		// 遍历每个像素
	for (int y = radius; y < height - radius; y++) {
		for (int x = radius; x < width - radius; x++) {
			// 计算局部加权均值
			float localMean = 0.0f;

			for (int dy = -radius; dy <= radius; dy++) {
				for (int dx = -radius; dx <= radius; dx++) {
					// 获取当前像素的灰度值
					QRgb pixel = pixmap.toImage().pixel(x + dx, y + dy);
					int grayValue = qGray(pixel);
					// 应用高斯权重
					localMean += grayValue * gaussianKernel[(dy + radius) * blockSize + (dx + radius)];
				}
			}

			// 计算局部阈值
			int localThreshold = static_cast<int>(localMean - C);

			// 获取当前像素的灰度值
			QRgb pixel = pixmap.toImage().pixel(x, y);
			int grayValue = qGray(pixel);

			// 根据局部阈值进行二值化
			int binaryValue = (grayValue > localThreshold) ? 255 : 0;

			// 将二值化后的像素值写入新图像
			binaryImage.setPixel(x, y, qRgb(binaryValue, binaryValue, binaryValue));
		}
	}

	// 返回二值化后的 QPixmap
	return QPixmap::fromImage(binaryImage);
}

int otsuThreshold(const QImage& region)
{
	int hist[256] = { 0 };
	int totalPixels = region.width() * region.height();

	// 计算灰度直方图
	for (int y = 0; y < region.height(); ++y) {
		for (int x = 0; x < region.width(); ++x) {
			int grayValue = qGray(region.pixel(x, y));
			hist[grayValue]++;
		}
	}

	// 初始化变量
	double sum = 0;
	for (int i = 0; i < 256; ++i) {
		sum += i * hist[i];
	}

	double sumB = 0;
	int wB = 0;
	int wF = 0;
	double varMax = 0.0;
	int threshold = 0;

	for (int i = 0; i < 256; ++i) {
		wB += hist[i];  // Weight Background
		if (wB == 0) continue;
		wF = totalPixels - wB;  // Weight Foreground
		if (wF == 0) break;

		sumB += (double)i * hist[i];

		double mB = sumB / wB;  // Mean Background
		double mF = (sum - sumB) / wF;  // Mean Foreground

		// Calculate Between-Class Variance
		double varBetween = wB * wF * (mB - mF) * (mB - mF);

		// Check if new maximum found
		if (varBetween > varMax) {
			varMax = varBetween;
			threshold = i;
		}
	}

	return threshold - 10;
}

QPixmap RockMainWindow::adaptiveOtsuThresholding(QPixmap& pixmap, int blockSize)
{
	// 获取原始图像的宽度和高度
	int width = pixmap.width();
	int height = pixmap.height();

	// 创建一个新的图像用于保存二值化后的结果
	QImage binaryImage(width, height, QImage::Format_Grayscale8);

	// 指定每个块的尺寸
	int blockWidth = width / blockSize;
	int blockHeight = height / blockSize;

	// 分割图像
	for (int y = 0; y < height; y += blockHeight) {
		for (int x = 0; x < width; x += blockWidth) {
			// 确定当前块的边界
			int endX = std::min(x + blockWidth, width);
			int endY = std::min(y + blockHeight, height);

			// 提取当前块
			QImage region = pixmap.toImage().copy(x, y, endX - x, endY - y);

			// 计算 Otsu 阈值
			int threshold = otsuThreshold(region);

			// 对当前块进行二值化
			for (int row = y; row < endY; ++row) {
				for (int col = x; col < endX; ++col) {
					QRgb pixel = pixmap.toImage().pixel(col, row);
					int grayValue = qGray(pixel);
					int binaryValue = (grayValue > threshold) ? 255 : 0;
					binaryImage.setPixel(col, row, qRgb(binaryValue, binaryValue, binaryValue));
				}
			}
		}
	}

	// 返回二值化后的 QPixmap
	return QPixmap::fromImage(binaryImage);
};

int computeMean(const QImage& region)
{
	int sum = 0;
	int totalPixels = region.width() * region.height();

	// 计算灰度总和
	for (int y = 0; y < region.height(); ++y) {
		for (int x = 0; x < region.width(); ++x) {
			int grayValue = qGray(region.pixel(x, y));
			sum += grayValue;
		}
	}

	// 计算平均灰度值
	return sum / totalPixels;
}
QPixmap RockMainWindow::adaptiveMeanThresholding(QPixmap& pixmap, int blockSize)
{
	// 获取原始图像的宽度和高度
	int width = pixmap.width();
	int height = pixmap.height();

	// 创建一个新的图像用于保存二值化后的结果
	QImage binaryImage(width, height, QImage::Format_Grayscale8);

	// 指定每个块的尺寸
	int blockWidth = width / blockSize;
	int blockHeight = height / blockSize;

	// 分割图像
	for (int y = 0; y < height; y += blockHeight) {
		for (int x = 0; x < width; x += blockWidth) {
			// 确定当前块的边界
			int endX = std::min(x + blockWidth, width);
			int endY = std::min(y + blockHeight, height);

			// 提取当前块
			QImage region = pixmap.toImage().copy(x, y, endX - x, endY - y);

			// 计算平均灰度值
			int mean = computeMean(region);

			// 对当前块进行二值化
			for (int row = y; row < endY; ++row) {
				for (int col = x; col < endX; ++col) {
					QRgb pixel = pixmap.toImage().pixel(col, row);
					int grayValue = qGray(pixel);
					int binaryValue = (grayValue > mean) ? 255 : 0;
					binaryImage.setPixel(col, row, qRgb(binaryValue, binaryValue, binaryValue));
				}
			}
		}
	}

	// 返回二值化后的 QPixmap
	return QPixmap::fromImage(binaryImage);
};
//降噪增强轮廓
QPixmap RockMainWindow::denoisePixmap(QPixmap& inputPixmap, int blockSize, int nums)
{
	// 获取输入的QPixmap并转换为QImage以便处理
	QImage inputImage = inputPixmap.toImage();
	int width = inputImage.width();
	int height = inputImage.height();

	// 创建一个空白的QImage用于输出
	QImage outputImage(width, height, QImage::Format_ARGB32);
	outputImage.fill(Qt::white);  // 初始化为纯白色

	// 定义5x5的结构元素
	const int structElementSize = blockSize;
	const int halfStructElement = structElementSize / 2;

	// 遍历输入图像的每个像素
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			QRgb pixel = inputImage.pixel(x, y);

			// 如果当前像素是黑色（或指定的二值化值）
			if (qAlpha(pixel) == 0 || qRed(pixel) == 0)
			{
				// 计算5x5邻域的边界
				int startX = std::max(x - halfStructElement, 0);
				int startY = std::max(y - halfStructElement, 0);
				int endX = std::min(x + halfStructElement, width - 1);
				int endY = std::min(y + halfStructElement, height - 1);

				bool hasBlackPixel = false;
				int tag = nums;
				// 检查5x5邻域内是否有黑色像素
				for (int j = startY; j <= endY; ++j)
				{
					for (int i = startX; i <= endX; ++i)
					{
						QRgb neighbor = inputImage.pixel(i, j);
						if (tag >= 0)
						{
							if (qAlpha(neighbor) == 0 || qRed(neighbor) == 0)
							{
								hasBlackPixel = true;
								for (int i1 = x; abs(i1 - i) != 0; i1 += (i - i1) / abs(i1 - i))
								{
									outputImage.setPixel(i1, j, qRgb(0, 0, 0));
									tag -= abs(i1 - i);
								};
								for (int j1 = y; abs(j1 - j) != 0; j1 += (j - j1) / abs(j - j1))
								{
									outputImage.setPixel(j1, j, qRgb(0, 0, 0));
									tag -= abs(j1 - j);
								}
							}
						};
					}
				}

				// 如果周围没有任何黑色像素，则将当前像素设为白色
				if (!hasBlackPixel)
				{
					outputImage.setPixel(x, y, qRgba(255, 255, 255, 255));
				}
				else
				{
					// 如果周围有黑色像素，则保持原样
					outputImage.setPixel(x, y, pixel);
				}
			}
			else
			{
				// 如果当前像素不是黑色，则直接复制到输出图像
				outputImage.setPixel(x, y, pixel);
			}
		}
	}

	// 将处理后的QImage转换回QPixmap
	return QPixmap::fromImage(outputImage);
}

void RockMainWindow::onSaveImage()
{
	// 弹出保存对话框
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("Images (*.png *.jpg *.bmp)"));
	if (!fileName.isEmpty())
	{
		// 获取当前显示在 mainScene 中的 QPixmap
		QList<QGraphicsItem*> items = mainScene->items();
		if (!items.isEmpty())
		{
			QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(items.first());
			if (pixmapItem)
			{
				// 保存当前的 QPixmap
				if (!pixmapItem->pixmap().save(fileName))
				{
					QMessageBox::critical(this, tr("Error"), tr("Failed to save the image."));
				}
			}
			else
			{
				QMessageBox::warning(this, tr("Warning"), tr("No image to save."));
			}
		}
		else
		{
			QMessageBox::warning(this, tr("Warning"), tr("No image to save."));
		}
	}
}