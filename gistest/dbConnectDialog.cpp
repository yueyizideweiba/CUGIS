#include "dbConnectDialog.h"

dbConnectDialog::dbConnectDialog(QWidget* parent = nullptr) : QDialog(parent) {
    QFormLayout* formLayout = new QFormLayout(this);
    // 设置窗口标题
    setWindowTitle("数据库连接");
    // 创建输入字段
    mpctrlHostName = new QLineEdit(this);
    mpctrlHostName->setText("localhost");  // 设置默认值
    mpctrlDatabaseName = new QLineEdit(this);
    mpctrlPort = new QLineEdit(this);
    mpctrlPort->setText("5432");  // 设置默认值
    mpctrlUserName = new QLineEdit(this);
    mpctrlPassword = new QLineEdit(this);
    // 设置密码字段的回显模式为密码模式
    mpctrlPassword->setEchoMode(QLineEdit::Password);
    // 将字段添加到表单布局
    formLayout->addRow("HostName:", mpctrlHostName);
    formLayout->addRow("DatabaseName:", mpctrlDatabaseName);
    formLayout->addRow("Port:", mpctrlPort);
    formLayout->addRow("UserName:", mpctrlUserName);
    formLayout->addRow("Password:", mpctrlPassword);
    // 创建按钮
    QPushButton* okButton = new QPushButton("确定");
    QPushButton* cancelButton = new QPushButton("取消");
    // 连接按钮的信号和槽
    connect(okButton, &QPushButton::clicked, this, &dbConnectDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &dbConnectDialog::reject);
    // 创建按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    // 将按钮布局添加到表单布局
    formLayout->addRow(buttonLayout);
    setLayout(formLayout);
}

QString dbConnectDialog::getHostName() const { return mpctrlHostName->text(); }
QString dbConnectDialog::getDatabaseName() const { return mpctrlDatabaseName->text(); }
QString dbConnectDialog::getPort() const { return mpctrlPort->text(); }
QString dbConnectDialog::getUserName() const { return mpctrlUserName->text(); }
QString dbConnectDialog::getPassword() const { return mpctrlPassword->text(); }