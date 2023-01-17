
#ifndef PHYSICALDRIVEINFO_H
#define PHYSICALDRIVEINFO_H

#include <QString>
#ifdef WIN32
#include <windows.h>
#include "windows/NvmeApp.h"
#endif

typedef struct {
     unsigned int LinkSpeed;
     unsigned int LinkWidth;
}PCI_LNK, *PPCI_LNK;
extern PCI_LNK LnkSts;
extern PCI_LNK LnkCap;

class PhysicalDriveInfo
{
public:
    PhysicalDriveInfo();
#ifndef WIN32
    PhysicalDriveInfo(char *dev);
#else
    PhysicalDriveInfo(int driveNum);
#endif

    //The info status
public:

#ifndef WIN32
    char currentDev[64];
#else
    int currentDrive;
#endif

/***************  Common Parameters ***************/
    /*struct PCI_LNK{
         unsigned int LinkSpeed;
         unsigned int LinkWidth;
    };
    PCI_LNK LnkSts;
    PCI_LNK LnkCap;*/

    //unsigned long ulIdentifySts;
    unsigned long ulSMARTSts;
    unsigned long ulErrorlogSts;
    unsigned long ulIdentifyNSSts;
    unsigned long ulIdentifyCtrlSts;
    //unsigned long ulFlashIDSts;
    //unsigned long ulEventlogSts;


    //unsigned char* m_pIdentifyInfo;
    unsigned char* m_pSmartInfo;
    unsigned char* m_pErrorLog;
    unsigned char* m_pIDYNamespaceInfo;
    unsigned char* m_pIDYControllerInfo;
    //unsigned char* m_pFlashIDInfo;
    //unsigned char* m_pEventLogInfo;

	
/**************** Common Functions ***************/
    void init();
    void GetInfo();
#ifdef WIN32
    void UpdateDriveInfo(int i);
#else
    void UpdateDriveInfo(char* dev);
#endif
    void GetDriveLinkStatus(struct nvme_namespace *n);

    //unsigned long GetIdetifyInfo(unsigned char* buff);
    unsigned long GetSmartInfo(unsigned char* buff);    
    unsigned long GetIDYNamespaceInfo(unsigned char * buff);
    unsigned long GetIDYControllerInfo(unsigned char * buff);
    unsigned long GetErrorLogInfo(unsigned char* buff, int entryCnt);
    //unsigned long GetFlashIDInfo(unsigned char* buff);
    //unsigned long GetEventLogInfo(unsigned char* buff);


};

#endif // PHYSICALDRIVEINFO_H
