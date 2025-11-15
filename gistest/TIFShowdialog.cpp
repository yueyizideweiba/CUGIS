#include"TIFShowdialog.h"

CustomDialog::CustomDialog(int Bands, QWidget* parent) : QDialog(parent) {
	setWindowTitle("波段选择");
	resize(300, 200);
	QVBoxLayout* layout = new QVBoxLayout(this);
	for (int i = 0; i < Bands; i++)
		// 添加复选框
	{
		QCheckBox* checkbox = new QCheckBox(QString::number(i), this);
		layout->addWidget(checkbox);
		mvTifBand.push_back(checkbox);
		qDebug() << "初始化波段：" << i;
	}
	// 添加按钮
	QPushButton* okButton = new QPushButton("确定", this);
	connect(okButton, &QPushButton::clicked, this, &CustomDialog::handleOkButtonClick);
	layout->addWidget(okButton);
	setLayout(layout);
};
void CustomDialog::handleOkButtonClick() {
	// ChosenBand.clear();
	 // 检查复选框状态并记录选中的索引
	for (int i = 0; i < mvTifBand.size(); ++i) {
		if (mvTifBand[i]->isChecked()) {
			mvChosenBand.push_back(i);
			qDebug() << "选中了波段：" << i;
		}
	}
	accept(); // 关闭对话框
};