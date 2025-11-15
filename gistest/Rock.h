#pragma once
#pragma once
// RockMainWindow.h
#ifndef ROCKMAINWINDOW_H
#define ROCKMAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QDockWidget>

class RockMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit RockMainWindow(QWidget* parent = nullptr);
	~RockMainWindow();

private slots:
	void onOpenImage();
	void onSaveImage();

private:
	QAction* openAction;
	QAction* saveAction;

	QAction* GaosiAxtion;
	QAction* OtsuAxtion;
	QAction* SelfDefineAction;
	QAction* BlockAction;
	QAction* BlockOtsuAction;
	QAction* RockAction;
	QAction* DetailEnhanceAction;


	QGraphicsScene* mainScene;
	QGraphicsScene* auxScene1, * auxScene2, * auxScene3;
	QGraphicsView* mainView, * auxView1, * auxView2, * auxView3;
	QSplitter* splitterMain;
	QSplitter* splitterAux;
	QDockWidget* layerDock;
	QTreeWidget* layerTreeWidget;
	QPixmap  mainmap;

	void drawHueHistogram(QGraphicsScene* scene, QPixmap& pixmap);
	void drawGrayscaleHistogram(QGraphicsScene* scene, QPixmap& pixmap);
	void GaosiImage();
	void SelfDefineImage();
	void OtsuImage();
	void BlockImage();
	void BlockOtsuImage();
	void DetailEnhanceImage();
	void RockImage();


	QPixmap adaptiveGaussianThresholding(QPixmap& pixmap, int blockSize, int C);
	QPixmap adaptiveOtsuThresholding(QPixmap& pixmap, int blockSize);
	QPixmap adaptiveMeanThresholding(QPixmap& pixmap, int blockSize);
	QPixmap denoisePixmap(QPixmap& inputPixmap, int blockSize, int nums);
};

#endif // ROCKMAINWINDOW_H