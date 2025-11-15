/************************************************************
FileName: BufferDialog.h
Author: 谢宇瀚
Version : 1.9
Date: 2024-07-28
Description: 缓冲区设置对话框类，用于用户输入缓冲区参数
Function List:
1. BufferDialog - 构造函数，初始化对话框及界面元素
2. getBufferRadius - 获取用户输入的缓冲区半径
3. getSelectedLayer - 获取用户选择的图层名称
4. isMergeChecked - 检查用户是否勾选了合并缓冲区选项
***********************************************************/

#ifndef BUFFERDIALOG_H
#define BUFFERDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>

class BufferDialog : public QDialog {
	Q_OBJECT

public:
	BufferDialog(const QStringList& layerNames, QWidget* parent = nullptr);

	double getBufferRadius() const; // 获取缓冲区半径
	QString getSelectedLayer() const; // 获取选中的图层
	bool isMergeChecked() const; // 检查是否勾选了合并选项

private:
	QComboBox* mctrlLayerComboBox; // 图层选择组合框
	QDoubleSpinBox* mctrlRadiusSpinBox; // 半径输入框
	QCheckBox* mctrlMergeCheckBox; // 合并选项复选框
};

#endif
