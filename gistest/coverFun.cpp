#pragma once
#include"coverFun.h"
CoverDialog::CoverDialog(QWidget* parent) : QMainWindow(parent)
{
	QWidget* centralWidget = new QWidget(this);
	setCentralWidget(centralWidget);

	QVBoxLayout* mainLayout = new QVBoxLayout;

	//**********************************************添加文本框和按钮**********************************************
	QLabel* label1 = new QLabel("输入栅格", this);
	QLabel* label2 = new QLabel("输入掩膜范围", this);
	QLabel* label3 = new QLabel("保存路径", this);
	QLabel* label4 = new QLabel("保存文件名", this);
	

	mpctrlTextBox1 = new QLineEdit(this);
	mpctrlTextBox2 = new QLineEdit(this);
	mpctrlTextBox3 = new QLineEdit(this);
	mpctrlTextBox4 = new QLineEdit(this);


	

	QPushButton* firstButton = new QPushButton("选择", this);
	QPushButton* secondButton = new QPushButton("选择", this);
	QPushButton* thirdButton = new QPushButton("选择", this);
	QPushButton* fourthButton = new QPushButton("OK", this);


	QPushButton* okButton = new QPushButton("确定", this);
	QPushButton* cancelButton = new QPushButton("取消", this);

	QHBoxLayout* textBoxLayout1 = new QHBoxLayout;
	textBoxLayout1->addWidget(label1);
	textBoxLayout1->addWidget(mpctrlTextBox1);
	textBoxLayout1->addWidget(firstButton);

	QHBoxLayout* textBoxLayout2 = new QHBoxLayout;
	textBoxLayout2->addWidget(label2);
	textBoxLayout2->addWidget(mpctrlTextBox2);
	textBoxLayout2->addWidget(secondButton);

	QHBoxLayout* textBoxLayout3 = new QHBoxLayout;
	textBoxLayout3->addWidget(label3);
	textBoxLayout3->addWidget(mpctrlTextBox3);
	textBoxLayout3->addWidget(thirdButton);

	QHBoxLayout* textBoxLayout4 = new QHBoxLayout;
	textBoxLayout4->addWidget(label4);
	textBoxLayout4->addWidget(mpctrlTextBox4);
	textBoxLayout4->addWidget(fourthButton);

	mainLayout->addLayout(textBoxLayout1);
	mainLayout->addLayout(textBoxLayout2);
	mainLayout->addLayout(textBoxLayout3);
	mainLayout->addLayout(textBoxLayout4);
	mainLayout->addWidget(okButton);
	mainLayout->addWidget(cancelButton);

	//**********************************************信号与槽****************************************
	connect(firstButton, &QPushButton::clicked, this, &CoverDialog::onFirstButtonClicked);
	connect(secondButton, &QPushButton::clicked, this, &CoverDialog::onSecondButtonClicked);
	connect(thirdButton, &QPushButton::clicked, this, &CoverDialog::onThirdButtonClicked);
	connect(fourthButton, &QPushButton::clicked, this, &CoverDialog::onFourthButtonClicked);
	connect(okButton, &QPushButton::clicked, this, &CoverDialog::onOkButtonClicked);
	connect(cancelButton, &QPushButton::clicked, this, &CoverDialog::onCancelButtonClicked);
	centralWidget->setLayout(mainLayout);
};

void CoverDialog::onFourthButtonClicked()//添加文件后缀
{
	mSelectedFile4 = mpctrlTextBox4->text() + ".tif";
};

void CoverDialog::onFirstButtonClicked()//打开文件对话框
{
	QString fileName = QFileDialog::getOpenFileName(this, "选择文件", "", "Image Files (*.tif)");
	if (!fileName.isEmpty()) {
		mpctrlTextBox1->setText(fileName);
		mSelectedFile1 = fileName;
	}
}

void CoverDialog::onSecondButtonClicked()//打开文件对话框
{
	QString fileName = QFileDialog::getOpenFileName(this, "选择文件", "", "Image Files (*.tif);;Shapefiles (*.shp);;WKT Files (*.wkt);");
	if (!fileName.isEmpty()) {
		mpctrlTextBox2->setText(fileName);
		mSelectedFile2 = fileName;
	}
}
void CoverDialog::onThirdButtonClicked()//打开文件夹对话框
{
	QString folderPath = QFileDialog::getExistingDirectory(
		nullptr,                          // 父窗口
		"Select Directory",               // 对话框标题
		"",                               // 起始目录
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks // 只显示文件夹
	);
	mpctrlTextBox3->setText(folderPath);
	mSelectedFile3 = folderPath;
}
void CoverDialog::onOkButtonClicked()//确认按钮
{
	mSelectedFile1 = mpctrlTextBox1->text();
	mSelectedFile2 = mpctrlTextBox2->text();
	mSelectedFile3 = mpctrlTextBox3->text();
	mSelectedFile4 = mpctrlTextBox4->text() + ".tif";

	emit confirmClicked();

	close(); // 关闭窗口
}
void CoverDialog::onCancelButtonClicked()
{
	close(); // 关闭窗口
}

