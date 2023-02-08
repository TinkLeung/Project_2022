#include "FolderOperation.h"
#include <time.h>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QCoreApplication>



FolderOperation::FolderOperation()
{


}

bool FolderOperation::Folder_CreateFileFolder(char *FolderName)
{
    char CurTCh[64];
    time_t CurTime = time(nullptr);
    QDir *dir = new QDir;

    strftime(CurTCh, sizeof(CurTCh), "%Y%m%d%H%M",localtime(&CurTime));

    if(dir->mkdir(QString::fromUtf8(CurTCh)) == 0)
    {
        QMessageBox::warning(nullptr, "Create Folder", "Folder already existed!");
        return false;
    }
    memcpy(FolderName, CurTCh, sizeof(CurTCh));
    return true;
}

bool FolderOperation::Folder_SaveTxtFile(QList<SPEC_ITEM> *infoList, QString filePath)
{
    int idx = 0;
    QString tmpStr, dataStr;
    QFile File(filePath);
    //char Header2[200] ={"Value"};

    //File.setFileName(FileName);
    if(File.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        QTextStream WriteStream(&File);
        tmpStr = dataStr.sprintf("%-50s", "Description");
        tmpStr += dataStr.sprintf("%-s", "Value\n");
        WriteStream<<tmpStr<<endl;

        for (idx = 0; idx < infoList->count(); idx++)
        {
            std::string str = infoList->at(idx).Description.toStdString();
            const char *TmpCh = str.c_str();
            tmpStr = dataStr.sprintf("%-50s", TmpCh);
            str = infoList->at(idx).RawValue.toStdString();
            TmpCh = str.c_str();
            tmpStr += dataStr.sprintf("%-s", TmpCh);
            WriteStream << tmpStr << endl;
        }
        File.close();
    }
    else {        
        return false;
    }
    return true;
}

bool FolderOperation::Folder_SaveBinaryFile(char *dataPtr, int datalen, QString filePath)
{
     QFile File(filePath);

     if(File.open(QIODevice::WriteOnly))
     {
         QDataStream dataStream(&File);
         dataStream.writeRawData(dataPtr,datalen);
     }
     else
     {
         return false;
     }
     File.close();
     return true;
}

bool FolderOperation::Folder_SaveBadBlkInfo(QString filePath)
{
    QFile File(filePath);

    if(File.open(QIODevice::WriteOnly))
    {
        QTextStream textStream(&File);
        textStream<<BadBlkDumpLog<<endl;
    }
    else
    {
        return false;
    }
    File.close();
    return true;
}
