#include "DebugWindows.h"
#include "ui_debugwindows.h"
#include <QLayout>
#include <QIntValidator>

DebugWindows::DebugWindows(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugWindows)
{
    ui->setupUi(this);
    this->setWindowTitle("Debug");
    this->setWindowIcon(QIcon(":/img/Logo.ico"));
    setFixedSize(this->frameGeometry().width(),this->frameGeometry().height());
    InitGUI();

}

void DebugWindows::InitGUI()
{
    ui->lineEdit_PwdCnt->setEchoMode(QLineEdit::Normal);
    ui->lineEdit_Pwd->setEchoMode(QLineEdit::PasswordEchoOnEdit);

    QIntValidator *pIntVal = new QIntValidator(this);
    pIntVal->setRange(1, 2);

    ui->lineEdit_PwdCnt->setValidator(pIntVal);
}


DebugWindows::~DebugWindows()
{
    delete ui;
}

void DebugWindows::on_pushButton_SetPwdCnt_clicked()
{
    QString testStr;
    testStr = ui->lineEdit_PwdCnt->text() + "\r\n" + ui->lineEdit_Pwd->text();

    QLabel *pStr = new QLabel;
    pStr->setText(testStr);

    /*QVBoxLayout *gLayout = new QVBoxLayout;
    gLayout->addWidget(pStr);

    this->setLayout(gLayout);
    return;*/

    ui->label_3->setText(testStr);
}

void DebugWindows::on_pushButton_Close_clicked()
{
    this->close();
}
