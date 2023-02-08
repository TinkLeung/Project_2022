#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
#include <QDebug>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include "FolderOperation.h"

#ifndef WIN32
#include "linux/nvme_cpp.h"
#else
#include "windows/SlotSpeedGetter.h"
#endif

extern PCI_LNK LnkSts;
extern PCI_LNK LnkCap;

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("XITC FATool_Ver 1.0.0");
    this->setWindowIcon(QIcon(":/img/Logo.ico"));
    setFixedSize(this->frameGeometry().width(),this->frameGeometry().height());
    textLayout = new QHBoxLayout(ui->frame_mainarea);
    //this->setLayout(textLayout);

    ReadyFlg = false;
    ItemFlg = 0;
    InitCommandList();
    InitStatuBar();
    InitMenuBar();
    InitGUI();

    /*PhysicalDriveInit();
    if(ui->comboBox_Disklist->currentText() != NULL)
        ui->comboBox_Disklist->setCurrentIndex(0);*/
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitStatuBar()
{
    statusBar()->setSizeGripEnabled(false);
    QLabel *firmName = new QLabel();
    firmName->setFrameStyle(QFrame::NoFrame);
    firmName->setText("2022-2023 @ 江苏芯盛智能科技有限公司");
    statusBar()->addPermanentWidget(firmName);

    QLabel *wetSite = new QLabel();
    wetSite->setFrameStyle(QFrame::Sunken);
    wetSite->setText(tr("<a href=\"http://www.xitcorp.com\">www.xitcorp.com</a>"));
    wetSite->setOpenExternalLinks(true);
    statusBar()->addPermanentWidget(wetSite);

   /* QWidget *spaceWidget = new QWidget();
    spaceWidget->setFixedWidth(600);
    spaceWidget->setVisible(true);
    spaceWidget->setHidden(true);
    statusBar()->addPermanentWidget(spaceWidget);*/

}

void MainWindow::InitMenuBar()
{
    QMenu *pOptMenu = menuBar()->addMenu("Options");
    QMenu *pDbgMenu = menuBar()->addMenu("Debug");
    QMenu *pHelpMenu = menuBar()->addMenu("Help");

    QAction *pRescanAction = pOptMenu->addAction("ReScan");
    connect(pRescanAction, &QAction::triggered, [=](){
        this->on_Button_Scan_clicked();
    });

    QAction *pSaveFiles = pOptMenu->addAction("Save Files");
    connect(pSaveFiles, &QAction::triggered, [=](){
        this->on_Button_Save_clicked();
    });

    QAction *pLoadDbgDialog = pDbgMenu->addAction("Debug");
    connect(pLoadDbgDialog, &QAction::triggered, [=](){
        this->UILoad_DebugView();
    });

    QAction *pExit = pHelpMenu->addAction("Exit");
    connect(pExit, &QAction::triggered, [=](){
        this->close();
    });

}

void MainWindow::InitGUI()
{
    ui->label_disk_info->clear();
    ui->label_diagnostic->clear();
    RefreshLayout(textLayout);
    QTableWidget *TableInfo = new QTableWidget();
    QStringList TableHeaderList;

    InitTableWidget(TableInfo);
    TableInfo->setColumnCount(2);
    TableHeaderList<<"Description"<<"Value";
    TableInfo->setHorizontalHeaderLabels(TableHeaderList);
    textLayout->addWidget(TableInfo);
    textLayout->setContentsMargins(0,0,0,0);
    //ui->textBrowser_mainTXT->clear();
}

void MainWindow::InitCommandList()
{
    ui->List_Command->setViewMode(QListView::ListMode);
    ui->List_Command->setSelectionMode(QAbstractItemView::SingleSelection);    
    ui->List_Command->addItem("Identify-Ctrl");
    ui->List_Command->addItem("Identify-Namespace");
    ui->List_Command->addItem("SMART Log");
    ui->List_Command->addItem("Error Log");
    ui->List_Command->addItem("Event Log");    
    ui->List_Command->addItem("Bad Block Table");
    ui->List_Command->addItem("Read Flash ID");
}

void MainWindow::SetControlState(bool bState)
{
    ui->List_Command->setEnabled(bState);
    ui->Button_Save->setEnabled(bState);
    ui->Button_Scan->setEnabled(bState);
    ui->comboBox_Disklist->setEnabled(bState);
}

bool MainWindow::PhysicalDriveInit()
{
#ifdef WIN32
    int i;
    HANDLE  drvHandle;
    TCHAR   ucbuffer[256] = {0};
    QString ComboxDrive_item = "";
    QString strModelNumber = "";
    OSSlotMaxCurrSpeed lnkSpeed;
    for (i = 0; i < 64; i++)
    {
        wsprintf(ucbuffer, (L"\\\\.\\PHYSICALDRIVE%d"), i);

        drvHandle = CreateFile(ucbuffer,GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
        if (drvHandle != INVALID_HANDLE_VALUE)
        {
            //QMessageBox::about(this, "Scan Dev","Test1");
            char local_buffer[4096];
            char modelNumber[64];
            memset(local_buffer, 0, sizeof(local_buffer));
            NVMeApp nvmeApp;
            nvmeApp.DeviceQueryProperty(drvHandle,(LPVOID)&local_buffer);
            //QMessageBox::about(this, "Scan Dev","Test2");
            STORAGE_DEVICE_DESCRIPTOR *descrip = (STORAGE_DEVICE_DESCRIPTOR *)&local_buffer;
            flipAndCodeBytes(local_buffer,descrip->ProductIdOffset,0, modelNumber);
            strModelNumber = QString(QLatin1String(modelNumber));
            //if(strModelNumber.indexOf("XITC") > 0)
            {
                CloseHandle(drvHandle);
                lnkSpeed = GetPCIeSlotSpeed(i);
                LnkSts.LinkSpeed = lnkSpeed.Current.SpecVersion;
                LnkSts.LinkWidth = lnkSpeed.Current.LinkWidth;
                LnkCap.LinkSpeed = lnkSpeed.Maximum.SpecVersion;
                LnkCap.LinkWidth = lnkSpeed.Maximum.SpecVersion;
                InitDriveInfo(i);
                ComboxDrive_item.sprintf("PHYSICALDRIVE%d", i);
                ui->comboBox_Disklist->addItem(ComboxDrive_item);
                ui->comboBox_Disklist->setCurrentText(ComboxDrive_item);
            }
        }       
        /*else {
            DWORD Status = GetLastError();
            QString StrSts;
            StrSts.sprintf("Can not Open Drive,Err:%d",Status);
            QMessageBox::about(this, "Scan Dev",StrSts);
        }*/
        CloseHandle(drvHandle);
    }

    SetControlState(true);
    return true;
#else
    struct nvme_topology t;
    struct nvme_subsystem *s;
    struct nvme_ctrl *c;
    struct nvme_namespace *n;
    QString ComboxDrive_item;
    QString strModelNumber;
    int ret=0;
    int i,j,k;
    //std::string ComboxDrive_item;


    ret = scan_subsystems(&t);
    if(ret)
    {
        printf("Failed to scan namespaces\n");
        return false;
    }

    for(i=0;i<t.nr_subsystems;i++)
    {
        s=&t.subsystems[i];
        for (j = 0; j < s->nr_ctrls; j++)
        {
            c = &s->ctrls[j];
            for(k=0;k<c->nr_namespaces;k++)
            {
                n = &c->namespaces[k];
                strModelNumber = QString(QLatin1String(n->ctrl->id.mn));
                //if(strModelNumber.indexOf("XITC") > 0)
                {
                    physicalDriveInfo->GetDriveLinkStatus(n);
                    InitDriveInfo(n->name);
                    ComboxDrive_item = n->name;

                    ui->comboBox_Disklist->addItem(ComboxDrive_item);
                    //ui->comboBox_Disklist->setCurrentText(ComboxDrive_item);
                }
            }
        }
    }

    free_topology(&t);
    //printf("drive init success\n");
    SetControlState(true);
    return true;

#endif

}

#ifdef WIN32
bool MainWindow::InitDriveInfo(int i)
{
    physicalDriveInfo = new PhysicalDriveInfo(i);
    return true;
}
#else
bool MainWindow::InitDriveInfo(char* dev)
{
    physicalDriveInfo = new PhysicalDriveInfo(dev);
    return true;
}
#endif


void MainWindow::GetCurrentDrive(int index)
{
    QString strDrv = "";
#ifdef WIN32
    int DrvNum;
    strDrv = ui->comboBox_Disklist->itemText(index);
    DrvNum = strDrv.mid(strDrv.size()-1,1).toInt();
    //strDrv.sprintf("Drive Num:%d",DrvNum);
    //QMessageBox::about(this,"Test_Msg", strDrv);
    physicalDriveInfo->UpdateDriveInfo(DrvNum);
#else
    char *ptrCh = nullptr;
    QByteArray ba = nullptr;
    strDrv = ui->comboBox_Disklist->itemText(index);
    ba = strDrv.toLatin1();
    ptrCh = ba.data();
    physicalDriveInfo->UpdateDriveInfo(ptrCh);
#endif
}

void MainWindow::on_Button_Scan_clicked()
{    
    SetControlState(false);
    ui->comboBox_Disklist->clear();
    if(!PhysicalDriveInit())
    {
        QMessageBox::warning(this, "Search Disk Error", "No Drive found!");
        return;
    }
    ItemFlg = 0;
    if(ui->comboBox_Disklist->currentText() != NULL)
        ui->comboBox_Disklist->setCurrentIndex(0);
}

void MainWindow::on_List_Command_itemDoubleClicked(QListWidgetItem *item)
{    
    if(!ReadyFlg)
    {
        //QApplication::setQuitOnLastWindowClosed(false);
        QMessageBox::warning(this, "Attention", "Pelease Select Disk first!!");
        return;
    }
    //clear layout widget first
    RefreshLayout(textLayout);

    switch(ui->List_Command->currentRow())
    {        
        case IDY_CTRL_ITEM:
            UIDisplay_FormatedInfo(ParseIdyCtrlData(physicalDriveInfo->m_pIDYControllerInfo));
            ItemFlg |=BIT0;
            break;
        case IDY_NS_ITEM:
            UIDisplay_FormatedInfo(ParseIdyCtrNSlData(physicalDriveInfo->m_pIDYNamespaceInfo));
            ItemFlg |=BIT1;
            break;
        case SMART_INFO_ITEM:
            UIDisplay_FormatedInfo(ParseSmartData(physicalDriveInfo->m_pSmartInfo));
            ItemFlg |=BIT2;
            break;
        case ERROR_LOG_ITEM:
            UIDisplay_FormatedInfo(ParseErrorLog(physicalDriveInfo->m_pErrorLog,ErrEntryCnt));
            ItemFlg |=BIT3;
            break;
        case EVENT_LOG_ITEM:
            UIDisplay_RawData(physicalDriveInfo->m_pSmartInfo, SMART_DATA_LENGTH);
            ItemFlg |=BIT4;
            break;

        case BBT_ITEM:
            if(ERROR_SUCCESS_STATUS == physicalDriveInfo->GetBadBlockInfo())
            {
                mBBInfo = new CBabBlkView(this);
                if(mBBInfo)
                {
                    textLayout->addWidget(mBBInfo);
                    textLayout->setContentsMargins(0, 0, 0, 0);
                }
            }
            ItemFlg |=BIT5;
            break;
        case FLASH_ID_ITEM:
        {
            if(ERROR_SUCCESS_STATUS == physicalDriveInfo->GetFlashIDInfo(physicalDriveInfo->m_pFlashIDInfo))
            {
                UIDisplay_RawData(physicalDriveInfo->m_pFlashIDInfo, FLASH_ID_DATA_LENGTH);
            }
            ItemFlg |=BIT6;
            break;
        }

        default:
        ;
    }

}

void MainWindow::on_comboBox_Disklist_currentIndexChanged(int index)
{

    ReadyFlg = false;
    ItemFlg = 0;
    InitGUI();
    GetCurrentDrive(index);
    if(physicalDriveInfo->ulIdentifyCtrlSts != ERROR_SUCCESS_STATUS ||
       physicalDriveInfo->ulIdentifyNSSts != ERROR_SUCCESS_STATUS ||
       physicalDriveInfo->ulSMARTSts != ERROR_SUCCESS_STATUS)
    {
        return;
    }
    UIDisplay_DiskInfo();
}

void MainWindow::on_Button_Save_clicked()
{
    char Folder[64] = {0};
    QString PublicPath, FilePath;
    QList<SPEC_ITEM>*infoList = nullptr;

    if(!ReadyFlg) //disk does not ready
    {
        QMessageBox::warning(this, "Save Files Error", "Please Select a Drive!");
        return;
    }

    if(!ItemFlg) //Select save item
    {
        QMessageBox::warning(this, "Save Files Error", "Please Select a function item!");
        return;
    }

    SetControlState(false);
    FolderOperation *FolderOp = new FolderOperation;
    if(!FolderOp->Folder_CreateFileFolder(Folder))
    {
        QMessageBox::warning(this, "Save Files Error", "Create Folder Fail!");
        return;
    }
    PublicPath = QString(QLatin1String(Folder))+ "/" ;
    //PublicPath = QString(QLatin1String(Folder));
    //PublicPath = QCoreApplication::applicationDirPath() + "/" + PublicPath + "/" ;


    if(ItemFlg & BIT0)
    {
        FilePath = PublicPath + "IdentifyControllerInfo.txt";
        infoList = ParseIdyCtrlData(physicalDriveInfo->m_pIDYControllerInfo);
        if(!FolderOp->Folder_SaveTxtFile(infoList, FilePath))
        {
            //DWORD errSts = GetLastError();
            //strTmp.sprintf("Create TXT Fail,Err:%d/",errSts);
            QMessageBox::warning(this, "Save Files Error", "Create TXT Fail");
            return;
        }
    }

    if(ItemFlg & BIT1)
    {
        FilePath = PublicPath + "IdentifyNamespace.txt";
        infoList = ParseIdyCtrNSlData(physicalDriveInfo->m_pIDYNamespaceInfo);
        if(!FolderOp->Folder_SaveTxtFile(infoList, FilePath))
        {
            QMessageBox::warning(this, "Save Files Error", "Create TXT Fail");
            return;
        }
    }

    //save SMART log
    if(ItemFlg & BIT2)
    {
        FilePath = PublicPath + "SMARTLog.txt";
        infoList = ParseSmartData(physicalDriveInfo->m_pSmartInfo);
        if(!FolderOp->Folder_SaveTxtFile(infoList, FilePath))
        {
            QMessageBox::warning(this, "Save Files Error", "Create TXT Fail");
            return;
        }
    }

    if(ItemFlg & BIT3)
    {
        FilePath = PublicPath + "ErrorLog.txt";
        infoList = ParseErrorLog(physicalDriveInfo->m_pSmartInfo,ErrEntryCnt);
        if(!FolderOp->Folder_SaveTxtFile(infoList, FilePath))
        {
            QMessageBox::warning(this, "Save Files Error", "Create TXT Fail");
            return;
        }
    }

    if(ItemFlg & BIT4)
    {
        FilePath = PublicPath + "EventLog.bin";
        if(!FolderOp->Folder_SaveBinaryFile((char*)physicalDriveInfo->m_pSmartInfo, SMART_DATA_LENGTH, FilePath))
        {
            QMessageBox::warning(this, "Save Files Error", "Create binary Fail");
            return;
        }
    }

    if(ItemFlg & BIT5)
    {
        FilePath = PublicPath + "BadBlockInfo.txt";
        if(!FolderOp->Folder_SaveBadBlkInfo(FilePath))
        {
            QMessageBox::warning(this, "Save Files Error", "Create TXT Fail");
            return;
        }

    }

    if(ItemFlg & BIT6)
    {
        FilePath = PublicPath + "NandFlashID.bin";
        if(!FolderOp->Folder_SaveBinaryFile((char*)physicalDriveInfo->m_pFlashIDInfo, FLASH_ID_DATA_LENGTH, FilePath))
        {
            QMessageBox::warning(this, "Save Files Error", "Create binary Fail");
            return;
        }
    }

    SetControlState(true);
    QMessageBox::information(this, "Save", "Save Files Success!");
}

void MainWindow::UILoad_DebugView()
{
    DebugWindows *debugDlg = new DebugWindows;
    debugDlg->show();
}

//show disk base info and diagnostic results
void MainWindow::UIDisplay_DiskInfo()
{
    QString DevInfo;
    QString DiagnosticInfo;
    QString Tmpstr,Font1,Font2,Font2_1,Font3,Font3_1,Font4,Font4_1;
    double DevCap = 0;
    unsigned int CurTemp,CcTemp,WcTemp,UsedCap;
    long double ErrEntries;
    //unsigned int StsLnkS,StsLnkW,CapLnkS,CapLnkW;

    Font1 = "<font style = 'font-size:14px;color:green'>";                      //green for normal
    Font2 = "<font style = 'font-size:14px;color:red'>";                        //red for abnormal
    Font2_1 = "<font style = 'font-size:9px;color:red;align:right'>";                      //for abnormal description
    Font3 = "<font style = 'font-size:14px;color:orange'>";                     //orange for warning
    Font3_1 = "<font style = 'font-size:10px;color:orange'>";
    Font4 = "<font style = 'font-size:15px;color:black'>";                      //black for common
    Font4_1 = "<font style = 'font-size:13px;color:blue'>";

    if(physicalDriveInfo->ulIdentifyCtrlSts != ERROR_SUCCESS_STATUS ||
       physicalDriveInfo->ulIdentifyNSSts != ERROR_SUCCESS_STATUS ||
       physicalDriveInfo->ulSMARTSts != ERROR_SUCCESS_STATUS)
    {
        QMessageBox::warning(this, "Warning", "Read Disk Error!\n Please Select Disk again.");
        return;
    }
#ifdef WIN32
    PNVME_IDENTIFY_CONTROLLER_DATA IdyCtrl = reinterpret_cast<PNVME_IDENTIFY_CONTROLLER_DATA>(physicalDriveInfo->m_pIDYControllerInfo);
    PNVME_IDENTIFY_NAMESPACE_DATA IdyNS = reinterpret_cast<PNVME_IDENTIFY_NAMESPACE_DATA>(physicalDriveInfo->m_pIDYNamespaceInfo);
    PNVME_HEALTH_INFO_LOG Smartlog = reinterpret_cast<PNVME_HEALTH_INFO_LOG>(physicalDriveInfo->m_pSmartInfo);

#else
    nvme_id_ctrl *IdyCtrl = reinterpret_cast<nvme_id_ctrl*>(physicalDriveInfo->m_pIDYControllerInfo);
    nvme_id_ns *IdyNS = reinterpret_cast<nvme_id_ns*>(physicalDriveInfo->m_pIDYNamespaceInfo);
    struct nvme_smart_log *Smartlog = reinterpret_cast<struct nvme_smart_log*>(physicalDriveInfo->m_pSmartInfo);
#endif
    //display disk base info
    Tmpstr.sprintf("%s", "Model Name:");
    Tmpstr = Font4 + Tmpstr+"<font>";
    DevInfo += Tmpstr;
#ifdef WIN32
    Tmpstr.sprintf("%s",IdyCtrl->MN);
#else
    Tmpstr.sprintf("%s",IdyCtrl->mn);
#endif
    Tmpstr = Tmpstr.mid(0,40);
    DevInfo += Font4_1 + Tmpstr + +"<font><br/>";

    Tmpstr =Font4 + "Serial Number: " + "<font>";
    DevInfo += Tmpstr;
#ifdef WIN32
    Tmpstr.sprintf("%s",IdyCtrl->SN);
#else
    Tmpstr.sprintf("%s",IdyCtrl->sn);
#endif
    Tmpstr = Tmpstr.mid(0,20);
    DevInfo += Font4_1 + Tmpstr +"<font><br/>";

    Tmpstr = Font4 + "Firmware Revision  : " + "<font>";
    DevInfo += Tmpstr;
#ifdef WIN32
    Tmpstr.sprintf("%s", IdyCtrl->FR);
#else
    Tmpstr.sprintf("%s", IdyCtrl->fr);
#endif
    Tmpstr = Tmpstr.mid(0,8);
    DevInfo += Font4_1 + Tmpstr + +"<font><br/>";

    Tmpstr = Font4 + "Interface  : " + "<font>";
    DevInfo += Tmpstr;
#ifdef WIN32
     Tmpstr.sprintf(" NVM Express %d.%d.%d", IdyCtrl->VER >> 16,(IdyCtrl->VER >> 8)& 0xFF, IdyCtrl->VER & 0xF);
#else
    Tmpstr.sprintf(" NVM Express %d.%d.%d", IdyCtrl->ver >> 16,(IdyCtrl->ver >> 8)& 0xFF, IdyCtrl->ver & 0xF);
#endif
    DevInfo += Font4_1 + Tmpstr + +"<font><br/>";

    //Tmpstr.sprintf("%s", IdyCtrl->tnvmcap);
    //Tmpstr = Tmpstr.mid(0,16);

    Tmpstr = Font4 + "Capacity               : " + "<font>";
    DevInfo +=Tmpstr;
#ifdef WIN32
    for(int idx=0; idx <= IdyNS->NLBAF; idx++)
    {
         DevCap = (1<<IdyNS->LBAF[idx].LBADS)*(IdyNS->NSZE + 1);
         if(IdyNS->FLBAS.S.LbaFormatIndex == idx) break;
    }
#else
    for(int idx=0; idx <= IdyNS->nlbaf; idx++)
    {
         DevCap = (1<<IdyNS->lbaf[idx].ds)*(IdyNS->nsze + 1);
         if(IdyNS->flbas == idx) break;
    }
#endif

    DevCap = DevCap/1000000000;
    Tmpstr.sprintf( "%d GB", (int)DevCap);
    DevInfo += Font4_1 + Tmpstr + +"<font><br/>";

    ui->label_disk_info->setText(DevInfo);
    ui->label_disk_info->adjustSize();

    //display disk diagnostic    

    Tmpstr = Font4 + "Critical Warning : "+"<font>";
    DiagnosticInfo += Tmpstr;
#ifdef WIN32
    Tmpstr.sprintf("0x%x", Smartlog->CriticalWarning.AsUchar);
    if(Smartlog->CriticalWarning.AsUchar == 0)
#else
    //Smartlog->critical_warning = 0x10; //for test
    Tmpstr.sprintf("0x%x",Smartlog->critical_warning);
    if(Smartlog->critical_warning == 0)
#endif
    {
        DiagnosticInfo += Font1 + "NORMAL (" + Tmpstr + ")" +"<font><br/>";
    }
    else
    {
        DiagnosticInfo += Font2 + Tmpstr +"<font><br/>";
#ifdef WIN32
        if (Smartlog->CriticalWarning.AvailableSpaceLow)
#else
        if (Smartlog->critical_warning & BIT0)
#endif
        {
            Tmpstr = "BIT0: available spare capacity has fallen below the threshold.\r\n";
            DiagnosticInfo += Font2_1 + Tmpstr +"<font><br/>";

        }
#ifdef WIN32
        if(Smartlog->CriticalWarning.TemperatureThreshold)
#else
        if (Smartlog->critical_warning & BIT1)
#endif
        {
            Tmpstr = "BIT1: temperature over or under threshold.\r\n ";
            DiagnosticInfo += Font2_1 + Tmpstr +"<font><br/>";
        }
#ifdef WIN32
        if(Smartlog->CriticalWarning.ReliabilityDegraded)
#else
        if (Smartlog->critical_warning & BIT2)
#endif
        {
            Tmpstr = "BIT2: system reliability warning.\r\n";
            DiagnosticInfo += Font2_1 + Tmpstr +"<font><br/>";

        }
#ifdef WIN32
        if(Smartlog->CriticalWarning.ReadOnly)
#else
        if (Smartlog->critical_warning & BIT3)
#endif
        {
            Tmpstr = "BIT3: read only mode.\r\n";
            DiagnosticInfo += Font2_1 + Tmpstr +"<font><br/>";

        }
#ifdef WIN32
        if(Smartlog->CriticalWarning.VolatileMemoryBackupDeviceFailed)
#else
        if (Smartlog->critical_warning & BIT4)
#endif
        {
            Tmpstr ="BIT4: volatile memory backup device has failed.\r\n";
            DiagnosticInfo += Font2_1 + Tmpstr +"<font><br/>";
        }

    }

    //temperature check
    Tmpstr = Font4 + "Temperature : "+"<font>";
    DiagnosticInfo += Tmpstr;
#ifdef WIN32
    CurTemp = Smartlog->Temperature[1]<<8 | Smartlog->Temperature[0];
    CurTemp = CurTemp - 273;
    CcTemp = IdyCtrl->CCTEMP -273;
    WcTemp =IdyCtrl->WCTEMP -273;
#else
    CurTemp = Smartlog->temperature[1]<<8 | Smartlog->temperature[0];
    CurTemp = CurTemp - 273;
    CcTemp = IdyCtrl->cctemp -273;
    WcTemp =IdyCtrl->wctemp -273;
#endif
    //CurTemp = 110; //for test
    Tmpstr.sprintf("%d ℃",CurTemp);
    if(CurTemp < CcTemp)
    {        
        DiagnosticInfo += Font1 + "NORMAL (" +Tmpstr + ")" + "<font><br/>";  //normal
    }
    else if((CurTemp >= WcTemp) && (CurTemp < CcTemp))
    {
        DiagnosticInfo += Font3 + "OVER THRESHOLD (" + Tmpstr + "|" + "<font>";
#ifdef WIN32
        Tmpstr.sprintf("W:%d℃/%u ; C:%d℃/%u)",WcTemp,Smartlog->WarningCompositeTemperatureTime, CcTemp, Smartlog->CriticalCompositeTemperatureTime);
#else
        Tmpstr.sprintf("W:%d℃/%u ; C:%d℃/%u)",WcTemp,Smartlog->warning_temp_time, CcTemp, Smartlog->critical_comp_time);
#endif
        DiagnosticInfo += Font3_1 + Tmpstr +"<font><br/>";  //Temperature over threshold
    }
    else if(CurTemp >= CcTemp)
    {
        DiagnosticInfo += Font2 + "OVER THRESHOLD (" + Tmpstr + "|" + "<font>";
#ifdef WIN32
        Tmpstr.sprintf("W:%d℃/%u)",WcTemp, Smartlog->WarningCompositeTemperatureTime);
#else
        Tmpstr.sprintf("C:%d℃/%u)",CcTemp, Smartlog->critical_comp_time);
#endif
        DiagnosticInfo += Font2 + Tmpstr +"<font><br/>";   //Critical
    }

    //available spare check
    //calculate:PE-cycle used / Max available PE-cycle
    Tmpstr = Font4 + "Drive Life Used : "+"<font>";
    DiagnosticInfo += Tmpstr;
#ifdef WIN32
    UsedCap = Smartlog->PercentageUsed;
#else
    UsedCap = Smartlog->percent_used;
#endif
    //UsedCap = 100;
    Tmpstr.sprintf("%d%%" , UsedCap);
    if(UsedCap < 90)
    {
        DiagnosticInfo += Font1 + "NORMAL (" +Tmpstr + ")"+ "<font><br/>";
    }
    /*else if((UsedCap>90) && (UsedCap< 98))                      //
    {
        DiagnosticInfo += Font3 + Tmpstr + "%" +"<font><br/>";
    }*/
    else
    {
        DiagnosticInfo += Font2 + "CRITICAL (" + Tmpstr + ")" + "<font><br/>";
    }

    //Error entries check
    Tmpstr = Font4 + "Error Log Entries : "+"<font>";
    DiagnosticInfo += Tmpstr;

    Tmpstr = "";
#ifdef WIN32
    ErrEntries = int128_to_double(Smartlog->ErrorInfoLogEntryCount);
#else
    ErrEntries = int128_to_double(Smartlog->num_err_log_entries);
#endif
    ErrEntryCnt = ErrEntries;
    Tmpstr.sprintf("%lu",ErrEntryCnt);
    if(ErrEntries <= 0)
    {
        Tmpstr = Font1 + "NO ERROR" + "<font><br/>";
    }
    else
    {
        Tmpstr = Font2 + Tmpstr + "<font><br/>";
#ifdef WIN32
#else
        ErrEntryCnt = (ErrEntryCnt < IdyCtrl->elpe+1)?(ErrEntryCnt):(IdyCtrl->elpe+1);
#endif
        physicalDriveInfo->m_pErrorLog = (unsigned char*) malloc(ERROR_ENTRY_LENGTH * ErrEntryCnt);
        physicalDriveInfo->ulErrorlogSts =  physicalDriveInfo->GetErrorLogInfo(physicalDriveInfo->m_pErrorLog, (int)ErrEntryCnt);
    }

    DiagnosticInfo += Tmpstr;

    //Link Status check
    Tmpstr = Font4 + "PCIe Link Status : "+"<font>";
    DiagnosticInfo += Tmpstr;
    //StsLnkS = LnkSts.LinkSpeed;
    //StsLnkW = LnkSts.LinkWidth;
    //CapLnkS = LnkCap.LinkSpeed;
    //CapLnkW = LnkCap.LinkWidth;
    /*QMessageBox Msg;
    Tmpstr.sprintf("Cap:%d-%d,Sts:%d-%d",physicalDriveInfo->LnkCap.LinkSpeed, physicalDriveInfo->LnkCap.LinkWidth,
                                         physicalDriveInfo->LnkSts.LinkSpeed, physicalDriveInfo->LnkSts.LinkWidth);
    Msg.setText(Tmpstr);
    Msg.exec();*/
    Tmpstr.sprintf("PCIe %d.0 x%d | %d .0 x%d",LnkSts.LinkSpeed, LnkSts.LinkWidth,LnkCap.LinkSpeed, LnkCap.LinkWidth);


    if(!LnkSts.LinkWidth && !LnkSts.LinkSpeed && !LnkCap.LinkSpeed && !LnkCap.LinkWidth
       && ((LnkSts.LinkSpeed < LnkCap.LinkSpeed) || (LnkSts.LinkWidth < LnkCap.LinkWidth)))
    {

        Tmpstr = Font2 + "LINK FAILED (" + Tmpstr + ")" + "<font><br/>";
    }
    else if((LnkSts.LinkSpeed == LnkCap.LinkSpeed) && (LnkSts.LinkWidth == LnkCap.LinkWidth))
    {
        Tmpstr = Font1 + "NORMAL (" + Tmpstr + ")" + "<font><br/>";
    }
    else if((LnkSts.LinkSpeed < LnkCap.LinkSpeed) || (LnkSts.LinkWidth < LnkCap.LinkWidth))
    {
        Tmpstr = Font3 + "LINK DOWN (" + Tmpstr + ")" + "<font><br/>";
    }
    DiagnosticInfo += Tmpstr;

    //PE Cycle check
    Tmpstr = Font4 + "Program/Erase Check : "+"<font>";
    DiagnosticInfo += Tmpstr;

    if(1)
    {
        Tmpstr = Font1 + "NORMAL" + "<font><br/>";
    }
    else
    {
        Tmpstr = Font3 + "OVER THRESHOLD" + "<font><br/>";
    }
    DiagnosticInfo += Tmpstr;

    //Bad block check
    Tmpstr = Font4 + "Bad Block Check : "+"<font>";
    DiagnosticInfo += Tmpstr+ "<font>";

    if(1)
    {
        Tmpstr = Font1 + "NORMAL" + "<font>";
    }
    else
    {
        Tmpstr = Font3 + "TOO MANY BAD BLOCK" + "<font>";
    }
    DiagnosticInfo += Tmpstr;

    //DiagnosticInfo += Tmpstr;
    ui->label_diagnostic->setText(DiagnosticInfo);
    //ui->label_diagnostic->adjustSize();
    ReadyFlg = true;
}

void MainWindow::RefreshLayout(QHBoxLayout *layout)
{
    QLayoutItem *item;
    if(layout != nullptr)
    {
        while((item = layout->takeAt(0)) != nullptr)
        {
            delete item->widget();
            delete item;
        }
    }
}

void MainWindow::InitTableWidget(QTableWidget* infoTbl)
{    
    infoTbl->setStyleSheet("QTableWidget{border:none; font:9pt; font-family:Verdana;background-color: white; }"
                           "QTableWidget::item:selected{background-color:#8fa0ff;}");
    infoTbl->horizontalHeader()->setStyleSheet("QHeaderView::section {border-bottom-color: rgb(200, 200, 200); "
                                               "border-right-color: rgb(200, 200, 200);"
                                               "border-left-color: rgb(255, 255, 255);"
                                               "border-top-color: rgb(255, 255, 255);"
                                               "font:10pt;font-family:Verdana;"
                                               "background-color: rgb(240, 240, 240);}");


    infoTbl->verticalHeader()->hide();
    infoTbl->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    infoTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    infoTbl->setSelectionBehavior(QAbstractItemView::SelectRows);

    infoTbl->horizontalHeader()->setDefaultSectionSize(300);
    infoTbl->horizontalHeader()->setStretchLastSection(true); //the last colum width will be auto set
    //infoTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //infoTbl->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    infoTbl->horizontalHeader()->setHighlightSections(false);
    infoTbl->setMouseTracking(true);

}

void MainWindow::UIDisplay_FormatedInfo(QList<SPEC_ITEM> *infoList)
{
    QTableWidget *TableInfo = new QTableWidget();
    QStringList TableHeaderList;

    InitTableWidget(TableInfo);
    TableInfo->setColumnCount(2);
    TableHeaderList<<"Description"<<"Value";
    TableInfo->setHorizontalHeaderLabels(TableHeaderList);

    for(int idx = 0; idx < infoList->count(); idx++)
    {
        TableInfo->insertRow(idx);
        TableInfo->setItem(idx, 0, new QTableWidgetItem(infoList->at(idx).Description));
        TableInfo->setItem(idx, 1, new QTableWidgetItem(infoList->at(idx).RawValue));
    }


    textLayout->addWidget(TableInfo);
    textLayout->setContentsMargins(0,0,0,0);
}

void MainWindow::UIDisplay_RawData(const unsigned char *dataPtr, int length)
{
    QTextEdit *textInfo = new QTextEdit();
    QByteArray dataBa = nullptr;
    QString dataTxt ="";
    QString headStr ="";
    QString tmpStr ="";
    QString addrStr,ascStr, hexStr;
    int i,j;
    textInfo->setStyleSheet("QTextEdit{background-color:white; font:10pt courier new;""border:none;}");
    textInfo->setReadOnly(true);

    if(dataPtr == nullptr)
    {
        textInfo->setText("Get Data Fail!!Please try again");
        return;
    }

    headStr = "          ";
    for(i=0; i<16; i++)
    {
        tmpStr.sprintf("%02X ",i);
        headStr +=tmpStr;
    }
    headStr +="\n";
    //textInfo->setText(headStr);

    dataBa = QByteArray::fromRawData((char*)dataPtr,length);
    for(i=0; i<length; i+=16)
    {
        addrStr = QString("%1").arg(i, 8, 16, QChar('0')).toUpper();        
        for(j=0; j<16; j++)
        {
            if(i+j < length)
            {
                hexStr.append(" ").append(dataBa.mid(i+j,1).toHex().toUpper());
                char ch = dataBa[i+j];
                if(ch < 0x20 || ch > 0x7e)
                    ch = '.';
                ascStr.append(QChar(ch));
            }
        }
        dataTxt += addrStr + " " + QString("%1").arg(hexStr, -48) + "   " + QString("%1").arg(ascStr, -17) + "\n";
        hexStr = "";
        ascStr = "";
    }


    textInfo->setText(headStr+dataTxt);
    textLayout->addWidget(textInfo);
    textLayout->setContentsMargins(0,0,0,0);

}
/*
 * void MainWindow::UIDisplay_IdentifyControllerInfo(unsigned char *data)
{
    QTableWidget *IdCtrlInfo = new QTableWidget();
    QStringList TableHeaderList;
    QList<SPEC_ITEM>*infoList = ParseSmartData(data);

    InitTableWidget(IdCtrlInfo);
    IdCtrlInfo->setColumnCount(2);
    TableHeaderList<<"Description"<<"RawValue";
    IdCtrlInfo->setHorizontalHeaderLabels(TableHeaderList);

    for(int idx = 0; idx < infoList->count(); idx++)
    {
        IdCtrlInfo->insertRow(idx);
        IdCtrlInfo->setItem(idx, 0, new QTableWidgetItem(infoList->at(idx).Description));
        IdCtrlInfo->setItem(idx, 1, new QTableWidgetItem(infoList->at(idx).RawValue));

    }

    textLayout->addWidget(IdCtrlInfo);
    textLayout->setContentsMargins(0,0,0,0);
}
*/



