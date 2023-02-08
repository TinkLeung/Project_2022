
#include "PhysicalDriveInfo.h"
#include "CommonFunc.h"
#ifdef WIN32
#include "windows/NvmeApp.h"
#else
#include <qdebug.h>
#include "linux/nvme_cpp.h"
#include "pci/pci.h"
#include "QMessageBox"
#endif

const char *devHeader = "/dev/";
struct config {
        unsigned char       opcode;
        unsigned char       flags;
        unsigned short      rsvd;
        unsigned int        namespace_id;
        unsigned int        data_len;
        unsigned int        metadata_len;
        unsigned int        timeout;
        unsigned int        cdw2;
        unsigned int        cdw3;
        unsigned int        cdw10;
        unsigned int        cdw11;
        unsigned int        cdw12;
        unsigned int        cdw13;
        unsigned int        cdw14;
        unsigned int        cdw15;
        char                *input_file;
        int                 raw_binary;
        int                 show_command;
        int                 dry_run;
        int                 read;
        int                 write;
        unsigned char       prefill;
    };

PCI_LNK LnkSts;
PCI_LNK LnkCap;

BAD_BLOCK_INFO BbInfo;

#ifdef WIN32
PhysicalDriveInfo::PhysicalDriveInfo(int driveNum)
{
    currentDrive = driveNum;
    init();
    GetInfo();
}
#else
PhysicalDriveInfo::PhysicalDriveInfo(char* dev)
{
    char *device;
    asprintf(&device, "%s%s", devHeader, dev);
    memcpy(currentDev,device,64);
    init();
    GetInfo();
    return;
}
#endif

void PhysicalDriveInfo::GetInfo()
{
    ulSMARTSts = GetSmartInfo(m_pSmartInfo);
    ulIdentifyNSSts = GetIDYNamespaceInfo(m_pIDYNamespaceInfo);
    ulIdentifyCtrlSts = GetIDYControllerInfo(m_pIDYControllerInfo);
    //flash_id_status = GetFlashIDInfo(m_pFlashIDInfo);
    //event_log_status = GetEventLogInfo(m_pEventLogInfo);

	return;
}

void PhysicalDriveInfo::init()
{
    //if(m_pSmartInfo != nullptr) free(m_pSmartInfo);
    //if(m_pIDYControllerInfo != nullptr) free(m_pIDYControllerInfo);
    //if(m_pIDYNamespaceInfo != nullptr) free(m_pIDYNamespaceInfo);
    //if(physicalDriveInfo->m_pErrorLog !=nullptr) free(physicalDriveInfo->m_pErrorLog);*/
    m_pSmartInfo = (unsigned char*) malloc(SMART_DATA_LENGTH);
    m_pIDYNamespaceInfo = (unsigned char*)malloc(IDY_NAMESPACE_INFO_LENGTH);
    m_pIDYControllerInfo = (unsigned char*)malloc(IDY_CONTROLLER_INFO_LENGTH);
    //m_pEventLogInfo = (unsigned char*) malloc(EVENT_LOG_DATA_LENGTH);
    m_pFlashIDInfo = (unsigned char*) malloc(FLASH_ID_DATA_LENGTH);

	return;
}

#ifdef WIN32
void PhysicalDriveInfo::UpdateDriveInfo(int i)
{
    currentDrive = i;
    GetInfo();
}
#else
void PhysicalDriveInfo::UpdateDriveInfo(char* dev)
{
    char *device;
    asprintf(&device, "%s%s", devHeader, dev);
    memcpy(currentDev,device,64);
    GetInfo();
}
#endif


#ifndef WIN32
void PhysicalDriveInfo::GetDriveLinkStatus(struct nvme_namespace *n)
{
    struct pci_access *pacc;
    struct pci_dev *dev;
    QString Tmp,Tmp1;
    u32 Ret1 = 0;
    u16 Ret2 = 0;
    int BusNum, DevNum, FuncNum;
    int Idx,Next,Pos;
    QMessageBox Msg;

    pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);

    BusNum = char_to_int(n->ctrl->address[5]*16 + n->ctrl->address[6]);
    DevNum = char_to_int(n->ctrl->address[8]*16 + n->ctrl->address[9]);
    FuncNum = char_to_int(n->ctrl->address[11]);

    for(dev = pacc->devices; dev ; dev = dev->next)
    {
        if(BusNum == dev->bus && DevNum == dev->dev && FuncNum == dev->func)
        {
            if(pci_read_word(dev,PCI_STATUS) & PCI_STATUS_CAP_LIST)
            {
                Pos = PCI_CAPABILITY_LIST;
                Pos = pci_read_byte(dev, Pos) & ~3;
                while(Pos)
                {
                    Idx = pci_read_byte(dev, Pos+PCI_CAP_LIST_ID);
                    Next = pci_read_byte(dev, Pos+PCI_CAP_LIST_NEXT) & ~3;
                    switch(Idx)
                    {                        
                        case PCI_CAP_ID_EXP:
                             Ret1 = pci_read_long(dev,Pos + PCI_EXP_LNKCAP);
                             LnkCap.LinkSpeed = Ret1 & PCI_EXP_LNKCAP_SPEED;
                             LnkCap.LinkWidth = (Ret1 & PCI_EXP_LNKCAP_WIDTH) >> 4;

                             Ret2 = pci_read_word(dev,Pos + PCI_EXP_LNKSTA);
                             LnkSts.LinkSpeed = Ret2 & PCI_EXP_LNKSTA_SPEED;
                             LnkSts.LinkWidth = (Ret2 & PCI_EXP_LNKSTA_WIDTH) >> 4;
                             //Tmp.sprintf("[1]Cap:%d-%d,Sts:%d-%d",LnkCap.LinkSpeed, LnkCap.LinkWidth,LnkSts.LinkSpeed,LnkSts.LinkWidth);
                             break;
                        default:
                             break;
                    }
                    Pos = Next;
                }
            }

        }
    }
    pci_cleanup(pacc);
}

#endif

#if 0
unsigned long PhysicalDriveInfo::GetIdetifyInfo(unsigned char* buff)
{

    unsigned char buffer[IDENTIFY_DATA_LENGTH];
    unsigned long status = ERROR_SUCCESS_STATUS;
#ifdef WIN32
    NVMeApp nvmeapp(currentDrive);
    status = nvmeapp.Identify(buffer);
    if(status == ERROR_SUCCESS)
        memcpy(buff,buffer,IDENTIFY_DATA_LENGTH);
    else
        printf("identify error %d",status);
#else

    int fd = open_dev(currentDev);
    if(fd<0)
    {
        status=fd;
        printf("open dev failed err %ld",status);
        close_dev(fd);
        return status;
    }

    status = nvme_identify_ctrl(fd, &buffer);
    if(!status)
    {
        memcpy(buff,buffer,IDENTIFY_DATA_LENGTH);
    }
    else
    {
        printf("identify error %ld",status);
    }

    close_dev(fd);
#endif
    return status;
}
#endif


unsigned long PhysicalDriveInfo::GetSmartInfo(unsigned char* buff)
{
    unsigned char buffer[SMART_DATA_LENGTH];
    unsigned long status = ERROR_SUCCESS_STATUS;
#ifdef WIN32
    NVMeApp nvmeapp(currentDrive);
    status = nvmeapp.SmartInfo(buffer);
    if(status == ERROR_SUCCESS)
        memcpy(buff,buffer,SMART_DATA_LENGTH);
    else
        printf("identify error %d",status);
#else

    int fd = open_dev(currentDev);
    if(fd<0)
    {
        status=fd;
        printf("open dev failed err %ld",status);
        return status;
    }
    status = nvme_smart_log(fd, NVME_NSID_ALL, (struct nvme_smart_log *)buffer);
    if(!status)
    {
        memcpy(buff,buffer,SMART_DATA_LENGTH);
    }
    else
    {
        printf("smart-log error %ld",status);
    }
    close_dev(fd);
#endif

     return status;
}
/*
unsigned long PhysicalDriveInfo::GetLbaInfo(unsigned char* buff)
{
	unsigned char buffer[LBA_INFO_LENGTH];
	unsigned long status = ERROR_SUCCESS_STATUS;
#ifdef WIN32
	NVMeApp nvmeapp(currentDrive);
	status = nvmeapp.ReadLbainfo(buffer);
	if(status == ERROR_SUCCESS)
        memcpy(buff,buffer,LBA_INFO_LENGTH);
	else
		printf("read flash %d\n",status);
#else
    __u32 result;
    void *data = NULL, *metadata = NULL;
    int fd = open_dev(currentDev);
    if(fd<0)
    {
        status=fd;
        printf("open dev failed err %d",status);
        return status;
    }



	close_dev(fd);
#endif
    return status;
}
*/


unsigned long PhysicalDriveInfo::GetFlashIDInfo(unsigned char* buff)
{
    unsigned char buffer[FLASH_ID_DATA_LENGTH];
    unsigned long status = ERROR_SUCCESS_STATUS;
#ifdef WIN32
    NVMeApp nvmeapp(currentDrive);
    status = nvmeapp.ReadFlashID(buffer);
    if(status == ERROR_SUCCESS)
        memcpy(buff,buffer,FLASH_ID_DATA_LENGTH);
    else
        printf("read Flash ID error %d",status);

#else

    __u32 result;
    void *data = NULL, *metadata = NULL;
    int fd = open_dev(currentDev);
    if(fd<0)
    {
        status=fd;
        printf("open dev failed err %d",status);
        return status;
    }

    struct config cfg = {0};
    cfg.opcode=0xfc;
    cfg.flags=0;
    cfg.rsvd=0;
    cfg.namespace_id=0;
    cfg.data_len= 0;
    cfg.metadata_len=0;
    cfg.timeout=0;
    cfg.cdw2=0;
    cfg.cdw3=0;
    cfg.cdw10=0;
    cfg.cdw11=0;
    cfg.cdw12=0;
    cfg.cdw13=0x50ffff;
    cfg.cdw14=0;
    cfg.cdw15=0;
    cfg.prefill =0;

    status = nvme_passthru(fd, NVME_IOCTL_ADMIN_CMD, cfg.opcode, cfg.flags, cfg.rsvd,
                cfg.namespace_id, cfg.cdw2, cfg.cdw3, cfg.cdw10,
                cfg.cdw11, cfg.cdw12, cfg.cdw13, cfg.cdw14, cfg.cdw15,
                cfg.data_len, (void *)buffer, cfg.metadata_len, metadata,
                cfg.timeout, &result);

    if(status<0)
    {
        printf("read-flash(0xfc) passthru error");
        close_dev(fd);
        return status;
    }
    else if(status)
    {
        printf("read-flash(0xfc) passthru error");
        show_nvme_status(status);
        close_dev(fd);
        return status;
    }

    cfg = {0};
    cfg.opcode=0xfa;
    cfg.flags=0;
    cfg.rsvd=0;
    cfg.namespace_id=0;
    cfg.data_len= FLASH_ID_DATA_LENGTH;
    cfg.metadata_len=0;
    cfg.timeout=0;
    cfg.cdw2=0;
    cfg.cdw3=0;
    cfg.cdw10=0x80;
    cfg.cdw11=0;
    cfg.cdw12=0xa200a020;
    cfg.cdw13=0x100;
    cfg.cdw14=0;
    cfg.cdw15=0;
    cfg.prefill =0;

    status = nvme_passthru(fd, NVME_IOCTL_ADMIN_CMD, cfg.opcode, cfg.flags, cfg.rsvd,
                cfg.namespace_id, cfg.cdw2, cfg.cdw3, cfg.cdw10,
                cfg.cdw11, cfg.cdw12, cfg.cdw13, cfg.cdw14, cfg.cdw15,
                cfg.data_len, (void *)buffer, cfg.metadata_len, metadata,
                cfg.timeout, &result);

    if(status<0)
    {
        printf("read-flash passthru error");
        close_dev(fd);
        return status;
    }
    else if(status)
    {
        printf("read-flash passthru error");
        show_nvme_status(status);
        close_dev(fd);
        return status;
    }

    memcpy(buff,buffer,FLASH_ID_DATA_LENGTH);
    close_dev(fd);
#endif

    return status;
}

#ifndef WIN32
unsigned long GetRDTResultInfo(unsigned char *buff, char chanel, char ce, char lun)
{
    unsigned long status = ERROR_SUCCESS_STATUS;
    unsigned char buffer[RDT_INFO_LENGTH];
    __u32 result;
    void *data = NULL, *metadata = NULL;
    int fd = open_dev(currentDev);
    if(fd<0)
    {
        status=fd;
        printf("open dev failed err %d",status);
        return status;
    }

    struct config cfg = {0};
    cfg.opcode=0xfc;
    cfg.flags=0;
    cfg.rsvd=0;
    cfg.namespace_id=0;
    cfg.data_len= RDT_INFO_LENGTH;
    cfg.metadata_len=0;
    cfg.timeout=0;
    cfg.cdw2=0;
    cfg.cdw3=0;
    cfg.cdw10= RDT_INFO_LENGTH >> 2;
    cfg.cdw11=0;
    cfg.cdw12=0;
    cfg.cdw13=0x43<<16 | (RDT_INFO_LENGTH>>9); //subCode:0x43
    cfg.cdw14=lun | (ce<<2) | (chanel<<5);
    cfg.cdw15=0;
    cfg.prefill =0;

    status = nvme_passthru(fd, NVME_IOCTL_ADMIN_CMD, cfg.opcode, cfg.flags, cfg.rsvd,
                cfg.namespace_id, cfg.cdw2, cfg.cdw3, cfg.cdw10,
                cfg.cdw11, cfg.cdw12, cfg.cdw13, cfg.cdw14, cfg.cdw15,
                cfg.data_len, (void *)buffer, cfg.metadata_len, metadata,
                cfg.timeout, &result);

    if(status<0)
    {
        printf("read-flash id(0xfc) passthru error");
        close_dev(fd);
        return status;
    }
    else if(status)
    {
        printf("read-flash id(0xfc) passthru error");
        show_nvme_status(status);
        close_dev(fd);
        return status;
    }
    memcpy(buff,buffer,FLASH_ID_DATA_LENGTH);
    close_dev(fd);
    return status;

}
#endif

unsigned long PhysicalDriveInfo::GetBadBlockInfo()
{
    BYTE MaxCHSupported = 0;
    BYTE MaxCESupported = 0;
    BYTE Chanel = 0, Ce = 0;
    unsigned long Status = ERROR_SUCCESS_STATUS;
    unsigned char tmpBuf[RDT_INFO_LENGTH] = { 0 };
    PRDT_RESULT pRdtInfo = (PRDT_RESULT)tmpBuf;
#ifdef WIN32
    NVMeApp nvmeapp(currentDrive);    //
    //check supported CE/Chanel
    nvmeapp.ReadFlashID(m_pFlashIDInfo);
#else
    GetFlashIDInfo(m_pFlashIDInfo);
#endif

    for(Chanel = 0; Chanel < MAX_CH; Chanel++)
    {
        for(Ce = 0; Ce < MAX_CE; Ce++)
        {
            BYTE FlashID[8] = {0};
            memcpy(FlashID, m_pFlashIDInfo+Chanel*64+Ce*8, 8);
            /*printf("%x_CH:%d, CE:%d, ID:%X %X %X %X %X %X %X %X\n",(m_pFlashIDInfo+Chanel*64+ce*8), Chanel, ce,
                   FlashID[0] ,FlashID[1] ,FlashID[2] ,FlashID[3] ,
                   FlashID[4] ,FlashID[5] ,FlashID[6] ,FlashID[7] );*/
            if(FlashID[0] == 0 && FlashID[1] == 0 && FlashID[2] == 0 && FlashID[3] == 0
               && FlashID[4] == 0 && FlashID[5] == 0 && FlashID[6] == 0 && FlashID[7] == 0)
            {
                continue;
            }
            else {
                MaxCHSupported = Chanel+1;
                MaxCESupported = Ce+1;
            }
        }
    }

    //printf("MaxCE:%d, MaxCH:%d\n", MaxCESupported, MaxCHSupported);
    ZeroMemory(&BbInfo, sizeof(BAD_BLOCK_INFO));
    BadBlkDumpLog.clear();
    BadBlkDumpLog.append("----------------------------------BAD BLOCK INFO for CH/CE/LUN----------------------------------\n\n");
    for(BYTE Lun = 0; Lun < 1; Lun++) //1 lun per CE
    {
        for(Ce = 0; Ce < MaxCESupported; Ce++)
        {
            for(Chanel = 0; Chanel < MaxCHSupported; Chanel++)
            {
#ifdef WIN32
                Status = nvmeapp.GetRDTResultInfo(tmpBuf, Chanel, Ce, Lun);
#else
                Status = GetRDTResultInfo(tmpBuf, Chanel, Ce, Lun);
#endif
                if(Status || RDT_RESULT_PASS != pRdtInfo->result)
                {
                   BadBlkDumpLog.append(QString::asprintf("CH%d CE%d LUN% RDT Result Fail:0x%X\n",Chanel, Ce, Lun, pRdtInfo->result));
                   continue;
                }
                else
                {
                    BbInfo.BadBlkPerLun[Chanel][Ce].OriBad = pRdtInfo->factoryBadNum;
                    BbInfo.BadBlkPerLun[Chanel][Ce].LaterBad = pRdtInfo->laterBadNum;
                    BbInfo.BadBlkPerLun[Chanel][Ce].EraseFail = pRdtInfo->eraseFailNum;
                    BbInfo.BadBlkPerLun[Chanel][Ce].ProgramFail = pRdtInfo->programFailNum;
                    BbInfo.BadBlkPerLun[Chanel][Ce].ReadFail = pRdtInfo->readFailNum;
                    BbInfo.BadBlkPerLun[Chanel][Ce].TotalBad = BbInfo.BadBlkPerLun[Chanel][Ce].OriBad + BbInfo.BadBlkPerLun[Chanel][Ce].EraseFail +
                                                            BbInfo.BadBlkPerLun[Chanel][Ce].ProgramFail + BbInfo.BadBlkPerLun[Chanel][Ce].ReadFail;
                    BbInfo.TotalOriBad += pRdtInfo->factoryBadNum;
                    BbInfo.TotalNewBad += pRdtInfo->laterBadNum;
                    BbInfo.TotalProgFail += pRdtInfo->programFailNum;
                    BbInfo.ToTalEraseFail += pRdtInfo->eraseFailNum;
                    BbInfo.TotalReadFail += pRdtInfo->readFailNum;
                    BbInfo.TotalBadBlock += BbInfo.BadBlkPerLun[Chanel][Ce].TotalBad;

                    BadBlkDumpLog.append(QString::asprintf("CH%d CE%d LUN%d :\n",Chanel, Ce, Lun));
                    BadBlkDumpLog.append(QString::asprintf("BadBlockCount: %d\t FactoryBad: %d\t NewBad: %d\n"
                                                           "ProgramFail: %d\t EraseFail: %d\t ReadFail: %d\n\n",
                                                           pRdtInfo->badBlcokNum, pRdtInfo->factoryBadNum, pRdtInfo->laterBadNum,
                                                           pRdtInfo->programFailNum, pRdtInfo->eraseFailNum, pRdtInfo->readFailNum));
                }
            }
        }
    }
    BadBlkDumpLog.append(QString::asprintf("Total Bad Block counts:%d\n",BbInfo.TotalBadBlock));
    BadBlkDumpLog.append(QString::asprintf("Total Factory Bad Block counts:%d\n",BbInfo.TotalOriBad));
    BadBlkDumpLog.append(QString::asprintf("Total New Bad Block counts:%d\n",BbInfo.TotalNewBad));
    BadBlkDumpLog.append(QString::asprintf("Total Program Fail Block counts:%d\n",BbInfo.TotalProgFail));
    BadBlkDumpLog.append(QString::asprintf("Total Erase Fail Block counts:%d\n",BbInfo.ToTalEraseFail));
    BadBlkDumpLog.append(QString::asprintf("Total Read Fail Block counts:%d\n",BbInfo.TotalReadFail));


    //free(pRdtInfo);
    return Status;
}

/*
unsigned long PhysicalDriveInfo::GetEventLogInfo(unsigned char* buff)
{
    unsigned char buffer[EVENT_LOG_DATA_LENGTH];
    unsigned long status = ERROR_SUCCESS_STATUS;
#ifdef WIN32
    NVMeApp nvmeapp(currentDrive);
    status = nvmeapp.EventLog(buffer);
    if(status == ERROR_SUCCESS)
        memcpy(buff,buffer,EVENT_LOG_DATA_LENGTH);
    else
        printf("identify error %d",status);
#else
    __u32 result;
    void *data = NULL, *metadata = NULL;
    memset(buffer,0,EVENT_LOG_DATA_LENGTH);

    int fd = open_dev(currentDev);
    if(fd<0)
    {
        status=fd;
        printf("open dev failed err %d",status);
        return status;
    }

    struct config cfg;
    cfg.opcode=0xc2;
    cfg.flags=0;
    cfg.rsvd=0;
    cfg.namespace_id=0;
    cfg.data_len=EVENT_LOG_DATA_LENGTH;
    cfg.metadata_len=0;
    cfg.timeout=0;
    cfg.cdw2=0;
    cfg.cdw3=0;
    cfg.cdw10=4096;
    cfg.cdw11=0;
    cfg.cdw12=87;
    cfg.cdw13=0;
    cfg.cdw14=0;
    cfg.cdw15=0;
    cfg.prefill =0;

    status = nvme_passthru(fd, NVME_IOCTL_ADMIN_CMD, cfg.opcode, cfg.flags, cfg.rsvd,
                cfg.namespace_id, cfg.cdw2, cfg.cdw3, cfg.cdw10,
                cfg.cdw11, cfg.cdw12, cfg.cdw13, cfg.cdw14, cfg.cdw15,
                cfg.data_len, (void *)buffer, cfg.metadata_len, metadata,
                cfg.timeout, &result);

    if(status<0)
    {
        printf("event-log passthru error");
        return status;
    }
    else if(status)
    {
        printf("event-log passthru error");
        show_nvme_status(status);
        return status;
    }

    memcpy(buff,buffer,EVENT_LOG_DATA_LENGTH);
    close_dev(fd);
#endif

    return status;
}
*/

unsigned long PhysicalDriveInfo::GetIDYNamespaceInfo(unsigned char* buff)
{
    unsigned char buffer[IDY_NAMESPACE_INFO_LENGTH];
    unsigned long status = ERROR_SUCCESS_STATUS;
#ifdef WIN32
    NVMeApp nvmeapp(currentDrive);
    status = nvmeapp.IDTableNamespaceInfo(buffer);
    if(status == ERROR_SUCCESS)
        memcpy(buff,buffer,IDY_NAMESPACE_INFO_LENGTH);
    else
        printf("id table namespace error %d",status);
#else

    int fd = open_dev(currentDev);
    if(fd<0)
    {
        status=fd;
        printf("open dev failed err %d",status);
        return status;
    }
    status = nvme_identify_ns(fd, 1, false, &buffer);
    if(!status)
    {
        memcpy(buff,buffer,IDY_NAMESPACE_INFO_LENGTH);
    }
    else
    {
        printf("smart-log error %d",status);
    }
    close_dev(fd);
#endif

     return status;
}

unsigned long PhysicalDriveInfo::GetIDYControllerInfo(unsigned char* buff)
{
    unsigned char buffer[IDY_CONTROLLER_INFO_LENGTH];
    unsigned long status = ERROR_SUCCESS_STATUS;
#ifdef WIN32
    NVMeApp nvmeapp(currentDrive);
    status = nvmeapp.IDTableControllerInfo(buffer);
    if(status == ERROR_SUCCESS)
        memcpy(buff,buffer,IDY_CONTROLLER_INFO_LENGTH);
    else
        printf("id table controller error %d",status);
#else

    int fd = open_dev(currentDev);
    if(fd<0)
    {
        status=fd;
        printf("open dev failed err %d",status);
        return status;
    }
    status = nvme_identify_ctrl(fd, &buffer);
    if(!status)
    {
        memcpy(buff,buffer,IDY_CONTROLLER_INFO_LENGTH);
    }
    else
    {
        printf("smart-log error %d",status);
    }
    close_dev(fd);
#endif

     return status;
}

unsigned long PhysicalDriveInfo::GetErrorLogInfo(unsigned char *buff, int entryCnt)
{
    unsigned char buffer[entryCnt * ERROR_ENTRY_LENGTH];
    unsigned long status = ERROR_SUCCESS_STATUS;
#ifdef WIN32
    NVMeApp nvmeapp(currentDrive);
    status = nvmeapp.SmartInfo(buffer);
    if(status == ERROR_SUCCESS)
        memcpy(buff,buffer,SMART_DATA_LENGTH);
    else
        printf("identify error %d",status);
#else

    int fd = open_dev(currentDev);
    if(fd<0)
    {
        status=fd;
        printf("open dev failed err %ld",status);
        return status;
    }
    status = nvme_error_log(fd, entryCnt, (struct nvme_error_log_page *)buffer);
    if(!status)
    {
        memcpy(buff,buffer,entryCnt * ERROR_ENTRY_LENGTH);
    }
    else
    {
        printf("error-log error %ld",status);
    }
    close_dev(fd);
#endif

     return status;
}


