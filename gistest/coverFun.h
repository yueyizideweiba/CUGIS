#ifndef _COVERFUN
#define _COVERFUN

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include<QLabel>
#include <QProgressBar>
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>

class CoverDialog : public QMainWindow
{
	Q_OBJECT

public:
	CoverDialog(QWidget* parent = nullptr);
signals:
	void confirmClicked();//当点击确定按钮时

private slots:
	void onFirstButtonClicked();//当点击第一个按钮时
	void onSecondButtonClicked();//当点击第二个按钮时
	void onThirdButtonClicked();//当点击第三个按钮时
	void onFourthButtonClicked();//当点击第四个按钮时
	void onOkButtonClicked();//当点击确定按钮时
	void onCancelButtonClicked();//当点击取消按钮时
public:
	QLineEdit* mpctrlTextBox1 = nullptr;
	QLineEdit* mpctrlTextBox2 = nullptr;
	QLineEdit* mpctrlTextBox3 = nullptr;
	QLineEdit* mpctrlTextBox4 = nullptr;
	QString mSelectedFile1;
	QString mSelectedFile2;
	QString mSelectedFile3;
	QString mSelectedFile4;
	//QProgressBar* progressBar;
};

#endif

