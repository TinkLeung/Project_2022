
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


typedef struct{
    unsigned long result;               //DWORD 0
    char assertFileNmae[128-4];         //DWORD 1-32
    int roundNum;                       //DWORD 32
    int runTime; //minutes              //DWORD 33
    int maxTemp;                        //DWORD 34
    int testRoundSetting;               //DWORD 35
    int badBlockLimit;                  //DWORD 36
    int eccBitLimit;                    //DWORD 37
    int programFailLimit;               //DWORD 38
    int eraseFailLimit;                 //DWORD 39
    int blockNumWs;                     //DWORD 40
    int mpBlockNumRdt;                  //DWORD 41
    int lunCount;                       //DWORD 42
    int maxEraseTime;                   //DWORD 43
    int totalMpBlockFail;               //DWORD 44
    int powerDrop;                      //DWORD 45
    int readOffsetFlag;                 //DWORD 46
    int readOffsetLevel;                //DOWRD 47
    int readOffsetLevelFirst;           //DWORD 48
    int slcRoundCount;                  //DOWRD 49
    int badBlockCountUI;                //DWORD 50
    int slcEccBitLimit;                 //DWORD 51
    int resv12DW[12];                   //DWORD 52-63 TableInfoForBank tableinfo[6];
    int firstRoundTime;                 //DWORD 64
    int writeLatency;                   //DWORD 65
    int readLatency;                    //DWORD 66
    int eraseLatency;                   //DWORD 67
    char rdtVersion[16];                //DWORD 68-71
    unsigned char specialSetting[4];    //DWORD 72
    int rdtStrategy;                    //DWORD 73
    unsigned char tableVersion[4];      //DWORD 74
    unsigned short badBlcokNum;         //DWORD 75[0-15]
    unsigned short failArea;            //DWORD 75[16-31]
    unsigned short programFailNum;      //DWORD 76[0-15]
    unsigned short factoryBadNum;       //DWORD 76[16-31]
    unsigned short readFailNum;         //DWORD 77[0-15]
    unsigned short eraseFailNum;        //DWORD 77[16-31]
    unsigned short slcProgramFailNum;   //DWORD 78[0-15]
    unsigned short laterBadNum;         //DWORD 78[16-31]
    unsigned short slcReadFailNum;      //DWORD 79[0-15]
    unsigned short slcEraseFailNum;     //DWORD 79[16-31]
    unsigned short eraseSlowNum;        //DWORD 80[0-15]
    unsigned short openReadNum;         //DWORD 80[16-31]
    char serialNum[24];                 //DWORD 81-86
    unsigned long controllerFailReason; //DWORD 87
    unsigned long bankFailReason[8];    //DWORD 88-95
    int reserved1[26];                  //DWORD 96-121
    short planeFailCount[6];            //DWORD 122[0-15]-124[16-32]:plane0-plane5
    int flashclass;                     //DWORD 125
    int reserved2;                      //DWORD 126
    int capacity;                       //DWORD 127
    int eccBitNum[128];                 //DWORD 128-387
    //int reserved3[132];                 //DWORD
    unsigned char uid[16];              //DWORD 388-391
    int reserved3[119];                 //DWORD 392-510
    char customerTag[4];                //DOWRD 511
    int resvDW512[512];                 //DWORD 512-1023 BadBlockDetailInfo badBlkInfo[512]

}RDT_RESULT, *PRDT_RESULT;//per lun

typedef struct {
    bool bDataValid;
    bool bRdtResult;
    bool bEccTableValid;
    bool bAssert;
    bool bHeaderFail;
    bool bBlock0Fail;
    bool bShowOrigin;
    bool bPerformanceFail;
    bool bPowerDropFail;
    bool bDramFail;
    bool bControllerFail;
    bool bCapacityOpFail;
    bool bBigEccInDiffRoundsFail;
    bool bHighTempFail;
    bool bDrivingIssue;
    bool bCacheProgramIssue;
    bool bNandPowerCutFail;
    bool bEarlyRetentionalFail;
    bool bSnapReadFail;
    bool bWriteReadMixFail;
    bool bReadRetryFail;
    bool bSpecialSLCRoundFail;
    unsigned int respErrorCode;
    int assertLine;
    int chNum;
    int ceNum;
    int lunNum;
    int eccBitsCount[256];
    int eccCount;
    int eccBitNum[128];
    int ecc10Count;
    int ecc20Count;
    int ecc30Count;
    int ecc40Count;
    int eccMoreCount;
    int totalBadBlockCount;
    int totalBadBlockCountOrigin;
    int badBlockInternal;
    int badBlockCountUI;
    int factoryBadCount;
    int factoryBadCountOrigin;
    int programFailCount;
    int eraseFailCount;
    int readFailCount;
    int laterBadCount;
    int grownBadCount;
    int grownBadCountOrigin;
    int slcProgramFailCount;
    int slcEraseFailCount;
    int slcReadFailCount;
    int openReadCount;
    int readOffSetCount;
    int rrtToManyCount;
    int WRMixSlcReadFailCount;
    int writeLatency;
    int eraseSlowCount;
    int dramFail[4];
    unsigned long controllerFailReason;
    RDT_RESULT rdtResultInfo;
    int resev1;                         //EccTableForBank eccTable[MAX_TABLE_NUM]
    unsigned char factoryDefect[512];
    int resev2;                         //BadBlockDetailInfo badBlockInfo[512 * 8]

}RDT_RESULT_ITEM, *PRDT_RESULT_ITEM;    //per lun

typedef struct{
    int OriBad;
    int LaterBad;
    int TotalBad;
    int EraseFail;
    int ProgramFail;
    int ReadFail;
}BAD_BLOCK_PER_LUN, *PBAD_BLOCK_PER_LUN;        //per lun

typedef struct
{
    BAD_BLOCK_PER_LUN BadBlkPerLun[MAX_CH][MAX_CE];
    int TotalProgFail;
    int ToTalEraseFail;
    int TotalReadFail;
    int TotalOriBad;
    int TotalNewBad;
    int TotalBadBlock;
}BAD_BLOCK_INFO, *PBAD_BLOCK_INFO;

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
    unsigned char* m_pFlashIDInfo;
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
    unsigned long GetFlashIDInfo(unsigned char* buff);
    unsigned long GetBadBlockInfo();
    //unsigned long GetEventLogInfo(unsigned char* buff);


};

#endif // PHYSICALDRIVEINFO_H
