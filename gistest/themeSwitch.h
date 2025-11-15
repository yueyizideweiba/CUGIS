#ifndef THEMESWITCH_H
#define THEMESWITCH_H

#include"Command.h"
#include"GISlog.h"
#include <QFile>
#include <QTextStream>
#include <QApplication>

class themeSwitch :public Command {
public:
	explicit themeSwitch(int index) : mnThemeIndex(index) {}

	void execute() override {
		mnPreviousThemeIndex = mnCurrentThemeIndex;
		changeTheme(mnThemeIndex);
		Logger::getInstance().logFunctionSuccess("修改主题风格");
	}

	void undo() override {
		changeTheme(mnPreviousThemeIndex);
		Logger::getInstance().logFunctionUndo("修改主题风格");
	}

	void changeTheme(int index) {
		QString strFilePath;
		if (index == 1) {
			// 加载清新样式表
			QFile file("./lightgreen.qss");
			if (file.open(QFile::ReadOnly | QFile::Text)) {
				QTextStream stream(&file);
				QString styleSheet = stream.readAll();
				file.close();
				qApp->setStyleSheet(styleSheet);
				mnThemeIndex = 1;
			}
		}
		else if (index == 2) {
			// 加载夜间模式样式表
			QFile file("./nightmode.qss");
			if (file.open(QFile::ReadOnly | QFile::Text)) {
				QTextStream stream(&file);
				QString styleSheet = stream.readAll();
				file.close();
				qApp->setStyleSheet(styleSheet);
				mnThemeIndex = 2;
			}
		}
		else if (index == 3) {
			// 加载粉色模式样式表
			QFile file("./pinkmode.qss");
			if (file.open(QFile::ReadOnly | QFile::Text)) {
				QTextStream stream(&file);
				QString styleSheet = stream.readAll();
				file.close();
				qApp->setStyleSheet(styleSheet);
				mnThemeIndex = 3;
			}
		}
		else {
			// 恢复默认样式
			qApp->setStyleSheet("");
			mnThemeIndex = 0;
		}
		mnPreviousThemeIndex = mnThemeIndex;
		mnCurrentThemeIndex = mnThemeIndex;
	}
private:
	int mnThemeIndex;
	int mnCurrentThemeIndex = 0;
	int mnPreviousThemeIndex = 0;
};

#endif