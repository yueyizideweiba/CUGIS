#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define NOMINMAX
#include <QMainWindow>
#include <QTreeWidget>
#include <QListWidget>
#include <QGraphicsView>
#include <QHeaderView>
#include <QGraphicsScene>
#include <QActionGroup>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QStatusBar>
#include <QWidget>
#include <QWheelEvent>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <unordered_map>
#include <QGraphicsItemGroup>
#include <QScrollBar>
#include <QToolButton>
#include <QWidgetAction>
#include <QInputDialog>
#include <QDoubleValidator>
#include <QTableWidget>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImageReader>
#include <exiv2/exiv2.hpp>
#include <queue>
#include <vector>
#include <limits>
#include <functional>
#include "drawVec.h"
#include "BufferDialog.h"
#include "RemoteControl.h"
#include "dbConnectDialog.h"
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/operation/buffer/BufferParameters.h>
#include <geos/operation/buffer/BufferOp.h>
#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <geos/triangulate/VoronoiDiagramBuilder.h>

// 前置声明
class myGraphicsView;

// 定义 QHash 函数
struct QPointFHash {
	std::size_t operator()(const QPointF& point) const {
		return std::hash<double>()(point.x()) ^ std::hash<double>()(point.y());
	}
};

// 定义比较函数
struct QPointFEqual {
	bool operator()(const QPointF& lhs, const QPointF& rhs) const {
		return qFuzzyCompare(lhs.x(), rhs.x()) && qFuzzyCompare(lhs.y(), rhs.y());
	}
};

struct DijkstraNode {
	int nodeId;
	double distance;

	bool operator>(const DijkstraNode& other) const {
		return distance > other.distance;
	}
};

struct NetworkNode {
	int id;
	QPointF position;
	std::set<int> connectedEdges; // 保存与此节点连接的边的ID
};

struct NetworkEdge {
	int id;
	int startNode;
	int endNode;
	double length;
};

class NetworkData {
public:
	std::vector<NetworkNode> nodes;
	std::vector<NetworkEdge> edges;
};

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);

	// 获取经纬度最值
	double getMinLon() const { return mdMinLon; }
	double getMaxLon() const { return mdMaxLon; }
	double getMinLat() const { return mdMinLat; }
	double getMaxLat() const { return mdMaxLat; }

	// 获取视图宽度和高度
	int getViewW() const { return mnViewWidth; }
	int getViewH() const { return mnViewHeight; }

	int getEditPointIndex() { return mnEditPointIndex; }  // 获取编辑点索引
	Store* getAddedStore() { return &mAddedStore; }  // 获取新增要素存储
	Store* getDeletedStore() { return &mDeletedStore; }  // 获取删除要素存储
	Store* getEditedStore() { return &mEditedStore; }  // 获取编辑要素存储
	LineString* getEditLineString() { return mpEditLineString; }  // 获取编辑中的线
	myPolygon* getEditPolygon() { return mpEditPolygon; }  // 获取编辑中的多边形
	drawVec getDraw() { return mDraw; }  // 获取绘图对象
	void drawEditLayer(); // 重绘编辑中的图层

	std::vector<int> performDijkstra(int startNodeId, int endNodeId, const NetworkData& networkData);
private slots:
	// 读取文件与基础功能部分
	bool ends_with(const std::string& fullString, const std::string& ending); // 判断字符串是否以指定后缀结尾
	void openProjectFile(); // 打开项目文件
	void openVectorData(); // 打开矢量数据
	void openRasterData(); // 打开栅格数据
	void readTIF(QString Path);
	void saveProject(); // 保存项目
	void exportToDatabase(); // 导出图层到数据库
	void exportSelectedLayer(); // 导出选中的图层为文件
	void undo(); // 撤销操作
	void redo(); // 重做操作
	void setZoomLevel(); // 设置缩放级别
	void resetView(); // 重置视图

	// 矢量数据分析部分
	void computeConvexHull(); // 计算凸包
	void computeBuffer(); // 计算缓冲区
	void overlayAnalysis(); // 叠加分析
	void computeDelaunay(); // 计算Delaunay三角剖分
	void computeVoronoi(); // 计算Voronoi图


	// 矢量数据统计分析部分
	void countFeatures(); // 统计要素
	void computeArea(); // 计算面积
	void computePerimeter(); // 计算周长
	void computeLength(); // 计算长度
	void onTableWidgetCellDoubleClicked(int row, int column); // 显示统计详细信息

	// 栅格数据分析部分
	void drawGrayHistogram(); // 绘制灰度直方图
	void createTrueColorImage(); // 创建真彩色图像
	void createFalseColorImage(); // 创建假彩色图像
	void extractRasterMask(); // 提取栅格掩膜
	void extractVectorMask(); // 提取矢量掩膜
	void neighborhoodMean(); // 计算邻域均值
	void neighborhoodMax(); // 计算邻域最大值
	void neighborhoodMin(); // 计算邻域最小值
	void smoothTif();//栅格平滑
	void D8plusTif();//D8+栅格流向分析
	void AspectAnaTif();//坡向计算
	void SlopeAnaTif();//坡度计算

	// 日志功能部分
	void exportLogFile(); // 导出日志文件

	// 野外实习功能部分
	void openFieldNote(); // 导入野簿文本文件生成图层

	// 编辑基础功能部分
	void setInteractionMode(QAction* action); // 设置交互模式
	void handleEditComboBoxChange(int index); // 处理编辑组合框变化
	void chooseActionTriggered(); // 选择操作
	void moveActionTriggered(); // 移动操作
	void revActionTriggered(); // 撤销操作
	void editActionTriggered(); // 编辑操作
	void showPathAnalysisSetupDialog(); // 路径分析窗口
	void prepareToSelectStartAndEndPoints(const QString& lineLayerName, const QString& pointLayerName, bool useAsViaPoints, bool useAsForbiddenPoints); // 选择起点和终点


	// 编辑创建要素功能部分
	void createPointActionTriggered(); // 创建点操作
	void createLineActionTriggered(); // 创建线操作
	void createPolygonActionTriggered(); // 创建多边形操作

	// 编辑修改要素功能部分
	void addTurningPointActionTriggered(); // 添加折点操作
	void delTurningPointActionTriggered(); // 删除折点操作
	void selTurningPointActionTriggered(); // 选择折点操作
	void moveTurningPointActionTriggered(); // 移动折点操作

	// 图层管理与符号系统设置部分
	bool startEditingLayer(); // 开始编辑图层
	void stopEditingLayer(); // 停止编辑图层
	void onSceneClicked(const QPointF& pos); // 鼠标点击事件
	void onLayerTreeContextMenu(const QPoint& point); // 删除图层

	// 皮肤切换
	void switchTheme(int index);

	// 更新状态栏
	void onStatusBarUpdated(const QString& mousePos, const QString& zoom, const QString& rotation);

	void convertPhotoToPoints();

signals:
	void rasterDataOpened(const QString& fileName);// 栅格数据打开信号

private:
	// 图层储存结构体
	struct Layer {
		Store store;
		QGraphicsItemGroup* group;
		QPen pen;
		QBrush brush;
		bool isRaster;// 栅格图层标记
		QString vectorFilePath; // 矢量图层的文件路径
		QString rasterFilePath; // 栅格图层的文件路径
	};
	NetworkData mCurrentNetworkData; // 网络数据集
	Layer* mCurrentLineLayer = nullptr;  // 用于存储当前选择的线图层
	Layer* mViaPointsLayer = nullptr;    // 用于存储路径途径点图层
	Layer* mForbiddenPointsLayer = nullptr;  // 用于存储禁止通过点图层

	std::unordered_map<QString, Layer> mmapNameLayer; // 图层哈希表

	NetworkData convertLineLayerToNetwork(const Layer& lineLayer);

	// 实现绘制灰度直方图
	void newGUIforGrayHistogram(QString strFilePath);

	// 图层与符号系统函数
	void updateSymList(); // 更新符号列表
	void addPointSymbolSettings(const QString& layerName, QPen& pen); // 添加点符号设置
	void chooseCustomPointImage(); // 选择自定义点图像
	void restoreDefaultPointStyle(); // 恢复默认点样式
	void restoreDefaultLineStyle(); // 恢复默认线样式
	void chooseCustomLineImage(); // 选择自定义线图像
	QPixmap mCustomPointImage; // 自定义点图像
	QPixmap mCustomLineImage; // 自定义线图像
	void addLineSymbolSettings(const QString& layerName, QPen& pen); // 添加线符号设置
	void addPolygonSymbolSettings(const QString& layerName, const Layer& layer); // 添加多边形符号设置
	void updateLayerTree(); // 更新图层树
	void drawLayers(); // 重绘所有图层

	void deleteLayer(QTreeWidgetItem* item); // 实现图层删除
	void moveLayerToTop(QTreeWidgetItem* item); // 图层置顶
	void moveLayerToBottom(QTreeWidgetItem* item); // 图层置底
	void onLayerItemChanged(QTreeWidgetItem* item, int column); // 触发更新符号系统显示

	// 删除编辑
	void deleteSelectedFeature();

	// 取消编辑内容
	void cancelEdit();

	// 保存编辑内容
	void saveEdit();

	// 显示和隐藏创建要素时的确认或取消按钮
	void showTemporaryButtons();

	// 显示创建要素时的确认逻辑
	void confirmCreation();

	// 显示创建要素时的取消逻辑
	void cancelCreation();

	// 矢量统计分析部分
	QString mstrCurrentStatType; // 当前统计类型
	bool checkSingleLayerSelected(QString& selectedLayerName); // 判断是否只选中一个图层
	void showStatTable(const QVector<QVector<QString>>& data, const QStringList& headers); // 显示统计结果
	void showLayerDetailStats(const QString& layerName); // 实现统计详细信息

	// GUI主要控件
	QGraphicsScene* mpctrlScene; // 图形场景
	QTreeWidget* mpctrlLayerTree; // 图层树
	QListWidget* mpctrlPropertyList; // 属性列表
	QListWidget* mpctrlSymList; // 符号列表
	QListWidget* mpctrlStatList; // 统计列表
	QActionGroup* mpActionGroup; // 操作组
	QActionGroup* mpEditGroup; // 编辑组
	QComboBox* mpctrlEditComboBox; // 编辑组合框
	QComboBox* mpctrlLayerComboBox; // 图层组合框
	QAction* mpResetAction; // 管理复位按钮
	QComboBox* mpctrlStyleComboBox;  // 皮肤切换控件
	BufferDialog* mpctrlBufferDialog; // 缓冲区参数对话框
	dbConnectDialog* mpctrlDbConnectDialog; // 数据库连接参数对话框

	QString mstrRasterFileName; // 栅格文件路径
	QString mstrVectorFileName; // 矢量文件路径

	// 选择、移动、修改、编辑、删除
	QAction* mpChooseAction;
	QAction* mpMoveAction;
	QAction* mpRevAction;
	QAction* mpEditAction;
	QAction* mpDelAction;
	myGraphicsView* mpctrlView;

	// 创建要素
	QAction* mpCreatePointAction;
	QAction* mpCreateLineAction;
	QAction* mpCreatePolygonAction;
	// 设置创建按钮样式
	void resizeEvent(QResizeEvent* event);

	// 确认或取消单次创建要素按钮
	QWidget* mpctrlButtonContainer;
	QHBoxLayout* mpButtonLayout;
	QPushButton* mpctrlConfirmButton;
	QPushButton* mpctrlCancelButton;

	// 编辑要素
	// 添加折点
	QAction* mpAddTurningPointAction;
	// 删除折点
	QAction* mpDelTurningPointAction;
	// 选择折点
	QAction* mpSelTurningPointAction;
	// 移动折点
	QAction* mpMoveTurningPointAction;

	// 可视化绘图相关变量
	QPen mPen;
	QBrush mBrush;
	Store mStore;
	double mdMinLon;
	double mdMaxLon;
	double mdMinLat;
	double mdMaxLat;
	int mnViewWidth;
	int mnViewHeight;
	drawVec mDraw;

	QString mstrCurrentEditingLayer; // 正在编辑的编辑图层名
	Store* mpCurrentEditingStore; // 选中图层的store

	// 保存与取消编辑操作
	QAction* mpSaveeditAction;
	QAction* mpCanceleditAction;

	Store mSelectedFeatures; // 当前选中的内容
	Store mAddedStore; // 即将被添加的内容
	Store mDeletedStore; // 临时存放删除内容
	Store mEditedStore; // 临时存放编辑中内容
	int mnEditPointIndex; // 某个编辑点的索引
	LineString* mpEditLineString; // 正在编辑的线
	myPolygon* mpEditPolygon; // 正在编辑的多边形

	// 从GEOS几何对象创建myPolygon
	myPolygon* createMyPolygonFromGeometry(const geos::geom::Geometry* geometry);

	// 状态栏
	QLabel* mpctrlMousePosLabel; // 鼠标位置标签
	QLabel* mpctrlZoomLabel; // 缩放标签
	QLabel* mpctrlRotationLabel; // 旋转标签
	void updateStatusBar(const QString& mousePos, const QString& zoom, const QString& rotation); // 更新状态栏信息
	void zoomToLayer(QTreeWidgetItem* item); // 缩放到图层

	// 凸包计算选择图层
	QString selectLayer(const QString& operation, bool requirePoints); // 选择要操作的图层

	void addEmptyLayer(); // 添加新要素图层

	RemoteControl mRemote; // 命令模式遥控器

	std::vector<int> performPathAnalysis(int startNodeId, int endNodeId); // 实现路径分析
	void createPathLayer(const std::vector<int>& path, const NetworkData& networkData);
	void drawPointsForSelection(const Layer& lineLayer);
	int mSelectedStartNode; // 选择的起点节点ID
	int mSelectedEndNode; // 选择的终点节点ID
	bool mIsPathAnalysisMode; // 路径分析模式标志
	std::vector<int> mAnalyzedPath; // 存储路径分析的结果
	int findNodeIdByPosition(const QPointF& position);
	void onPathClicked(const QPointF& pos);

	double convertExifToDecimal(const QString& exifCoordinate);

	void RockAna();//岩石分析

};

#endif