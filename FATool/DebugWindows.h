#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <QWidget>

namespace Ui {
class DebugWindows;
}

class DebugWindows : public QWidget
{
    Q_OBJECT

public:
    explicit DebugWindows(QWidget *parent = nullptr);
    ~DebugWindows();


public:
    void InitGUI();

private slots:
    void on_pushButton_SetPwdCnt_clicked();
    void on_pushButton_Close_clicked();

private:
    Ui::DebugWindows *ui;
};

#endif // DEBUGWINDOWS_H
