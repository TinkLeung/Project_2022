#ifndef FOLDEROPERATION_H
#define FOLDEROPERATION_H

#include <QString>
#include "CommonFunc.h"

class FolderOperation
{
public:
    FolderOperation();

public:
    bool Folder_CreateFileFolder(char *FolderName);

    bool Folder_SaveTxtFile(QList<SPEC_ITEM>*infoList, QString filePath);
    bool Folder_SaveBinaryFile(char *dataPtr, int datalen, QString filePath);

};




#endif // FOLDEROPERATION_H
