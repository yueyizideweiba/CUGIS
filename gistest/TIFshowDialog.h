#ifndef  TIFshowDialog_h
#define TIFshowDialog_h

#include <QDialog>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include<vector>
//缺少选超过三个波段的提示
//用于显示波段伪色彩
class CustomDialog : public QDialog {
public:
	//复选框
	std::vector< QCheckBox*> mvTifBand;
	std::vector<int> mvChosenBand;
	CustomDialog(int Bands, QWidget* parent = nullptr);
private slots:
	void handleOkButtonClick();
};

#endif


