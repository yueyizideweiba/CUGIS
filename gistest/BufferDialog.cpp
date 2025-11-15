#include "BufferDialog.h"

// 构造函数，初始化缓冲区分析对话框
BufferDialog::BufferDialog(const QStringList& layerNames, QWidget* parent)
	: QDialog(parent) {
	// 设置窗口标题
	setWindowTitle("缓冲区分析");

	// 创建图层选择标签和下拉框
	QLabel* layerLabel = new QLabel("选择图层:");
	mctrlLayerComboBox = new QComboBox();
	mctrlLayerComboBox->addItems(layerNames);  // 向下拉框中添加图层名称

	// 创建缓冲区半径标签和数值输入框
	QLabel* radiusLabel = new QLabel("缓冲区半径:");
	mctrlRadiusSpinBox = new QDoubleSpinBox();
	mctrlRadiusSpinBox->setRange(-10000.0, 10000.0);  // 设置半径输入框的范围为-10000到10000
	mctrlRadiusSpinBox->setSingleStep(0.1);  // 设置步长为0.1米
	mctrlRadiusSpinBox->setSuffix(" 米");  // 在数值后添加"米"作为单位
	mctrlRadiusSpinBox->setValue(10);  // 设置初始值为10米

	// 创建融合选项的复选框
	QCheckBox* mergeCheckBox = new QCheckBox("融合重叠部分");

	// 创建确定和取消按钮
	QPushButton* okButton = new QPushButton("确定");
	QPushButton* cancelButton = new QPushButton("取消");

	// 连接确定按钮的点击事件，触发对话框的accept()方法
	connect(okButton, &QPushButton::clicked, this, &BufferDialog::accept);
	// 连接取消按钮的点击事件，触发对话框的reject()方法
	connect(cancelButton, &QPushButton::clicked, this, &BufferDialog::reject);

	// 创建图层选择部分的布局
	QHBoxLayout* layerLayout = new QHBoxLayout();
	layerLayout->addWidget(layerLabel);
	layerLayout->addWidget(mctrlLayerComboBox);

	// 创建缓冲区半径部分的布局
	QHBoxLayout* radiusLayout = new QHBoxLayout();
	radiusLayout->addWidget(radiusLabel);
	radiusLayout->addWidget(mctrlRadiusSpinBox);

	// 创建融合选项部分的布局
	QHBoxLayout* mergeLayout = new QHBoxLayout();
	mergeLayout->addWidget(mergeCheckBox);

	// 创建按钮部分的布局
	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();  // 在按钮前添加伸缩空间，使按钮靠右对齐
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);

	// 创建主布局，并将各部分布局添加到主布局中
	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(layerLayout);
	mainLayout->addLayout(radiusLayout);
	mainLayout->addLayout(mergeLayout);
	mainLayout->addLayout(buttonLayout);

	// 设置对话框的主布局
	setLayout(mainLayout);

	// 保存融合复选框的指针到成员变量
	mctrlMergeCheckBox = mergeCheckBox;
}

// 获取缓冲区半径的值
double BufferDialog::getBufferRadius() const {
	return mctrlRadiusSpinBox->value();
}

// 获取用户选择的图层名称
QString BufferDialog::getSelectedLayer() const {
	return mctrlLayerComboBox->currentText();
}

// 检查是否选中了融合复选框
bool BufferDialog::isMergeChecked() const {
	return mctrlMergeCheckBox->isChecked();
}
