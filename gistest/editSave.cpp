#include "editSave.h"

editSave::editSave(Store* pCurrentStore, MainWindow* mainWindow) : mpCurrentStore(pCurrentStore), mpMainWindow(mainWindow) {}

void editSave::execute() {
	mExecuteAddedStore.clear();
	mExecuteDeletedStore.clear();
	// 清空回收站中的要素
	for (auto point : mpMainWindow->getDeletedStore()->getStorePoints())
		mExecuteDeletedStore.addPoint(point);
	for (auto line : mpMainWindow->getDeletedStore()->getStoreLines())
		mExecuteDeletedStore.addLine(line);
	for (auto polygon : mpMainWindow->getDeletedStore()->getStorePolygons())
		mExecuteDeletedStore.addPolygon(polygon);
	mpMainWindow->getDeletedStore()->clear();
	// 将新增要素添加到当前编辑图层中
	for (auto point : mpMainWindow->getAddedStore()->getStorePoints()) {
		mpCurrentStore->addPoint(point);
		mExecuteAddedStore.addPoint(point);
	}
	for (auto line : mpMainWindow->getAddedStore()->getStoreLines()) {
		mpCurrentStore->addLine(line);
		mExecuteAddedStore.addLine(line);
	}
	for (auto polygon : mpMainWindow->getAddedStore()->getStorePolygons()) {
		mpCurrentStore->addPolygon(polygon);
		mExecuteAddedStore.addPolygon(polygon);
	}
	mpMainWindow->getAddedStore()->clear();
	mpMainWindow->drawEditLayer();
}

void editSave::undo() {
	// 将添加的要素删除
	for (auto point : mExecuteAddedStore.getStorePoints()) {
		mpCurrentStore->removePoint(point);
		mpMainWindow->getAddedStore()->addPoint(point);
	}
	for (auto line : mExecuteAddedStore.getStoreLines()) {
		mpCurrentStore->removeLineString(line);
		mpMainWindow->getAddedStore()->addLine(line);
	}
	for (auto polygon : mExecuteAddedStore.getStorePolygons()) {
		mpCurrentStore->removePolygon(polygon);
		mpMainWindow->getAddedStore()->addPolygon(polygon);
	}
	mExecuteAddedStore.clear();
	// 将删除的要素重新添加
	for (auto point : mExecuteDeletedStore.getStorePoints())
		mpMainWindow->getDeletedStore()->addPoint(point);
	for (auto line : mExecuteDeletedStore.getStoreLines())
		mpMainWindow->getDeletedStore()->addLine(line);
	for (auto polygon : mExecuteDeletedStore.getStorePolygons())
		mpMainWindow->getDeletedStore()->addPolygon(polygon);
	mExecuteDeletedStore.clear();
	mpMainWindow->drawEditLayer();
}