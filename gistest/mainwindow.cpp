#include "MainWindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTreeWidget>
#include <QListWidget>
#include <QComboBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFile>
#include <QTextStream>
#include <errno.h>
#include <QSqlError>
#include "editSave.h"
#include "editCancel.h"
#include "store_read_TIF.h"
#include "writeVec.h"
#include "Store.h"
#include "myGraphicsView.h"
#include "GeometryRelation.h"
#include "draw_histogram.h"
#include "TIFshowDialog.h"
#include "ConvexHullCalculator.h"
#include "BufferAnalyzer.h"
#include "OverlayAnalysis.h"
#include "GISlog.h"
#include "coverFun.h"
#include "ImageCut.h"
#include "themeSwitch.h"
#include"Rock.h"
#include <string>
#include <algorithm>
#include <windows.h>
#include <cmath>

// *****************************************初始化部分******************************************

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent),
	mpctrlScene(new QGraphicsScene(this)),
	mpctrlView(new myGraphicsView(mpctrlScene, this)),
	mStore(),
	mdMinLon(0.0),
	mdMaxLon(0.0),
	mdMinLat(0.0),
	mdMaxLat(0.0),
	mnViewWidth(1600),
	mnViewHeight(1200),
	mDraw(),
	mstrCurrentEditingLayer("")
{
	// 主窗口标题和尺寸
	setWindowTitle("CUGIS");
	resize(1200, 800);
	mSelectedStartNode = -1;
	mSelectedEndNode = -1;

	// 初始化主菜单栏
	QMenuBar* menuBar = new QMenuBar(this);
	QMenu* fileMenu = menuBar->addMenu("文件操作");
	fileMenu->addAction(QIcon("./icon/folder.png"), "打开工程文件", this, &MainWindow::openProjectFile);
	fileMenu->addAction(QIcon("./icon/layerman.png"), "添加矢量图层", this, &MainWindow::openVectorData);
	fileMenu->addAction(QIcon("./icon/raster.png"), "添加栅格图层", this, &MainWindow::openRasterData);
	fileMenu->addAction(QIcon("./icon/wb.png"), "添加空要素图层", this, &MainWindow::addEmptyLayer);
	fileMenu->addAction(QIcon("./icon/export.png"), "导出选中的图层为文件", this, &MainWindow::exportSelectedLayer);
	fileMenu->addAction(QIcon("./icon/database.png"), "导出选中的图层到数据库", this, &MainWindow::exportToDatabase);
	fileMenu->addAction(QIcon("./icon/save.png"), "保存工程", this, &MainWindow::saveProject);
	fileMenu->addAction(QIcon("./icon/picpoint.png"), "照片转点", this, &MainWindow::convertPhotoToPoints);
	fileMenu->addAction(QIcon("./icon/exit.png"), "退出", this, &MainWindow::close);

	QMenu* viewMenu = menuBar->addMenu("视图设置");
	viewMenu->addAction(QIcon("./icon/pp.png"), "设置缩放比例", this, &MainWindow::setZoomLevel);
	viewMenu->addAction(QIcon("./icon/reset.png"), "复位", this, &MainWindow::resetView);

	QMenu* vecgeoMenu = menuBar->addMenu("矢量几何分析");
	vecgeoMenu->addAction(QIcon("./icon/veccon.png"), "凸包计算", this, &MainWindow::computeConvexHull);
	vecgeoMenu->addAction(QIcon("./icon/vecbuffer.png"), "缓冲区分析", this, &MainWindow::computeBuffer);
	vecgeoMenu->addAction(QIcon("./icon/vecover.png"), "叠加分析", this, &MainWindow::overlayAnalysis);
	vecgeoMenu->addAction(QIcon("./icon/vectri.png"), "Delaunay", this, &MainWindow::computeDelaunay);
	vecgeoMenu->addAction(QIcon("./icon/vecvor.png"), "Voronoi图", this, &MainWindow::computeVoronoi);
	vecgeoMenu->addAction(QIcon("./icon/path.png"), "路径分析", this, &MainWindow::showPathAnalysisSetupDialog);

	QMenu* vecstatMenu = menuBar->addMenu("矢量统计分析");
	vecstatMenu->addAction(QIcon("./icon/count.png"), "数量", this, &MainWindow::countFeatures);
	vecstatMenu->addAction(QIcon("./icon/area.png"), "面积", this, &MainWindow::computeArea);
	vecstatMenu->addAction(QIcon("./icon/parameter.png"), "周长", this, &MainWindow::computePerimeter);
	vecstatMenu->addAction(QIcon("./icon/length.png"), "长度", this, &MainWindow::computeLength);

	QMenu* rasterMenu = menuBar->addMenu("栅格分析");
	QMenu* extractMenu = rasterMenu->addMenu(QIcon("./icon/mask.png"), "按掩膜提取");
	extractMenu->addAction(QIcon("./icon/vecmask.png"), "栅格掩膜", this, &MainWindow::extractRasterMask);
	extractMenu->addAction(QIcon("./icon/layerraster.png"), "矢量掩膜", this, &MainWindow::extractVectorMask);

	QMenu* neighborhoodMenu = rasterMenu->addMenu(QIcon("./icon/domain.png"), "邻域统计");
	neighborhoodMenu->addAction(QIcon("./icon/average.png"), "均值", this, &MainWindow::neighborhoodMean);
	neighborhoodMenu->addAction(QIcon("./icon/max.png"), "最大值", this, &MainWindow::neighborhoodMax);
	neighborhoodMenu->addAction(QIcon("./icon/min.png"), "最小值", this, &MainWindow::neighborhoodMin);

	rasterMenu->addAction(QIcon("./icon/grayscale.png"), "绘制灰度直方图", this, &MainWindow::drawGrayHistogram);
	rasterMenu->addAction(QIcon("./icon/rgb.png"), "真色彩影像", this, &MainWindow::createTrueColorImage);
	rasterMenu->addAction(QIcon("./icon/spectrum.png"), "伪色彩影像", this, &MainWindow::createFalseColorImage);
	rasterMenu->addAction(QIcon("./icon/spectrum.png"), "平滑栅格", this, &MainWindow::smoothTif);
	rasterMenu->addAction(QIcon("./icon/spectrum.png"), "D8+流向分析", this, &MainWindow::D8plusTif);
	rasterMenu->addAction(QIcon("./icon/spectrum.png"), "坡向分析", this, &MainWindow::AspectAnaTif);
	rasterMenu->addAction(QIcon("./icon/spectrum.png"), "坡度分析", this, &MainWindow::SlopeAnaTif);

	QMenu* logMenu = menuBar->addMenu("日志");
	logMenu->addAction(QIcon("./icon/log.png"), "输出日志文件", this, &MainWindow::exportLogFile);

	QMenu* fieldMenu = menuBar->addMenu("野外实习");
	fieldMenu->addAction(QIcon("./icon/icons8-notebook-100.png"), "野簿文本导入", this, &MainWindow::openFieldNote);

	QMenu* RockMenu = menuBar->addMenu("岩石分析");
	RockMenu->addAction(QIcon("./icon/log.png"), "岩石分析", this, &MainWindow::RockAna);

	QMenu* aboutmenuBar = new QMenu(this);
	setMenuBar(menuBar);

	QAction* aboutAction = new QAction(QIcon("./icon/icons8-facebook-like-96.png"), "关于", this);
	menuBar->addAction(aboutAction);

	connect(aboutAction, &QAction::triggered, this, [this]() {
		QDialog aboutDialog(this);
		QLabel* label = new QLabel(&aboutDialog);
		QPixmap pixmap("./icon/gif.gif");
		label->setPixmap(pixmap);
		label->setAlignment(Qt::AlignCenter);
		aboutDialog.resize(pixmap.size());
		aboutDialog.exec();
		});

	// 常规操作工具栏
	QToolBar* toolBar = new QToolBar("常规工具栏", this);
	mpActionGroup = new QActionGroup(this);
	QAction* saveAction = toolBar->addAction(QIcon("./icon/icons8-save-64.png"), "保存工程", this, &MainWindow::saveProject);
	saveAction->setActionGroup(mpActionGroup);
	QAction* undoAction = toolBar->addAction(QIcon("./icon/icons8-u-turn-to-left-96.png"), "撤销", this, &MainWindow::undo);
	undoAction->setActionGroup(mpActionGroup);
	QAction* redoAction = toolBar->addAction(QIcon("./icon/icons8-u-turn-to-right-96.png"), "重做", this, &MainWindow::redo);
	redoAction->setActionGroup(mpActionGroup);

	// 添加切换皮肤的功能控件
	QComboBox* themeComboBox = new QComboBox(this);
	themeComboBox->addItem("经典");
	themeComboBox->addItem("清新");
	themeComboBox->addItem("夜晚");
	themeComboBox->addItem("粉色");
	themeComboBox->setCurrentIndex(0);
	toolBar->addWidget(themeComboBox);

	addToolBar(toolBar);

	// 连接切换皮肤信号槽
	connect(themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::switchTheme);

	// 编辑工具栏
	QToolBar* editBar = new QToolBar("编辑工具栏", this);
	mpEditGroup = new QActionGroup(this);

	// 添加编辑QComboBox
	mpctrlEditComboBox = new QComboBox(this);
	mpctrlEditComboBox->addItem("编辑");
	mpctrlEditComboBox->addItem("开始");
	mpctrlEditComboBox->addItem("停止");
	mpctrlEditComboBox->setCurrentIndex(0);
	editBar->insertWidget(saveAction, mpctrlEditComboBox);

	// 添加图层选择下拉框
	editBar->addWidget(new QLabel("  图层  ", this));
	mpctrlLayerComboBox = new QComboBox(this);
	mpctrlLayerComboBox->setFixedWidth(150); // 设置宽度
	editBar->addWidget(mpctrlLayerComboBox); // 将图层选择下拉框添加到编辑工具栏

	mpChooseAction = editBar->addAction(QIcon("./icon/icons8-select-cursor-96.png"), "选择");
	mpChooseAction->setCheckable(true);
	mpChooseAction->setEnabled(false);
	mpChooseAction->setActionGroup(mpEditGroup);

	mpMoveAction = editBar->addAction(QIcon("./icon/icons8-move-96.png"), "移动");
	mpMoveAction->setCheckable(true);
	mpMoveAction->setEnabled(false);
	mpMoveAction->setActionGroup(mpEditGroup);

	mpRevAction = editBar->addAction(QIcon("./icon/icons8-create-100.png"), "创建要素");
	mpRevAction->setCheckable(true);
	mpRevAction->setEnabled(false);
	mpRevAction->setActionGroup(mpEditGroup);

	// 创建点、线、面按钮，初始隐藏
	mpCreatePointAction = editBar->addAction(QIcon("./icon/point.png"), "创建点");
	mpCreatePointAction->setCheckable(true);
	mpCreatePointAction->setVisible(false);
	mpCreatePointAction->setActionGroup(mpEditGroup);

	mpCreateLineAction = editBar->addAction(QIcon("./icon/line.png"), "创建线");
	mpCreateLineAction->setCheckable(true);
	mpCreateLineAction->setVisible(false);
	mpCreateLineAction->setActionGroup(mpEditGroup);

	mpCreatePolygonAction = editBar->addAction(QIcon("./icon/polygon.png"), "创建面");
	mpCreatePolygonAction->setCheckable(true);
	mpCreatePolygonAction->setVisible(false);
	mpCreatePolygonAction->setActionGroup(mpEditGroup);

	mpEditAction = editBar->addAction(QIcon("./icon/icons8-support-96.png"), "编辑要素");
	mpEditAction->setCheckable(true);
	mpEditAction->setEnabled(false);
	mpEditAction->setActionGroup(mpEditGroup);

	// 选择、添加、移动、删除折点按钮，初始隐藏
	mpSelTurningPointAction = editBar->addAction(QIcon("./icon/icons8-line-chart-64.png"), "选择折点");
	mpSelTurningPointAction->setCheckable(true);
	mpSelTurningPointAction->setVisible(false);
	mpSelTurningPointAction->setActionGroup(mpEditGroup);

	mpAddTurningPointAction = editBar->addAction(QIcon("./icon/addTurningPoint.png"), "添加折点");
	mpAddTurningPointAction->setCheckable(true);
	mpAddTurningPointAction->setVisible(false);
	mpAddTurningPointAction->setActionGroup(mpEditGroup);

	mpMoveTurningPointAction = editBar->addAction(QIcon("./icon/icons8-drag-one-place-to-another-point-on-multi-touch-screen-devices-96.png"), "移动折点");
	mpMoveTurningPointAction->setCheckable(true);
	mpMoveTurningPointAction->setVisible(false);
	mpMoveTurningPointAction->setActionGroup(mpEditGroup);

	mpDelTurningPointAction = editBar->addAction(QIcon("./icon/delTurningPoint.png"), "删除折点");
	mpDelTurningPointAction->setCheckable(true);
	mpDelTurningPointAction->setVisible(false);
	mpDelTurningPointAction->setActionGroup(mpEditGroup);

	mpDelAction = editBar->addAction(QIcon("./icon/icons8-bin-96.png"), "删除");
	mpDelAction->setCheckable(false);
	mpDelAction->setEnabled(false);
	mpDelAction->setActionGroup(mpEditGroup);

	mpSaveeditAction = editBar->addAction(QIcon("./icon/icons8-done-96.png"), "保存");
	mpSaveeditAction->setCheckable(false);
	mpSaveeditAction->setEnabled(false);
	mpSaveeditAction->setActionGroup(mpEditGroup);

	mpCanceleditAction = editBar->addAction(QIcon("./icon/icons8-close-96.png"), "取消");
	mpCanceleditAction->setCheckable(false);
	mpCanceleditAction->setEnabled(false);
	mpCanceleditAction->setActionGroup(mpEditGroup);


	addToolBar(editBar);

	// 连接信号槽
	connect(mpChooseAction, &QAction::triggered, this, &MainWindow::chooseActionTriggered);
	connect(mpMoveAction, &QAction::triggered, this, &MainWindow::moveActionTriggered);
	connect(mpRevAction, &QAction::triggered, this, &MainWindow::revActionTriggered);
	connect(mpEditAction, &QAction::triggered, this, &MainWindow::editActionTriggered);
	connect(mpCreatePointAction, &QAction::triggered, this, &MainWindow::createPointActionTriggered);
	connect(mpCreateLineAction, &QAction::triggered, this, &MainWindow::createLineActionTriggered);
	connect(mpCreatePolygonAction, &QAction::triggered, this, &MainWindow::createPolygonActionTriggered);
	connect(mpSelTurningPointAction, &QAction::triggered, this, &MainWindow::selTurningPointActionTriggered);
	connect(mpAddTurningPointAction, &QAction::triggered, this, &MainWindow::addTurningPointActionTriggered);
	connect(mpDelTurningPointAction, &QAction::triggered, this, &MainWindow::delTurningPointActionTriggered);
	connect(mpMoveTurningPointAction, &QAction::triggered, this, &MainWindow::moveTurningPointActionTriggered);
	connect(mpDelAction, &QAction::triggered, this, &MainWindow::deleteSelectedFeature);
	connect(mpSaveeditAction, &QAction::triggered, this, &MainWindow::saveEdit);
	connect(mpCanceleditAction, &QAction::triggered, this, &MainWindow::cancelEdit);
	connect(mpctrlEditComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::handleEditComboBoxChange);

	// 初始设置
	mpChooseAction->setChecked(false);
	mpctrlView->setInteractionMode(false);

	// 初始化图层管理面板
	QDockWidget* layerDock = new QDockWidget("图层管理", this);
	layerDock->setWindowIcon(QIcon("./icon/layerman.png"));
	mpctrlLayerTree = new QTreeWidget(layerDock);
	mpctrlLayerTree->setHeaderLabels(QStringList() << "图层");
	layerDock->setWidget(mpctrlLayerTree);
	addDockWidget(Qt::LeftDockWidgetArea, layerDock);

	// 图层右键菜单
	mpctrlLayerTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(mpctrlLayerTree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::onLayerTreeContextMenu);

	// 有图层勾选状态改变，更新符号系统的信号
	connect(mpctrlLayerTree, &QTreeWidget::itemChanged, this, &MainWindow::onLayerItemChanged);

	QDockWidget* mpLeftDock = new QDockWidget(tr("工具箱"), this);
	mpLeftDock->setWindowIcon(QIcon("./icon/toolbox.png"));
	mpLeftDock->setAllowedAreas(Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, mpLeftDock);

	QVBoxLayout* mpToolLayout = new QVBoxLayout();
	QWidget* toolWidget = new QWidget(this);

	// 使用QTreeWidget创建工具箱树形结构
	QTreeWidget* mpToolTree = new QTreeWidget(toolWidget);
	mpToolTree->setHeaderHidden(true);

	// 矢量几何分析
	QTreeWidgetItem* mpVectorAnalysisItem = new QTreeWidgetItem(mpToolTree);
	mpVectorAnalysisItem->setText(0, tr("矢量几何分析"));
	mpVectorAnalysisItem->setIcon(0, QIcon("./icon/vecgeo.png"));

	QTreeWidgetItem* mpConvexHullItem = new QTreeWidgetItem(mpVectorAnalysisItem);
	mpConvexHullItem->setText(0, tr("凸包计算"));
	mpConvexHullItem->setIcon(0, QIcon("./icon/veccon.png"));

	QTreeWidgetItem* mpBufferItem = new QTreeWidgetItem(mpVectorAnalysisItem);
	mpBufferItem->setText(0, tr("缓冲区分析"));
	mpBufferItem->setIcon(0, QIcon("./icon/vecbuffer.png"));

	QTreeWidgetItem* mpOverlayItem = new QTreeWidgetItem(mpVectorAnalysisItem);
	mpOverlayItem->setText(0, tr("叠加分析"));
	mpOverlayItem->setIcon(0, QIcon("./icon/vecover.png"));

	QTreeWidgetItem* mpDelaunayItem = new QTreeWidgetItem(mpVectorAnalysisItem);
	mpDelaunayItem->setText(0, tr("Delaunay剖分"));
	mpDelaunayItem->setIcon(0, QIcon("./icon/vectri.png"));

	QTreeWidgetItem* mpVoronoiItem = new QTreeWidgetItem(mpVectorAnalysisItem);
	mpVoronoiItem->setText(0, tr("Voronoi图"));
	mpVoronoiItem->setIcon(0, QIcon("./icon/vecvor.png"));

	QTreeWidgetItem* mpPathItem = new QTreeWidgetItem(mpVectorAnalysisItem);
	mpPathItem->setText(0, tr("路径分析"));
	mpPathItem->setIcon(0, QIcon("./icon/path.png"));

	// 矢量统计分析
	QTreeWidgetItem* mpVectorStatisticsItem = new QTreeWidgetItem(mpToolTree);
	mpVectorStatisticsItem->setText(0, tr("矢量统计分析"));
	mpVectorStatisticsItem->setIcon(0, QIcon("./icon/vecsta.png"));

	QTreeWidgetItem* mpCountItem = new QTreeWidgetItem(mpVectorStatisticsItem);
	mpCountItem->setText(0, tr("数量"));
	mpCountItem->setIcon(0, QIcon("./icon/count.png"));

	QTreeWidgetItem* mpAreaItem = new QTreeWidgetItem(mpVectorStatisticsItem);
	mpAreaItem->setText(0, tr("面积"));
	mpAreaItem->setIcon(0, QIcon("./icon/area.png"));

	QTreeWidgetItem* mpPerimeterItem = new QTreeWidgetItem(mpVectorStatisticsItem);
	mpPerimeterItem->setText(0, tr("周长"));
	mpPerimeterItem->setIcon(0, QIcon("./icon/parameter.png"));

	QTreeWidgetItem* mpLengthItem = new QTreeWidgetItem(mpVectorStatisticsItem);
	mpLengthItem->setText(0, tr("长度"));
	mpLengthItem->setIcon(0, QIcon("./icon/length.png"));

	// 栅格分析
	QTreeWidgetItem* mpRrasterAnalysisItem = new QTreeWidgetItem(mpToolTree);
	mpRrasterAnalysisItem->setText(0, tr("栅格分析"));
	mpRrasterAnalysisItem->setIcon(0, QIcon("./icon/raster.png"));

	QTreeWidgetItem* mpMaskItem = new QTreeWidgetItem(mpRrasterAnalysisItem);
	mpMaskItem->setText(0, tr("按掩膜提取"));
	mpMaskItem->setIcon(0, QIcon("./icon/mask.png"));

	QTreeWidgetItem* mpRasterMaskItem = new QTreeWidgetItem(mpMaskItem);
	mpRasterMaskItem->setText(0, tr("栅格掩膜"));
	mpRasterMaskItem->setIcon(0, QIcon("./icon/layerraster.png"));

	QTreeWidgetItem* mpVectorMaskItem = new QTreeWidgetItem(mpMaskItem);
	mpVectorMaskItem->setText(0, tr("矢量掩膜"));
	mpVectorMaskItem->setIcon(0, QIcon("./icon/vecmask.png"));

	QTreeWidgetItem* mpDomainItem = new QTreeWidgetItem(mpRrasterAnalysisItem);
	mpDomainItem->setText(0, tr("领域统计"));
	mpDomainItem->setIcon(0, QIcon("./icon/domain.png"));

	QTreeWidgetItem* mpMeanItem = new QTreeWidgetItem(mpDomainItem);
	mpMeanItem->setText(0, tr("均值"));
	mpMeanItem->setIcon(0, QIcon("./icon/average.png"));

	QTreeWidgetItem* mpMaxItem = new QTreeWidgetItem(mpDomainItem);
	mpMaxItem->setText(0, tr("最大值"));
	mpMaxItem->setIcon(0, QIcon("./icon/max.png"));

	QTreeWidgetItem* mpMinItem = new QTreeWidgetItem(mpDomainItem);
	mpMinItem->setText(0, tr("最小值"));
	mpMinItem->setIcon(0, QIcon("./icon/min.png"));

	QTreeWidgetItem* mpGrayscaleItem = new QTreeWidgetItem(mpRrasterAnalysisItem);
	mpGrayscaleItem->setText(0, tr("绘制灰度直方图"));
	mpGrayscaleItem->setIcon(0, QIcon("./icon/grayscale.png"));

	QTreeWidgetItem* mpTrueColorItem = new QTreeWidgetItem(mpRrasterAnalysisItem);
	mpTrueColorItem->setText(0, tr("真色彩影像"));
	mpTrueColorItem->setIcon(0, QIcon("./icon/rgb.png"));

	QTreeWidgetItem* mpPseudoColorItem = new QTreeWidgetItem(mpRrasterAnalysisItem);
	mpPseudoColorItem->setText(0, tr("伪色彩影像"));
	mpPseudoColorItem->setIcon(0, QIcon("./icon/spectrum.png"));

	mpToolLayout->addWidget(mpToolTree);
	toolWidget->setLayout(mpToolLayout);
	mpLeftDock->setWidget(toolWidget);

	// 连接树形结构的项到槽函数
	connect(mpToolTree, &QTreeWidget::itemClicked, this, [=](QTreeWidgetItem* item, int) {
		if (item == mpConvexHullItem) {
			computeConvexHull();
		}
		else if (item == mpBufferItem) {
			computeBuffer();
		}
		else if (item == mpOverlayItem) {
			overlayAnalysis();
		}
		else if (item == mpDelaunayItem) {
			computeDelaunay();
		}
		else if (item == mpVoronoiItem) {
			computeVoronoi();
		}
		else if (item == mpCountItem) {
			countFeatures();
		}
		else if (item == mpAreaItem) {
			computeArea();
		}
		else if (item == mpPerimeterItem) {
			computePerimeter();
		}
		else if (item == mpLengthItem) {
			computeLength();
		}
		else if (item == mpRasterMaskItem) {
			extractRasterMask();
		}
		else if (item == mpVectorMaskItem) {
			extractVectorMask();
		}
		else if (item == mpMeanItem) {
			neighborhoodMean();
		}
		else if (item == mpMaxItem) {
			neighborhoodMax();
		}
		else if (item == mpMinItem) {
			neighborhoodMin();
		}
		else if (item == mpGrayscaleItem) {
			drawGrayHistogram();
		}
		else if (item == mpTrueColorItem) {
			createTrueColorImage();
		}
		else if (item == mpPseudoColorItem) {
			createFalseColorImage();
		}
		else if (item == mpPathItem) {
			showPathAnalysisSetupDialog();
		}
		});


	// 初始化符号系统面板
	QDockWidget* symDock = new QDockWidget("符号系统", this);
	symDock->setWindowIcon(QIcon("./icon/sym.png"));
	mpctrlSymList = new QListWidget(symDock);
	symDock->setWidget(mpctrlSymList);
	addDockWidget(Qt::RightDockWidgetArea, symDock);

	QDockWidget* statDock = new QDockWidget("统计结果", this);
	statDock->setWindowIcon(QIcon("./icon/form.png"));
	mpctrlStatList = new QListWidget(statDock);
	statDock->setWidget(mpctrlStatList);
	addDockWidget(Qt::RightDockWidgetArea, statDock);

	// 初始化主绘图区域
	mpctrlView->setDragMode(QGraphicsView::ScrollHandDrag);
	mpctrlView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	mpctrlView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
	mpctrlView->setInteractive(true);
	mpctrlView->scale(0.4, 0.4);

	// 创建中心部件和布局
	QWidget* centralWidget = new QWidget(this);
	QVBoxLayout* centralLayout = new QVBoxLayout(centralWidget);
	centralLayout->addWidget(mpctrlView);

	// 创建确认和取消按钮
	mpctrlConfirmButton = new QPushButton(QIcon("./icon/icons8-done-96.png"), "", this);
	mpctrlCancelButton = new QPushButton(QIcon("./icon/icons8-close-96.png"), "", this);

	QString buttonStyle = R"(
        QPushButton {
            background-color: rgba(128, 128, 128, 128);  // 半透明灰色
            border: 1px solid #FFFFFF;
            border-radius: 10px;
            color: white;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 128);  // 半透明白色
        }
        QPushButton:pressed {
            background-color: rgba(100, 100, 100, 128);  // 半透明深灰色
        }
    )";

	mpctrlConfirmButton->setStyleSheet(buttonStyle);
	mpctrlCancelButton->setStyleSheet(buttonStyle);

	// 设置按钮可见性
	mpctrlConfirmButton->setVisible(false);
	mpctrlCancelButton->setVisible(false);

	// 将按钮布局添加到中心布局的下面
	centralLayout->addLayout(mpButtonLayout);

	// 设置主绘图区域为主窗口的中心部件
	setCentralWidget(centralWidget);

	// 初始化标签
	mpctrlMousePosLabel = new QLabel(this);
	mpctrlZoomLabel = new QLabel(this);
	mpctrlRotationLabel = new QLabel(this);
	// 创建状态栏并添加标签
	QStatusBar* statusBar = new QStatusBar(this);
	statusBar->addPermanentWidget(mpctrlMousePosLabel);
	statusBar->addPermanentWidget(mpctrlZoomLabel);
	statusBar->addPermanentWidget(mpctrlRotationLabel);
	setStatusBar(statusBar);

	statusBar->setStyleSheet("QStatusBar { background-color: lightgray; }");

	// 连接信号和槽
	connect(mpctrlLayerTree, &QTreeWidget::itemClicked, this, &MainWindow::drawLayers);

	// 鼠标点击信号
	connect(mpctrlView, &myGraphicsView::sceneClicked, this, &MainWindow::onSceneClicked);

	// 更新状态栏
	connect(mpctrlView, &myGraphicsView::statusBarUpdated, this, &MainWindow::onStatusBarUpdated);
	// 打开栅格信号
	connect(this, &MainWindow::rasterDataOpened, this, &MainWindow::readTIF);
	// 创建要素保存与取消信号
	connect(mpctrlConfirmButton, &QPushButton::clicked, this, &MainWindow::confirmCreation);
	connect(mpctrlCancelButton, &QPushButton::clicked, this, &MainWindow::cancelCreation);
}

// 设置创建要素确认按钮样式
void MainWindow::resizeEvent(QResizeEvent* event) {
	QMainWindow::resizeEvent(event);

	// 计算按钮的位置
	int buttonWidth = 50;
	int buttonHeight = 25;
	int spacing = 20;
	int bottomMargin = 60;

	int confirmButtonX = (width() / 2) - buttonWidth - (spacing / 2);
	int cancelButtonX = (width() / 2) + (spacing / 2);
	int buttonY = height() - buttonHeight - bottomMargin;

	// 设置按钮的位置
	mpctrlConfirmButton->setGeometry(confirmButtonX, buttonY, buttonWidth, buttonHeight);
	mpctrlCancelButton->setGeometry(cancelButtonX, buttonY, buttonWidth, buttonHeight);
}

// *****************************************读取文件与基础功能部分******************************************

// 判断后缀名，用于读取时判断文件类型
bool MainWindow::ends_with(const std::string& fullString, const std::string& ending) {
	if (fullString.length() >= ending.length()) {
		return std::equal(ending.rbegin(), ending.rend(), fullString.rbegin());
	}
	return false;
}

// 打开工程文件
void MainWindow::openProjectFile() {
	QString fileName = QFileDialog::getOpenFileName(this, "打开工程文件", "", "工程文件 (*.xml)");
	if (fileName.isEmpty()) return;

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(this, "打开工程文件", "无法打开文件进行读取");
		return;
	}

	QXmlStreamReader xmlReader(&file);

	mmapNameLayer.clear();
	mpctrlLayerTree->clear();
	mpctrlLayerComboBox->clear();
	mpctrlScene->clear();

	while (!xmlReader.atEnd() && !xmlReader.hasError()) {
		xmlReader.readNext();
		if (xmlReader.isStartElement()) {
			if (xmlReader.name() == QStringLiteral("Layer")) {
				QString layerName = xmlReader.attributes().value("name").toString();
				QString type = xmlReader.attributes().value("type").toString();
				QString filePath = xmlReader.attributes().value("filePath").toString();
				Layer newLayer;
				newLayer.group = new QGraphicsItemGroup();

				if (!filePath.isEmpty()) {
					if (type == "raster") {
						newLayer.isRaster = true;
						newLayer.rasterFilePath = filePath;
					}
					else if (type == "vector") {
						newLayer.isRaster = false;
						newLayer.vectorFilePath = filePath;
						// 加载矢量数据
						mStore = Store();
						vecReader reader;
						std::string strFilePath = filePath.toStdString();
						if (ends_with(strFilePath, ".json") || ends_with(strFilePath, ".geojson")) {
							reader.readGeojson(strFilePath.c_str(), mStore);
						}
						else if (ends_with(strFilePath, ".shp")) {
							reader.readShapefile(strFilePath.c_str(), mStore);
						}
						else {
							reader.readWKT(strFilePath.c_str(), mStore);
						}
						newLayer.store = mStore;

						// 获取储存中XY的最值，用于可视化时转换坐标
						mdMinLon = mStore.getMinX();
						mdMaxLon = mStore.getMaxX();
						mdMinLat = mStore.getMinY();
						mdMaxLat = mStore.getMaxY();

						// 生成随机颜色
						QColor randomColor = QColor::fromRgb(rand() % 256, rand() % 256, rand() % 256);
						newLayer.brush = QBrush(randomColor);

						// 设置初始线条宽度和填充模式
						newLayer.pen.setWidth(4);
						newLayer.brush.setStyle(Qt::SolidPattern);
						mDraw.setPen(newLayer.pen);
						mDraw.setBrush(newLayer.brush);

						mmapNameLayer[layerName] = newLayer;

						// 更新图层树
						updateLayerTree();
						// 重新绘制所有图层
						drawLayers();

						// 使用图形项的边界来初次调整视图，使图形顶到边界
						QRectF itemsBoundingRect = mpctrlScene->itemsBoundingRect();
						QGraphicsView* view = static_cast<QGraphicsView*>(centralWidget()->layout()->itemAt(0)->widget());
						view->fitInView(itemsBoundingRect, Qt::KeepAspectRatio);
					}
				}
				else {
					// 读取并解析临时图层的几何数据
					Store layerStore;
					while (!(xmlReader.isEndElement() && xmlReader.name().toString() == "Layer")) {
						xmlReader.readNext();
						if (xmlReader.isStartElement()) {
							if (xmlReader.name().toString() == "Point") {
								double x = xmlReader.attributes().value("x").toDouble();
								double y = xmlReader.attributes().value("y").toDouble();
								layerStore.addPoint(new Point(x, y));
							}
							else if (xmlReader.name().toString() == "Line") {
								LineString* line = new LineString();
								while (!(xmlReader.isEndElement() && xmlReader.name().toString() == "Line")) {
									xmlReader.readNext();
									if (xmlReader.isStartElement() && xmlReader.name().toString() == "Point") {
										double x = xmlReader.attributes().value("x").toDouble();
										double y = xmlReader.attributes().value("y").toDouble();
										line->addPoint(Point(x, y));
									}
								}
								layerStore.addLine(line);
							}
							else if (xmlReader.name().toString() == "Polygon") {
								myPolygon* polygon = new myPolygon();
								while (!(xmlReader.isEndElement() && xmlReader.name().toString() == "Polygon")) {
									xmlReader.readNext();
									if (xmlReader.isStartElement() && xmlReader.name().toString() == "Point") {
										double x = xmlReader.attributes().value("x").toDouble();
										double y = xmlReader.attributes().value("y").toDouble();
										polygon->addExteriorPoint(Point(x, y));
									}
								}
								layerStore.addPolygon(polygon);
							}
						}
					}
					newLayer.store = layerStore;
				}

				mmapNameLayer[layerName] = newLayer;
				updateLayerTree();
				drawLayers();
			}
		}
	}

	if (mmapNameLayer.empty()) {
		Layer defaultLayer;
		defaultLayer.group = new QGraphicsItemGroup();
		defaultLayer.isRaster = false;
		defaultLayer.vectorFilePath = "";
		mmapNameLayer["默认图层"] = defaultLayer;
		updateLayerTree();
	}

	if (xmlReader.hasError()) {
		QMessageBox::warning(this, "打开工程文件", "解析XML文件时发生错误");
	}

	file.close();
	QMessageBox::information(this, "打开工程文件", "工程文件已成功打开: " + fileName);
}

// 保存工程文件
void MainWindow::saveProject() {
	QString fileName = QFileDialog::getSaveFileName(this, "保存工程文件", "", "工程文件 (*.xml)");
	if (fileName.isEmpty()) return;

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this, "保存工程文件", "无法打开文件进行写入");
		return;
	}

	QXmlStreamWriter xmlWriter(&file);
	xmlWriter.setAutoFormatting(true);
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("Project");

	for (const auto& layer : mmapNameLayer) {
		xmlWriter.writeStartElement("Layer");
		xmlWriter.writeAttribute("name", layer.first);
		if (layer.second.isRaster) {
			xmlWriter.writeAttribute("type", "raster");
			xmlWriter.writeAttribute("filePath", layer.second.rasterFilePath);
		}
		else if (!layer.second.vectorFilePath.isEmpty()) {
			xmlWriter.writeAttribute("type", "vector");
			xmlWriter.writeAttribute("filePath", layer.second.vectorFilePath);
		}
		else {
			// 保存临时图层的几何数据
			xmlWriter.writeAttribute("type", "vector");

			// 保存点几何数据
			for (const auto& point : layer.second.store.getStorePoints()) {
				xmlWriter.writeStartElement("Point");
				xmlWriter.writeAttribute("x", QString::number(point->getX()));
				xmlWriter.writeAttribute("y", QString::number(point->getY()));
				xmlWriter.writeEndElement();
			}

			// 保存线几何数据
			for (const auto& line : layer.second.store.getStoreLines()) {
				xmlWriter.writeStartElement("Line");
				for (const auto& point : line->getPoints()) {
					xmlWriter.writeStartElement("Point");
					xmlWriter.writeAttribute("x", QString::number(point.getX()));
					xmlWriter.writeAttribute("y", QString::number(point.getY()));
					xmlWriter.writeEndElement();
				}
				xmlWriter.writeEndElement();
			}

			// 保存面几何数据
			for (const auto& polygon : layer.second.store.getStorePolygons()) {
				xmlWriter.writeStartElement("Polygon");
				for (const auto& point : polygon->getExteriorRing().getPoints()) {
					xmlWriter.writeStartElement("Point");
					xmlWriter.writeAttribute("x", QString::number(point.getX()));
					xmlWriter.writeAttribute("y", QString::number(point.getY()));
					xmlWriter.writeEndElement();
				}
				xmlWriter.writeEndElement();
			}
		}
		xmlWriter.writeEndElement();
	}

	xmlWriter.writeEndElement();
	xmlWriter.writeEndDocument();

	file.close();
	QMessageBox::information(this, "保存工程文件", "工程文件已保存到: " + fileName);
}

// 打开矢量数据
void MainWindow::openVectorData()
{
	// 获取读取文件的路径
	mstrVectorFileName = QFileDialog::getOpenFileName(this, "打开矢量数据文件", "", "矢量数据文件 (*.shp *.json *.wkt)");
	if (mstrVectorFileName.isEmpty()) return;
	std::string strFilePath = mstrVectorFileName.toUtf8().toStdString();

	// 重新初始化 store 以防止数据混乱
	mStore = Store();
	vecReader reader;

	// 根据后缀名调用对应读取函数
	if (ends_with(strFilePath, ".json") || ends_with(strFilePath, ".geojson")) {
		reader.readGeojson(strFilePath.c_str(), mStore);
	}
	else if (ends_with(strFilePath, ".shp")) {
		reader.readShapefile(strFilePath.c_str(), mStore);
	}
	else {
		reader.readWKT(strFilePath.c_str(), mStore);
	}

	// 获取储存中XY的最值，用于可视化时转换坐标
	mdMinLon = mStore.getMinX();
	mdMaxLon = mStore.getMaxX();
	mdMinLat = mStore.getMinY();
	mdMaxLat = mStore.getMaxY();

	mpctrlScene->clear();  // 清空现有场景内容

	// 设置初始线条宽度和填充模式
	mPen.setWidth(4);
	mBrush.setStyle(Qt::SolidPattern);
	mDraw.setPen(mPen);
	mDraw.setBrush(mBrush);

	// 添加新图层
	QString layerName = QFileInfo(mstrVectorFileName).fileName();
	// 生成随机数种子
	srand(time(nullptr));
	// 获取随机颜色
	QColor randomColor = QColor::fromRgb(rand() % 256, rand() % 256, rand() % 256);
	Layer newLayer{ mStore, new QGraphicsItemGroup(), QPen(Qt::black), QBrush(randomColor) };
	newLayer.vectorFilePath = mstrVectorFileName; // 保存矢量图层的文件路径
	mmapNameLayer[layerName] = newLayer;

	// 更新图层树
	updateLayerTree();
	// 重新绘制所有图层
	drawLayers();

	// 使用图形项的边界来初次调整视图，使图形顶到边界
	QRectF itemsBoundingRect = mpctrlScene->itemsBoundingRect();
	QGraphicsView* view = static_cast<QGraphicsView*>(centralWidget()->layout()->itemAt(0)->widget());
	view->fitInView(itemsBoundingRect, Qt::KeepAspectRatio);

	// 更新符号系统列表
	updateSymList();
}

// 读取栅格数据
void MainWindow::openRasterData() {
	mstrRasterFileName = QFileDialog::getOpenFileName(this, "打开栅格数据文件", "", "栅格数据文件 (*.tif *.img *.png *.jpg)");
	if (mstrRasterFileName.isEmpty()) return;

	// 添加栅格图层到 layers 容器
	QString layerName = QFileInfo(mstrRasterFileName).fileName();
	Layer newLayer;
	newLayer.isRaster = true; // 标记为栅格图层
	newLayer.group = new QGraphicsItemGroup();
	newLayer.rasterFilePath = mstrRasterFileName; // 保存栅格图层的文件路径
	mmapNameLayer[layerName] = newLayer;

	updateLayerTree(); // 更新图层管理树
	drawLayers(); // 重新绘制所有图层

	// 显示栅格数据
	emit rasterDataOpened(mstrRasterFileName);
}

// 读取TIF文件
void MainWindow::readTIF(QString Path)
{
	gdal_engine gtif(Path, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
	//从中获取带有像素的矢量缓冲
	std::vector<unsigned char> read;
	qDebug() << "读取TIF文件";
	//读取TIF文件

	gtif.quickTifShow(mpctrlView, mpctrlScene, gtif.mdSCale, 0);
	std::vector<float> position;
	position = gtif.mvGetposition();

	mdMinLon = position[1];
	mdMaxLon = position[3];
	mdMinLat = position[0];
	mdMaxLat = position[2];

};

// 导出图层到数据库
void MainWindow::exportToDatabase() {
	// 选择图层
	QString selectedLayerName = selectLayer("导出图层", true);
	auto it = mmapNameLayer.find(selectedLayerName);
	Store* pSelectedStore = nullptr;
	if (it != mmapNameLayer.end()) {
		pSelectedStore = &(it->second.store);
	}
	// 创建数据库连接对话框
	mpctrlDbConnectDialog = new dbConnectDialog(this);
	if (mpctrlDbConnectDialog->exec() == QDialog::Accepted) {
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
		db.setHostName(mpctrlDbConnectDialog->getHostName());
		db.setDatabaseName(mpctrlDbConnectDialog->getDatabaseName());
		db.setPort(mpctrlDbConnectDialog->getPort().toInt());
		db.setUserName(mpctrlDbConnectDialog->getUserName());
		db.setPassword(mpctrlDbConnectDialog->getPassword());
		bool bIsOpen = db.open();
		if (!bIsOpen) {
			QMessageBox::information(this, "提示", "数据库连接失败");
			return;
		}
		QSqlQuery query(db);
		query.exec("SET NAMES 'Latin1'");//使数据库支持中文
		// 存储点
		int nStoreId = pSelectedStore->getID();
		if (pSelectedStore->getStorePoints().size() > 0) {
			// 创建表
			std::vector<Point*> vpoints = pSelectedStore->getStorePoints();
			QString tableName = QString("Store") + QString::number(nStoreId) + QString("Points_tab");
			QString sql = QString("CREATE TABLE ") + tableName + QString(" (id integer, point point");
			// 收集所有唯一的属性键
			std::set<QString> attributeKeys;
			for (const auto& point : vpoints) {
				auto attrs = pSelectedStore->getAttributes(point);
				for (const auto& attr : attrs) {
					attributeKeys.insert(attr.first);
				}
			}
			for (const auto& key : attributeKeys) {
				sql += QString(", %1 text").arg(key);
			}
			sql += ");";
			bool bSuccess = query.exec(sql);
			if (bSuccess) {
				qDebug() << "创建表成功";
			}
			else {
				qDebug() << "创建表失败";
			}
			// 插入数据
			int nPointId = 0;
			for (const auto& point : vpoints) {
				sql = QString("INSERT INTO ") + tableName + QString(" (id, point");
				for (const auto& key : attributeKeys) {
					sql += QString(", %1").arg(key);
				}
				sql += QString(") VALUES (") + QString::number(nPointId) + QString(", '(") + QString::number(point->getX()) + QString(", ") + QString::number(point->getY()) + QString(")'");
				auto attrs = pSelectedStore->getAttributes(point);
				for (const auto& attr : attrs) {
					sql += QString(", '%1'").arg(attr.second);
				}
				sql += QString(");");
				bSuccess = query.exec(sql);
				if (bSuccess) {
					qDebug() << "插入数据成功";
				}
				else {
					qDebug() << "插入数据失败";
				}
				nPointId++;
			}
		}
		// 存储线
		if (pSelectedStore->getStoreLines().size() > 0) {
			// 创建表
			std::vector<LineString*> vlines = pSelectedStore->getStoreLines();
			QString tableName = QString("Store") + QString::number(nStoreId) + QString("LineStrings_tab");
			QString sql = QString("CREATE TABLE ") + tableName + QString(" (id integer, lineString path");
			// 收集所有唯一的属性键
			std::set<QString> attributeKeys;
			for (const auto& line : vlines) {
				auto attrs = pSelectedStore->getAttributes(line);
				for (const auto& attr : attrs) {
					attributeKeys.insert(attr.first);
				}
			}
			for (const auto& key : attributeKeys) {
				sql += QString(", %1 text").arg(key);
			}
			sql += ");";
			bool bSuccess = query.exec(sql);
			if (bSuccess) {
				qDebug() << "创建表成功";
			}
			else {
				qDebug() << "创建表失败";
			}
			// 插入数据
			int nLineId = 0;
			for (const auto& line : vlines) {
				sql = QString("INSERT INTO ") + tableName + QString(" (id, lineString");
				for (const auto& key : attributeKeys) {
					sql += QString(", %1").arg(key);
				}
				vector<Point> vpoints = line->getPoints();
				sql += QString(") VALUES (") + QString::number(nLineId) + QString(", '[(") + QString::number(vpoints[0].getX()) + QString(", ") + QString::number(vpoints[0].getY()) + QString(")");
				for (int i = 1; i < vpoints.size(); i++) {
					sql += QString(", (") + QString::number(vpoints[i].getX()) + QString(", ") + QString::number(vpoints[i].getY()) + QString(")");
				}
				sql += QString("]'");
				auto attrs = pSelectedStore->getAttributes(line);
				for (const auto& attr : attrs) {
					sql += QString(", '%1'").arg(attr.second);
				}
				sql += QString(");");
				bSuccess = query.exec(sql);
				if (bSuccess) {
					qDebug() << "插入数据成功";
				}
				else {
					qDebug() << "插入数据失败";
				}
				nLineId++;
			}
		}
		// 存储面
		if (pSelectedStore->getStorePolygons().size() > 0) {
			// 创建表
			std::vector<myPolygon*> vpolygons = pSelectedStore->getStorePolygons();
			QString tableName = QString("Store") + QString::number(nStoreId) + QString("Polygons_tab");
			QString sql = QString("CREATE TABLE ") + tableName + QString(" (id integer, Polygon polygon");
			// 收集所有唯一的属性键
			std::set<QString> attributeKeys;
			for (const auto& polygon : vpolygons) {
				auto attrs = pSelectedStore->getAttributes(polygon);
				for (const auto& attr : attrs) {
					attributeKeys.insert(attr.first);
				}
			}
			for (const auto& key : attributeKeys) {
				sql += QString(", %1 text").arg(key);
			}
			sql += ");";
			bool bSuccess = query.exec(sql);
			if (bSuccess) {
				qDebug() << "创建表成功";
			}
			else {
				qDebug() << "创建表失败";
			}
			// 插入数据
			int nPolygonId = 0;
			for (const auto& polygon : vpolygons) {
				sql = QString("INSERT INTO ") + tableName + QString(" (id, Polygon");
				for (const auto& key : attributeKeys) {
					sql += QString(", %1").arg(key);
				}
				vector<Point> vpoints = polygon->getExteriorRing().getPoints();
				sql += QString(") VALUES (") + QString::number(nPolygonId) + QString(", '((") + QString::number(vpoints[0].getX()) + QString(", ") + QString::number(vpoints[0].getY()) + QString(")");
				for (int i = 1; i < vpoints.size(); i++) {
					sql += QString(", (") + QString::number(vpoints[i].getX()) + QString(", ") + QString::number(vpoints[i].getY()) + QString(")");
				}
				sql += QString(")'");
				auto attrs = pSelectedStore->getAttributes(polygon);
				for (const auto& attr : attrs) {
					sql += QString(", '%1'").arg(attr.second);
				}
				sql += QString(");");
				bSuccess = query.exec(sql);
				if (bSuccess) {
					qDebug() << "插入数据成功";
				}
				else {
					qDebug() << "插入数据失败";
				}
				nPolygonId++;
			}
		}
		QMessageBox::information(this, "提示", "成功保存到数据库");
		db.close();
	}
}

// 导出数据为 shp 文件
void MainWindow::exportSelectedLayer() {
	// 选择图层
	QString selectedLayerName = selectLayer("导出图层", true);
	auto it = mmapNameLayer.find(selectedLayerName);
	Store* pSelectedStore = nullptr;
	if (it != mmapNameLayer.end()) {
		pSelectedStore = &(it->second.store);
	}
	if (pSelectedStore != nullptr) {
		// 判断当前图层是否为单一要素类型
		int nTypeCount = 0;
		if (pSelectedStore->getStorePoints().size() > 0)
			nTypeCount++;
		if (pSelectedStore->getStoreLines().size() > 0)
			nTypeCount++;
		if (pSelectedStore->getStorePolygons().size() > 0)
			nTypeCount++;
		if (nTypeCount > 1) {
			QMessageBox::warning(this, "警告", "只能导出单一要素类型的图层为shp文件");
			return;
		}

		QString fileName = QFileDialog::getSaveFileName(this, "导出选中的图层", "", "矢量数据文件 (*.shp);;栅格数据文件 (*.tif *.img)");
		if (fileName.isEmpty()) return;
		vecWriter writer;
		writer.writeShapefile(fileName.toUtf8().toStdString(), *pSelectedStore);
		QMessageBox::information(this, "提示", "保存成功");
	}

}

// 设置缩放比例
void MainWindow::setZoomLevel() {
	// 获取当前主绘图区域的缩放比例
	QGraphicsView* view = static_cast<QGraphicsView*>(centralWidget()->layout()->itemAt(0)->widget());
	qreal initialScaleFactor = view->transform().m11(); // 获取当前水平缩放比例

	// 创建一个对话框
	QDialog dialog(this);
	dialog.setWindowTitle("设置缩放比例");

	// 创建滑动条
	QSlider slider(Qt::Horizontal);
	slider.setRange(1, 500); // 设置范围
	slider.setValue(initialScaleFactor * 100); // 设置初始值

	// 创建输入框
	QLineEdit inputBox;
	inputBox.setValidator(new QDoubleValidator(0.01, 2.0, 2)); // 设置输入范围和小数点位数
	inputBox.setText(QString::number(initialScaleFactor));

	// 连接滑动条数值改变的信号
	QObject::connect(&slider, &QSlider::valueChanged, [&](int value) {
		double scaleFactor = static_cast<double>(value) / 100.0;
		inputBox.setText(QString::number(scaleFactor, 'g', 2));
		view->resetTransform();
		view->scale(scaleFactor, scaleFactor);
		});

	// 连接输入框文本改变的信号
	QObject::connect(&inputBox, &QLineEdit::textChanged, [&](const QString& text) {
		bool ok;
		double scaleFactor = text.toDouble(&ok);
		if (ok) {
			int sliderValue = static_cast<int>(scaleFactor * 100);
			slider.setValue(sliderValue);
			view->resetTransform();
			view->scale(scaleFactor, scaleFactor);
		}
		});

	// 创建确定和取消按钮
	QPushButton okButton("确定");
	QPushButton cancelButton("取消");

	// 设置布局
	QVBoxLayout layout(&dialog);
	layout.addWidget(&slider);
	layout.addWidget(&inputBox);
	layout.addWidget(&okButton);
	layout.addWidget(&cancelButton);

	// 连接按钮点击的信号
	QObject::connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
	QObject::connect(&cancelButton, &QPushButton::clicked, [&]() {
		view->resetTransform();
		view->scale(initialScaleFactor, initialScaleFactor);
		dialog.reject();
		});

	// 打开对话框
	if (dialog.exec() == QDialog::Accepted) {
		// 用户点击了确定按钮，保留用户设置的缩放比例
	}
	else {
		// 用户点击了取消按钮，恢复到最初的缩放比例
		view->resetTransform();
		view->scale(initialScaleFactor, initialScaleFactor);
	}
}

// 复位操作
void MainWindow::resetView() {
	// 获取绘图视图和场景项的边界矩形
	QGraphicsView* view = static_cast<QGraphicsView*>(centralWidget()->layout()->itemAt(0)->widget());
	QRectF sceneRect = mpctrlScene->itemsBoundingRect();

	// 使用 fitInView 将视图调整到能够恰好显示所有内容
	view->fitInView(sceneRect, Qt::KeepAspectRatio);
	Logger::getInstance().logFunctionSuccess("复位");
}

// *****************************************矢量数据分析部分******************************************

// 获取用户选择的进行操作的图层的名称
QString MainWindow::selectLayer(const QString& operation, bool requirePoints) {
	// 创建一个对话框
	QDialog dialog(this);
	dialog.setWindowTitle("选择图层");

	// 创建布局和列表控件
	QVBoxLayout layout(&dialog);
	QComboBox layerComboBox(&dialog);

	// 根据操作类型决定图层选择条件
	for (const auto& layer : mmapNameLayer) {
		bool addLayer = false;

		if (operation == "凸包计算" || operation == "导出图层") {
			// 凸包计算支持点、线、面图层
			addLayer = true;
		}
		else if (operation == "Delaunay三角剖分" || operation == "Voronoi图") {
			// Delaunay三角剖分和Voronoi图只支持点图层
			addLayer = !layer.second.isRaster && (!requirePoints || !layer.second.store.getStorePoints().empty());
		}

		if (addLayer) {
			layerComboBox.addItem(layer.first);
		}
	}

	if (layerComboBox.count() == 0) {
		// 如果没有符合条件的图层
		QMessageBox::warning(this, "选择图层", "没有符合条件的图层可供选择");
		return QString();
	}

	layout.addWidget(&layerComboBox);

	// 添加确认和取消按钮
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	layout.addWidget(&buttonBox);
	connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	// 显示对话框并等待用户选择
	if (dialog.exec() == QDialog::Accepted) {
		return layerComboBox.currentText();
	}

	return QString();
}

// 凸包计算
void MainWindow::computeConvexHull() {
	QString selectedLayerName = selectLayer("凸包计算", true);
	if (selectedLayerName.isEmpty()) {
		QMessageBox::warning(this, "凸包计算", "未选择图层或选择无效");
		return;
	}

	auto it = mmapNameLayer.find(selectedLayerName);
	if (it == mmapNameLayer.end()) {
		QMessageBox::warning(this, "凸包计算", "所选图层不存在");
		return;
	}

	try {
		ConvexHullCalculator calculator;
		Store convexHullStore = calculator.compute(it->second.store);

		int dotIndex = selectedLayerName.lastIndexOf('.');
		QString baseName = selectedLayerName.left(dotIndex);
		QString extension = selectedLayerName.mid(dotIndex);
		QString hullLayerName = baseName + "_ConvexHull" + extension;

		mmapNameLayer[hullLayerName] = { convexHullStore, new QGraphicsItemGroup(), QPen(Qt::red), QBrush(Qt::transparent), false, "", "" };
		updateLayerTree();
		drawLayers();
	}
	catch (const std::invalid_argument& e) {
		QMessageBox::warning(this, "凸包计算", e.what());
	}
}

// Delaunay三角剖分
void MainWindow::computeDelaunay() {
	QString selectedLayerName = selectLayer("Delaunay三角剖分", true);
	if (selectedLayerName.isEmpty()) return;

	Layer& layer = mmapNameLayer[selectedLayerName];
	if (layer.store.getStorePoints().empty()) {
		QMessageBox::warning(this, "警告", "该图层没有点要素进行Delaunay三角剖分");
		return;
	}

	const geos::geom::GeometryFactory* factory = geos::geom::GeometryFactory::getDefaultInstance();
	std::vector<geos::geom::Coordinate> coords;

	for (const auto& point : layer.store.getStorePoints()) {
		QPointF epsgPoint = myGraphicsView::wgs84ToEPSG3857(point->getX(), point->getY());
		coords.emplace_back(epsgPoint.x(), epsgPoint.y());
	}

	std::unique_ptr<geos::geom::MultiPoint> multiPoint(factory->createMultiPoint(coords));

	geos::triangulate::DelaunayTriangulationBuilder dtb;
	dtb.setSites(*multiPoint);

	std::unique_ptr<geos::geom::GeometryCollection> triangles(dtb.getTriangles(*factory));

	Store delaunayStore;
	for (size_t i = 0; i < triangles->getNumGeometries(); ++i) {
		const geos::geom::Geometry* geom = triangles->getGeometryN(i);
		const geos::geom::Polygon* polygon = dynamic_cast<const geos::geom::Polygon*>(geom);
		if (polygon) {
			delaunayStore.addPolygon(createMyPolygonFromGeometry(polygon));
		}
	}

	// 创建新的图层
	int dotIndex = selectedLayerName.lastIndexOf('.');
	QString baseName = selectedLayerName.left(dotIndex);
	QString extension = selectedLayerName.mid(dotIndex);
	QString delaunayLayerName = baseName + "_Delaunay" + extension;
	mmapNameLayer[delaunayLayerName] = { delaunayStore, new QGraphicsItemGroup(), QPen(Qt::black), QBrush(Qt::cyan) };
	updateLayerTree();
	drawLayers();

	QMessageBox::information(this, "Delaunay三角剖分", "Delaunay三角剖分图层已生成");
	Logger::getInstance().logFunctionSuccess("Delaunay三角剖分图层生成");
}

// Voronoi图
void MainWindow::computeVoronoi() {
	QString selectedLayerName = selectLayer("Voronoi图", true);
	if (selectedLayerName.isEmpty()) return;

	Layer& layer = mmapNameLayer[selectedLayerName];
	if (layer.store.getStorePoints().empty()) {
		QMessageBox::warning(this, "警告", "该图层没有点要素进行Voronoi图计算");
		return;
	}

	const geos::geom::GeometryFactory* factory = geos::geom::GeometryFactory::getDefaultInstance();
	std::vector<geos::geom::Coordinate> coords;

	for (const auto& point : layer.store.getStorePoints()) {
		QPointF epsgPoint = myGraphicsView::wgs84ToEPSG3857(point->getX(), point->getY());
		coords.emplace_back(epsgPoint.x(), epsgPoint.y());
	}

	std::unique_ptr<geos::geom::MultiPoint> multiPoint(factory->createMultiPoint(coords));

	geos::triangulate::VoronoiDiagramBuilder vdb;
	vdb.setSites(*multiPoint);

	std::unique_ptr<geos::geom::GeometryCollection> cells(vdb.getDiagram(*factory));

	Store voronoiStore;
	for (size_t i = 0; i < cells->getNumGeometries(); ++i) {
		const geos::geom::Geometry* geom = cells->getGeometryN(i);
		const geos::geom::Polygon* polygon = dynamic_cast<const geos::geom::Polygon*>(geom);
		if (polygon) {
			voronoiStore.addPolygon(createMyPolygonFromGeometry(polygon));
		}
	}

	// 创建新的图层
	int dotIndex = selectedLayerName.lastIndexOf('.');
	QString baseName = selectedLayerName.left(dotIndex);
	QString extension = selectedLayerName.mid(dotIndex);
	QString voronoiLayerName = baseName + "_Voronoi" + extension;
	mmapNameLayer[voronoiLayerName] = { voronoiStore, new QGraphicsItemGroup(), QPen(Qt::black), QBrush(Qt::yellow) };
	updateLayerTree();
	drawLayers();

	QMessageBox::information(this, "Voronoi 图计算", "Voronoi 图层已生成");
	Logger::getInstance().logFunctionSuccess("Voronoi 图层已生成");
}

// 从GEOS几何对象创建myPolygon
myPolygon* MainWindow::createMyPolygonFromGeometry(const geos::geom::Geometry* geometry) {
	const geos::geom::Polygon* polygon = dynamic_cast<const geos::geom::Polygon*>(geometry);
	if (!polygon) return nullptr;

	std::vector<Point> exteriorPoints;
	std::unique_ptr<geos::geom::CoordinateSequence> exteriorCoords(polygon->getExteriorRing()->getCoordinates());
	for (std::size_t i = 0; i < exteriorCoords->size(); ++i) {
		const geos::geom::Coordinate& c = exteriorCoords->getAt(i);
		Point geoPoint = myGraphicsView::epsg3857ToWGS84(c.x, c.y);
		exteriorPoints.emplace_back(geoPoint.getX(), geoPoint.getY());
	}
	LineString exteriorRing(exteriorPoints);

	std::vector<LineString> interiorRings;
	for (std::size_t i = 0; i < polygon->getNumInteriorRing(); ++i) {
		std::vector<Point> interiorPoints;
		std::unique_ptr<geos::geom::CoordinateSequence> interiorCoords(polygon->getInteriorRingN(i)->getCoordinates());
		for (std::size_t j = 0; j < interiorCoords->size(); ++j) {
			const geos::geom::Coordinate& c = interiorCoords->getAt(j);
			Point geoPoint = myGraphicsView::epsg3857ToWGS84(c.x, c.y);
			interiorPoints.emplace_back(geoPoint.getX(), geoPoint.getY());
		}
		interiorRings.emplace_back(interiorPoints);
	}

	return new myPolygon(exteriorRing, interiorRings);
}

// 生成缓冲区
void MainWindow::computeBuffer() {
	QStringList layerNames;
	for (const auto& layer : mmapNameLayer) {
		layerNames.append(layer.first); // 获取图层名称
	}

	// 缓冲区参数对话框
	mpctrlBufferDialog = new BufferDialog(layerNames, this);
	mpctrlBufferDialog->resize(100, 160);
	if (mpctrlBufferDialog->exec() == QDialog::Accepted) {
		double radius = mpctrlBufferDialog->getBufferRadius();
		QString selectedLayer = mpctrlBufferDialog->getSelectedLayer();
		bool merge = mpctrlBufferDialog->isMergeChecked(); // 获取是否融合重叠部分

		if (selectedLayer.isEmpty()) {
			QMessageBox::warning(this, "警告", "请正确选择图层并输入有效的缓冲区半径！");
			Logger::getInstance().logUserInputWarning(radius);
			return;
		}

		Layer& layer = mmapNameLayer[selectedLayer];
		BufferAnalyzer bufferAnalyzer;
		Store bufferStore = bufferAnalyzer.computeBuffer(layer.store, radius, merge);

		int dotIndex = selectedLayer.lastIndexOf('.');
		QString baseName = selectedLayer.left(dotIndex);
		QString extension = selectedLayer.mid(dotIndex);
		QString bufferLayerName = baseName + "_Buffer" + extension;

		mmapNameLayer[bufferLayerName] = { bufferStore, new QGraphicsItemGroup(), QPen(Qt::black), QBrush(Qt::yellow) };
		updateLayerTree();
		drawLayers();
		QMessageBox::information(this, "缓冲区分析", "缓冲区图层已生成");
		Logger::getInstance().logFunctionSuccess("缓冲区分析成功");
	}
	delete mpctrlBufferDialog;
}

// 叠加分析
void MainWindow::overlayAnalysis() {
	// 创建一个对话框
	QDialog dialog(this);
	dialog.setWindowTitle("叠加分析设置");

	// 创建布局
	QVBoxLayout layout(&dialog);

	// 添加图层选择
	QComboBox layer1ComboBox;
	QComboBox layer2ComboBox;
	QStringList layerNames;
	for (const auto& layer : mmapNameLayer) {
		layerNames.append(layer.first);
	}
	layer1ComboBox.addItems(layerNames);
	layer2ComboBox.addItems(layerNames);

	layout.addWidget(new QLabel("选择图层1"));
	layout.addWidget(&layer1ComboBox);
	layout.addWidget(new QLabel("选择图层2"));
	layout.addWidget(&layer2ComboBox);

	// 添加叠加分析类型选择
	QComboBox operationComboBox;
	operationComboBox.addItems({ "擦除(图层1被图层2擦除)", "交集取反", "联合", "相交" });
	layout.addWidget(new QLabel("选择叠加分析类型"));
	layout.addWidget(&operationComboBox);

	// 添加确定和取消按钮
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	layout.addWidget(&buttonBox);

	// 连接信号与槽
	connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	// 显示对话框并处理用户输入
	if (dialog.exec() == QDialog::Accepted) {
		QString layer1Name = layer1ComboBox.currentText();
		QString layer2Name = layer2ComboBox.currentText();
		QString operationName = operationComboBox.currentText();

		// 确保两个图层不为空且不同
		if (layer1Name.isEmpty() || layer2Name.isEmpty() || layer1Name == layer2Name) {
			QMessageBox::warning(this, "无效选择", "请选择不同的两个图层进行叠加分析。");
			return;
		}

		Layer& layer1 = mmapNameLayer[layer1Name];
		Layer& layer2 = mmapNameLayer[layer2Name];

		// 确定叠加分析类型
		OverlayAnalysis::OperationType operationType = OverlayAnalysis::Intersection;
		QString resultLayerName;
		if (operationName == "擦除(图层1被图层2擦除)") {
			operationType = OverlayAnalysis::Difference;
			resultLayerName = layer1Name + "被" + layer2Name + "擦除";
		}
		else if (operationName == "交集取反") {
			operationType = OverlayAnalysis::SymDifference;
			resultLayerName = layer1Name + "与" + layer2Name + "交集取反";
		}
		else if (operationName == "联合") {
			operationType = OverlayAnalysis::Union;
			resultLayerName = layer1Name + "与" + layer2Name + "联合";
		}
		else if (operationName == "相交") {
			operationType = OverlayAnalysis::Intersection;
			resultLayerName = layer1Name + "与" + layer2Name + "相交";
		}

		// 执行叠加分析
		Store resultStore = OverlayAnalysis::performOverlay(layer1.store, layer2.store, operationType);

		// 创建新的图层
		// 生成随机数种子
		srand(time(nullptr));
		// 获取随机颜色
		QColor randomColor = QColor::fromRgb(rand() % 256, rand() % 256, rand() % 256);
		mmapNameLayer[resultLayerName] = { resultStore, new QGraphicsItemGroup(), QPen(Qt::black), QBrush(randomColor) };
		updateLayerTree();
		drawLayers();

		QMessageBox::information(this, "叠加分析", "叠加分析结果图层已生成: " + resultLayerName);
	}
}


// *****************************************栅格数据分析部分******************************************

void MainWindow::extractRasterMask() {
	// 栅格掩膜提取
	//QMessageBox::information(this, "栅格掩膜提取", "执行栅格掩膜提取");
	CoverDialog* customWindow = new CoverDialog(this);
	customWindow->setWindowTitle("栅格分析");
	bool* _tag = new bool(0);
	connect(customWindow, &CoverDialog::confirmClicked, this, [=]() {
		*_tag = 1;
		QMessageBox::information(this, "栅格掩膜提取", "正在执行栅格掩膜提取");
		// 执行栅格掩膜提取操作
		qDebug() << "执行栅格掩膜提取操作";
		gdal_engine beCovered(customWindow->mSelectedFile1, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
		qDebug() << customWindow->mSelectedFile1 << customWindow->mSelectedFile2;
		gdal_engine Cover(customWindow->mSelectedFile2, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
		std::vector<float>  posBeCovered = beCovered.mvGetposition();
		std::vector<float> posCover = Cover.mvGetposition();
		qDebug() << "被裁切栅格" << posBeCovered[0] << "  " << posBeCovered[1] << "  " << posBeCovered[2] << "  " << posBeCovered[3];
		qDebug() << "裁切栅格" << posCover[0] << "  " << posCover[1] << "  " << posCover[2] << "  " << posCover[3];

		qDebug() << posBeCovered[3] << "  " << posCover[1] << "  " << posBeCovered[2] << "  " << posCover[0];
		if ((posBeCovered[3] > posCover[1]) && (posBeCovered[2] > posCover[0]))
		{
			//QMessageBox::information(this, "栅格掩膜提取", "执行栅格掩膜提取操作");
			float pos[4];
			if (posBeCovered[0] > posCover[0])
			{
				pos[0] = posBeCovered[0];
			}
			else
			{
				pos[0] = posCover[0];
			};
			if (posBeCovered[1] > posCover[1])
			{
				pos[1] = posBeCovered[1];
			}
			else
			{
				pos[1] = posCover[1];
			}
			if (posBeCovered[2] > posCover[2])
			{
				pos[2] = posCover[2];

			}
			else
			{
				pos[2] = posBeCovered[2];
			}
			if (posBeCovered[3] > posCover[3])
			{
				pos[3] = posCover[3];
			}
			else
			{
				pos[3] = posBeCovered[3];
			};
			qDebug() << "pos" << pos[0] << "  " << pos[1] << "  " << pos[2] << "  " << pos[3];
			//get field
			GDALAllRegister();
			GDALDataset* srcDataset = static_cast<GDALDataset*>(GDALOpen(beCovered.mpcQPath, GA_ReadOnly));
			GDALDriver* driver = srcDataset->GetDriver();
			GDALDataset* srcDataseting = static_cast<GDALDataset*>(GDALOpen(Cover.mpcQPath, GA_ReadOnly));
			GDALDriver* drivering = srcDataseting->GetDriver();

			QString folderPath = customWindow->mSelectedFile3;
			QString fileName = customWindow->mSelectedFile4;
			QString filePath = QDir(folderPath).filePath(fileName);
			char* destPath;
			const char* charStr = filePath.toUtf8().constData();//UTF8格式编码
			size_t length = std::strlen(charStr) + 1; // 加 1 是为了包括结尾的空字符
			char* TruecharStr = new char[length]; // 动态分配内存
			beCovered.my_strcpy(TruecharStr, charStr); // 复制字符串内容
			GDALDataset* destDataset = driver->CreateCopy(TruecharStr, srcDataset, FALSE, nullptr, nullptr, nullptr);

			int BandNums = (beCovered.mpvBand).size();
			//
			qDebug() << "NUMS" << BandNums;
			for (int i = 0; i < BandNums; i++)
			{
				int tag = 0;
				std::vector<unsigned char> data(beCovered.mnheight * beCovered.mnwidth + 1);
				std::vector<unsigned char> dataing(Cover.mnheight * Cover.mnwidth + 1);

				qDebug() << "enter" << beCovered.mnheight << "  " << beCovered.mnwidth << "  " << beCovered.mnheight * beCovered.mnwidth + 1;
				destDataset->GetRasterBand(i + 1)->RasterIO(GF_Read, 0, 0, beCovered.mnwidth, beCovered.mnheight, data.data(), beCovered.mnwidth, beCovered.mnheight, GDT_Byte, 0, 0);
				srcDataseting->GetRasterBand(i + 1)->RasterIO(GF_Read, 0, 0, Cover.mnwidth, Cover.mnheight, dataing.data(), Cover.mnwidth, Cover.mnheight, GDT_Byte, 0, 0);
				for (int y = 0; y < beCovered.mnheight; ++y) {
					for (int x = 0; x < beCovered.mnwidth; ++x) {
						//qDebug() << "NUMS" << y * beCovered.mnwidth + x;
						std::pair<float, float> LonLat = beCovered.mCoordinate.GetLonLatByPixel(y * beCovered.mnwidth + x);
						if (tag % 100000 == 0)
						{
							qDebug() << "LonLat" << LonLat.first << "  " << LonLat.second;
						}
						if ((LonLat.first > pos[1]) && (LonLat.first < pos[3]) && (LonLat.second > pos[0]) && (LonLat.second < pos[2]))
						{
							int _GRBtag = dataing[Cover.mCoordinate.GetNumsOfBand(LonLat.first, LonLat.second).first + Cover.mCoordinate.GetNumsOfBand(LonLat.first, LonLat.second).second * Cover.mnwidth];
							if ((_GRBtag == 255) || (_GRBtag == 0))
							{
								data[y * beCovered.mnwidth + x] = 0;
							}

							if (y * beCovered.mnwidth + x % 100000 == 0)
							{
								qDebug() << "NUMS" << y * beCovered.mnwidth + x;
							};
						}
						else
						{
							data[y * beCovered.mnwidth + x] = 0;
							tag++;

							if (tag % 1000000 == 0)
							{
								//qDebug() << "tag" << tag;
							}
						};
					}
				};
				destDataset->GetRasterBand(i + 1)->RasterIO(GF_Write, 0, 0, beCovered.mnwidth, beCovered.mnheight, data.data(), beCovered.mnwidth, beCovered.mnheight, GDT_Byte, 0, 0);
			}
			GDALClose(destDataset);
			mstrRasterFileName = filePath;
			QString _message = "栅格位于：" + filePath;
			QMessageBox::information(this, "栅格掩膜提取成功", _message);
		};
		});
	customWindow->resize(400, 300);
	customWindow->show();
	qDebug() << "TAGING_NUMber" << *_tag;


};

char* Translate(const char* str)
{
	const char* charStr1 = QString(str).toUtf8().constData();
	size_t length = std::strlen(charStr1) + 1; // 加 1 是为了包括结尾的空字符
	char* mstrQPath = new char[length]; // 动态分配内存
	while ((*mstrQPath++ = *charStr1++) != '\0');
};

void MainWindow::extractVectorMask() {
	// 矢量掩膜提取
	QMessageBox::information(this, "矢量掩膜提取", "执行矢量掩膜提取");
	CoverDialog* customWindow = new CoverDialog(this);
	customWindow->resize(400, 300);
	customWindow->show();
	connect(customWindow, &CoverDialog::confirmClicked, this, [=]() {
		gdal_engine gtif(QString("C:\\Users\\dell\\Desktop\\BigHomeWord_Tif\\IIF.tif"), mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);


		ImageCut imageCut;
		const char* charStr = customWindow->mSelectedFile1.toUtf8().constData();//UTF8格式编码
		size_t length = std::strlen(charStr) + 1; // 加 1 是为了包括结尾的空字符
		char* mstrQPath = new char[length]; // 动态分配内存
		gtif.my_strcpy(mstrQPath, charStr); // 复制字符串内容
		qDebug() << "setSrcImageFilename" << "mstrQPath" << mstrQPath;

		imageCut.setSrcImageFilename(mstrQPath);


		const char* charStr1 = (customWindow->mSelectedFile3 + "//" + customWindow->mSelectedFile4).toUtf8().constData();
		qDebug() << customWindow->mSelectedFile3 + customWindow->mSelectedFile4;
		length = std::strlen(charStr1) + 1; // 加 1 是为了包括结尾的空字符
		char* mstrQPath1 = new char[length]; // 动态分配内存
		gtif.my_strcpy(mstrQPath1, charStr1); // 复制字符串内容
		imageCut.setDstImageFilename(mstrQPath1);
		qDebug() << mstrQPath;

		const char* charStr2 = customWindow->mSelectedFile2.toUtf8().constData();
		length = std::strlen(charStr2) + 1; // 加 1 是为了包括结尾的空字符
		mstrQPath = new char[length]; // 动态分配内存
		gtif.my_strcpy(mstrQPath, charStr2); // 复制字符串内容

		imageCut.setShapeFilename(mstrQPath);


		//qDebug() << "输出为" << ATT;
		const char* charStr3 = ("GTiff");
		length = std::strlen(charStr3) + 1; // 加 1 是为了包括结尾的空字符
		mstrQPath = new char[length]; // 动态分配内存
		gtif.my_strcpy(mstrQPath, charStr3); // 复制字符串内容
		imageCut.setImageFormat(mstrQPath);


		imageCut.ImageCutByAOI();

		QString _message = "栅格位于：" + customWindow->mSelectedFile3 + customWindow->mSelectedFile4;
		QMessageBox::information(this, "栅格掩膜提取成功", _message);
		});

}

void MainWindow::neighborhoodMean() {
	// 邻域均值计算
	QMessageBox::information(this, "邻域均值计算", "执行邻域均值计算");
	GDALAllRegister();
	// 打开栅格文件
	gdal_engine gtif(mstrRasterFileName, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
	gtif.tifValueProcess(mpctrlView, mpctrlScene, 0, AVERAGE);
	QMessageBox::information(this, "领域分析", "执行完毕");

}

void MainWindow::neighborhoodMax() {
	// 邻域最大值计算
	QMessageBox::information(this, "邻域最大值计算", "执行邻域最大值计算");
	gdal_engine mg_Gtif(mstrRasterFileName, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
	//mg_Gtif.update_coordinate();
	mg_Gtif.tifValueProcess(mpctrlView, mpctrlScene, 0, MAXIMUM);
	QMessageBox::information(this, "领域分析", "执行完毕");
}

void MainWindow::neighborhoodMin() {
	// 邻域最小值计算
	QMessageBox::information(this, "邻域最小值计算", "执行邻域最小值计算");
	gdal_engine gtif(mstrRasterFileName, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
	gtif.tifValueProcess(mpctrlView, mpctrlScene, 0, MINMUM);
	QMessageBox::information(this, "领域分析", "执行完毕");
}

void MainWindow::drawGrayHistogram() {
	// 绘制灰度直方图
	QMessageBox::information(this, "绘制灰度直方图", "绘制灰度直方图");
	newGUIforGrayHistogram(mstrRasterFileName);
}

// 绘制灰度直方图
void MainWindow::newGUIforGrayHistogram(QString fileName) {
	int nBuckets = 256;

	GDALAllRegister();

	// 打开栅格文件
	gdal_engine gtif(fileName, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);

	// 获取图像大小
	int nXSize = gtif.mpvBand[0]->GetXSize();
	int nYSize = gtif.mpvBand[0]->GetYSize();

	// 读取图像数据
	std::vector<unsigned char> imageData(nXSize * nYSize);
	gtif.mpvBand[0]->RasterIO(GF_Read, 0, 0, nXSize, nYSize, imageData.data(), nXSize, nYSize, GDT_Byte, 0, 0);

	// 将图像数据转换为QImage以便显示
	QImage img(imageData.data(), nXSize, nYSize, nXSize, QImage::Format_Grayscale8);
	displayImage(img, "原始图像");

	// 计算原始图像的直方图
	std::vector<GUIntBig> histogram = computeHistogram(imageData, nBuckets);

	// 应用直方图均衡化
	std::vector<unsigned char> equalizedImageData = equalizeHistogram(imageData, histogram);

	// 将均衡化后的图像数据转换为QImage以便显示
	QImage equalizedImg(equalizedImageData.data(), nXSize, nYSize, nXSize, QImage::Format_Grayscale8);
	displayImage(equalizedImg, "均衡化后的图像");

	// 重新计算均衡化后的直方图
	std::vector<GUIntBig> equalizedHistogram = computeHistogram(equalizedImageData, nBuckets);

	// 绘制原始直方图和均衡化后的直方图
	int histSize = histogram.size();
	int hist_w = 512; // 直方图的宽度
	int hist_h = 400; // 直方图的高度
	int bin_w = hist_w / histSize; // 每个bin的宽度

	QDialog* dialog = new QDialog(this);
	dialog->setWindowTitle("灰度直方图");

	// 创建一个QGraphicsScene和QGraphicsView
	QGraphicsScene* scene = new QGraphicsScene(0, 0, hist_w, hist_h);

	// 将直方图归一化到指定高度
	GUIntBig max_val = *std::max_element(histogram.begin(), histogram.end());
	std::vector<int> normalizedHist(histSize);
	for (int i = 0; i < histSize; i++) {
		normalizedHist[i] = ((double)histogram[i] / max_val) * hist_h;
	}

	// 将均衡化后的直方图归一化到指定高度
	GUIntBig max_equalized_val = *std::max_element(equalizedHistogram.begin(), equalizedHistogram.end());
	std::vector<int> normalizedEqualizedHist(histSize);
	for (int i = 0; i < histSize; i++) {
		normalizedEqualizedHist[i] = ((double)equalizedHistogram[i] / max_equalized_val) * hist_h;
	}

	// 绘制原始直方图
	for (int i = 0; i < histSize; i++) {
		int x = i * bin_w;
		int y = hist_h - normalizedHist[i];
		scene->addLine(x, hist_h, x, y, QPen(Qt::black));
	}

	// 绘制均衡化后的直方图
	for (int i = 0; i < histSize; i++) {
		int x = i * bin_w + bin_w / 2; // Offset to distinguish from the original histogram
		int y = hist_h - normalizedEqualizedHist[i];
		scene->addLine(x, hist_h, x, y, QPen(Qt::red));
	}

	// 创建一个QGraphicsView来显示场景
	QGraphicsView* view = new QGraphicsView(scene);
	view->setWindowTitle(QString::fromStdString("Gray Histogram"));

	// 创建一个垂直布局并将QGraphicsView添加到布局中
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(view);
	dialog->setLayout(layout);
	// 显示对话框
	dialog->exec();
	// 清理内存
	delete dialog;
}

void MainWindow::createTrueColorImage() {
	// 创建真色彩影像
	QMessageBox::information(this, "创建真色彩影像", "创建真色彩影像");
	gdal_engine gtif(mstrRasterFileName, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
	if (gtif.mpvBand.size() < 3)
		QMessageBox::information(this, "创建真色彩影像", "选择的波段不是3个，无效选择");
	else
	{
		gtif.BandChose(0, 1, 2);
		gtif.quickTifShow(mpctrlView, mpctrlScene, gtif.mdSCale, 1);
	};

}

void MainWindow::createFalseColorImage() {
	// 创建伪色彩影像
	QMessageBox::information(this, "创建伪色彩影像", "创建伪色彩影像");
	gdal_engine gtif(mstrRasterFileName, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
	int num = gtif.mpvBand.size();
	CustomDialog dialog(num, this); // 传入正确的父对象
	int result = dialog.exec();

	if (result == QDialog::Accepted) {
		std::vector<int> Bands = dialog.mvChosenBand;
		qDebug() << "波段数为" << Bands.size() << "分别是" << Bands[0] << Bands[1] << Bands[2];
		if (Bands.size() != 3) {
			QMessageBox::information(this, "创建伪色彩影像", "选择的波段不是3个，无效选择");
		}
		else {
			gtif.BandChose(Bands[0], Bands[1], Bands[2]);
			gtif.quickTifShow(mpctrlView, mpctrlScene, gtif.mdSCale, 1);
		}
	}
}
void  MainWindow::smoothTif()
{
	//QMessageBox::information(this, "创建伪色彩影像", "创建伪色彩影像");
	gdal_engine gtif(mstrRasterFileName, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
	int num = gtif.mpvBand.size();
	CustomDialog dialog(num, this); // 传入正确的父对象
	int result = dialog.exec();
	gtif.fillHoles(mpctrlView);
}
void MainWindow::D8plusTif()
{
	gdal_engine gtif(mstrRasterFileName, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
	gtif.D8FlowDirectionAnalysis(mpctrlView);
};
void MainWindow::AspectAnaTif()
{
	gdal_engine gtif(mstrRasterFileName, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
	gtif.AspectAnalysis(mpctrlView);
};
void MainWindow::SlopeAnaTif()
{
	gdal_engine gtif(mstrRasterFileName, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
	gtif.SlopeAnalysis(mpctrlView, 30, 30);
};

void MainWindow::RockAna()
{

	RockMainWindow* rockWindow = new RockMainWindow;


	if (rockWindow == nullptr) {
		rockWindow = new RockMainWindow(this);
	}

	if (!rockWindow->isVisible()) {
		rockWindow->show();
	}
	else {
		rockWindow->raise(); // 如果窗口已经被隐藏，则将其置于最前面
	}
}



// *****************************************日志功能部分******************************************

void MainWindow::exportLogFile() {
	// 获取用户指定的文件路径
	QString fileName = QFileDialog::getSaveFileName(this, "导出日志文件", "", "文本文件 (*.txt)");
	if (fileName.isEmpty()) return;

	// 定义同目录下的日志文件路径
	QString logFilePath = "./log.txt";

	// 打开源日志文件
	QFile logFile(logFilePath);
	if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::warning(this, "导出日志文件", "无法打开源日志文件: " + logFilePath);
		return;
	}

	// 打开目标日志文件
	QFile outFile(fileName);
	if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, "导出日志文件", "无法打开目标文件进行写入: " + fileName);
		logFile.close();
		return;
	}

	// 读取源日志文件内容并写入目标文件
	QTextStream in(&logFile);
	QTextStream out(&outFile);
	out << in.readAll();

	// 关闭文件
	logFile.close();
	outFile.close();

	// 显示成功信息
	QMessageBox::information(this, "导出日志文件", "日志文件已成功导出到: " + fileName);
}


// *****************************************撤消与重做部分******************************************

void MainWindow::undo() {
	// 撤销操作
	mRemote.pressUndo();
}

void MainWindow::redo() {
	// 重做操作
	mRemote.pressRedo();
}

// *****************************************图层管理与符号系统设置部分******************************************

// 更新符号系统显示
void MainWindow::updateSymList() {
	// 首先清空
	mpctrlSymList->clear();

	// 遍历所有图层来更新符号系统
	for (auto& layer : mmapNameLayer) {
		if (layer.second.isRaster) {
			continue; // 忽略栅格图层
		}

		QString layerName = layer.first;
		const Store& store = layer.second.store;

		// 添加图层名称作为分隔符
		QListWidgetItem* layerNameItem = new QListWidgetItem(QString("图层: %1").arg(layerName));
		layerNameItem->setBackground(Qt::lightGray);
		mpctrlSymList->addItem(layerNameItem);

		// 根据图层类型设置符号系统选项
		if (!store.getStorePoints().empty()) {
			// 点要素图层
			addPointSymbolSettings(layerName, layer.second.pen);
		}
		else if (!store.getStoreLines().empty()) {
			// 线要素图层
			addLineSymbolSettings(layerName, layer.second.pen);
		}
		else if (!store.getStorePolygons().empty()) {
			// 面要素图层
			addPolygonSymbolSettings(layerName, layer.second);
		}
	}
}

void MainWindow::addPointSymbolSettings(const QString& layerName, QPen& pen) {
	// 点颜色设置
	QWidget* penColorWidget = new QWidget();
	QHBoxLayout* penColorLayout = new QHBoxLayout(penColorWidget);
	penColorLayout->addWidget(new QLabel("点颜色:"));
	QPushButton* penColorButton = new QPushButton(pen.color().name());
	connect(penColorButton, &QPushButton::clicked, this, [this, layerName, &pen, penColorButton]() {
		QColor color = QColorDialog::getColor(pen.color(), this, "选择点颜色");
		if (color.isValid()) {
			pen.setColor(color);
			penColorButton->setText(color.name());
			drawLayers();
		}
		});
	penColorLayout->addWidget(penColorButton);
	penColorLayout->setContentsMargins(0, 0, 0, 0);
	QListWidgetItem* penColorItem = new QListWidgetItem();
	penColorItem->setSizeHint(penColorWidget->sizeHint());
	mpctrlSymList->addItem(penColorItem);
	mpctrlSymList->setItemWidget(penColorItem, penColorWidget);

	// 点样式设置
	QWidget* pointStyleWidget = new QWidget();
	QHBoxLayout* pointStyleLayout = new QHBoxLayout(pointStyleWidget);
	pointStyleLayout->addWidget(new QLabel("点样式:"));
	QComboBox* pointStyleComboBox = new QComboBox();
	pointStyleComboBox->addItems({ "默认", "自定义" });
	connect(pointStyleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, &pen](int index) {
		switch (index) {
		case 0:
			restoreDefaultPointStyle();
			break;
		case 1:
			// 自定义样式

			chooseCustomPointImage();
			break;
		}
		drawLayers();
		});
	pointStyleLayout->addWidget(pointStyleComboBox);
	pointStyleLayout->setContentsMargins(0, 0, 0, 0);
	QListWidgetItem* pointStyleItem = new QListWidgetItem();
	pointStyleItem->setSizeHint(pointStyleWidget->sizeHint());
	mpctrlSymList->addItem(pointStyleItem);
	mpctrlSymList->setItemWidget(pointStyleItem, pointStyleWidget);
}

void MainWindow::chooseCustomPointImage() {
	QString filePath = QFileDialog::getOpenFileName(this, "选择自定义点样式图片", "", "Images (*.png *.xpm *.jpg)");
	if (!filePath.isEmpty()) {
		QPixmap customPixmap(filePath);
		mDraw.setCustomPointImage(customPixmap);
		drawLayers();
	}
}

void MainWindow::restoreDefaultPointStyle() {
	QPen defaultPen(Qt::black);  // 替换为默认的点样式
	mDraw.setDefaultPointStyle(defaultPen);
	drawLayers();
}

void MainWindow::addLineSymbolSettings(const QString& layerName, QPen& pen) {
	// 线条颜色设置
	QWidget* penColorWidget = new QWidget();
	QHBoxLayout* penColorLayout = new QHBoxLayout(penColorWidget);
	penColorLayout->addWidget(new QLabel("线条颜色:"));
	QPushButton* penColorButton = new QPushButton(pen.color().name());
	connect(penColorButton, &QPushButton::clicked, this, [this, layerName, &pen, penColorButton]() {
		QColor color = QColorDialog::getColor(pen.color(), this, "选择线条颜色");
		if (color.isValid()) {
			pen.setColor(color);
			penColorButton->setText(color.name());
			drawLayers();
		}
		});
	penColorLayout->addWidget(penColorButton);
	penColorLayout->setContentsMargins(0, 0, 0, 0);
	QListWidgetItem* penColorItem = new QListWidgetItem();
	penColorItem->setSizeHint(penColorWidget->sizeHint());
	mpctrlSymList->addItem(penColorItem);
	mpctrlSymList->setItemWidget(penColorItem, penColorWidget);

	// 线样式设置
	QWidget* lineStyleWidget = new QWidget();
	QHBoxLayout* lineStyleLayout = new QHBoxLayout(lineStyleWidget);
	lineStyleLayout->addWidget(new QLabel("线样式:"));
	QComboBox* lineStyleComboBox = new QComboBox();
	lineStyleComboBox->addItems({ "实线", "虚线", "点线", "双点划线", "自定义" });
	connect(lineStyleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, &pen](int index) {
		switch (index) {
		case 0:
			restoreDefaultLineStyle();
			pen.setStyle(Qt::SolidLine);
			break;
		case 1:
			restoreDefaultLineStyle();
			pen.setStyle(Qt::DashLine);
			break;
		case 2:
			restoreDefaultLineStyle();
			pen.setStyle(Qt::DotLine);
			break;
		case 3:
			restoreDefaultLineStyle();
			pen.setStyle(Qt::DashDotLine);
			break;
		case 4:
			// 自定义
			chooseCustomLineImage();
			break;
		}
		mDraw.setCurrentLineStyle(pen);
		drawLayers();
		});
	lineStyleLayout->addWidget(lineStyleComboBox);
	lineStyleLayout->setContentsMargins(0, 0, 0, 0);
	QListWidgetItem* lineStyleItem = new QListWidgetItem();
	lineStyleItem->setSizeHint(lineStyleWidget->sizeHint());
	mpctrlSymList->addItem(lineStyleItem);
	mpctrlSymList->setItemWidget(lineStyleItem, lineStyleWidget);
}

void MainWindow::chooseCustomLineImage() {
	QString filePath = QFileDialog::getOpenFileName(this, "选择自定义线样式图片", "", "Images (*.png *.xpm *.jpg)");
	if (!filePath.isEmpty()) {
		QPixmap customPixmap(filePath);
		mDraw.setCustomLineImage(customPixmap);
		mDraw.setCurrentLineStyle(QPen(Qt::black));
		drawLayers();
	}
}

void MainWindow::restoreDefaultLineStyle() {
	QPen defaultPen(Qt::black, 2);
	mDraw.setDefaultLineStyle(defaultPen);
	mDraw.setCurrentLineStyle(defaultPen);  // 恢复为默认的线样式
	drawLayers();
}

void MainWindow::addPolygonSymbolSettings(const QString& layerName, const Layer& layer) {
	const Layer& layerData = layer;

	// 描边颜色设置
	QWidget* penColorWidget = new QWidget();
	QHBoxLayout* penColorLayout = new QHBoxLayout(penColorWidget);
	penColorLayout->addWidget(new QLabel("描边颜色:"));
	QPushButton* penColorButton = new QPushButton(layerData.pen.color().name());
	connect(penColorButton, &QPushButton::clicked, this, [this, layerName, penColorButton]() {
		Layer& layer = mmapNameLayer[layerName];
		QColor color = QColorDialog::getColor(layer.pen.color(), this, "选择描边颜色");
		if (color.isValid()) {
			layer.pen.setColor(color);
			penColorButton->setText(color.name());
			drawLayers();
		}
		});
	penColorLayout->addWidget(penColorButton);
	penColorLayout->setContentsMargins(0, 0, 0, 0);
	QListWidgetItem* penColorItem = new QListWidgetItem();
	penColorItem->setSizeHint(penColorWidget->sizeHint());
	mpctrlSymList->addItem(penColorItem);
	mpctrlSymList->setItemWidget(penColorItem, penColorWidget);

	// 填充颜色设置
	QWidget* brushColorWidget = new QWidget();
	QHBoxLayout* brushColorLayout = new QHBoxLayout(brushColorWidget);
	brushColorLayout->addWidget(new QLabel("填充颜色:"));
	QPushButton* brushColorButton = new QPushButton(layerData.brush.color().name());
	connect(brushColorButton, &QPushButton::clicked, this, [this, layerName, brushColorButton]() {
		Layer& layer = mmapNameLayer[layerName];
		QColor color = QColorDialog::getColor(layer.brush.color(), this, "选择填充颜色");
		if (color.isValid()) {
			layer.brush.setColor(color);
			brushColorButton->setText(color.name());
			drawLayers();
		}
		});
	brushColorLayout->addWidget(brushColorButton);
	brushColorLayout->setContentsMargins(0, 0, 0, 0);
	QListWidgetItem* brushColorItem = new QListWidgetItem();
	brushColorItem->setSizeHint(brushColorWidget->sizeHint());
	mpctrlSymList->addItem(brushColorItem);
	mpctrlSymList->setItemWidget(brushColorItem, brushColorWidget);

	// 填充样式设置
	QWidget* brushStyleWidget = new QWidget();
	QHBoxLayout* brushStyleLayout = new QHBoxLayout(brushStyleWidget);
	brushStyleLayout->addWidget(new QLabel("填充样式:"));
	QComboBox* brushStyleComboBox = new QComboBox();
	brushStyleComboBox->addItem("填充颜色");
	brushStyleComboBox->addItem("交叉影线");
	brushStyleComboBox->addItem("简单影线");
	brushStyleComboBox->addItem("点填充");
	brushStyleComboBox->setCurrentIndex(0);
	connect(brushStyleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, layerName, brushStyleComboBox, brushColorWidget, brushColorButton]() {
		Layer& layer = mmapNameLayer[layerName];
		int index = brushStyleComboBox->currentIndex();
		QBrush& brush = layer.brush;
		switch (index) {
		case 0:  // 填充颜色
			brush.setStyle(Qt::SolidPattern);
			brushColorButton->setText(layer.brush.color().name());
			break;
		case 1:  // 交叉影线
			brush.setStyle(Qt::CrossPattern);
			brushColorWidget->setVisible(true);
			break;
		case 2:  // 简单影线
			brush.setStyle(Qt::BDiagPattern);
			brushColorWidget->setVisible(true);
			break;
		case 3:  // 点填充
			brush.setStyle(Qt::Dense1Pattern);
			brushColorWidget->setVisible(true);
			break;
		}
		drawLayers();
		});
	brushStyleLayout->addWidget(brushStyleComboBox);
	brushStyleLayout->setContentsMargins(0, 0, 0, 0);
	QListWidgetItem* brushStyleItem = new QListWidgetItem();
	brushStyleItem->setSizeHint(brushStyleWidget->sizeHint());
	mpctrlSymList->addItem(brushStyleItem);
	mpctrlSymList->setItemWidget(brushStyleItem, brushStyleWidget);


}

void MainWindow::onLayerItemChanged(QTreeWidgetItem* item, int column) {
	if (column == 0) {
		updateSymList();
	}
}

// 更新图层树
void MainWindow::updateLayerTree() {
	disconnect(mpctrlLayerTree, &QTreeWidget::itemChanged, this, &MainWindow::onLayerItemChanged);

	// 保存当前选中状态
	QMap<QString, bool> layerCheckStates;
	for (int i = 0; i < mpctrlLayerTree->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = mpctrlLayerTree->topLevelItem(i);
		layerCheckStates[item->text(0)] = (item->checkState(0) == Qt::Checked);
	}

	mpctrlLayerTree->clear();
	mpctrlLayerComboBox->clear();

	for (const auto& layer : mmapNameLayer) {
		QTreeWidgetItem* item = new QTreeWidgetItem(mpctrlLayerTree);
		item->setText(0, layer.first);
		item->setCheckState(0, layerCheckStates.value(layer.first, true) ? Qt::Checked : Qt::Unchecked);

		// 根据图层内容设置图标
		if (layer.second.isRaster) {
			item->setIcon(0, QIcon("./icon/layerraster.png"));
		}
		else {
			if (!layer.second.store.getStorePoints().empty()) {
				item->setIcon(0, QIcon("./icon/layerpoint.png"));
			}
			else if (!layer.second.store.getStoreLines().empty()) {
				item->setIcon(0, QIcon("./icon/layerline.png"));
			}
			else if (!layer.second.store.getStorePolygons().empty()) {
				item->setIcon(0, QIcon("./icon/layerpolygon.png"));
			}
			else {
				// 如果图层是空的，根据用户选择的类型设置默认图标
				if (layer.second.vectorFilePath.contains("point")) {
					item->setIcon(0, QIcon("./icon/layerpoint.png"));
				}
				else if (layer.second.vectorFilePath.contains("line")) {
					item->setIcon(0, QIcon("./icon/layerline.png"));
				}
				else if (layer.second.vectorFilePath.contains("polygon")) {
					item->setIcon(0, QIcon("./icon/layerpolygon.png"));
				}
				else {
					item->setIcon(0, QIcon("./icon/layerunknown.png")); // 默认图标
				}
			}
			mpctrlLayerComboBox->addItem(layer.first);
		}
	}

	// 更新符号系统列表
	updateSymList();

	connect(mpctrlLayerTree, &QTreeWidget::itemChanged, this, &MainWindow::onLayerItemChanged);
}

// 绘制所有图层
void MainWindow::drawLayers() {
	mpctrlScene->clear();  // 清空现有场景内容

	// 调整初始线条宽度
	static_cast<myGraphicsView*>(mpctrlView)->adjustItemWidths();

	for (auto& layer : mmapNameLayer) {
		if (mpctrlLayerTree->findItems(layer.first, Qt::MatchExactly).first()->checkState(0) == Qt::Checked) {
			if (layer.second.isRaster) {
				// 使用栅格图层的文件路径重新加载并显示栅格图层
				gdal_engine gtif(layer.second.rasterFilePath, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
				gtif.quickTifShow(mpctrlView, mpctrlScene, gtif.mdSCale, 1); // 使用 gdal_engine 的方法绘制栅格图层
			}
			else {
				// 绘制矢量图层
				layer.second.group->addToGroup(mDraw.showVec(layer.second.store, *mpctrlScene, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat, mnViewWidth, mnViewHeight, layer.second.pen, layer.second.brush));
			}
		}
	}

	// 调整视图
	QRectF itemsBoundingRect = mpctrlScene->itemsBoundingRect();
	QGraphicsView* view = static_cast<QGraphicsView*>(centralWidget()->layout()->itemAt(0)->widget());
	view->fitInView(itemsBoundingRect, Qt::KeepAspectRatio);

	QRectF newSceneRect = mpctrlScene->itemsBoundingRect();
	mpctrlScene->setSceneRect(QRectF(-1e9, -1e9, 2e9, 2e9));

	// 模拟一次滚轮事件
	QWheelEvent wheelEvent(QPointF(0, 0), QPointF(0, 0), QPoint(0, 0), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::ScrollBegin, false);
	static_cast<myGraphicsView*>(mpctrlView)->wheelEvent(&wheelEvent);
}


// 开始编辑
bool MainWindow::startEditingLayer() {
	mstrCurrentEditingLayer = mpctrlLayerComboBox->currentText();
	if (mstrCurrentEditingLayer.isEmpty() && mIsPathAnalysisMode == false) {
		QMessageBox::warning(this, "警告", "请选择一个图层进行编辑");
		mpctrlEditComboBox->setCurrentText("编辑");
		return 0;
	}

	// 获取当前编辑图层的 store 对象
	auto it = mmapNameLayer.find(mstrCurrentEditingLayer);
	if (it != mmapNameLayer.end()) {
		mpCurrentEditingStore = &(it->second.store);
	}
	else {
		QMessageBox::warning(this, "警告", "无法找到选择的图层");
		mpctrlEditComboBox->setCurrentText("编辑");
		return 0;
	}

	// 启用编辑操作
	mpctrlView->setEditingEnabled(true);

	QMessageBox::information(this, "开始编辑", "开始编辑图层: " + mstrCurrentEditingLayer);
	return 1;
}

// 停止编辑
void MainWindow::stopEditingLayer() {
	if (mstrCurrentEditingLayer.isEmpty()) {
		QMessageBox::warning(this, "警告", "没有正在编辑的图层");
		mpctrlEditComboBox->setCurrentText("编辑");
		return;
	}

	// 禁用编辑操作
	mpctrlView->setEditingEnabled(false);

	QMessageBox::information(this, "停止编辑", "停止编辑图层: " + mstrCurrentEditingLayer);
	mpctrlEditComboBox->setCurrentText("编辑");
	mstrCurrentEditingLayer = "";
}

// 重绘编辑中的图层
void MainWindow::drawEditLayer() {
	mpctrlScene->clear();  // 清空现有场景内容
	for (auto& layer : mmapNameLayer) {
		if (mpctrlLayerTree->findItems(layer.first, Qt::MatchExactly).first()->checkState(0) == Qt::Checked) {
			if (layer.second.isRaster) {
				// 使用栅格图层的文件路径重新加载并显示栅格图层
				gdal_engine gtif(layer.second.rasterFilePath, mnViewWidth, mnViewHeight, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat);
				gtif.quickTifShow(mpctrlView, mpctrlScene, gtif.mdSCale, 1); // 使用 gdal_engine 的方法绘制栅格图层
			}
			else {
				// 绘制矢量图层
				layer.second.group->addToGroup(mDraw.showVec(layer.second.store, *mpctrlScene, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat, mnViewWidth, mnViewHeight, layer.second.pen, layer.second.brush));
			}
		}
	}
	mDraw.showVec(mAddedStore, *mpctrlScene, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat, mnViewWidth, mnViewHeight, QPen(Qt::black), QBrush(Qt::lightGray));
	mDraw.showVec(mEditedStore, *mpctrlScene, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat, mnViewWidth, mnViewHeight, QPen(Qt::black), QBrush(Qt::yellow));
	mDraw.setCurrentLineStyle(QPen(QColor(51, 255, 255), 2, Qt::SolidLine));
	mDraw.showVec(mSelectedFeatures, *mpctrlScene, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat, mnViewWidth, mnViewHeight, QPen(QColor(51, 255, 255), 2, Qt::SolidLine), QBrush(Qt::red, Qt::SolidPattern));
	mDraw.setCurrentLineStyle(QPen(Qt::black));
	mpctrlView->adjustItemWidths();
}


// 图层右键菜单
void MainWindow::onLayerTreeContextMenu(const QPoint& point) {
	QTreeWidgetItem* item = mpctrlLayerTree->itemAt(point);
	if (item) {
		QMenu contextMenu;
		QAction* zoomToLayerAction = new QAction("缩放到图层", this);
		QAction* deleteAction = new QAction("删除图层", this);
		QAction* moveToTopAction = new QAction("图层置顶", this);
		QAction* moveToBottomAction = new QAction("图层置底", this);
		contextMenu.addAction(zoomToLayerAction);
		contextMenu.addAction(deleteAction);
		contextMenu.addAction(moveToTopAction);
		contextMenu.addAction(moveToBottomAction);

		connect(zoomToLayerAction, &QAction::triggered, this, [this, item]() {
			zoomToLayer(item);
			});
		connect(deleteAction, &QAction::triggered, this, [this, item]() {
			deleteLayer(item);
			});
		connect(moveToTopAction, &QAction::triggered, this, [this, item]() {
			moveLayerToTop(item);
			});
		connect(moveToBottomAction, &QAction::triggered, this, [this, item]() {
			moveLayerToBottom(item);
			});

		contextMenu.exec(mpctrlLayerTree->viewport()->mapToGlobal(point));
	}
}

// 缩放到图层
void MainWindow::zoomToLayer(QTreeWidgetItem* item) {
	QString layerName = item->text(0);
	auto it = mmapNameLayer.find(layerName);
	if (it == mmapNameLayer.end()) {
		return; // 图层不存在
	}

	Layer& layer = it->second;
	double minX = std::numeric_limits<double>::max();
	double minY = std::numeric_limits<double>::max();
	double maxX = std::numeric_limits<double>::min();
	double maxY = std::numeric_limits<double>::min();

	if (layer.isRaster) {
		// 获取栅格图层的边界信息
		gdal_engine engine;
		engine.update_info(layer.rasterFilePath);  // 初始化栅格引擎
		engine.update_coordinate();  // 获取栅格图层的边界

		double minX = engine.getMinX();
		double maxX = engine.getMaxX();
		double minY = engine.getMinY();
		double maxY = engine.getMaxY();

		// 将地理坐标转换为像素坐标
		drawVec drawer;
		QPointF topLeft = drawer.geoToPixel(minX, maxY, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat, mnViewWidth, mnViewHeight);
		QPointF bottomRight = drawer.geoToPixel(maxX, minY, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat, mnViewWidth, mnViewHeight);

		QRectF boundingRect(topLeft, bottomRight);

		QGraphicsView* view = static_cast<QGraphicsView*>(centralWidget()->layout()->itemAt(0)->widget());
		view->fitInView(boundingRect, Qt::KeepAspectRatio);

		// 模拟一次滚轮事件
		QWheelEvent wheelEvent(QPointF(0, 0), QPointF(0, 0), QPoint(0, 0), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::ScrollBegin, false);
		static_cast<myGraphicsView*>(mpctrlView)->wheelEvent(&wheelEvent);
	}
	else {
		// 矢量图层
		for (const auto& point : layer.store.getStorePoints()) {
			minX = std::min(minX, point->getX());
			minY = std::min(minY, point->getY());
			maxX = std::max(maxX, point->getX());
			maxY = std::max(maxY, point->getY());
		}

		for (const auto& line : layer.store.getStoreLines()) {
			for (const Point& point : line->getPoints()) {
				minX = std::min(minX, point.getX());
				minY = std::min(minY, point.getY());
				maxX = std::max(maxX, point.getX());
				maxY = std::max(maxY, point.getY());
			}
		}

		for (const auto& polygon : layer.store.getStorePolygons()) {
			for (const Point& point : polygon->getExteriorRing().getPoints()) {
				minX = std::min(minX, point.getX());
				minY = std::min(minY, point.getY());
				maxX = std::max(maxX, point.getX());
				maxY = std::max(maxY, point.getY());
			}
			for (const LineString& ring : polygon->getInteriorRings()) {
				for (const Point& point : ring.getPoints()) {
					minX = std::min(minX, point.getX());
					minY = std::min(minY, point.getY());
					maxX = std::max(maxX, point.getX());
					maxY = std::max(maxY, point.getY());
				}
			}
		}
	}

	// 将地理坐标转换为像素坐标
	drawVec drawer;
	QPointF topLeft = drawer.geoToPixel(minX, maxY, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat, mnViewWidth, mnViewHeight);
	QPointF bottomRight = drawer.geoToPixel(maxX, minY, mdMinLon, mdMaxLon, mdMinLat, mdMaxLat, mnViewWidth, mnViewHeight);

	QRectF boundingRect(topLeft, bottomRight);

	QGraphicsView* view = static_cast<QGraphicsView*>(centralWidget()->layout()->itemAt(0)->widget());
	view->fitInView(boundingRect, Qt::KeepAspectRatio);

	// 模拟一次滚轮事件
	QWheelEvent wheelEvent(QPointF(0, 0), QPointF(0, 0), QPoint(0, 0), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::ScrollBegin, false);
	static_cast<myGraphicsView*>(mpctrlView)->wheelEvent(&wheelEvent);
}

void MainWindow::deleteLayer(QTreeWidgetItem* item) {
	QString layerName = item->text(0);

	// 从图层管理器中移除该图层
	delete item;

	// 从图层集合中移除该图层
	auto it = mmapNameLayer.find(layerName);
	if (it != mmapNameLayer.end()) {
		// 处理图层的资源释放
		if (it->second.group) {
			mpctrlScene->removeItem(it->second.group);
			delete it->second.group;
			it->second.group = nullptr;
		}
		// 移除图层
		mmapNameLayer.erase(it);
	}

	// 重新绘制图层
	drawLayers();

	// 更新图层树
	updateLayerTree();

	// 更新符号系统列表
	updateSymList();
}

void MainWindow::moveLayerToTop(QTreeWidgetItem* item) {
	QString layerName = item->text(0);

	// 检查图层是否存在
	if (mmapNameLayer.find(layerName) == mmapNameLayer.end()) return;

	// 从图层集合中移除并重新插入以将其置顶
	Layer layer = mmapNameLayer[layerName];
	mmapNameLayer.erase(layerName);
	mmapNameLayer.insert({ layerName, layer });

	// 重新绘制图层
	drawLayers();

	// 更新图层树
	updateLayerTree();

	// 更新符号系统列表
	updateSymList();
}

void MainWindow::moveLayerToBottom(QTreeWidgetItem* item) {
	QString layerName = item->text(0);

	// 检查图层是否存在
	if (mmapNameLayer.find(layerName) == mmapNameLayer.end()) return;

	// 从图层集合中移除并在头部插入以将其置底
	Layer layer = mmapNameLayer[layerName];
	mmapNameLayer.erase(layerName);
	mmapNameLayer.insert(mmapNameLayer.begin(), { layerName, layer });

	// 清空并重新绘制图层
	mpctrlScene->clear();
	drawLayers();

	// 更新图层树
	updateLayerTree();

	// 更新符号系统列表
	updateSymList();
}


// *****************************************矢量数据统计分析部分******************************************

// 确认勾选图层数
bool MainWindow::checkSingleLayerSelected(QString& selectedLayerName) {
	int selectedCount = 0;
	for (int i = 0; i < mpctrlLayerTree->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = mpctrlLayerTree->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked) {
			selectedLayerName = item->text(0);
			selectedCount++;
		}
	}

	if (selectedCount != 1) {
		QMessageBox::warning(this, "警告", "勾选了多个图层");
		return false;
	}
	return true;
}

// 实现显示统计详细信息
void MainWindow::showLayerDetailStats(const QString& layerName) {
	Layer& layer = mmapNameLayer[layerName];

	// 创建详细统计的表格
	QTableWidget* detailTable = new QTableWidget();
	QStringList headers;

	// 根据当前统计类型设置表头
	if (mstrCurrentStatType == "面积") {
		headers << "要素序号" << "面积 (km²)";
	}
	else if (mstrCurrentStatType == "周长") {
		headers << "要素序号" << "周长 (km)";
	}
	else if (mstrCurrentStatType == "长度") {
		headers << "要素序号" << "长度 (km)";
	}

	detailTable->setColumnCount(headers.size());
	detailTable->setHorizontalHeaderLabels(headers);

	// 填充表格数据
	int row = 0;
	if (mstrCurrentStatType == "面积" && !layer.store.getStorePolygons().empty()) {
		detailTable->setRowCount(layer.store.getStorePolygons().size());
		for (int i = 0; i < layer.store.getStorePolygons().size(); ++i) {
			const auto& polygon = layer.store.getStorePolygons()[i];
			double area = polygon->calculateArea() / 1e6; // 面积转换为平方千米
			detailTable->setItem(row, 0, new QTableWidgetItem(QString::number(i + 1)));
			detailTable->setItem(row, 1, new QTableWidgetItem(QString::number(area)));
			++row;
		}
	}
	else if (mstrCurrentStatType == "周长" && !layer.store.getStorePolygons().empty()) {
		detailTable->setRowCount(layer.store.getStorePolygons().size());
		for (int i = 0; i < layer.store.getStorePolygons().size(); ++i) {
			const auto& polygon = layer.store.getStorePolygons()[i];
			double perimeter = polygon->calculatePerimeter() / 1e3; // 周长转换为千米 
			detailTable->setItem(row, 0, new QTableWidgetItem(QString::number(i + 1)));
			detailTable->setItem(row, 1, new QTableWidgetItem(QString::number(perimeter)));
			++row;
		}
	}
	else if (mstrCurrentStatType == "长度" && !layer.store.getStoreLines().empty()) {
		detailTable->setRowCount(layer.store.getStoreLines().size());
		for (int i = 0; i < layer.store.getStoreLines().size(); ++i) {
			const auto& line = layer.store.getStoreLines()[i];
			double length = line->calculateLength() / 1e3; // 长度转换为千米
			detailTable->setItem(row, 0, new QTableWidgetItem(QString::number(i + 1)));
			detailTable->setItem(row, 1, new QTableWidgetItem(QString::number(length)));
			++row;
		}
	}
	else {
		QMessageBox::information(this, "详细统计", "该图层没有可统计的要素");
		return;
	}

	// 隐藏默认的垂直表头序号
	detailTable->verticalHeader()->setVisible(false);

	// 自动调整列宽和行高，使表格适应内容
	detailTable->resizeColumnsToContents();
	detailTable->resizeRowsToContents();

	// 显示统计结果的弹出窗口
	QDialog* detailDialog = new QDialog(this);
	detailDialog->setWindowTitle(layerName + " - 详细统计");
	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(detailTable);
	detailDialog->setLayout(layout);
	detailDialog->exec();
}


// 显示统计详细信息
void MainWindow::onTableWidgetCellDoubleClicked(int row, int column) {
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(sender());
	if (!tableWidget || column != 0) return;  // 只处理图层名称的双击

	QString layerName = tableWidget->item(row, column)->text();
	if (layerName == "总计" || layerName.isEmpty()) return;  // 忽略总计行或空行

	// 显示图层的详细统计信息
	showLayerDetailStats(layerName);
}


// 统计结果显示表格
void MainWindow::showStatTable(const QVector<QVector<QString>>& data, const QStringList& headers) {
	mpctrlStatList->clear();

	// 创建一个新的 QTableWidget
	QTableWidget* tableWidget = new QTableWidget(data.size(), headers.size());
	tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(tableWidget, &QTableWidget::cellDoubleClicked, this, &MainWindow::onTableWidgetCellDoubleClicked);
	tableWidget->setHorizontalHeaderLabels(headers);

	// 隐藏默认的垂直表头序号
	tableWidget->verticalHeader()->setVisible(false);

	// 设置表格的大小策略
	tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// 填充表格内容
	for (int i = 0; i < data.size(); ++i) {
		for (int j = 0; j < data[i].size(); ++j) {
			tableWidget->setItem(i, j, new QTableWidgetItem(data[i][j]));
		}
	}

	// 自动调整列宽和行高，使表格适应内容
	tableWidget->resizeColumnsToContents();
	tableWidget->resizeRowsToContents();

	// 设置表格为自适应窗口大小
	tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	// 使用 QVBoxLayout 来管理布局
	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(tableWidget);

	QWidget* statWidget = new QWidget();
	statWidget->setLayout(layout);

	QListWidgetItem* tableItem = new QListWidgetItem(mpctrlStatList);
	mpctrlStatList->addItem(tableItem);
	mpctrlStatList->setItemWidget(tableItem, statWidget);

	// 调整 QListWidgetItem 的尺寸以适应表格
	tableItem->setSizeHint(tableWidget->sizeHint());
}

// 数量统计
void MainWindow::countFeatures() {
	QVector<QVector<QString>> data;
	QStringList headers = { "图层名称", "要素类型", "数量" };

	int totalPointCount = 0;
	int totalLineCount = 0;
	int totalPolygonCount = 0;
	bool anyChecked = false;

	for (int i = 0; i < mpctrlLayerTree->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = mpctrlLayerTree->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked) {
			anyChecked = true;
			QString layerName = item->text(0);
			Layer& layer = mmapNameLayer[layerName];
			int pointCount = layer.store.getStorePoints().size();
			int lineCount = layer.store.getStoreLines().size();
			int polygonCount = layer.store.getExteriorPolygonCount();

			totalPointCount += pointCount;
			totalLineCount += lineCount;
			totalPolygonCount += polygonCount;

			if (pointCount > 0) {
				data.append({ layerName, "点", QString::number(pointCount) });
			}
			if (lineCount > 0) {
				data.append({ layerName, "线", QString::number(lineCount) });
			}
			if (polygonCount > 0) {
				data.append({ layerName, "面", QString::number(polygonCount) });
			}
		}
	}

	if (!anyChecked) {
		QMessageBox::warning(this, "警告", "至少勾选一个图层");
		return;
	}

	data.append({ "", "点", QString::number(totalPointCount) });
	data.append({ "总计", "线", QString::number(totalLineCount) });
	data.append({ "", "面", QString::number(totalPolygonCount) });

	showStatTable(data, headers);
}

// 面积统计
void MainWindow::computeArea() {
	mstrCurrentStatType = "面积";
	QVector<QVector<QString>> data;
	QStringList headers = { "图层名称", "要素类型", "面积 (km²)" };

	double totalArea = 0.0;
	bool anyChecked = false;
	bool invalidLayer = false;

	for (int i = 0; i < mpctrlLayerTree->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = mpctrlLayerTree->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked) {
			anyChecked = true;
			QString layerName = item->text(0);
			Layer& layer = mmapNameLayer[layerName];
			if (!layer.store.getStorePolygons().empty()) {
				double area = layer.store.getTotalArea() / 1e6; // 转换为平方千米
				totalArea += area;
				data.append({ layerName, "面", QString::number(area) });
			}
			else {
				invalidLayer = true;
			}
		}
	}

	if (!anyChecked) {
		QMessageBox::warning(this, "警告", "至少勾选一个图层");
		return;
	}

	if (invalidLayer) {
		QMessageBox::warning(this, "警告", "勾选了无法进行面积统计的图层");
		return;
	}

	data.append({ "总计", "面", QString::number(totalArea) });
	showStatTable(data, headers);
}

// 周长统计
void MainWindow::computePerimeter() {
	mstrCurrentStatType = "周长";
	QVector<QVector<QString>> data;
	QStringList headers = { "图层名称", "要素类型", "周长 (km)" };

	double totalPerimeter = 0.0;
	bool anyChecked = false;
	bool invalidLayer = false;

	for (int i = 0; i < mpctrlLayerTree->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = mpctrlLayerTree->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked) {
			anyChecked = true;
			QString layerName = item->text(0);
			Layer& layer = mmapNameLayer[layerName];
			if (!layer.store.getStorePolygons().empty()) {
				double perimeter = layer.store.getTotalPerimeter() / 1e3; // 转换为千米
				totalPerimeter += perimeter;
				data.append({ layerName, "面", QString::number(perimeter) });
			}
			else {
				invalidLayer = true;
			}
		}
	}

	if (!anyChecked) {
		QMessageBox::warning(this, "警告", "至少勾选一个图层");
		return;
	}

	if (invalidLayer) {
		QMessageBox::warning(this, "警告", "勾选了无法进行周长统计的图层");
		return;
	}

	data.append({ "总计", "面", QString::number(totalPerimeter) });
	showStatTable(data, headers);
}

// 长度统计
void MainWindow::computeLength() {
	mstrCurrentStatType = "长度";
	QVector<QVector<QString>> data;
	QStringList headers = { "图层名称", "要素类型", "长度 (km)" };

	double totalLength = 0.0;
	bool anyChecked = false;
	bool invalidLayer = false;

	for (int i = 0; i < mpctrlLayerTree->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = mpctrlLayerTree->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked) {
			anyChecked = true;
			QString layerName = item->text(0);
			Layer& layer = mmapNameLayer[layerName];
			if (!layer.store.getStoreLines().empty()) {
				double length = layer.store.getTotalLength() / 1e3; // 转换为千米
				totalLength += length;
				data.append({ layerName, "线", QString::number(length) });
			}
			else {
				invalidLayer = true;
			}
		}
	}

	if (!anyChecked) {
		QMessageBox::warning(this, "警告", "至少勾选一个图层");
		return;
	}

	if (invalidLayer) {
		QMessageBox::warning(this, "警告", "勾选了无法进行长度统计的图层");
		return;
	}

	data.append({ "总计", "线", QString::number(totalLength) });
	showStatTable(data, headers);
}

// *****************************************皮肤切换部分******************************************

// 切换皮肤
void MainWindow::switchTheme(int index) {
	switch (index)
	{
	case 1:mRemote.setCommand(std::make_unique<themeSwitch>(1));
		mRemote.pressExecute();
		break;
	case 2:mRemote.setCommand(std::make_unique<themeSwitch>(2));
		mRemote.pressExecute();
		break;
	case 3:mRemote.setCommand(std::make_unique<themeSwitch>(3));
		mRemote.pressExecute();
		break;
	default:mRemote.setCommand(std::make_unique<themeSwitch>(0));
		mRemote.pressExecute();
		break;
	}
}


// *****************************************编辑基础功能部分******************************************

// 选择、移动、删除操作及按钮显示逻辑
void MainWindow::setInteractionMode(QAction* action) {
	if (mstrCurrentEditingLayer.isEmpty() && mIsPathAnalysisMode == false) {
		QMessageBox::warning(this, "警告", "请先选择一个图层进行编辑");
		return;
	}
	if (mpctrlLayerComboBox->currentText() != mstrCurrentEditingLayer && mIsPathAnalysisMode == false) {
		QMessageBox::warning(this, "警告", "只能编辑选择的图层");
		return;
	}

	if (action == mpSaveeditAction || action == mpCanceleditAction) {
		// 重置按钮状态
		// 将选择、移动、修改按钮设置为未选中状态
		mpChooseAction->setChecked(false);
		mpMoveAction->setChecked(false);
		mpRevAction->setChecked(false);
		mpEditAction->setChecked(false);
		// 恢复其他按钮
		mpChooseAction->setEnabled(true);
		mpMoveAction->setEnabled(true);
		mpRevAction->setEnabled(true);
		mpEditAction->setEnabled(true);
		mpDelAction->setEnabled(true);
		mpSaveeditAction->setEnabled(true);
		mpCanceleditAction->setEnabled(true);
		// 关闭选择功能
		mpctrlView->setInteractionMode(false);
	}

	if (action == mpRevAction) {
		// 显示创建点、线、面按钮
		mpCreatePointAction->setVisible(true);
		mpCreateLineAction->setVisible(true);
		mpCreatePolygonAction->setVisible(true);
	}
	else {
		// 隐藏创建点、线、面按钮
		mpCreatePointAction->setVisible(false);
		mpCreateLineAction->setVisible(false);
		mpCreatePolygonAction->setVisible(false);
	}

	if (action == mpEditAction) {
		// 禁用无关按钮
		mpChooseAction->setEnabled(false);
		mpMoveAction->setEnabled(false);
		mpDelAction->setEnabled(false);
		mpRevAction->setEnabled(false);
		// 显示添加、删除、移动折点按钮
		mpAddTurningPointAction->setVisible(true);
		mpAddTurningPointAction->setEnabled(true);
		mpDelTurningPointAction->setVisible(true);
		mpDelTurningPointAction->setEnabled(false);
		mpSelTurningPointAction->setVisible(true);
		mpSelTurningPointAction->setEnabled(true);
		mpMoveTurningPointAction->setVisible(true);
		mpMoveTurningPointAction->setEnabled(true);
	}
	else {
		// 隐藏添加、删除、移动折点按钮
		mpAddTurningPointAction->setVisible(false);
		mpDelTurningPointAction->setVisible(false);
		mpSelTurningPointAction->setVisible(false);
		mpMoveTurningPointAction->setVisible(false);
	}

	if (action != mpChooseAction) {
		// 禁用删除按钮
		mpDelAction->setEnabled(false);
		// 禁用编辑要素按钮
		mpEditAction->setEnabled(false);
		// 清空选择
		mSelectedFeatures.clear();
		// 重绘
		drawEditLayer();
		// 取消选择按钮的选中状态
		mpChooseAction->setChecked(false);
		mpctrlView->setInteractionMode(false);
	}
}

// 选择、移动、删除槽函数
void MainWindow::chooseActionTriggered() {
	setInteractionMode(mpChooseAction);
	bool isChecked = mpChooseAction->isChecked();
	mpctrlView->setDragMode(QGraphicsView::NoDrag);
	mpctrlView->setInteractionMode(isChecked);
	mpctrlView->setEditingMode(0);
}
void MainWindow::moveActionTriggered() {
	setInteractionMode(mpMoveAction);
	mpctrlView->setDragMode(QGraphicsView::ScrollHandDrag);
}
void MainWindow::revActionTriggered() {
	setInteractionMode(mpRevAction);
}
void MainWindow::editActionTriggered() {
	// 只能对单个要素进行编辑
	if (mSelectedFeatures.getStoreLines().size() + mSelectedFeatures.getStorePolygons().size() != 1) {
		QMessageBox::warning(this, "警告", "只能选择单个要素进行编辑");
		return;
	}
	// 判断所选要素类型
	if (mSelectedFeatures.getStoreLines().size() == 1) {
		mpctrlView->setEditingObjectType(1);
		// 创建编辑中的要素
		mpEditLineString = mSelectedFeatures.getStoreLines()[0];
		mAddedStore.addLine(mpEditLineString);
		// 单独显示要素中的点
		for (auto& point : mpEditLineString->getPoints()) {
			mEditedStore.addPoint(new Point(point));
		}
		// 删除原始要素
		mpCurrentEditingStore->removeLineString(mpEditLineString);
		mDeletedStore.addLine(new LineString(*mpEditLineString));
	}
	else {
		mpctrlView->setEditingObjectType(2);
		// 创建编辑中的要素
		mpEditPolygon = mSelectedFeatures.getStorePolygons()[0];
		mAddedStore.addPolygon(mpEditPolygon);
		// 单独显示要素中的点
		for (int i = 0; i < mpEditPolygon->getExteriorRing().getPoints().size() - 1; i++) {
			mEditedStore.addPoint(new Point(mpEditPolygon->getExteriorRing().getPoints()[i]));
		}
		// 删除原始要素
		mpCurrentEditingStore->removePolygon(mpEditPolygon);
		mDeletedStore.addPolygon(new myPolygon(*mpEditPolygon));
	}
	mSelectedFeatures.clear();
	setInteractionMode(mpEditAction);
	mpctrlView->setInteractionMode(true);
	mpctrlView->setEditingMode(4);
	mpSelTurningPointAction->setChecked(true);
	// 重绘
	drawEditLayer();
}

void MainWindow::onPathClicked(const QPointF& pos) {
	mpctrlView->setDragMode(QGraphicsView::NoDrag);
	// 获取点击位置的地理坐标
	Point cliclPoint = mDraw.pixelToGeo(pos.x(), pos.y(), mdMinLon, mdMaxLon, mdMinLat, mdMaxLat, mnViewWidth, mnViewHeight);
	double dMinDistance = cliclPoint.distance(*mEditedStore.getStorePoints()[0]);
	Point* pSelectedPoint = nullptr;
	for (auto& point : mEditedStore.getStorePoints()) {
		if (geometryRelation::IsOnPoint(cliclPoint, *point)) {
			if (cliclPoint.distance(*point) <= dMinDistance) {
				dMinDistance = cliclPoint.distance(*point);
				pSelectedPoint = point;
			}
		}
	}
	if (pSelectedPoint != nullptr) {
		// 如果点击的位置在已选择的点上，则取消选择
		if (mSelectedFeatures.containsPoint(*pSelectedPoint))
			mSelectedFeatures.removePoint(pSelectedPoint);
		else
			mSelectedFeatures.addPoint(pSelectedPoint);
	}

	// 处理用户选择的点
	if (pSelectedPoint) {
		// 检查是否已经选择了起点
		if (mSelectedStartNode == -1) {
			mSelectedStartNode = findNodeIdByPosition(QPointF(pSelectedPoint->getX(), pSelectedPoint->getY()));
			drawEditLayer();
			QMessageBox::information(this, "路径分析", "起点已选择");
		}
		// 如果起点已选择但终点未选择
		else if (mSelectedEndNode == -1) {
			// 检查终点是否与起点相同
			if (findNodeIdByPosition(QPointF(pSelectedPoint->getX(), pSelectedPoint->getY())) == mSelectedStartNode) {
				QMessageBox::warning(this, "路径分析", "终点不能与起点相同");
			}
			else {
				mSelectedEndNode = findNodeIdByPosition(QPointF(pSelectedPoint->getX(), pSelectedPoint->getY()));
				drawEditLayer();
				QMessageBox::information(this, "路径分析", "终点已选择，开始路径分析");

				// 执行路径分析
				mAnalyzedPath = performPathAnalysis(mSelectedStartNode, mSelectedEndNode);

				// 重置选择状态
				mSelectedStartNode = -1;
				mSelectedEndNode = -1;

				// 关闭路径分析模式
				mIsPathAnalysisMode = false;

				// 重新绘制图层以显示路径
				drawLayers();
			}
		}
	}
	return; // 处理完路径分析点击后，直接返回
}

// 点击事件
void MainWindow::onSceneClicked(const QPointF& pos) {
	if (mstrCurrentEditingLayer.isEmpty()) {
		return;
	}
	// 获取当前编辑图层
	auto it = mmapNameLayer.find(mstrCurrentEditingLayer);
	// 输出点击位置的经纬度坐标
	Point cliclPoint = mDraw.pixelToGeo(pos.x(), pos.y(), mdMinLon, mdMaxLon, mdMinLat, mdMaxLat, mnViewWidth, mnViewHeight);
	qDebug() << "Clicked position: (" << cliclPoint.getX() << ", " << cliclPoint.getY() << ")";
	// 获取编辑模式
	int nEditingMode = mpctrlView->getEditingMode();
	// 选择对象
	if (nEditingMode == 0)
	{
		mpCurrentEditingStore = &(it->second.store);
		// 判断本次点击是否选择了对象
		bool isSelected = false;
		// 判断点
		if (mpCurrentEditingStore->getStorePoints().size() > 0 && !isSelected) {
			double dMinDistance = cliclPoint.distance(*mpCurrentEditingStore->getStorePoints()[0]);
			Point* pSelectedPoint = nullptr;
			for (auto& point : mpCurrentEditingStore->getStorePoints()) {
				if (geometryRelation::IsOnPoint(cliclPoint, *point)) {
					if (cliclPoint.distance(*point) <= dMinDistance) {
						dMinDistance = cliclPoint.distance(*point);
						pSelectedPoint = point;
					}
					isSelected = true;
				}
			}
			if (pSelectedPoint != nullptr) {
				// 如果点击的位置在已选择的点上，则取消选择或打开信息框
				if (mSelectedFeatures.containsPoint(*pSelectedPoint)) {
					if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier)) {
						mSelectedFeatures.removePoint(pSelectedPoint);
					}
					else {
						QDialog dialog(this);
						dialog.setWindowTitle("详细信息");
						QVBoxLayout layout;
						QLabel* imageLabel = new QLabel;
						bool hasImage = false; // 标记是否有照片

						QHash<QString, QString> hash = pSelectedPoint->getFieldValueMap();
						for (auto& key : hash.keys()) {
							if (key == "照片") {
								QString imagePath = hash[key];
								qDebug() << "Image path: " << imagePath;
								QImageReader reader(imagePath);
								reader.setAutoTransform(true);
								QImage image = reader.read();
								if (!image.isNull()) {
									qDebug() << "Image loaded successfully";
									imageLabel->setPixmap(QPixmap::fromImage(image));
									imageLabel->setScaledContents(true);
									imageLabel->setFixedSize(600, 400);
									hasImage = true;
								}
							}
							else {
								QLabel* textLabel = new QLabel(key + ": " + hash[key]);
								layout.addWidget(textLabel);
							}
						}
						if (hasImage) {
							layout.addWidget(imageLabel);
						}
						dialog.setLayout(&layout);
						// 设置对话框大小
						dialog.resize(600, 400);
						dialog.exec();
					}
				}
				else {
					// 如果本次点击没有按住 Ctrl 键，则清空之前的选择
					if (!QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier))
						mSelectedFeatures.clear();
					mSelectedFeatures.addPoint(pSelectedPoint);
				}
			}
		}
		// 判断线
		if (mpCurrentEditingStore->getStoreLines().size() > 0 && !isSelected) {
			double dMinDistance = mpCurrentEditingStore->getStoreLines()[0]->distanceToLineString(cliclPoint);
			LineString* pSelectedLine = nullptr;
			for (auto& line : mpCurrentEditingStore->getStoreLines()) {
				if (geometryRelation::IsOnLine(cliclPoint, *line)) {
					if (line->distanceToLineString(cliclPoint) <= dMinDistance) {
						dMinDistance = line->distanceToLineString(cliclPoint);
						pSelectedLine = line;
					}
					isSelected = true;
				}
			}
			if (pSelectedLine != nullptr) {
				// 如果点击的位置在已选择的线上，则取消选择
				if (mSelectedFeatures.containsLineString(*pSelectedLine))
					mSelectedFeatures.removeLineString(pSelectedLine);
				else
					mSelectedFeatures.addLine(pSelectedLine);
			}

		}
		// 判断面
		if (mpCurrentEditingStore->getStorePolygons().size() > 0 && !isSelected) {
			for (auto& polygon : mpCurrentEditingStore->getStorePolygons()) {
				if (geometryRelation::IsInPolygon(cliclPoint, *polygon)) {
					isSelected = true;
					// 如果点击的位置在已选择的面内，则取消选择
					if (mSelectedFeatures.containsPolygon(*polygon))
						mSelectedFeatures.removePolygon(polygon);
					else
						mSelectedFeatures.addPolygon(polygon);
					break;
				}
			}
		}
		// 重绘
		drawEditLayer();
		// 如果选择了要素，则激活删除按钮
		if (!mSelectedFeatures.isEmpty()) {
			mpDelAction->setEnabled(true);
			// 如果只选择了线或面要素，则激活编辑按钮
			if (mSelectedFeatures.getStorePoints().size() == 0) {
				mpEditAction->setEnabled(true);
			}
			else {
				mpEditAction->setEnabled(false);
			}
		}
		else {
			mpDelAction->setEnabled(false);
			mpEditAction->setEnabled(false);
		}
	}
	// 创建对象
	else if (nEditingMode >= 1 && nEditingMode <= 3)
	{
		// 添加点
		if (nEditingMode == 1) {
			mAddedStore.addPoint(new Point(cliclPoint));
		}
		// 添加线
		else if (nEditingMode == 2) {
			if (mpEditLineString == nullptr) {
				mpEditLineString = new LineString();
				mAddedStore.addLine(mpEditLineString);
			}
			mpEditLineString->addPoint(cliclPoint);
		}
		// 添加面
		else {
			if (mpEditPolygon == nullptr) {
				mpEditPolygon = new myPolygon();
				mAddedStore.addPolygon(mpEditPolygon);
			}
			if (!mpEditPolygon->isEmpty())
				mpEditPolygon->insertExteriorPoint(cliclPoint, mpEditPolygon->getExteriorRing().getPoints().size() - 1);
			else
			{
				mpEditPolygon->addExteriorPoint(cliclPoint);
				mpEditPolygon->addExteriorPoint(cliclPoint);
				return;
			}
		}
		// 重绘
		drawEditLayer();

	}
	// 选择折点
	else if (nEditingMode == 4) {
		mSelectedFeatures.clear();
		mnEditPointIndex = -1;
		double dMinDistance = cliclPoint.distance(*mEditedStore.getStorePoints()[0]);
		for (int i = 0; i < mEditedStore.getStorePoints().size(); i++) {
			if (geometryRelation::IsOnPoint(cliclPoint, *mEditedStore.getStorePoints()[i])) {
				if (cliclPoint.distance(*mEditedStore.getStorePoints()[i]) <= dMinDistance) {
					dMinDistance = cliclPoint.distance(*mEditedStore.getStorePoints()[i]);
					mnEditPointIndex = i;
				}
			}
		}
		if (mnEditPointIndex > -1) {
			mSelectedFeatures.addPoint(mEditedStore.getStorePoints()[mnEditPointIndex]);
		}
		// 重绘
		drawEditLayer();
		// 如果选择了折点，则激活删除折点按钮
		if (!mSelectedFeatures.isEmpty()) {
			mpDelTurningPointAction->setEnabled(true);
		}
		else {
			mpDelTurningPointAction->setEnabled(false);
		}
	}
	// 添加折点
	else if (nEditingMode == 5) {
		// 计算点击点与每条边的距离，找到距离最小的边
		double minDistance = cliclPoint.distanceToSegment(*mEditedStore.getStorePoints()[0], *mEditedStore.getStorePoints()[mEditedStore.getStorePoints().size() - 1]);
		mnEditPointIndex = 0;
		for (int i = 0; i < mEditedStore.getStorePoints().size() - 1; i++) {
			if (cliclPoint.distanceToSegment(*mEditedStore.getStorePoints()[i], *mEditedStore.getStorePoints()[i + 1]) < minDistance) {
				minDistance = cliclPoint.distanceToSegment(*mEditedStore.getStorePoints()[i], *mEditedStore.getStorePoints()[i + 1]);
				mnEditPointIndex = i + 1;
			}
		}
		// 在索引位置添加点
		mEditedStore.addPoint(new Point(cliclPoint.getX(), cliclPoint.getY()), {}, mnEditPointIndex);
		if (mpctrlView->getEditingObjectType() == 1) {
			mpEditLineString->insertPoint(cliclPoint, mnEditPointIndex);
		}
		else if (mpctrlView->getEditingObjectType() == 2) {
			mpEditPolygon->insertExteriorPoint(cliclPoint, mnEditPointIndex);
		}
		// 重绘
		drawEditLayer();
	}
	// 移动折点
	else if (nEditingMode == 6) {
		if (mSelectedFeatures.isEmpty()) {
			mnEditPointIndex = -1;
			double dMinDistance = cliclPoint.distance(*mEditedStore.getStorePoints()[0]);
			for (int i = 0; i < mEditedStore.getStorePoints().size(); i++) {
				if (geometryRelation::IsOnPoint(cliclPoint, *mEditedStore.getStorePoints()[i])) {
					if (cliclPoint.distance(*mEditedStore.getStorePoints()[i]) <= dMinDistance) {
						dMinDistance = cliclPoint.distance(*mEditedStore.getStorePoints()[i]);
						mnEditPointIndex = i;
					}
				}
			}
			if (mnEditPointIndex > -1) {
				mSelectedFeatures.addPoint(mEditedStore.getStorePoints()[mnEditPointIndex]);
			}
			// 重绘
			drawEditLayer();
		}
		else {
			mSelectedFeatures.clear();
			mnEditPointIndex = -1;
			// 重绘
			drawEditLayer();
		}
	}
}

// 删除函数
void MainWindow::deleteSelectedFeature() {
	if (mstrCurrentEditingLayer.isEmpty()) {
		QMessageBox::warning(this, "警告", "请先选择一个图层进行编辑");
		return;
	}
	if (mpctrlLayerComboBox->currentText() != mstrCurrentEditingLayer) {
		QMessageBox::warning(this, "警告", "只能编辑选择的图层");
		return;
	}
	auto it = mmapNameLayer.find(mstrCurrentEditingLayer);
	if (it != mmapNameLayer.end()) {
		mpCurrentEditingStore = &(it->second.store);
	}
	// 删除选中要素的逻辑
	for (auto point : mSelectedFeatures.getStorePoints())
	{
		mDeletedStore.addPoint(point);
		mpCurrentEditingStore->removePoint(point);
	}
	for (auto line : mSelectedFeatures.getStoreLines())
	{
		mDeletedStore.addLine(line);
		mpCurrentEditingStore->removeLineString(line);
	}
	for (auto polygon : mSelectedFeatures.getStorePolygons())
	{
		mDeletedStore.addPolygon(polygon);
		mpCurrentEditingStore->removePolygon(polygon);
	}
	mSelectedFeatures.clear();
	// 重绘
	drawEditLayer();

}

// 撤销编辑
void MainWindow::cancelEdit() {
	auto it = mmapNameLayer.find(mstrCurrentEditingLayer);
	if (it != mmapNameLayer.end()) {
		mpCurrentEditingStore = &(it->second.store);
	}
	if ((!mAddedStore.isEmpty()) || (!mDeletedStore.isEmpty())) {
		mRemote.setCommand(std::make_unique<editCancel>(mpCurrentEditingStore, this));
		mRemote.pressExecute();
		// 清空编辑中的要素
		mEditedStore.clear();
		mpEditLineString = nullptr;
		mpEditPolygon = nullptr;
		// 清空选择的要素
		mSelectedFeatures.clear();
		// 重绘
		drawEditLayer();
	}
	setInteractionMode(mpCanceleditAction);
}

// 保存编辑
void MainWindow::saveEdit() {
	auto it = mmapNameLayer.find(mstrCurrentEditingLayer);
	if (it != mmapNameLayer.end()) {
		mpCurrentEditingStore = &(it->second.store);
	}
	if ((!mAddedStore.isEmpty()) || (!mDeletedStore.isEmpty())) {
		mRemote.setCommand(std::make_unique<editSave>(mpCurrentEditingStore, this));
		mRemote.pressExecute();
		// 清空编辑中的要素
		mEditedStore.clear();
		mpEditLineString = nullptr;
		mpEditPolygon = nullptr;
		// 清空选择的要素
		mSelectedFeatures.clear();
		// 重绘
		drawEditLayer();
	}
	setInteractionMode(mpSaveeditAction);
}

// 编辑模式改变
void MainWindow::handleEditComboBoxChange(int index) {
	if (mpctrlEditComboBox->currentText() == "开始") {
		startEditingLayer();
		// 启用选择、移动、修改按钮
		mpChooseAction->setEnabled(true);
		mpMoveAction->setEnabled(true);
		mpRevAction->setEnabled(true);
		mpDelAction->setEnabled(true);
		mpSaveeditAction->setEnabled(true);
		mpCanceleditAction->setEnabled(true);
	}
	else if (mpctrlEditComboBox->currentText() == "停止") {
		stopEditingLayer();
		// 禁用所有相关按钮
		mpChooseAction->setEnabled(false);
		mpMoveAction->setEnabled(false);
		mpRevAction->setEnabled(false);
		mpDelAction->setEnabled(false);
		mpSaveeditAction->setEnabled(false);
		mpCanceleditAction->setEnabled(false);

		// 隐藏创建按钮
		mpCreatePointAction->setVisible(false);
		mpCreateLineAction->setVisible(false);
		mpCreatePolygonAction->setVisible(false);
		mpSelTurningPointAction->setVisible(false);
		mpAddTurningPointAction->setVisible(false);
		mpMoveTurningPointAction->setVisible(false);
		mpDelTurningPointAction->setVisible(false);
		mpDelAction->setVisible(false);
	}
}

// *****************************************编辑创建要素功能部分******************************************

// 显示单次创建要素确认和取消按钮
void MainWindow::showTemporaryButtons() {
	mpctrlConfirmButton->setVisible(true);
	mpctrlCancelButton->setVisible(true);
}

// 创建点、创建线、创建面槽函数
void MainWindow::createPointActionTriggered() {
	// 别的按钮全禁了
	mpCreateLineAction->setEnabled(false);
	mpCreatePolygonAction->setEnabled(false);
	mpChooseAction->setEnabled(false);
	mpMoveAction->setEnabled(false);
	mpRevAction->setEnabled(false);
	mpEditAction->setEnabled(false);
	mpDelAction->setEnabled(false);
	mpSaveeditAction->setEnabled(false);
	mpCanceleditAction->setEnabled(false);
	showTemporaryButtons();
	// 设置可交互
	mpctrlView->setInteractionMode(true);
	mpctrlView->setEditingMode(1);
}

void MainWindow::createLineActionTriggered() {
	// 别的按钮全禁了
	mpCreatePointAction->setEnabled(false);
	mpCreatePolygonAction->setEnabled(false);
	mpChooseAction->setEnabled(false);
	mpMoveAction->setEnabled(false);
	mpRevAction->setEnabled(false);
	mpEditAction->setEnabled(false);
	mpDelAction->setEnabled(false);
	mpSaveeditAction->setEnabled(false);
	mpCanceleditAction->setEnabled(false);
	showTemporaryButtons();
	// 设置可交互
	mpctrlView->setInteractionMode(true);
	mpctrlView->setEditingMode(2);
}

void MainWindow::createPolygonActionTriggered() {
	// 别的按钮全禁了
	mpCreatePointAction->setEnabled(false);
	mpCreateLineAction->setEnabled(false);
	mpChooseAction->setEnabled(false);
	mpMoveAction->setEnabled(false);
	mpRevAction->setEnabled(false);
	mpEditAction->setEnabled(false);
	mpDelAction->setEnabled(false);
	mpSaveeditAction->setEnabled(false);
	mpCanceleditAction->setEnabled(false);
	showTemporaryButtons();
	// 设置可交互
	mpctrlView->setInteractionMode(true);
	mpctrlView->setEditingMode(3);
}

// 单次创建要素确认按钮槽函数
void MainWindow::confirmCreation() {
	mpEditLineString = nullptr;
	mpEditPolygon = nullptr;
	mpctrlView->setEditingMode(0);
	// 重绘
	drawEditLayer();
	// 隐藏临时按钮
	mpctrlConfirmButton->setVisible(false);
	mpctrlCancelButton->setVisible(false);

	// 恢复创建按钮的未选中状态
	mpCreatePointAction->setChecked(false);
	mpCreateLineAction->setChecked(false);
	mpCreatePolygonAction->setChecked(false);

	// 恢复其他按钮
	mpCreatePointAction->setEnabled(true);
	mpCreateLineAction->setEnabled(true);
	mpCreatePolygonAction->setEnabled(true);
	mpChooseAction->setEnabled(true);
	mpMoveAction->setEnabled(true);
	mpRevAction->setEnabled(true);
	mpEditAction->setEnabled(true);
	mpDelAction->setEnabled(true);
	mpSaveeditAction->setEnabled(true);
	mpCanceleditAction->setEnabled(true);
}

// 单次创建要素取消按钮槽函数
void MainWindow::cancelCreation() {
	mAddedStore.clear();
	mpEditLineString = nullptr;
	mpEditPolygon = nullptr;
	mpctrlView->setEditingMode(0);
	// 重绘
	drawEditLayer();

	// 隐藏临时按钮
	mpctrlConfirmButton->setVisible(false);
	mpctrlCancelButton->setVisible(false);

	// 恢复创建按钮的未选中状态
	mpCreatePointAction->setChecked(false);
	mpCreateLineAction->setChecked(false);
	mpCreatePolygonAction->setChecked(false);

	// 恢复其他按钮
	mpCreatePointAction->setEnabled(true);
	mpCreateLineAction->setEnabled(true);
	mpCreatePolygonAction->setEnabled(true);
	mpChooseAction->setEnabled(true);
	mpMoveAction->setEnabled(true);
	mpRevAction->setEnabled(true);
	mpEditAction->setEnabled(true);
	mpDelAction->setEnabled(true);
	mpSaveeditAction->setEnabled(true);
	mpCanceleditAction->setEnabled(true);
}

// 编辑要素槽函数
void MainWindow::selTurningPointActionTriggered() {
	// 设置交互模式
	mpctrlView->setInteractionMode(true);
	mpctrlView->setEditingMode(4);
}

void MainWindow::addTurningPointActionTriggered() {
	// 设置交互模式
	mpctrlView->setInteractionMode(true);
	mpctrlView->setEditingMode(5);
}

void MainWindow::moveTurningPointActionTriggered() {
	mSelectedFeatures.clear();
	mnEditPointIndex = -1;
	drawEditLayer();
	// 设置交互模式
	mpctrlView->setInteractionMode(true);
	mpctrlView->setEditingMode(6);
}

void MainWindow::delTurningPointActionTriggered() {
	mEditedStore.removePoint(mnEditPointIndex);
	if (mpctrlView->getEditingObjectType() == 1) {
		mpEditLineString->removePoint(mnEditPointIndex);
	}
	else if (mpctrlView->getEditingObjectType() == 2) {
		mpEditPolygon->removeExteriorPoint(mnEditPointIndex);
	}
	mSelectedFeatures.clear();
	mpDelTurningPointAction->setEnabled(false);
	mpDelTurningPointAction->setChecked(false);
	mpSelTurningPointAction->setChecked(true);
	// 重绘
	drawEditLayer();
}

// *****************************************状态栏更新部分******************************************

void MainWindow::updateStatusBar(const QString& mousePos, const QString& zoom, const QString& rotation) {
	mpctrlMousePosLabel->setText(mousePos);
	mpctrlZoomLabel->setText(zoom);
	mpctrlRotationLabel->setText(rotation);
}

void MainWindow::onStatusBarUpdated(const QString& mousePos, const QString& zoom, const QString& rotation) {
	updateStatusBar(mousePos, zoom, rotation);
}

// 添加空要素图层的槽函数
void MainWindow::addEmptyLayer() {
	// 显示输入对话框，获取图层名称
	bool ok;
	QString layerName = QInputDialog::getText(this, tr("添加空要素图层"),
		tr("请输入图层名称:"), QLineEdit::Normal,
		"", &ok);
	if (!ok || layerName.isEmpty()) {
		return;  // 用户取消输入或未输入图层名称
	}

	// 显示输入对话框，选择图层类型
	QStringList layerTypes;
	layerTypes << "点" << "线" << "面";
	QString layerType = QInputDialog::getItem(this, tr("选择图层类型"),
		tr("请选择图层类型:"), layerTypes, 0, false, &ok);
	if (!ok) {
		return;  // 用户取消选择
	}

	// 根据图层类型创建空图层
	Store emptyStore;
	Layer newLayer{ emptyStore, new QGraphicsItemGroup(), QPen(Qt::black), QBrush(Qt::transparent), false, layerName, "" };

	if (layerType == "点") {
		// 创建空的点图层
		newLayer.vectorFilePath = "point";
	}
	else if (layerType == "线") {
		// 创建空的线图层
		newLayer.vectorFilePath = "line";
	}
	else if (layerType == "面") {
		// 创建空的面图层
		newLayer.vectorFilePath = "polygon";
	}

	// 添加图层到图层列表
	mmapNameLayer[layerName] = newLayer;

	// 更新图层树和符号系统列表
	updateLayerTree();
	updateSymList();
}

// *****************************************路径分析功能部分******************************************
NetworkData MainWindow::convertLineLayerToNetwork(const Layer& lineLayer) {
	NetworkData networkData;
	std::unordered_map<QPointF, int, QPointFHash, QPointFEqual> pointToNodeIdMap; // 使用自定义的哈希和比较函数

	int nodeIdCounter = 0;
	int edgeIdCounter = 0;

	// 遍历线要素图层中的每一条线
	for (const auto& line : lineLayer.store.getStoreLines()) {
		const auto& points = line->getPoints();
		if (points.size() < 2) continue;

		int startNodeId, endNodeId;

		// 检查起点是否已经存在于节点集中
		QPointF startPos(points.front().getX(), points.front().getY());
		if (pointToNodeIdMap.find(startPos) == pointToNodeIdMap.end()) {
			startNodeId = nodeIdCounter++;
			NetworkNode startNode = { startNodeId, startPos };
			networkData.nodes.push_back(startNode);
			pointToNodeIdMap[startPos] = startNodeId;
		}
		else {
			startNodeId = pointToNodeIdMap[startPos];
		}

		// 检查终点是否已经存在于节点集中
		QPointF endPos(points.back().getX(), points.back().getY());
		if (pointToNodeIdMap.find(endPos) == pointToNodeIdMap.end()) {
			endNodeId = nodeIdCounter++;
			NetworkNode endNode = { endNodeId, endPos };
			networkData.nodes.push_back(endNode);
			pointToNodeIdMap[endPos] = endNodeId;
		}
		else {
			endNodeId = pointToNodeIdMap[endPos];
		}

		// 创建边并加入网络数据集
		double length = line->calculateLength();
		NetworkEdge edge = { edgeIdCounter++, startNodeId, endNodeId, length };
		networkData.edges.push_back(edge);

		// 更新节点的连接边信息
		networkData.nodes[startNodeId].connectedEdges.insert(edge.id);
		networkData.nodes[endNodeId].connectedEdges.insert(edge.id);
	}

	return networkData;
}

void MainWindow::showPathAnalysisSetupDialog() {
	// 创建路径分析设置窗口
	QDialog* dialog = new QDialog(this);
	dialog->setWindowTitle("路径分析设置");

	// 创建布局
	QVBoxLayout* layout = new QVBoxLayout(dialog);

	// 选择线图层
	QLabel* lineLayerLabel = new QLabel("选择线图层:", dialog);
	QComboBox* lineLayerComboBox = new QComboBox(dialog);
	layout->addWidget(lineLayerLabel);
	layout->addWidget(lineLayerComboBox);

	// 选择点图层
	QLabel* pointLayerLabel = new QLabel("选择点图层:", dialog);
	QComboBox* pointLayerComboBox = new QComboBox(dialog);
	pointLayerComboBox->addItem("");
	layout->addWidget(pointLayerLabel);
	layout->addWidget(pointLayerComboBox);

	// 途径点和禁止点的选项
	QRadioButton* viaPointsRadioButton = new QRadioButton("作为路径途径点", dialog);
	QRadioButton* forbiddenPointsRadioButton = new QRadioButton("作为禁止通过点", dialog);
	QButtonGroup* pointOptionsGroup = new QButtonGroup(dialog);
	pointOptionsGroup->addButton(viaPointsRadioButton);
	pointOptionsGroup->addButton(forbiddenPointsRadioButton);
	layout->addWidget(viaPointsRadioButton);
	layout->addWidget(forbiddenPointsRadioButton);

	// 添加线图层选项
	for (const auto& layer : mmapNameLayer) {
		if (!layer.second.isRaster && !layer.second.store.getStoreLines().empty()) {
			lineLayerComboBox->addItem(layer.first);
		}
	}

	// 添加点图层选项
	for (const auto& layer : mmapNameLayer) {
		if (!layer.second.isRaster && !layer.second.store.getStorePoints().empty()) {
			pointLayerComboBox->addItem(layer.first);
		}
	}

	// 添加确认和取消按钮
	QHBoxLayout* buttonLayout = new QHBoxLayout();
	QPushButton* okButton = new QPushButton("确定", dialog);
	QPushButton* cancelButton = new QPushButton("取消", dialog);
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);
	layout->addLayout(buttonLayout);

	// 连接确认按钮的点击事件
	connect(okButton, &QPushButton::clicked, this, [this, dialog, lineLayerComboBox, pointLayerComboBox, viaPointsRadioButton, forbiddenPointsRadioButton]() {
		QString lineLayerName = lineLayerComboBox->currentText();
		QString pointLayerName = pointLayerComboBox->currentText();
		bool useAsViaPoints = viaPointsRadioButton->isChecked();
		bool useAsForbiddenPoints = forbiddenPointsRadioButton->isChecked();

		// 调用下一步：显示线图层的端点以供选择起点和终点
		prepareToSelectStartAndEndPoints(lineLayerName, pointLayerName, useAsViaPoints, useAsForbiddenPoints);

		dialog->accept();
		});

	connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

	// 显示对话框
	dialog->exec();
}

void MainWindow::prepareToSelectStartAndEndPoints(const QString& lineLayerName, const QString& pointLayerName, bool useAsViaPoints, bool useAsForbiddenPoints) {
	// 获取选择的线图层
	auto lineLayerIt = mmapNameLayer.find(lineLayerName);

	if (lineLayerIt == mmapNameLayer.end()) {
		QMessageBox::warning(this, "错误", "线图层不存在");
		return;
	}

	// 转化为网络数据集
	mCurrentNetworkData = convertLineLayerToNetwork(lineLayerIt->second);

	// 如果用户选择了点图层，处理路径途径点或禁止通过点的逻辑
	if (!pointLayerName.isEmpty()) {
		auto pointLayerIt = mmapNameLayer.find(pointLayerName);
		if (pointLayerIt == mmapNameLayer.end()) {
			QMessageBox::warning(this, "错误", "点图层不存在");
			return;
		}

		if (useAsViaPoints) {
			mViaPointsLayer = &pointLayerIt->second;  // 记录途径点图层
		}
		else if (useAsForbiddenPoints) {
			mForbiddenPointsLayer = &pointLayerIt->second;  // 记录禁止通过点图层
		}
	}

	// 设置路径分析模式标志为 true
	mIsPathAnalysisMode = true;

	// 将线图层的所有折点显示在场景中供用户选择
	drawPointsForSelection(lineLayerIt->second);
	mpChooseAction->setEnabled(true);

	disconnect(mpctrlView, &myGraphicsView::sceneClicked, this, &MainWindow::onSceneClicked);
	connect(mpctrlView, &myGraphicsView::sceneClicked, this, &MainWindow::onPathClicked);
}

void MainWindow::drawPointsForSelection(const Layer& lineLayer) {
	// 清空现有场景内容
	mpctrlScene->clear();

	// 创建一个 Store 用于存储所有的点
	Store mPathStore;

	// 遍历线要素图层中的所有 LineString
	for (auto& line : lineLayer.store.getStoreLines()) {
		// 遍历每个 LineString 中的所有点
		for (const auto& point : line->getPoints()) {
			// 将每个点添加到 mEditedStore 中
			mEditedStore.addPoint(new Point(point));
		}
	}
	drawEditLayer();
}

std::vector<int> MainWindow::performDijkstra(int startNodeId, int endNodeId, const NetworkData& networkData) {
	std::unordered_map<int, double> distances;
	std::unordered_map<int, int> previous;
	std::priority_queue<DijkstraNode, std::vector<DijkstraNode>, std::greater<DijkstraNode>> pq;

	// 初始化所有节点的距离为正无穷大，起点的距离为0
	for (const auto& node : networkData.nodes) {
		distances[node.id] = std::numeric_limits<double>::infinity();
	}
	distances[startNodeId] = 0;

	pq.push({ startNodeId, 0 });

	while (!pq.empty()) {
		DijkstraNode current = pq.top();
		pq.pop();

		if (current.nodeId == endNodeId) {
			break;
		}

		const auto& currentNode = networkData.nodes[current.nodeId];

		for (int edgeId : currentNode.connectedEdges) {
			const auto& edge = networkData.edges[edgeId];
			int neighborId = (edge.startNode == currentNode.id) ? edge.endNode : edge.startNode;
			double newDist = distances[current.nodeId] + edge.length;

			if (newDist < distances[neighborId]) {
				distances[neighborId] = newDist;
				previous[neighborId] = current.nodeId;
				pq.push({ neighborId, newDist });
			}
		}
	}

	// 构建路径
	std::vector<int> path;
	for (int at = endNodeId; at != startNodeId; at = previous[at]) {
		path.push_back(at);
	}
	path.push_back(startNodeId);

	std::reverse(path.begin(), path.end());
	return path;
}

std::vector<int> MainWindow::performPathAnalysis(int startNodeId, int endNodeId) {
	std::set<int> forbiddenNodes;

	// 处理禁止通过的点
	if (mForbiddenPointsLayer) {
		for (const auto& point : mForbiddenPointsLayer->store.getStorePoints()) {
			QPointF pos(point->getX(), point->getY());
			for (const auto& node : mCurrentNetworkData.nodes) {
				if (node.position == pos) {
					forbiddenNodes.insert(node.id);
				}
			}
		}
	}

	// 过滤网络数据，移除禁止通过的节点
	NetworkData filteredNetworkData = mCurrentNetworkData;
	filteredNetworkData.nodes.erase(
		std::remove_if(filteredNetworkData.nodes.begin(), filteredNetworkData.nodes.end(),
			[&](const NetworkNode& node) { return forbiddenNodes.count(node.id) > 0; }),
		filteredNetworkData.nodes.end()
	);

	std::vector<int> fullPath;
	int currentStart = startNodeId;

	// 处理途径点
	if (mViaPointsLayer) {
		for (const auto& point : mViaPointsLayer->store.getStorePoints()) {
			QPointF pos(point->getX(), point->getY());
			int viaNodeId = -1;

			for (const auto& node : filteredNetworkData.nodes) {
				if (node.position == pos) {
					viaNodeId = node.id;
					break;
				}
			}

			if (viaNodeId != -1) {
				if (currentStart != viaNodeId) {  // 确保不计算相同点之间的路径
					std::vector<int> partialPath = performDijkstra(currentStart, viaNodeId, filteredNetworkData);
					if (!partialPath.empty()) {
						fullPath.insert(fullPath.end(), partialPath.begin(), partialPath.end() - 1); // 插入路径但不重复添加 viaNode
						currentStart = viaNodeId;
					}
					else {
						QMessageBox::information(this, "路径分析", "无法找到途径点的路径。");
						return {}; // 无法找到路径，直接返回空路径
					}
				}
			}
		}
	}

	// 最后计算从最后一个途径点到终点的路径
	if (currentStart != endNodeId) {  // 确保不计算相同点之间的路径
		std::vector<int> finalPath = performDijkstra(currentStart, endNodeId, filteredNetworkData);
		if (!finalPath.empty()) {
			fullPath.insert(fullPath.end(), finalPath.begin(), finalPath.end());
		}
		else {
			QMessageBox::information(this, "路径分析", "无法找到从最后一个途径点到终点的路径。");
			return {}; // 无法找到路径，直接返回空路径
		}
	}

	// 创建路径图层并返回路径
	if (!fullPath.empty()) {
		createPathLayer(fullPath, filteredNetworkData);
	}
	else {
		QMessageBox::information(this, "路径分析", "无法找到从起点到终点的路径。");
	}

	return fullPath; // 返回最终的路径节点列表
}



void MainWindow::createPathLayer(const std::vector<int>& path, const NetworkData& networkData) {
	Store pathStore;
	LineString* lineString = new LineString();

	for (int nodeId : path) {
		const QPointF& pos = networkData.nodes[nodeId].position;
		lineString->addPoint(Point(pos.x(), pos.y()));
	}

	pathStore.addLine(lineString);

	QString newLayerName = QString("路径_%1").arg(mmapNameLayer.size());
	Layer newLayer{ pathStore, new QGraphicsItemGroup(), QPen(Qt::red, 4), QBrush(Qt::NoBrush), false, newLayerName, "" };

	mmapNameLayer[newLayerName] = newLayer;

	updateLayerTree();
	updateSymList();
	drawLayers();

	mpChooseAction->setEnabled(false);
	disconnect(mpctrlView, &myGraphicsView::sceneClicked, this, &MainWindow::onPathClicked);
	connect(mpctrlView, &myGraphicsView::sceneClicked, this, &MainWindow::onSceneClicked);
	QMessageBox::information(this, "路径分析", "路径分析已完成，新图层已创建并显示。");
}

int MainWindow::findNodeIdByPosition(const QPointF& position) {
	for (const auto& node : mCurrentNetworkData.nodes) {
		if (QLineF(position, node.position).length() < 1e-6) { // 考虑浮点误差
			return node.id;
		}
	}
	return -1; // 如果没有找到对应的节点
}

// *****************************************野外实习功能部分******************************************

// 读取照片的经纬度信息
std::optional<Point> readGeoCoordinates(const QString& filePath) {
	try {
		// 打开照片文件
		Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(filePath.toStdString());
		if (!image) return std::nullopt;

		// 读取EXIF数据
		image->readMetadata();
		Exiv2::ExifData& exifData = image->exifData();
		if (exifData.empty()) return std::nullopt;

		// 提取经度和纬度信息
		Exiv2::Exifdatum& longitudeDatum = exifData["Exif.GPSInfo.GPSLongitude"];
		Exiv2::Exifdatum& latitudeDatum = exifData["Exif.GPSInfo.GPSLatitude"];
		Exiv2::Exifdatum& longitudeRef = exifData["Exif.GPSInfo.GPSLongitudeRef"];
		Exiv2::Exifdatum& latitudeRef = exifData["Exif.GPSInfo.GPSLatitudeRef"];

		if (longitudeDatum.count() == 3 && latitudeDatum.count() == 3) {
			double lon = longitudeDatum.toFloat(0) + longitudeDatum.toFloat(1) / 60 + longitudeDatum.toFloat(2) / 3600;
			double lat = latitudeDatum.toFloat(0) + latitudeDatum.toFloat(1) / 60 + latitudeDatum.toFloat(2) / 3600;

			if (longitudeRef.toString() == "W") lon = -lon;
			if (latitudeRef.toString() == "S") lat = -lat;

			return Point(lon, lat);
		}
	}
	catch (Exiv2::Error& e) {
		qDebug() << "Error reading EXIF data:" << e.what();
	}

	return std::nullopt;
}

void MainWindow::openFieldNote() {
	// 获取读取文件的路径
	QString strFileName = QFileDialog::getOpenFileName(this, "打开文本文件", "", "文本文件 (*.txt)");
	if (strFileName.isEmpty()) return;
	// 重新初始化 store 以防止数据混乱
	mStore = Store();
	// 读取文件内容
	QFile file(strFileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::warning(this, "警告", "无法打开文件: " + strFileName);
		return;
	}
	QTextStream in(&file);
	QString line;
	LineString* path = nullptr;
	if (!in.atEnd()) {
		line = in.readLine();
		mstrVectorFileName = line;
		path = new LineString();
	}
	line = in.readLine();
	while (line.startsWith("No.")) {
		Point point;
		line = in.readLine();
		while (!line.startsWith("No.")) {
			QStringList keyValue = line.split("：");
			if (keyValue.size() == 2) {
				point.setFieldValue(keyValue[0], keyValue[1]);
				if (keyValue[0] == "GPS") {
					QStringList gpsValues = keyValue[1].split("，");
					if (gpsValues.size() == 2) {
						double lon = gpsValues[0].toDouble();
						double lat = gpsValues[1].toDouble();
						point.setX(lon);
						point.setY(lat);
					}
				}
			}
			if (in.atEnd()) break;
			line = in.readLine();
		}
		path->addPoint(point);
		Point* newPoint = new Point(point);
		mStore.addPoint(newPoint);
	}
	mStore.addLine(path);
	file.close();

	mpctrlScene->clear();  // 清空现有场景内容

	// 添加新图层
	QString layerName = QFileInfo(mstrVectorFileName).fileName();
	// 生成随机数种子
	srand(time(nullptr));
	// 获取随机颜色
	QColor randomColor = QColor::fromRgb(rand() % 256, rand() % 256, rand() % 256);
	Layer newLayer{ mStore, new QGraphicsItemGroup(), QPen(Qt::black), QBrush(randomColor) };
	newLayer.vectorFilePath = mstrVectorFileName; // 保存矢量图层的文件路径
	mmapNameLayer[layerName] = newLayer;

	// 更新图层树
	updateLayerTree();
	// 重新绘制所有图层
	drawLayers();

	// 使用图形项的边界来初次调整视图，使图形顶到边界
	QRectF itemsBoundingRect = mpctrlScene->itemsBoundingRect();
	QGraphicsView* view = static_cast<QGraphicsView*>(centralWidget()->layout()->itemAt(0)->widget());
	view->fitInView(itemsBoundingRect, Qt::KeepAspectRatio);

	// 更新符号系统列表
	updateSymList();
}

void MainWindow::convertPhotoToPoints() {
	QStringList fileNames = QFileDialog::getOpenFileNames(this, "选择照片", "", "Images (*.jpg *.jpeg *.png *.tiff *.tif)");
	if (fileNames.isEmpty()) return;

	QString layerName;
	if (fileNames.size() == 1) {
		// 如果只有一张照片，图层名即为文件名（不包含路径）
		QFileInfo fileInfo(fileNames.first());
		layerName = fileInfo.baseName();
	}
	else {
		// 如果有多张照片，图层命名为“图片组n”，n为当前图层数量
		layerName = QString("图片组%1").arg(mmapNameLayer.size() + 1);
	}

	// 创建空的点图层
	Store store;
	Layer newLayer{ store, new QGraphicsItemGroup(), QPen(Qt::black), QBrush(Qt::transparent), false, layerName, "" };
	newLayer.vectorFilePath = "point";

	for (const QString& fileName : fileNames) {
		// 读取照片经纬度信息
		auto point = readGeoCoordinates(fileName);
		if (point) {
			// 添加点到图层
			Point* newPoint = new Point(point->getX(), point->getY());
			newLayer.store.addPoint(newPoint);

			newPoint->setFieldValue("照片", fileName);  // 存储照片文件路径
		}
		else {
			QMessageBox::warning(this, "警告", QString("无法读取照片 %1 的地理信息").arg(fileName));
		}
	}

	if (!newLayer.store.isEmpty()) {
		// 如果图层不为空，将图层添加到图层管理中
		mmapNameLayer[layerName] = newLayer;

		// 更新图层树和符号系统列表
		updateLayerTree();
		updateSymList();
		drawLayers();
	}
	else {
		QMessageBox::information(this, "信息", "未添加任何点到图层，因为没有成功读取照片的地理信息。");
	}
}