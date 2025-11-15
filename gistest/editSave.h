#ifndef EDITSAVE_H
#define EDITSAVE_H

#include"Command.h"
#include"GISlog.h"
#include"Store.h"
#include "mainwindow.h"

// 前置声明
class MainWindow;

class editSave :public Command {
public:
	explicit editSave(Store* pCurrentStore, MainWindow* mainWindow);

	void execute() override;

	void undo() override;

private:
	Store* mpCurrentStore; // 指向主窗口当前图层
	Store mExecuteAddedStore; // 执行中添加的要素
	Store mExecuteDeletedStore; // 执行中删除的要素
	MainWindow* mpMainWindow;
};

#endif