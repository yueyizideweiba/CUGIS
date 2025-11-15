#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	// 启动页
	QPixmap pixmap("./icon/start2.png");
	pixmap = pixmap.scaled(QSize(800, 800), Qt::KeepAspectRatio);
	QSplashScreen splash(pixmap);
	splash.show();

	QTimer::singleShot(1000, &splash, &QSplashScreen::close); // 3秒后关闭启动页

	MainWindow mainWindow;
	QTimer::singleShot(1000, &mainWindow, &QMainWindow::show); // 3秒后显示主窗口

	return app.exec();
}
