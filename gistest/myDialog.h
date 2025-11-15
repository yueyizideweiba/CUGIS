#pragma once
#pragma once
#include<QMainWindow>
#include<QLineEdit>
#include<QLabel>
#include<QVBoxLayout>
#include<QPushButton>
#include<QDialog>
class myDialog :public QDialog
{

	Q_OBJECT
public:
	myDialog(QWidget* parent = nullptr)
	{
		QWidget* centralWidget = new QWidget(this);
		//setCentralWidget(centralWidget);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mpctrlTextBox1 = new QLineEdit(this);
		mpctrlTextBox2 = new QLineEdit(this);

		QVBoxLayout* FirstLayout = new QVBoxLayout;
		FirstLayout->addWidget(mpctrlTextBox1);
		FirstLayout->addWidget(label1);
		mainLayout->addLayout(FirstLayout);

		QVBoxLayout* SecondLayout = new QVBoxLayout;
		SecondLayout->addWidget(mpctrlTextBox2);
		SecondLayout->addWidget(label2);
		mainLayout->addLayout(SecondLayout);

		QPushButton* okButton = new QPushButton("确定", this);
		QPushButton* cancelButton = new QPushButton("取消", this);
		QHBoxLayout* buttonLayout = new QHBoxLayout;
		buttonLayout->addWidget(okButton);
		buttonLayout->addWidget(cancelButton);
		mainLayout->addLayout(buttonLayout);

		connect(okButton, &QPushButton::clicked, this, &myDialog::onFirstButtonClicked);
		connect(cancelButton, &QPushButton::clicked, this, &myDialog::onSecondButtonClicked);

		setLayout(mainLayout);
	}
signals:
	///void confirmClicked();//当点击确定按钮时
private slots:
	void onFirstButtonClicked()
	{
		FristBotton = mpctrlTextBox1->text().toInt();
		qDebug() << "" << "FristBotton; " << FristBotton;

		SecondBotton = mpctrlTextBox2->text().toInt();
		qDebug()<<""<<"SecondBotton; " << SecondBotton;
		QDialog::accept();

	}//当点击第一个按钮时
	void onSecondButtonClicked()
	{
		QDialog::reject();
	};
	//当点击第二个按钮时

public:
	QLineEdit* mpctrlTextBox1 = nullptr;
	QLineEdit* mpctrlTextBox2 = nullptr;
	QString mSelectedFile1;
	QString mSelectedFile2;
	QLabel* label1 = new QLabel("参数1", this);
	QLabel* label2 = new QLabel("参数2", this);
	int FristBotton;
	int SecondBotton;
};
