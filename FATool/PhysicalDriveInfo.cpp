
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
        printf("identify error %d",status);

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

    struct config cfg;
    cfg.opcode=0xc2;
    cfg.flags=0;
    cfg.rsvd=0;
    cfg.namespace_id=0;
    cfg.data_len= FLASH_ID_DATA_LENGTH;
    cfg.metadata_len=0;
    cfg.timeout=0;
    cfg.cdw2=0;
    cfg.cdw3=0;
    cfg.cdw10=2048;
    cfg.cdw11=0;
    cfg.cdw12=64;
    cfg.cdw13=1;
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


