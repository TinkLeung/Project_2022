#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QListWidget>
#include <QHBoxLayout>
#include <QTableWidget>
#include "CommonFunc.h"
#include "PhysicalDriveInfo.h"
#include "CBadBlkView.h"
#include "DebugWindows.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_Button_Scan_clicked();
    void on_Button_Save_clicked();
    void on_List_Command_itemDoubleClicked(QListWidgetItem *item);
    void on_comboBox_Disklist_currentIndexChanged(int index);

private:
    void InitStatuBar();
    void InitMenuBar();
    void InitGUI();
    void GetCurrentDrive(int index);
    bool PhysicalDriveInit();
    void RefreshLayout(QHBoxLayout *layout);
    void InitTableWidget(QTableWidget *infoTbl);

    void UIDisplay_DiskInfo();
    void UIDisplay_FormatedInfo(QList<SPEC_ITEM> *infoList);
    void UIDisplay_RawData(const unsigned char *dataPtr, int length);

    void UILoad_DebugView();

    void SetControlState(bool bState);
    //UI init
    void InitCommandList();

    #ifdef WIN32
    bool InitDriveInfo(int dev);
    #else
    bool InitDriveInfo(char* dev);
    #endif

public:
    QHBoxLayout *textLayout;
    PhysicalDriveInfo *physicalDriveInfo;
    uint64_t ErrEntryCnt;
    bool ReadyFlg;
    char ItemFlg;

    CBabBlkView *mBBInfo;


public:
    enum CommandListItem
    {       
       IDY_CTRL_ITEM = 0,
       IDY_NS_ITEM,
       SMART_INFO_ITEM,
       ERROR_LOG_ITEM,
       EVENT_LOG_ITEM,
       BBT_ITEM,
       FLASH_ID_ITEM,

    };
    Q_ENUM(CommandListItem)

private:
    Ui::MainWindow *ui;
};



#endif // MAINWINDOW_H
