#ifndef DBCONNECTDIALOG_H
#define DBCONNECTDIALOG_H

#include <QApplication>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

class dbConnectDialog : public QDialog {
	Q_OBJECT

public:
	dbConnectDialog(QWidget* parent);

	QString getHostName() const;
	QString getDatabaseName() const;
	QString getPort() const;
	QString getUserName() const;
	QString getPassword() const;

private:
	QLineEdit* mpctrlHostName; // 主机名或IP地址
	QLineEdit* mpctrlDatabaseName; // 数据库名称
	QLineEdit* mpctrlPort; // 端口号
	QLineEdit* mpctrlUserName; // 用户名
	QLineEdit* mpctrlPassword; // 密码
};

#endif