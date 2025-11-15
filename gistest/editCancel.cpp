#include "editCancel.h"

editCancel::editCancel(Store* pCurrentStore, MainWindow* mainWindow) : mpCurrentStore(pCurrentStore), mpMainWindow(mainWindow) {}

void editCancel::execute() {
	mExecuteAddedStore.clear();
	mExecuteDeletedStore.clear();
	// 清空要添加的要素
	for (auto point : mpMainWindow->getAddedStore()->getStorePoints())
		mExecuteDeletedStore.addPoint(point);
	for (auto line : mpMainWindow->getAddedStore()->getStoreLines())
		mExecuteDeletedStore.addLine(line);
	for (auto polygon : mpMainWindow->getAddedStore()->getStorePolygons())
		mExecuteDeletedStore.addPolygon(polygon);
	mpMainWindow->getAddedStore()->clear();
	// 将回收站的要素添加回当前图层
	for (auto point : mpMainWindow->getDeletedStore()->getStorePoints()) {
		mpCurrentStore->addPoint(point);
		mExecuteAddedStore.addPoint(point);
	}
	for (auto line : mpMainWindow->getDeletedStore()->getStoreLines()) {
		mpCurrentStore->addLine(line);
		mExecuteAddedStore.addLine(line);
	}
	for (auto polygon : mpMainWindow->getDeletedStore()->getStorePolygons()) {
		mpCurrentStore->addPolygon(polygon);
		mExecuteAddedStore.addPolygon(polygon);
	}
	mpMainWindow->getDeletedStore()->clear();
	mpMainWindow->drawEditLayer();
}

void editCancel::undo() {
	// 将添加的要素删除
	for (auto point : mExecuteAddedStore.getStorePoints()) {
		mpCurrentStore->removePoint(point);
		mpMainWindow->getDeletedStore()->addPoint(point);
	}
	for (auto line : mExecuteAddedStore.getStoreLines()) {
		mpCurrentStore->removeLineString(line);
		mpMainWindow->getDeletedStore()->addLine(line);
	}
	for (auto polygon : mExecuteAddedStore.getStorePolygons()) {
		mpCurrentStore->removePolygon(polygon);
		mpMainWindow->getDeletedStore()->addPolygon(polygon);
	}
	mExecuteAddedStore.clear();
	// 将删除的要素重新添加
	for (auto point : mExecuteDeletedStore.getStorePoints())
		mpMainWindow->getAddedStore()->addPoint(point);
	for (auto line : mExecuteDeletedStore.getStoreLines())
		mpMainWindow->getAddedStore()->addLine(line);
	for (auto polygon : mExecuteDeletedStore.getStorePolygons())
		mpMainWindow->getAddedStore()->addPolygon(polygon);
	mExecuteDeletedStore.clear();
	mpMainWindow->drawEditLayer();
}