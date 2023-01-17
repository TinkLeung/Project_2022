/****************************************************************************
** Copyright © ShenZhen TIGO Semiconductor Co, Ltd.
** Contact: http://www.tigo.com.hk/
$Module Element:
$Discription:
$Generated Date	:JU, 29th 2021
****************************************************************************/


#include "FirmwareUpdate.h"
#include <ntddscsi.h>
#include <string>
#include <ntddscsi.h>
#include <sysinfoapi.h>
#include "Utils.h"

typedef struct Nvme_Pass_Througn_Ioctl {
    SRB_IO_CONTROL SrbIoCtrl;
    DWORD		   VendorSpecific[NVME_IOCTL_VENDOR_SPECIFIC_DW_SIZE];
    DWORD		   NVMeCmd[NVME_IOCTL_CMD_DW_SIZE];
    DWORD		   CplEntry[NVME_IOCTL_COMPLETE_DW_SIZE];
    DWORD		   Direction;
    DWORD		   QueueId;
    DWORD		   DataBufferLen;
    DWORD		   MetaDataLen;
    DWORD		   ReturnBufferLen;
    UCHAR		   DataBuffer[4096];
}NVME_PASS_THROUGH_IOCTL;


/*
typedef struct _SCSI_PASS_THROUGH 
{
    USHORT Length;
    UCHAR ScsiStatus;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR CdbLength;
    UCHAR SenseInfoLength;
    UCHAR DataIn;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    ULONG_PTR DataBufferOffset;
    ULONG SenseInfoOffset;
    UCHAR Cdb[16];
}SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;
*/

typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS24 
{
    SCSI_PASS_THROUGH Spt;
    UCHAR             SenseBuf[24];
    UCHAR             DataBuf[4096];
} SCSI_PASS_THROUGH_WITH_BUFFERS24, *PSCSI_PASS_THROUGH_WITH_BUFFERS24;

using namespace std;


CFirmwareUpdate* CFirmwareUpdate::m_s_pInstance(nullptr);

CFirmwareUpdate::CFirmwareUpdate()
{
	// nothing to do;
}

CFirmwareUpdate::~CFirmwareUpdate()
{
	if(m_s_pInstance!=nullptr)
	{
		delete m_s_pInstance;
		m_s_pInstance = nullptr;
	}
}

CFirmwareUpdate* CFirmwareUpdate::GetInstance() 
{
    if (0 == m_s_pInstance)
    {
        m_s_pInstance = new CFirmwareUpdate();
    }
    return m_s_pInstance; 
}


BOOL CFirmwareUpdate::FirmWareUpdateProcess(INT driveNumber,QString FileName, qint64 transize)
{
    BOOL result;
    DEVICE_LIST DeviceList;
	UCHAR Fwslot;

    result = DeviceFirmwareHandle(driveNumber,&DeviceList);
    if(result == FALSE)
    {   
        qDebug (" FirmWareUpdateProcess: get Device handle failed! \n");
		return FALSE;
	}

    result = DeviceStorageFirmwareWin10Download(driveNumber, FileName, transize);
    //result = DeviceStorageFirmwareWin10Upgrade(driveNumber, FileName, TRUE);
	if(result == FALSE)
	{
		qDebug (" Device Storage Firmware Upgrade successfully ! \n");
		return FALSE;
	}

    qDebug (" FirmWareUpdateProcess: Firmware update successfully slot: %d \n",m_slot);

	return TRUE;
}


BOOL CFirmwareUpdate::DeviceFirmwareHandle(INT driveNumber, PDEVICE_LIST pDeviceList)
{
    TCHAR buffer[64];
    BOOL result;
    STORAGE_ADAPTER_DESCRIPTOR StgAdpDsp;

    wsprintf(buffer, TEXT("\\\\.\\PhysicalDrive%d"), driveNumber);
    HANDLE hIoCtrl = CreateFile(buffer,	GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);

    if(hIoCtrl != INVALID_HANDLE_VALUE)
    {
        qDebug (" FirmwareHandle: Find Firmware drive: %s handle! \n", buffer);
        result = DeviceQueryProperty(hIoCtrl,&StgAdpDsp);
        if(!result)
        {
            qDebug (" FirmwareHandle: get drive properties failed! \n");
            CloseHandle(hIoCtrl);
            return FALSE;
        }
		ZeroMemory(&m_storageadptdecripter, sizeof(STORAGE_ADAPTER_DESCRIPTOR));
        /* Get the drive handle number and drive properity , maybe now, it is useless */
        RtlMoveMemory(&(m_storageadptdecripter), &(StgAdpDsp), sizeof(STORAGE_ADAPTER_DESCRIPTOR));
        pDeviceList->Handle = hIoCtrl;
    }
    else
    {
        CloseHandle(hIoCtrl);
        qDebug (" FirmwareHandle: Create Firmware Io handle failed! \n");
        return FALSE;
    }

	qDebug (" FirmwareHandle: Find Firmware drive successfully ! \n");

    CloseHandle(hIoCtrl);
    return TRUE;
}

BOOL CFirmwareUpdate::DeviceQueryProperty(HANDLE hioctrl, PSTORAGE_ADAPTER_DESCRIPTOR pStorageAdpDsp)
{
    PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor = NULL;
    STORAGE_DESCRIPTOR_HEADER header = { 0 };

    BOOL result = TRUE;
    //BOOL failed = TRUE;

    /* Get size required for storage adapter descriptor */
    PVOID buffer = NULL;
    ULONG bufferSize = 0;
    ULONG returnedData;

    STORAGE_PROPERTY_QUERY query;// = { 0 };
    query.QueryType = PropertyStandardQuery;
    query.PropertyId = StorageAdapterProperty;
    bufferSize = sizeof(STORAGE_DESCRIPTOR_HEADER);
    buffer = &header;

    ZeroMemory(buffer, bufferSize);

    result = DeviceIoControl(hioctrl, IOCTL_STORAGE_QUERY_PROPERTY, &query,
                            sizeof(STORAGE_PROPERTY_QUERY),buffer,bufferSize,&returnedData,FALSE);

    if (!result)
    {
        qDebug("\t Get Storage Adapter Property  Header Failed: 0x%X\n", GetLastError());
		
        return FALSE;
    }

    /* Allocate and retrieve storage adapter descriptor */
    query.QueryType = PropertyStandardQuery;
    query.PropertyId = StorageAdapterProperty;
    bufferSize = header.Size;

    if (bufferSize != 0)
    {
        buffer = (PSTORAGE_ADAPTER_DESCRIPTOR)malloc(bufferSize);
        if (buffer == NULL)
        {
            free(buffer);
            return FALSE;
        }
    }

    ZeroMemory(buffer, bufferSize);

    /* All setup, do the ioctl */
    result = DeviceIoControl(hioctrl,IOCTL_STORAGE_QUERY_PROPERTY,&query,sizeof(STORAGE_PROPERTY_QUERY),
                                buffer,bufferSize,&returnedData,FALSE);

    if (!result)
    {
        qDebug("\t Get Storage Adapter Property Failed: 0x%X\n", GetLastError());
        free(buffer);
        return FALSE;
    }

    /* AdapterDescriptor is now allocated and full of data. */

    if (returnedData == 0)
    {
        printf("   ***** No adapter descriptor supported on the device *****\n");
    }
    else
    {
        *pStorageAdpDsp = *(PSTORAGE_ADAPTER_DESCRIPTOR)buffer;
    }

    free(buffer);
    return TRUE;

}

string CFirmwareUpdate::GetScsiPath(const TCHAR* Path)
{
    HANDLE hIoCtrl = CreateFile(Path, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    SCSI_ADDRESS sadr;
    BOOL bRet = 0;
    DWORD dwReturned;

    bRet = DeviceIoControl(hIoCtrl, IOCTL_SCSI_GET_ADDRESS,
        NULL, 0, &sadr, sizeof(sadr), &dwReturned, NULL);

    string result;
    result=format("\\\\.\\SCSI%d:", sadr.PortNumber);

    CloseHandle(hIoCtrl);
    return result;
}

BOOL CFirmwareUpdate::DeviceGetFirmwareSlotInfoIntelLog(INT physicalDriveId, STORAGE_FIRMWARE_EX_INFO *StoratgeFirmwareInfo)
{

	string strpath,scsipath;
	TCHAR tStrpath[256] = {0};
	TCHAR tScsipath[256] = {0};
	DWORD count = 0;
	BOOL bRet = FALSE;
	NVME_PASS_THROUGH_IOCTL nptwb;
	DWORD length = sizeof(nptwb);
	DWORD dwReturned;

	wsprintf(tStrpath, (L"\\\\.\\PhysicalDrive%d"),physicalDriveId);
	scsipath = GetScsiPath(tStrpath);
#ifdef UNICODE
	_stprintf_s(tScsipath,256,_T("%S"), scsipath.c_str());
#else
	_stprintf_s(tScsipath,256,_T("%s"), scsipath.c_str());
#endif

	/* Get Log page - Firmware Slot Information Log */
	HANDLE hIoCtrl1 = CreateFile(tScsipath, GENERIC_READ | GENERIC_WRITE,
	   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	ZeroMemory(&nptwb, sizeof(NVME_PASS_THROUGH_IOCTL));

	nptwb.SrbIoCtrl.ControlCode = NVME_PASS_THROUGH_SRB_IO_CODE;
	nptwb.SrbIoCtrl.HeaderLength = sizeof(SRB_IO_CONTROL);
	memcpy((UCHAR*)(&nptwb.SrbIoCtrl.Signature[0]), NVME_SIG_STR, NVME_SIG_STR_LEN);
	nptwb.SrbIoCtrl.Timeout = NVME_PT_TIMEOUT;
	nptwb.SrbIoCtrl.Length = length - sizeof(SRB_IO_CONTROL);
	nptwb.DataBufferLen = sizeof(nptwb.DataBuffer);
	nptwb.ReturnBufferLen = sizeof(nptwb);
	nptwb.Direction = NVME_FROM_DEV_TO_HOST;

	nptwb.NVMeCmd[0] = 0x002F0002;  //Comand Dword 0
	nptwb.NVMeCmd[1] = 0x0;  		//Comand Dword 1
    nptwb.NVMeCmd[10] = 0x007F0003;     //Command Dword 10

	bRet = DeviceIoControl(hIoCtrl1, IOCTL_SCSI_MINIPORT,
	   &nptwb, length, &nptwb, length, &dwReturned, NULL);
	if (bRet == FALSE)
	{
	   qDebug(" Get Firmware Slot Information Log Failed: errorcode:%d\n", GetLastError());
	   ::CloseHandle(hIoCtrl1);
	   return	FALSE;
	}

	for (int i = 0; i < 512; i++)
	   count += nptwb.DataBuffer[i];

	if (count == 0)
	{
	   qDebug(" Get Firmware Slot Information Log Failed No data \n");
	   ::CloseHandle(hIoCtrl1);
	   return	FALSE;
	}

	/* Active Firmware Info(AFI).*/
	StoratgeFirmwareInfo->ActiveSlot = nptwb.DataBuffer[0]& 0x7;  	// Bit: 0-2 active slot.
	StoratgeFirmwareInfo->PendingActivateSlot = (nptwb.DataBuffer[0]>>4)& 0x7;	// slot that is going to be actived at the next controller reset.

	//for (int i=0; i<StorageFirmwareInfo.SlotCount; i++)
	//{
	   RtlCopyMemory(&StoratgeFirmwareInfo->Slot[0].Revision.Info,&nptwb.DataBuffer[8], 8);
	//}
	//memcpy_s(data, sizeof(Bin_IDENTIFY_DEVICE), nptwb.DataBuffer, sizeof(Bin_IDENTIFY_DEVICE));

	CloseHandle(hIoCtrl1);
	return bRet;

}


BOOL CFirmwareUpdate::DeviceGetFirmwareSlotInfoIntelIdentify(INT physicalDriveId, STORAGE_FIRMWARE_EX_INFO *StoratgeFirmwareInfo)
{

		string strpath,scsipath;
		TCHAR tStrpath[256] = {0};
		TCHAR tScsipath[256] = {0};
		DWORD count = 0;
		BOOL bRet = FALSE;
		NVME_PASS_THROUGH_IOCTL nptwb;
		DWORD length = sizeof(nptwb);
		DWORD dwReturned;
		
		wsprintf(tStrpath, (L"\\\\.\\PhysicalDrive%d"),physicalDriveId);
		scsipath = GetScsiPath(tStrpath);
		
#ifdef UNICODE
		_stprintf_s(tScsipath,256,_T("%S"), scsipath.c_str());
#else
		_stprintf_s(tScsipath,256,_T("%s"), scsipath.c_str());
#endif

		HANDLE hIoCtrl = CreateFile(tScsipath, GENERIC_READ | GENERIC_WRITE,
										FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	
		if(hIoCtrl == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hIoCtrl);
			qDebug (" FirmwareHandle: Create Firmware Io handle failed! \n");
			return FALSE;
		}
	
		ZeroMemory(&nptwb, sizeof(NVME_PASS_THROUGH_IOCTL));
	
		nptwb.SrbIoCtrl.ControlCode = NVME_PASS_THROUGH_SRB_IO_CODE;
		nptwb.SrbIoCtrl.HeaderLength = sizeof(SRB_IO_CONTROL);
		memcpy((UCHAR*)(&nptwb.SrbIoCtrl.Signature[0]), NVME_SIG_STR, NVME_SIG_STR_LEN);
		nptwb.SrbIoCtrl.Timeout = NVME_PT_TIMEOUT;
		nptwb.SrbIoCtrl.Length = length - sizeof(SRB_IO_CONTROL);
		nptwb.DataBufferLen = sizeof(nptwb.DataBuffer);
		nptwb.ReturnBufferLen = sizeof(nptwb);
		nptwb.Direction = NVME_FROM_DEV_TO_HOST;
	
		nptwb.NVMeCmd[0] = 0x00070006;	// Identify
		nptwb.NVMeCmd[1] = 0x0; 
		nptwb.NVMeCmd[10] = 0x1;	// Return to Host
	
		bRet = DeviceIoControl(hIoCtrl, IOCTL_SCSI_MINIPORT,&nptwb, length, &nptwb, length, &dwReturned, NULL);
	
		if (bRet == FALSE)
		{
			qDebug(" Get Firmware Slot Information Identify Failed: errorcode:%d\n", GetLastError());
			::CloseHandle(hIoCtrl);
			return	FALSE;
		}
	
		count = 0;
		for (int i = 0; i < 512; i++)
		{
			count += nptwb.DataBuffer[i];
		}
		if (count == 0)
		{
			::CloseHandle(hIoCtrl);
			return	FALSE;
		}
	
		/* Optional Admin Command Support(OACS)*/
		StoratgeFirmwareInfo->UpgradeSupport = (BOOL)((nptwb.DataBuffer[256]>>2)&0x1); //OACS Bit:2 support Firmware commit/Image Download.
	
		/* Firmware Update(FRMW)*/
		StoratgeFirmwareInfo->SlotCount = (nptwb.DataBuffer[260]>>1)&0x7;	//FRMW Bit:3~1 slot count [1~7]
		StoratgeFirmwareInfo->IsFirmwareActWithoutRest = (BOOL)((nptwb.DataBuffer[260]>>4)&0x1); // Bit:4 check firmwareactive without reset.
		if((StoratgeFirmwareInfo->SlotCount<1)||(StoratgeFirmwareInfo->SlotCount>7))	// check slot count.
		{
			qDebug(" Get Firmware Slot Information Identify Failed: SlotCount error\n");
			::CloseHandle(hIoCtrl);
			return FALSE;
		}
	
		for (int i=0; i<StoratgeFirmwareInfo->SlotCount; i++)				//initilize slot information.
		{
			StoratgeFirmwareInfo->Slot[i].SlotNumber = i+1;
			if(i==0)
				StoratgeFirmwareInfo->Slot[i].ReadOnly = (BOOL)nptwb.DataBuffer[260]&0x1; //Bit:1 check 1 slot is readonly,or not.
			else
				StoratgeFirmwareInfo->Slot[i].ReadOnly = FALSE;
			StoratgeFirmwareInfo->Slot[i].Revision.AsUlonglong =0;
		}
		/* Firmware Update Granularity(FWUG)*/
		StoratgeFirmwareInfo->FirmwareUptGranularity = nptwb.DataBuffer[319];
	
		CloseHandle(hIoCtrl);
	
		return TRUE;
	
}

HANDLE CFirmwareUpdate::GetIoCtrlHandle(BYTE index)
{
    //string	strDevice;
	TCHAR tStrDevice[256] = {0};
    //strDevice=format( ("\\\\.\\PhysicalDrive%d"), index);
	wsprintf(tStrDevice, (L"\\\\.\\PhysicalDrive%d"),index);

    return ::CreateFile(tStrDevice, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, 0, NULL);
}

BOOL CFirmwareUpdate::DeviceGetFirmwareSlotInfoSamsungLog(INT physicalDriveId, STORAGE_FIRMWARE_EX_INFO *StoratgeFirmwareInfo)
{

	return TRUE;
}

BOOL CFirmwareUpdate::DeviceGetFirmwareSlotInfoSamsungIdentify(INT physicalDriveId, STORAGE_FIRMWARE_EX_INFO *StoratgeFirmwareInfo)
{

	BOOL	bRet;
	HANDLE	hIoCtrl;
	DWORD	dwReturned;
	DWORD	length;

	SCSI_PASS_THROUGH_WITH_BUFFERS24 sptwb;

	hIoCtrl = GetIoCtrlHandle(physicalDriveId);

	if (hIoCtrl == INVALID_HANDLE_VALUE)
	{
		return	FALSE;
	}

	ZeroMemory(&sptwb, sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS24));

	sptwb.Spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.Spt.PathId = 0;
	sptwb.Spt.TargetId = 0;
	sptwb.Spt.Lun = 0;
	sptwb.Spt.SenseInfoLength = 24;
	sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.Spt.DataTransferLength = IDENTIFY_BUFFER_SIZE;
	sptwb.Spt.TimeOutValue = 2;
	sptwb.Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS24, DataBuf);
	sptwb.Spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS24, SenseBuf);

	sptwb.Spt.CdbLength = 16;
	sptwb.Spt.Cdb[0] = 0xB5; // SECURITY PROTOCOL OUT
	sptwb.Spt.Cdb[1] = 0xFE; // SAMSUNG PROTOCOL
	sptwb.Spt.Cdb[2] = 0;
	sptwb.Spt.Cdb[3] = 5;
	sptwb.Spt.Cdb[4] = 0;
	sptwb.Spt.Cdb[5] = 0;
	sptwb.Spt.Cdb[6] = 0;
	sptwb.Spt.Cdb[7] = 0;
	sptwb.Spt.Cdb[8] = 0;
	sptwb.Spt.Cdb[9] = 0x40;
	sptwb.Spt.DataIn = SCSI_IOCTL_DATA_OUT;
	sptwb.DataBuf[0] = 1;


	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS24, DataBuf) + sptwb.Spt.DataTransferLength;

	bRet = ::DeviceIoControl(hIoCtrl, IOCTL_SCSI_PASS_THROUGH,
									&sptwb, length,&sptwb, length, &dwReturned, NULL);

	if (bRet == FALSE)
	{
		::CloseHandle(hIoCtrl);
		return	FALSE;
	}

	sptwb.Spt.CdbLength = 16;
	sptwb.Spt.Cdb[0] = 0xA2; // SECURITY PROTOCOL IN
	sptwb.Spt.Cdb[1] = 0xFE; // SAMSUNG PROTOCOL
	sptwb.Spt.Cdb[2] = 0;
	sptwb.Spt.Cdb[3] = 5;
	sptwb.Spt.Cdb[4] = 0;
	sptwb.Spt.Cdb[5] = 0;
	sptwb.Spt.Cdb[6] = 0;
	sptwb.Spt.Cdb[7] = 0;
	sptwb.Spt.Cdb[8] = 1;
	sptwb.Spt.Cdb[9] = 0;

	sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.DataBuf[0] = 0;

	bRet = ::DeviceIoControl(hIoCtrl, IOCTL_SCSI_PASS_THROUGH,
		&sptwb, length,
		&sptwb, length, &dwReturned, NULL);

	if (bRet == FALSE)
	{
		::CloseHandle(hIoCtrl);
		return	FALSE;
	}

	DWORD count = 0;
	for (int i = 0; i < 512; i++)
	{
		count += sptwb.DataBuf[i];
	}

	if(count == 0)
	{
		::CloseHandle(hIoCtrl);
		return	FALSE;
	}

	::CloseHandle(hIoCtrl);


	/* Optional Admin Command Support(OACS)*/
	StoratgeFirmwareInfo->UpgradeSupport = (BOOL)((sptwb.DataBuf[256]>>2)&0x1); //OACS Bit:2 support Firmware commit/Image Download.
	
	/* Firmware Update(FRMW)*/
	StoratgeFirmwareInfo->SlotCount = (sptwb.DataBuf[260]>>1)&0x7;	//FRMW Bit:3~1 slot count [1~7]
	StoratgeFirmwareInfo->IsFirmwareActWithoutRest = (BOOL)((sptwb.DataBuf[260]>>4)&0x1); // Bit:4 check firmwareactive without reset.
	if((StoratgeFirmwareInfo->SlotCount<1)||(StoratgeFirmwareInfo->SlotCount>7))	// check slot count.
	{
		qDebug(" Get Firmware Slot Information Identify Failed: SlotCount error\n");
		return FALSE;
	}
	
	for (int i=0; i<StoratgeFirmwareInfo->SlotCount; i++)				//initilize slot information.
	{
		StoratgeFirmwareInfo->Slot[i].SlotNumber = i+1;
		if(i==0)
			StoratgeFirmwareInfo->Slot[i].ReadOnly = (BOOL)sptwb.DataBuf[260]&0x1; //Bit:1 check 1 slot is readonly,or not.
		else
			StoratgeFirmwareInfo->Slot[i].ReadOnly = FALSE;
		StoratgeFirmwareInfo->Slot[i].Revision.AsUlonglong =0;
	}
	/* Firmware Update Granularity(FWUG)*/
	StoratgeFirmwareInfo->FirmwareUptGranularity = sptwb.DataBuf[319];


	return TRUE;

}




BOOL CFirmwareUpdate::DeviceGetFirmwareInfo(INT physicalDriveId, PSTORAGE_FIRMWARE_INFO pStorageFirmwareInfo, BOOL DisplayResult)
{

	BOOL bRet;
	
	//bRet = DeviceGetFirmwareSlotInformationIdentify(physicalDriveId,pStorageFirmwareInfo);
	//if(!bRet)
		//return FALSE;
	
   //bRet = DeviceGetFirmwareSlotInformationLog(physicalDriveId, pStorageFirmwareInfo);
	//if(!bRet)
		//return FALSE;

	return TRUE;
	
}


BOOL  CFirmwareUpdate::DeviceFirmwareIntelUpgrade(INT physicalDriveId, string FileName)
{
    string strpath,scsipath,filenamepath;
	TCHAR tStrpath[256] = {0};
	TCHAR tScsipath[256] = {0};
    UINT count = 0;
    BOOL bRet = FALSE;
    NVME_PASS_THROUGH_IOCTL nptwb;
    DWORD length = sizeof(nptwb);
    DWORD dwReturned;
    HANDLE hIoCtrl = NULL;
    HANDLE hFile = NULL;
	BOOL moreToDownload = TRUE;
    UCHAR FileContext[4097];
    DWORD ReadInt = 0;
    DWORD NVMeCmd0Val = 0x00170011;
    DWORD NVMeCmd10Val = 0x0;
    DWORD NVMeCmd11Val = 0x0;
    DWORD tmpNVMeCmd11Val = 0x0;
    UINT Count =0;

    STORAGE_FIRMWARE_INFO	StorageFirmwareInfo;
    ZeroMemory(&StorageFirmwareInfo, sizeof(STORAGE_FIRMWARE_INFO));

    //char buff[1024];
    //GetCurrentDirectory(1024, buff);

	TCHAR tbuff[1024];
	GetCurrentDirectory(1024, tbuff);

	wsprintf(tStrpath,(L"\\\\.\\PhysicalDrive%d"),physicalDriveId);
	//strpath = format("\\\\.\\PhysicalDrive%d", physicalDriveId);
    scsipath = GetScsiPath(tStrpath);
#ifdef UNICODE
	_stprintf_s(tScsipath,256,_T("%S"), scsipath.c_str());
#else
	_stprintf_s(tScsipath,256,_T("%s"), scsipath.c_str());
#endif


	/* 1. Select a suitable firmware slot. */

    /* 1.1 Get firmware slot information data. */
	
	bRet = DeviceGetFirmwareInfo(physicalDriveId, &StorageFirmwareInfo, TRUE);
    if(bRet == FALSE)
    {
        qDebug("FirmwareUpgrade: Get Firmware Information Failed.\n");
        return FALSE; 
    }

	/* 1.2 Check to support firmware update.*/
    if( StorageFirmwareInfo.UpgradeSupport == FALSE)
    {
        qDebug("FirmwareUpgrade: Firmware doesnot support upgrade!\n");
        return FALSE; 
    }

	if( StorageFirmwareInfo.SlotCount ==0 )
	{
		qDebug("FirmwareUpgrade: Firmware no slot!\n");
		return FALSE; 
	}
	else 
	{
		if(StorageFirmwareInfo.SlotCount ==1)
		{
			if(StorageFirmwareInfo.Slot[0].ReadOnly)
			{
				qDebug("FirmwareUpgrade: Firmware one slot and readonly!\n");
				return FALSE; 
			}	
		}
	}

	/* 1.3 Select a suitable slot. */


	/* 1.4 SelectFind the first writable slot */


	/* 2. Download the firmware image to the controller. */

    /* Check path of firmware bin file. */
    //if (FileName == NULL) 
	//{
        //qDebug(("\t FirmwareUpgrade: No firmware file specified.\n"));
        //return FALSE ;
    //}
	

    filenamepath = Replace(FileName, "\\","\\\\");

	//qDebug(("FirmwareUpgrade: firmware file name: %s\n"), FileName.c_str());
	TCHAR tFileName[1024] = {0};
#ifdef UNICODE
	_stprintf_s(tFileName,1024,_T("%S"),FileName.c_str());
#else
	_stprintf_s(tFileName,1024,_T("%s"),FileName.c_str());
#endif
  
    // Get firmware bin file handle 
    hFile = CreateFile(tFileName,    //"C:\\works\\code\\build-Qttest-Desktop_Qt_5_13_2_MinGW_64_bit-Debug\\2262_ISP_Package.bin",
                       GENERIC_READ, FILE_SHARE_READ,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        //qDebug(("FirmwareUpgrade: unable to open file \"%s\" for read.0x%x\n"), FileName,GetLastError());
        ::CloseHandle(hFile);
        return FALSE ;
    }


    hIoCtrl = CreateFile(tScsipath, GENERIC_READ | GENERIC_WRITE,
       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hIoCtrl == INVALID_HANDLE_VALUE)
    {
        //qDebug(("\t FirmwareUpgrade: %s device Io control failed \n"),scsipath.c_str());
        ::CloseHandle(hIoCtrl);
        ::CloseHandle(hFile);
        return FALSE ;
    }

    /*
     * 
                  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                  +	1024		+	1024		+	1024		+.........+	  1024	  +	remainlen   +
                  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    offset(DWORD) 0              1024            2048            3072       N*1024  	(N+1)
                                                                                        *1024
                  +																				 	+
                  ++++++++++++++++++The Total length of firmware download file+++++++++++++++++++++++
                  +																				 	+

    */

    NVMeCmd0Val = 0x11;  // start value of Command Dword 0(CID+PSDT+FUSE+Opcode)  CID 00010111b
    count = 0;
    while (ReadFile(hFile, FileContext, 4096, &ReadInt, NULL) > 0 && ReadInt > 0)
    {
        ZeroMemory(&nptwb, sizeof(NVME_PASS_THROUGH_IOCTL));
        ZeroMemory(&nptwb.DataBuffer,4096);

        //qDebug((" FirmwareUpgrade: Readfile length %d count: %d \n"),ReadInt,count);

        if(count>0)
        {
            if(ReadInt == 4096)
            {
                NVMeCmd10Val = 0x000003FF;              //0’s based value       
                NVMeCmd11Val = NVMeCmd11Val+ 1024;
            }
            else
            {
                if(ReadInt%4!=0)
                    NVMeCmd10Val = (DWORD)(ReadInt/4);
                else
                    NVMeCmd10Val = (DWORD)ReadInt/4-1;

                NVMeCmd11Val = (DWORD)(NVMeCmd11Val + 1024);   //offset
            }	
        }
		else
		{
            NVMeCmd10Val =(DWORD)ReadInt/4-1;
			NVMeCmd11Val = 0x0;
		}


        nptwb.SrbIoCtrl.ControlCode = NVME_PASS_THROUGH_SRB_IO_CODE;
        nptwb.SrbIoCtrl.HeaderLength = sizeof(SRB_IO_CONTROL);
        memcpy((UCHAR*)(&nptwb.SrbIoCtrl.Signature[0]), NVME_SIG_STR, NVME_SIG_STR_LEN);
        nptwb.SrbIoCtrl.Timeout = NVME_PT_TIMEOUT;
        nptwb.SrbIoCtrl.Length = length - sizeof(SRB_IO_CONTROL);
        nptwb.DataBufferLen = sizeof(nptwb.DataBuffer);
        nptwb.ReturnBufferLen = sizeof(nptwb);
        nptwb.Direction = NVME_FROM_DEV_TO_HOST;
        //nptwb.Direction = 1;

		
        RtlMoveMemory(&nptwb.DataBuffer, &FileContext, ReadInt);

        nptwb.NVMeCmd[0] = NVMeCmd0Val;     //DWORD0
        nptwb.NVMeCmd[1] = 0x0;         //DWORD1
        nptwb.NVMeCmd[10] = NVMeCmd10Val;     //DWORD10: Number of Dwords(NUMD) in Firmware Image Downlad(4096 BYTE data(1024 DWORD))
        nptwb.NVMeCmd[11] = NVMeCmd11Val;    //DWORD11  Offset

        qDebug(" FirmwareUpgrade: Readfile length:%d count:%d DWORD0:0x%x DWORD10:0x%x  DWORD11:0x%x\n",
                            ReadInt,count,NVMeCmd0Val,NVMeCmd10Val,NVMeCmd11Val);


        bRet = DeviceIoControl(hIoCtrl, IOCTL_SCSI_MINIPORT,&nptwb, length, &nptwb, length, &dwReturned, NULL);
        if (bRet == FALSE)
        {
            qDebug(" FirmwareUpgrade: Firmware download failed. errorcode:%d\n", GetLastError());
            ::CloseHandle(hIoCtrl);
            return FALSE ;
        }
        qDebug(" FirmwareUpgrade: Firmware download process returncode:%d\n",dwReturned);
        count++;
        NVMeCmd0Val = NVMeCmd0Val + 0x10000;      // Command Idenifer(CID) +1

    }
	
    /*3. Active the firmware slot assigned to upgrade.*/
    ZeroMemory(&nptwb, sizeof(NVME_PASS_THROUGH_IOCTL));
    ZeroMemory(&nptwb.DataBuffer,4096);

    nptwb.SrbIoCtrl.ControlCode = NVME_PASS_THROUGH_SRB_IO_CODE;
    nptwb.SrbIoCtrl.HeaderLength = sizeof(SRB_IO_CONTROL);
    memcpy((UCHAR*)(&nptwb.SrbIoCtrl.Signature[0]), NVME_SIG_STR, NVME_SIG_STR_LEN);
    nptwb.SrbIoCtrl.Timeout = NVME_PT_TIMEOUT;
    nptwb.SrbIoCtrl.Length = length - sizeof(SRB_IO_CONTROL);
    nptwb.DataBufferLen = sizeof(nptwb.DataBuffer);
    nptwb.ReturnBufferLen = sizeof(nptwb);
    nptwb.Direction = NVME_FROM_DEV_TO_HOST;

    RtlMoveMemory(&nptwb.DataBuffer, &FileContext, ReadInt);

    nptwb.NVMeCmd[0] = 0x008E0010;     //DWORD0
    nptwb.NVMeCmd[1] = 0x0;             //DWORD1
    nptwb.NVMeCmd[10] = 0x19;           //DWORD10:

    bRet = DeviceIoControl(hIoCtrl, IOCTL_SCSI_MINIPORT,&nptwb, length, &nptwb, length, &dwReturned, NULL);
    if (bRet == FALSE)
    {
        qDebug("FirmwareUpgrade: Active the firmware slot failed: errorcode:%d\n", GetLastError());
        ::CloseHandle(hIoCtrl);
        return FALSE ;
    }

	CloseHandle(hIoCtrl);

	return TRUE;
}


#if 1

BOOL CFirmwareUpdate::DeviceGetStorageFirmwareWin10Info(INT physicalDriveId,PUCHAR* Buffer, DWORD* BufferLength,BOOL DisplayResult)
{

    BOOL    result = FALSE;
    ULONG   returnedLength = 0;
    PUCHAR  buffer = NULL;
    ULONG   bufferLength = 0;
    ULONG   retryCount = 0;

	HANDLE hIoCtrl = NULL;

    STORAGE_HW_FIRMWARE_INFO_QUERY  query = {0};
    PSTORAGE_HW_FIRMWARE_INFO       firmwareInfo = NULL;
    PSTORAGE_DEVICE_DESCRIPTOR      deviceDesc; 
	UCHAR                       FirmwareRevision[17];

    hIoCtrl = GetIoCtrlHandle(physicalDriveId);
	
    if(hIoCtrl == INVALID_HANDLE_VALUE)
    {
		qDebug (" firmwareupdate: create Io control handel failed ! \n");
		return FALSE;
	}

	bufferLength = FIELD_OFFSET(STORAGE_HW_FIRMWARE_INFO, Slot) + sizeof(STORAGE_HW_FIRMWARE_SLOT_INFO) * 8;

RetryOnce:

	buffer = (PUCHAR)malloc(bufferLength);
    if (buffer == NULL) 
	{
        /* Exit if memory allocation failed.*/
        if (DisplayResult) {
            qDebug("Allocate Firmware Information Buffer Failed.\n");
        }
        bufferLength = 0;
		 goto Exit;
    }

	RtlZeroMemory(buffer, bufferLength);
	firmwareInfo = (PSTORAGE_HW_FIRMWARE_INFO)buffer;
	
    query.Version = sizeof(STORAGE_HW_FIRMWARE_INFO_QUERY);
    query.Size = sizeof(STORAGE_HW_FIRMWARE_INFO_QUERY);

	query.Flags = STORAGE_HW_FIRMWARE_REQUEST_FLAG_CONTROLLER;
	//query.Flags = 0;

	result = DeviceIoControl(hIoCtrl,
                             IOCTL_STORAGE_FIRMWARE_GET_INFO,
                             &query,
                             sizeof(STORAGE_HW_FIRMWARE_INFO_QUERY),
                             buffer,
                             bufferLength,
                             &returnedLength,
                             NULL
                             );

	if (result && (retryCount == 0)) 
	{
		//
		// Check if bigger buffer is needed. If yes, re-allocate buffer and retry the IOCTL request.
		//
		if (firmwareInfo->SlotCount > 8) 
		{
			if (DisplayResult) 
			{
                qDebug("Slot Count %d is more than 8, allocate a bigger buffer and retry the request.\n", firmwareInfo->SlotCount);
			}

			bufferLength = FIELD_OFFSET(STORAGE_HW_FIRMWARE_INFO, Slot) + sizeof(STORAGE_HW_FIRMWARE_SLOT_INFO) * firmwareInfo->SlotCount;
			retryCount++;

			free(buffer);
			buffer = NULL;

			goto RetryOnce;
		}
	}

	if (result && (retryCount > 0)) 
	{
		//
		// Check if bigger buffer is still needed. This should be a driver issue.
		//
		ULONG	tempLength = 0;

		tempLength = FIELD_OFFSET(STORAGE_HW_FIRMWARE_INFO, Slot) + sizeof(STORAGE_HW_FIRMWARE_SLOT_INFO) * firmwareInfo->SlotCount;

		if (tempLength > bufferLength) {

			if (DisplayResult) {
				qDebug( ("Driver reports slot count %d (unexpected) after retried.\n"), firmwareInfo->SlotCount);
			}

			result = FALSE;
			goto Exit;
		}
	}

	if (!result) 
	{
		if (DisplayResult) 
		{
			qDebug( ("Get Firmware Information - IOCTL_STORAGE_FIRMWARE_GET_INFO Failed. Error code: %d\n"), GetLastError());
		}
		goto Exit;
	} 
	else 
	{
		UCHAR	i;
		WCHAR	revision[32] = {0};

		RtlZeroMemory(m_FirmwareRevision, 17);
		for(i=0; i< firmwareInfo->SlotCount;i++)
		{
			if (firmwareInfo->Slot[i].SlotNumber == firmwareInfo->ActiveSlot) 
			{
				RtlCopyMemory(m_FirmwareRevision, &firmwareInfo->Slot[i].Revision, 16);
				break;
			}
		}

		if (DisplayResult) {
			qDebug( ("\n\tSupport upgrade command: %s\n"), firmwareInfo->SupportUpgrade ?  ("Yes") :  ("No"));
			qDebug( ("\tSlot Count: %d\n"), firmwareInfo->SlotCount);
			qDebug( ("\tCurrent Active Slot: %d\n"), firmwareInfo->ActiveSlot);

			if (firmwareInfo->PendingActivateSlot == STORAGE_FIRMWARE_INFO_INVALID_SLOT) {
				qDebug( ("\tPending Active Slot: %s\n"),  ("N/A"));
			}
			else {
				qDebug( ("\tPending Active Slot: %d\n"), firmwareInfo->PendingActivateSlot);
			}

			qDebug( ("\tFirmware applies to both controller and device: %s\n"), firmwareInfo->FirmwareShared ?  ("Yes") :  ("No"));
			qDebug( ("\tFirmware payload alignment: %d\n"), firmwareInfo->ImagePayloadAlignment);
			qDebug( ("\tFirmware payload max size: %d\n"), firmwareInfo->ImagePayloadMaxSize);

			for (i = 0; i < firmwareInfo->SlotCount; i++) {
				RtlZeroMemory(revision, sizeof(revision));

				MultiByteToWideChar(CP_ACP,
									0,
									(LPCCH)firmwareInfo->Slot[i].Revision,
									-1,
									revision,
									sizeof(revision) / sizeof(revision[0]) - 1
									);

				qDebug( ("\tSlot ID: %d\n"), firmwareInfo->Slot[i].SlotNumber);
				qDebug( ("\tSlot Read Only: %s\n"), firmwareInfo->Slot[i].ReadOnly ?  ("Yes") :  ("No"));
				qDebug( ("\tRevision: %s\n"), revision);
			}
		}

		result = TRUE;
	}

Exit:

	if (!result) {
		if (buffer != NULL) {
			free(buffer);
			buffer = NULL;
		}

		bufferLength = 0;
	}

	*Buffer = buffer;
	*BufferLength = bufferLength;

    CloseHandle(hIoCtrl);
	
	return result;
}

BOOL CFirmwareUpdate::DeviceValidateFirmwareUpgradeSupport(PSTORAGE_HW_FIRMWARE_INFO FirmwareInfo)
{
	
    BOOL    result = TRUE;
    UCHAR   slotId = STORAGE_HW_FIRMWARE_INVALID_SLOT;
    int     i;

    qDebug( ("Validate firmware upgrade support information.\n"));

    if (FirmwareInfo->SupportUpgrade == FALSE) 
	{
        qDebug( ("IOCTL_STORAGE_FIRMWARE_GET_INFO - The device does not support firmware upgrade.\n"));
        result = FALSE;
    } 
	else if (FirmwareInfo->SlotCount == 0) 
	{
        qDebug( ("IOCTL_STORAGE_FIRMWARE_GET_INFO - reported slot count is 0.\n"));
        result = FALSE;
    } 
	else if (FirmwareInfo->ImagePayloadAlignment == 0) 
	{
        qDebug( ("IOCTL_STORAGE_FIRMWARE_GET_INFO - reported ImagePayloadAlignment is 0.\n"));
        result = FALSE;
    } 
	else if (FirmwareInfo->ImagePayloadMaxSize == 0) 
	{
        qDebug( ("IOCTL_STORAGE_FIRMWARE_GET_INFO - reported ImagePayloadMaxSize is 0.\n"));
        result = FALSE;
    }

    /* Check if there is writable slot available.*/
    if (result) 
	{
        for (i = 0; i < FirmwareInfo->SlotCount; i++) 
		{
            if (FirmwareInfo->Slot[i].ReadOnly == FALSE) 
			{
                slotId = FirmwareInfo->Slot[i].SlotNumber;
                break;
            }
        }

        if (slotId == STORAGE_HW_FIRMWARE_INVALID_SLOT) 
		{
            qDebug( ("ERROR: IOCTL_STORAGE_FIRMWARE_GET_INFO - no writable firmware slot reported.\n"));
            result = FALSE;
        }
    }

    return result;

}

BOOL CFirmwareUpdate::DeviceStorageFirmwareWin10Download(INT physicalDriveId, QString FileName, qint64 transize)
{
    BOOL                    result = TRUE;

    PUCHAR                  infoBuffer = NULL;
    ULONG                   infoBufferLength = 0;
    PUCHAR                  buffer = NULL;
    ULONG                   bufferSize;
    ULONG                   imageBufferLength;

    PSTORAGE_HW_FIRMWARE_INFO       firmwareInfo = NULL;
    PSTORAGE_HW_FIRMWARE_DOWNLOAD   firmwareDownload = NULL;
    PSTORAGE_HW_FIRMWARE_ACTIVATE   firmwareActivate = NULL;

    UCHAR                   slotID;
    ULONG                   returnedLength;
    ULONG                   i;

    HANDLE                  fileHandle = NULL;
    ULONG                   imageOffset;
    ULONG                   readLength;
    BOOLEAN                 moreToDownload;

    ULONGLONG               tickCount1 = 0;
    ULONGLONG               tickCount2 = 0;


    HANDLE hIoCtrl = NULL;

    /* Get firmware information */
    result = DeviceGetStorageFirmwareWin10Info (physicalDriveId, &infoBuffer, &infoBufferLength, TRUE);

    if (result == FALSE)
    {
        qDebug(" Get firmeare information failed \n");
        if(infoBuffer!=NULL)
            free(infoBuffer);
        return FALSE;
    }

    /* Check firmware if it support firmware update */
    firmwareInfo = (PSTORAGE_HW_FIRMWARE_INFO)infoBuffer;
    result = DeviceValidateFirmwareUpgradeSupport(firmwareInfo);
    if (result == FALSE)
    {
        qDebug(" Device cannot support firmware updarte \n");
        if(infoBuffer!=NULL)
            free(infoBuffer);
        return FALSE;
    }

    /* The Max Transfer Length limits the part of buffer that may need to transfer to controller, not the whole buffer.*/
    bufferSize = FIELD_OFFSET(STORAGE_HW_FIRMWARE_DOWNLOAD, ImageBuffer);
    bufferSize += min(m_storageadptdecripter.MaximumTransferLength, firmwareInfo->ImagePayloadMaxSize);

    buffer = (PUCHAR)malloc(bufferSize);
    if (buffer == NULL)
    {
        qDebug("Allocate buffer for sending firmware image file failed. Error code: %d\n", GetLastError());
        if(infoBuffer!=NULL)
            free(infoBuffer);
        return FALSE;
    }

    /* Setup header of firmware download data structure.*/
    RtlZeroMemory(buffer, bufferSize);

    firmwareDownload = (PSTORAGE_HW_FIRMWARE_DOWNLOAD)buffer;
    firmwareDownload->Version = sizeof(STORAGE_HW_FIRMWARE_DOWNLOAD);
    firmwareDownload->Size = bufferSize;

    if (firmwareInfo->FirmwareShared)
    {
        firmwareDownload->Flags = STORAGE_HW_FIRMWARE_REQUEST_FLAG_CONTROLLER;
    }
    else
    {
        firmwareDownload->Flags = 0;
    }

    //firmwareDownload->Slot = slotID;

    /* Open image file and download it to controller.*/
    //imageBufferLength = bufferSize - FIELD_OFFSET(STORAGE_HW_FIRMWARE_DOWNLOAD, ImageBuffer);
    imageBufferLength = min(bufferSize - FIELD_OFFSET(STORAGE_HW_FIRMWARE_DOWNLOAD, ImageBuffer),static_cast<ULONG>(transize*1024));
    hIoCtrl = GetIoCtrlHandle(physicalDriveId);
    if(hIoCtrl == INVALID_HANDLE_VALUE)
    {
        qDebug ("firmwareupdate: create Io control handel failed ! \n");

        if(infoBuffer!=NULL)
            free(infoBuffer);
        if (buffer != NULL)
            free(buffer);
        return FALSE;
    }

    imageOffset = 0;
    readLength = 0;
    moreToDownload = TRUE;

#if 0
    TCHAR tFileName[1024] = {0};
#ifdef UNICODE
    _stprintf_s(tFileName,1024,_T("%S"),FileName.c_str());
#else
    _stprintf_s(tFileName,1024,_T("%s"),FileName.c_str());
#endif
#endif

    fileHandle = CreateFile(FileName.toStdWString().c_str(),   // file to open
                            GENERIC_READ,          // open for reading
                            FILE_SHARE_READ,       // share for reading
                            NULL,                  // default security
                            OPEN_EXISTING,         // existing file only
                            FILE_ATTRIBUTE_NORMAL, // normal file
                            NULL);                 // no attr. template

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        qDebug("Unable to open handle for firmware image file errocode %d\n", GetLastError());
        if(infoBuffer!=NULL)
            free(infoBuffer);
        if (buffer != NULL)
            free(buffer);
        return FALSE;
    }

    while (moreToDownload)
    {
        RtlZeroMemory(firmwareDownload->ImageBuffer, imageBufferLength);
        //if (ReadFile(fileHandle, firmwareDownload->ImageBuffer, imageBufferLength, &readLength, NULL) == FALSE)
        result = ReadFile(fileHandle, firmwareDownload->ImageBuffer, imageBufferLength, &readLength, NULL);
        if(result == FALSE)
        {
            qDebug("Read firmware file failed. Error code: %d\n", GetLastError());

            if (fileHandle != NULL) {
                CloseHandle(fileHandle);
            }

            if (infoBuffer != NULL) {
                free(infoBuffer);
            }

            if (buffer != NULL) {
                free(buffer);
            }

            return FALSE;
        }

        if (readLength == 0)
        {
            if (imageOffset == 0)
            {
                qDebug("Firmware image file read return length value 0. Error code: %d\n", GetLastError());
                if (fileHandle != NULL) {
                    CloseHandle(fileHandle);
                }

                if (infoBuffer != NULL) {
                    free(infoBuffer);
                }

                if (buffer != NULL) {
                    free(buffer);
                }

                return FALSE;
            }

            moreToDownload = FALSE;
            break;
        }

        firmwareDownload->Offset = imageOffset;

        if (readLength > 0)
        {
            firmwareDownload->BufferSize = ((readLength - 1) / firmwareInfo->ImagePayloadAlignment + 1) * firmwareInfo->ImagePayloadAlignment;
        }
        else
        {
            firmwareDownload->BufferSize = 0;
        }

        /* download this piece of firmware to device */
        result = DeviceIoControl(hIoCtrl,
                                 IOCTL_STORAGE_FIRMWARE_DOWNLOAD,
                                 buffer,
                                 bufferSize,
                                 buffer,
                                 bufferSize,
                                 &returnedLength,
                                 NULL
                                 );

        if (result == FALSE)
        {
            qDebug(("\t DeviceStorageFirmwareUpgrade - firmware download IOCTL failed. Error code: %d\n"), GetLastError());
                if (fileHandle != NULL) {
                    CloseHandle(fileHandle);
                }

                if (infoBuffer != NULL) {
                    free(infoBuffer);
                }

                if (buffer != NULL) {
                    free(buffer);
                }

                return FALSE;
        }

        /* Update Image Offset for next loop.*/
        imageOffset += (ULONG)firmwareDownload->BufferSize;
    }
    return result;
}

BOOL CFirmwareUpdate::DeviceStorageFirmwareWin10Upgrade(INT physicalDriveId, INT slotID, ULONG action)
{
    BOOL                    result = TRUE;

    PUCHAR                  infoBuffer = NULL;
    ULONG                   infoBufferLength = 0;
    PUCHAR                  buffer = NULL;
    ULONG                   bufferSize;

    PSTORAGE_HW_FIRMWARE_INFO       firmwareInfo = NULL;
    PSTORAGE_HW_FIRMWARE_ACTIVATE   firmwareActivate = NULL;

    ULONG                   returnedLength;

    HANDLE                  fileHandle = NULL;

    ULONGLONG               tickCount1 = 0;
    ULONGLONG               tickCount2 = 0;


    HANDLE hIoCtrl = NULL;
    hIoCtrl = GetIoCtrlHandle(physicalDriveId);
    DeviceGetStorageFirmwareWin10Info (physicalDriveId, &infoBuffer, &infoBufferLength, TRUE);
    firmwareInfo = (PSTORAGE_HW_FIRMWARE_INFO)infoBuffer;


    /* The Max Transfer Length limits the part of buffer that may need to transfer to controller, not the whole buffer.*/
    bufferSize = FIELD_OFFSET(STORAGE_HW_FIRMWARE_DOWNLOAD, ImageBuffer);
    bufferSize += min(m_storageadptdecripter.MaximumTransferLength, firmwareInfo->ImagePayloadMaxSize);

    /* Activate the newly downloaded image.*/
    qDebug(("Activate the new firmware image using IOCTL_STORAGE_FIRMWARE_ACTIVATE.\n"));

    buffer = (PUCHAR)malloc(bufferSize);
    RtlZeroMemory(buffer, bufferSize);

    firmwareActivate = (PSTORAGE_HW_FIRMWARE_ACTIVATE)buffer;
    firmwareActivate->Version = sizeof(STORAGE_HW_FIRMWARE_ACTIVATE);
    firmwareActivate->Size = sizeof(STORAGE_HW_FIRMWARE_ACTIVATE);
    firmwareActivate->Slot = (uchar)slotID;

    if (firmwareInfo->FirmwareShared)
    {
        firmwareActivate->Flags = action;
    }
    else
    {
        firmwareActivate->Flags = 0;
    }

    tickCount1 = GetTickCount();

    /* activate firmware */
    result = DeviceIoControl(hIoCtrl,IOCTL_STORAGE_FIRMWARE_ACTIVATE,
                        buffer,bufferSize,buffer, bufferSize,&returnedLength,NULL);

    tickCount2 = GetTickCount();


    ULONG   seconds = (ULONG)((tickCount2 - tickCount1) / 1000);
    ULONG   milliseconds = (ULONG)((tickCount2 - tickCount1) % 1000);

    if (seconds < 5)
    {
        qDebug( ("\n\tFirmware activation process took %d.%d seconds.\n"), seconds, milliseconds);
    }
    else
    {
        qDebug( ("\n\tFirmware activation process took %d.%d seconds.\n"), seconds, milliseconds);
    }

    if (result)
    {
        qDebug( ("\tNew firmware has been successfully applied to device.\n"));
    }
    else if (GetLastError() == STG_S_POWER_CYCLE_REQUIRED)
    {
        qDebug( ("\tWarning: Upgrade completed. Power cycle is required to activate the new firmware.\n"));

    }
    else
    {
        qDebug( ("\tFirmware activate IOCTL failed. Error code: %d\n"), GetLastError());
        if (fileHandle != NULL) {
            CloseHandle(fileHandle);
        }

        if (infoBuffer != NULL) {
            free(infoBuffer);
        }
        if (buffer != NULL) {
            free(buffer);
        }
        return FALSE;
    }

    CloseHandle(hIoCtrl);

    if (fileHandle != NULL) {
        CloseHandle(fileHandle);
    }

    if (infoBuffer != NULL) {
        free(infoBuffer);
    }

    if (buffer != NULL) {
        free(buffer);
    }

    return result;
}

BOOL CFirmwareUpdate::DeviceStorageFirmwareWin10Upgrade1(INT physicalDriveId, QString FileName,BOOL VerboseDisplay)
{

    BOOL                    result = TRUE;

    PUCHAR                  infoBuffer = NULL;
    ULONG                   infoBufferLength = 0;
    PUCHAR                  buffer = NULL;
    ULONG                   bufferSize;
    ULONG                   imageBufferLength;

    PSTORAGE_HW_FIRMWARE_INFO       firmwareInfo = NULL;
    PSTORAGE_HW_FIRMWARE_DOWNLOAD   firmwareDownload = NULL;
    PSTORAGE_HW_FIRMWARE_ACTIVATE   firmwareActivate = NULL;

    UCHAR                   slotID;
    ULONG                   returnedLength;
    ULONG                   i;

    HANDLE                  fileHandle = NULL;
    ULONG                   imageOffset;
    ULONG                   readLength;
    BOOLEAN                 moreToDownload;

    ULONGLONG               tickCount1 = 0;
    ULONGLONG               tickCount2 = 0;


	HANDLE hIoCtrl = NULL;

    /* Get firmware information */
    result = DeviceGetStorageFirmwareWin10Info (physicalDriveId, &infoBuffer, &infoBufferLength, VerboseDisplay);

    if (result == FALSE) 
	{
		qDebug(" Get firmeare information failed \n");
		if(infoBuffer!=NULL)
			free(infoBuffer);
		return FALSE;
    }

	/* Check firmware if it support firmware update */
	firmwareInfo = (PSTORAGE_HW_FIRMWARE_INFO)infoBuffer;
	result = DeviceValidateFirmwareUpgradeSupport(firmwareInfo);	
	if (result == FALSE) 
	{
		qDebug(" Device cannot support firmware updarte \n");
		if(infoBuffer!=NULL)
			free(infoBuffer);
		return FALSE;
	}

    /* Find the first writable slot */
    slotID = STORAGE_HW_FIRMWARE_INVALID_SLOT;
    for (i = 0; i < firmwareInfo->SlotCount; i++) 
	{
		if (firmwareInfo->Slot[i].ReadOnly == FALSE) 
		{
		    slotID = firmwareInfo->Slot[i].SlotNumber;
		    break;
		}
    }

    /* The Max Transfer Length limits the part of buffer that may need to transfer to controller, not the whole buffer.*/
    bufferSize = FIELD_OFFSET(STORAGE_HW_FIRMWARE_DOWNLOAD, ImageBuffer);
    bufferSize += min(m_storageadptdecripter.MaximumTransferLength, firmwareInfo->ImagePayloadMaxSize);

    buffer = (PUCHAR)malloc(bufferSize);
    if (buffer == NULL) 
	{
        qDebug("Allocate buffer for sending firmware image file failed. Error code: %d\n", GetLastError());
		if(infoBuffer!=NULL)
			free(infoBuffer);
		return FALSE;
    }

    /* Setup header of firmware download data structure.*/
    RtlZeroMemory(buffer, bufferSize);

    firmwareDownload = (PSTORAGE_HW_FIRMWARE_DOWNLOAD)buffer;
    firmwareDownload->Version = sizeof(STORAGE_HW_FIRMWARE_DOWNLOAD);
    firmwareDownload->Size = bufferSize;

    if (firmwareInfo->FirmwareShared) 
	{
        firmwareDownload->Flags = STORAGE_HW_FIRMWARE_REQUEST_FLAG_CONTROLLER;
    }
    else
	{
        firmwareDownload->Flags = 0;
    }

    firmwareDownload->Slot = slotID;

    /* Open image file and download it to controller.*/
    imageBufferLength = bufferSize - FIELD_OFFSET(STORAGE_HW_FIRMWARE_DOWNLOAD, ImageBuffer);

    hIoCtrl = GetIoCtrlHandle(physicalDriveId);
    if(hIoCtrl == INVALID_HANDLE_VALUE)
    {
		qDebug ("firmwareupdate: create Io control handel failed ! \n");
		
		if(infoBuffer!=NULL)
			free(infoBuffer);
		if (buffer != NULL) 
        	free(buffer);
		return FALSE;
	}

    imageOffset = 0;
    readLength = 0;
    moreToDownload = TRUE;

#if 0
	TCHAR tFileName[1024] = {0};
#ifdef UNICODE
	_stprintf_s(tFileName,1024,_T("%S"),FileName.c_str());
#else
	_stprintf_s(tFileName,1024,_T("%s"),FileName.c_str());
#endif
#endif

    fileHandle = CreateFile(FileName.toStdWString().c_str(),   // file to open
                            GENERIC_READ,          // open for reading
                            FILE_SHARE_READ,       // share for reading
                            NULL,                  // default security
                            OPEN_EXISTING,         // existing file only
                            FILE_ATTRIBUTE_NORMAL, // normal file
                            NULL);                 // no attr. template

    if (fileHandle == INVALID_HANDLE_VALUE) 
	{
        qDebug("Unable to open handle for firmware image file errocode %d\n", GetLastError());
		if(infoBuffer!=NULL)
			free(infoBuffer);
		if (buffer != NULL) 
        	free(buffer);
		return FALSE;
    }

    while (moreToDownload) 
	{
        RtlZeroMemory(firmwareDownload->ImageBuffer, imageBufferLength);
        if (ReadFile(fileHandle, firmwareDownload->ImageBuffer, imageBufferLength, &readLength, NULL) == FALSE) 
		result = ReadFile(fileHandle, firmwareDownload->ImageBuffer, imageBufferLength, &readLength, NULL);
		if(result == FALSE)
		{
            qDebug("Read firmware file failed. Error code: %d\n", GetLastError());

			if (fileHandle != NULL) {
				CloseHandle(fileHandle);
			}
		
			if (infoBuffer != NULL) {
				free(infoBuffer);
			}
		
			if (buffer != NULL) {
				free(buffer);
			}

			return FALSE;
		}

        if (readLength == 0) 
		{
            if (imageOffset == 0) 
			{
                qDebug("Firmware image file read return length value 0. Error code: %d\n", GetLastError());
				if (fileHandle != NULL) {
					CloseHandle(fileHandle);
				}
				
				if (infoBuffer != NULL) {
					free(infoBuffer);
				}
				
				if (buffer != NULL) {
					free(buffer);
				}

				return FALSE;
            }

            moreToDownload = FALSE;
            break;
        }

        firmwareDownload->Offset = imageOffset;

        if (readLength > 0) 
		{
            firmwareDownload->BufferSize = ((readLength - 1) / firmwareInfo->ImagePayloadAlignment + 1) * firmwareInfo->ImagePayloadAlignment;
        } 
		else 
		{
            firmwareDownload->BufferSize = 0;
        }

        /* download this piece of firmware to device */
        result = DeviceIoControl(hIoCtrl,
                                 IOCTL_STORAGE_FIRMWARE_DOWNLOAD,
                                 buffer,
                                 bufferSize,
                                 buffer,
                                 bufferSize,
                                 &returnedLength,
                                 NULL
                                 );

        if (result == FALSE) 
		{
            qDebug(("\t DeviceStorageFirmwareUpgrade - firmware download IOCTL failed. Error code: %d\n"), GetLastError());
            	if (fileHandle != NULL) {
					CloseHandle(fileHandle);
				}
				
				if (infoBuffer != NULL) {
					free(infoBuffer);
				}
				
				if (buffer != NULL) {
					free(buffer);
				}

				return FALSE;
        }

        /* Update Image Offset for next loop.*/
        imageOffset += (ULONG)firmwareDownload->BufferSize;
    }
    

    /* Activate the newly downloaded image.*/
    qDebug(("Activate the new firmware image using IOCTL_STORAGE_FIRMWARE_ACTIVATE.\n"));

    RtlZeroMemory(buffer, bufferSize);

    firmwareActivate = (PSTORAGE_HW_FIRMWARE_ACTIVATE)buffer;
    firmwareActivate->Version = sizeof(STORAGE_HW_FIRMWARE_ACTIVATE);
    firmwareActivate->Size = sizeof(STORAGE_HW_FIRMWARE_ACTIVATE);
    firmwareActivate->Slot = slotID;

    if (firmwareInfo->FirmwareShared) 
	{
        firmwareActivate->Flags = STORAGE_HW_FIRMWARE_REQUEST_FLAG_CONTROLLER;
    }
    else 
	{
        firmwareActivate->Flags = 0;
    }

    // If firmware image is not specified, the intension is to activate the existing firmware in slot.
    // if (CommandOptions->Parameters.FileName == NULL) 
	//{
        //firmwareActivate->Flags |= STORAGE_HW_FIRMWARE_REQUEST_FLAG_SWITCH_TO_EXISTING_FIRMWARE;
    //}

    tickCount1 = GetTickCount();

    /* activate firmware */
    result = DeviceIoControl(hIoCtrl,IOCTL_STORAGE_FIRMWARE_ACTIVATE,
    					buffer,bufferSize,buffer, bufferSize,&returnedLength,NULL);

    tickCount2 = GetTickCount();


    ULONG   seconds = (ULONG)((tickCount2 - tickCount1) / 1000);
    ULONG   milliseconds = (ULONG)((tickCount2 - tickCount1) % 1000);

    if (seconds < 5) 
	{
        qDebug( ("\n\tFirmware activation process took %d.%d seconds.\n"), seconds, milliseconds);
    } 
	else 
	{
        qDebug( ("\n\tFirmware activation process took %d.%d seconds.\n"), seconds, milliseconds);
    }

    if (result) 
	{
        qDebug( ("\tNew firmware has been successfully applied to device.\n"));
    } 
	else if (GetLastError() == STG_S_POWER_CYCLE_REQUIRED) 
	{
        qDebug( ("\tWarning: Upgrade completed. Power cycle is required to activate the new firmware.\n"));
		
    } 
	else
	{
        qDebug( ("\tFirmware activate IOCTL failed. Error code: %d\n"), GetLastError());
        if (fileHandle != NULL) {
			CloseHandle(fileHandle);
		}
				
		if (infoBuffer != NULL) {
			free(infoBuffer);
		}
		if (buffer != NULL) {
			free(buffer);
		}
		return FALSE;
    }

	CloseHandle(hIoCtrl);
	
    /* Validate the new firmware revision being different from old one.*/
    if (result) 
	{
        UCHAR   firmwareRevision[17] = {0};
        RtlCopyMemory(firmwareRevision, m_FirmwareRevision, 17);

        /* Get new firmware revision.*/
        if (infoBuffer != NULL) 
		{
            free(infoBuffer);
            infoBuffer = NULL;
        }

        /* Wait 2 seconds before retrieving disk information so that system has time to get it ready.*/
        Sleep(2000);
        
        result = DeviceGetStorageFirmwareWin10Info(physicalDriveId, &infoBuffer, &infoBufferLength, TRUE);

        if (result) 
		{
            WCHAR revision[32] = {0};
            MultiByteToWideChar(CP_ACP,0,
                                (LPCCH)firmwareInfo->Slot[i].Revision,-1,
                                revision,sizeof(revision) / sizeof(revision[0]) - 1);

            qDebug("\n\tNew Firmware Revision - %s\n", revision);
            //if (RtlCompareMemory(firmwareRevision, m_FirmwareRevision, 17) == 17)
			{
                //qDebug("\tWarning: New Firmware Revision is same as old one - %s\n", revision);
            }
        } 
		else 
        {
            qDebug( ("\n\tFailed to retrieve new Firmware Revision. Error code: %d\n"), GetLastError());
        }
    }


    if (fileHandle != NULL) {
        CloseHandle(fileHandle);
    }

    if (infoBuffer != NULL) {
        free(infoBuffer);
    }

    if (buffer != NULL) {
        free(buffer);
    }

	return TRUE;

}




#endif





#if 0


BOOL CFirmwareUpdate::DeviceGetFirmwareWin10Info(INT physicalDriveId,PUCHAR Buffer,DWORD BufferLength,BOOL DisplayResult)   // win10 NVMe
{
	BOOL	result;
	ULONG	returnedLength;
	ULONG	firmwareInfoOffset;
	TCHAR buffer[64];
	HANDLE hIoCtrl = NULL;

	PSRB_IO_CONTROL 		srbControl;
	PFIRMWARE_REQUEST_BLOCK firmwareRequest;
	PSTORAGE_FIRMWARE_INFO	firmwareInfo;

    wsprintf(buffer, TEXT("\\\\.\\PhysicalDrive%d"), physicalDriveId);
    hIoCtrl = CreateFile(buffer,	GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
	
    if(hIoCtrl == INVALID_HANDLE_VALUE)
    {
		qDebug (" firmwareupdate: create Io control handel failed ! \n");
		return FALSE;
	}

	srbControl = (PSRB_IO_CONTROL)Buffer;
	firmwareRequest = (PFIRMWARE_REQUEST_BLOCK)(srbControl + 1);

	//
	// The STORAGE_FIRMWARE_INFO is located after SRB_IO_CONTROL and FIRMWARE_RESQUEST_BLOCK
	//
	firmwareInfoOffset = ((sizeof(SRB_IO_CONTROL) + sizeof(FIRMWARE_REQUEST_BLOCK) - 1) / sizeof(PVOID) + 1) * sizeof(PVOID);

	//
	// Setup the SRB control with the firmware ioctl control info
	//
	srbControl->HeaderLength = sizeof(SRB_IO_CONTROL);
	srbControl->ControlCode = IOCTL_SCSI_MINIPORT_FIRMWARE;
	RtlMoveMemory(srbControl->Signature, IOCTL_MINIPORT_SIGNATURE_FIRMWARE, 8);
	srbControl->Timeout = 30;
	srbControl->Length = BufferLength - sizeof(SRB_IO_CONTROL);

	//
	// Set firmware request fields for FIRMWARE_FUNCTION_GET_INFO. This request is to the controller so
	// FIRMWARE_REQUEST_FLAG_CONTROLLER is set in the flags
	//
	firmwareRequest->Version = FIRMWARE_REQUEST_BLOCK_STRUCTURE_VERSION;
	firmwareRequest->Size = sizeof(FIRMWARE_REQUEST_BLOCK);
	firmwareRequest->Function = FIRMWARE_FUNCTION_GET_INFO;
	firmwareRequest->Flags = FIRMWARE_REQUEST_FLAG_CONTROLLER;
	firmwareRequest->DataBufferOffset = firmwareInfoOffset;
	firmwareRequest->DataBufferLength = BufferLength - firmwareInfoOffset;

	//
	// Send the request to get the device firmware info
	//
	result = DeviceIoControl(hIoCtrl,IOCTL_SCSI_MINIPORT,
							Buffer, BufferLength,Buffer,BufferLength,&returnedLength, NULL);

	//
	// Format and display the firmware info
	//
	if (DisplayResult) 
	{
		if (!result) 
		{
			qDebug("\t Get Firmware Information Failed: 0x%X\n", GetLastError());
			CloseHandle(hIoCtrl);
			return FALSE;
		} 
		else 
		{
			UCHAR	i;
			TCHAR	revision[16] = {0};

			firmwareInfo = (PSTORAGE_FIRMWARE_INFO)((PUCHAR)srbControl + firmwareRequest->DataBufferOffset);

			qDebug(("\t ----Firmware Information----\n"));
			qDebug(("\t Support upgrade command: %s\n"), firmwareInfo->UpgradeSupport ?  ("Yes") :  ("No"));
			qDebug(("\t Slot Count: %d\n"), firmwareInfo->SlotCount);
			qDebug(("\t Current Active Slot: %d\n"), firmwareInfo->ActiveSlot);

			if (firmwareInfo->PendingActivateSlot == STORAGE_FIRMWARE_INFO_INVALID_SLOT) {
				qDebug(("\t Pending Active Slot: %s\n\n"),	 ("No"));
			} else {
				qDebug(("\t Pending Active Slot: %d\n\n"), firmwareInfo->PendingActivateSlot);
			}

			for (i = 0; i < firmwareInfo->SlotCount; i++) {
				RtlCopyMemory(revision, &firmwareInfo->Slot[i].Revision.AsUlonglong, 8);

				qDebug(("\t\t Slot Number: %d\n"), firmwareInfo->Slot[i].SlotNumber);
				qDebug(("\t\t Slot Read Only: %s\n"), firmwareInfo->Slot[i].ReadOnly ?  ("Yes") :  ("No"));
				qDebug(("\t\t Revision: %s\n"), revision);
				qDebug(("\n"));
			}
		}
	}

	CloseHandle(hIoCtrl);
	return result;
}


void CFirmwareUpdate::DeviceFirmwareWin10Upgrade(INT physicalDriveId,string FileName, UCHAR *pFwslot)
{
    BOOL                    result;
    PUCHAR                  buffer = NULL;
    ULONG                   bufferSize;
    ULONG                   firmwareStructureOffset;
    ULONG                   imageBufferLength;

    PSRB_IO_CONTROL         srbControl;
    PFIRMWARE_REQUEST_BLOCK firmwareRequest;

    PSTORAGE_FIRMWARE_INFO      firmwareInfo;
    PSTORAGE_FIRMWARE_DOWNLOAD  firmwareDownload;
    PSTORAGE_FIRMWARE_ACTIVATE  firmwareActivate;

    ULONG                   slotNumber;
    ULONG                   returnedLength;
    ULONG                   i;

    HANDLE                  fileHandle = NULL;
    ULONG                   imageOffset;
    ULONG                   readLength;
    BOOLEAN                 moreToDownload;

	HANDLE hIoCtrl = NULL;
	TCHAR tbuffer[64];

    // The STORAGE_FIRMWARE_INFO is located after SRB_IO_CONTROL and FIRMWARE_RESQUEST_BLOCK
    firmwareStructureOffset = ((sizeof(SRB_IO_CONTROL) + sizeof(FIRMWARE_REQUEST_BLOCK) - 1) / sizeof(PVOID) + 1) * sizeof(PVOID);

    //
    // The Max Transfer Length limits the part of buffer that may need to transfer to controller, not the whole buffer.
    //
    //bufferSize = min(DeviceList[Index].AdapterDescriptor.MaximumTransferLength, 2 * 1024 * 1024);
    bufferSize = min(m_storageadptdecripter.MaximumTransferLength, 2 * 1024 * 1024);
    bufferSize += firmwareStructureOffset;
    bufferSize += FIELD_OFFSET(STORAGE_FIRMWARE_DOWNLOAD, ImageBuffer);

    buffer = (PUCHAR)malloc(bufferSize);
    if (buffer == NULL) 
	{
        qDebug("\t FirmwareUpgrade - Allocate buffer failed: 0x%X\n", GetLastError());
        return;
    }

    //
    // calculate the space available for the firmware image portion of the buffer allocation
    // 
    imageBufferLength = bufferSize - firmwareStructureOffset - sizeof(STORAGE_FIRMWARE_DOWNLOAD);

    RtlZeroMemory(buffer, bufferSize);

    // ---------------------------------------------------------------------------
    // ( 1 ) SELECT A SUITABLE FIRMWARE SLOT
    // ---------------------------------------------------------------------------

    //
    // Get firmware slot information data.
    //
    result = DeviceGetFirmwareWin10Info(physicalDriveId, buffer, bufferSize, TRUE);

    if (result == FALSE) 
	{
        qDebug("\t FirmwareUpgrade: Get Firmware Information Failed: 0x%X\n", GetLastError());
        goto Exit;
    }

    qDebug("\t FirmwareUpgrade: Get Firmware Information success \n");

    //
    // Set the request structure pointers
                //
    srbControl = (PSRB_IO_CONTROL)buffer;
    firmwareRequest = (PFIRMWARE_REQUEST_BLOCK)(srbControl + 1);
    firmwareInfo = (PSTORAGE_FIRMWARE_INFO)((PUCHAR)srbControl + firmwareRequest->DataBufferOffset);

    if (srbControl->ReturnCode != FIRMWARE_STATUS_SUCCESS) 
	{
        qDebug("\t FirmwareUpgrade - get firmware info failed. srbControl->ReturnCode %d.\n", srbControl->ReturnCode);
        goto Exit;
    }

    //
    // SelectFind the first writable slot.
    //
    slotNumber = (ULONG)-1;

    if (firmwareInfo->UpgradeSupport) 
	{
        for (i = 0; i < firmwareInfo->SlotCount; i++) 
		{
            if (firmwareInfo->Slot[i].ReadOnly == FALSE) 
			{
                slotNumber = firmwareInfo->Slot[i].SlotNumber;
                break;
            }
        }
    }

    
    // If no writable slot is found, bypass downloading and activation
    //
    if (slotNumber == (ULONG)-1) 
	{
        qDebug( ("\t FirmwareUpgrade - No writable Firmware slot.\n"));
        goto Exit;
    }

    *pFwslot = (UCHAR)slotNumber;
    // ---------------------------------------------------------------------------
    // ( 2 ) DOWNLOAD THE FIRMWARE IMAGE TO THE CONTROLLER
    // ---------------------------------------------------------------------------

    //
    // initialize image length and offset
    //
    imageBufferLength = (imageBufferLength / sizeof(PVOID)) * sizeof(PVOID);
    imageOffset = 0;
    readLength = 0;
    moreToDownload = TRUE;

    //
    // Open image file and download it to controller.
    //
    //if (FileName == NULL)
    //{
       // qDebug( ("\t FirmwareUpgrade - No firmware file specified.\n"));
        //return FALSE;
    //}

    fileHandle = CreateFile(FileName.c_str(),              // file to open
                            GENERIC_READ,          // open for reading
                            FILE_SHARE_READ,       // share for reading
                            NULL,                  // default security
                            OPEN_EXISTING,         // existing file only
                            FILE_ATTRIBUTE_NORMAL, // normal file
                            NULL);                 // no attr. template

    if (fileHandle == INVALID_HANDLE_VALUE) 
	{
        qDebug("\t FirmwareUpgrade - unable to open file \"%s\" for read.\n", FileName.c_str());
        goto Exit;
    }

	
    wsprintf(tbuffer, TEXT("\\\\.\\PhysicalDrive%d"), physicalDriveId);
    hIoCtrl = CreateFile(tbuffer,	GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
	
    if(hIoCtrl == INVALID_HANDLE_VALUE)
    {
		qDebug (" firmwareupdate: create Io control handel failed ! \n");
        goto Exit;
	}

    qDebug("\t FirmwareUpgrade: starting download firmware \n");

    //
    // Read and download the firmware from the image file into image buffer length portions. Send the
    // image portion to the controller.
    //
    while (moreToDownload) 
	{
        qDebug("\t FirmwareUpgrade: starting download firmware process %d \n",__LINE__);

        RtlZeroMemory(buffer, bufferSize);

		srbControl = (PSRB_IO_CONTROL)buffer;
		firmwareRequest = (PFIRMWARE_REQUEST_BLOCK)(srbControl + 1);
        //
        // Setup the SRB control with the firmware ioctl control info
        //
        srbControl->HeaderLength = sizeof(SRB_IO_CONTROL);
        srbControl->ControlCode = IOCTL_SCSI_MINIPORT_FIRMWARE;
        RtlMoveMemory(srbControl->Signature, IOCTL_MINIPORT_SIGNATURE_FIRMWARE, 8);
		srbControl->Timeout = 300;
        srbControl->Length = bufferSize - sizeof(SRB_IO_CONTROL);

        //
        // Set firmware request fields for FIRMWARE_FUNCTION_DOWNLOAD. This request is to the controller so
        // FIRMWARE_REQUEST_FLAG_CONTROLLER is set in the flags
        //
        firmwareRequest->Version = FIRMWARE_REQUEST_BLOCK_STRUCTURE_VERSION;
        firmwareRequest->Size = sizeof(FIRMWARE_REQUEST_BLOCK);
        firmwareRequest->Function = FIRMWARE_FUNCTION_DOWNLOAD;
        firmwareRequest->Flags = FIRMWARE_REQUEST_FLAG_CONTROLLER;
        firmwareRequest->DataBufferOffset = firmwareStructureOffset;
        firmwareRequest->DataBufferLength = bufferSize - firmwareStructureOffset;

        //
        // Initialize the firmware data buffer pointer to the proper position after the request structure
        //
        firmwareDownload = (PSTORAGE_FIRMWARE_DOWNLOAD)((PUCHAR)srbControl + firmwareRequest->DataBufferOffset);

        if (ReadFile(fileHandle, firmwareDownload->ImageBuffer, imageBufferLength, &readLength, NULL) == FALSE) 
		{
            qDebug("\t FirmwareUpgrade - Read firmware file failed.\n");
            goto Exit;
        }

        qDebug("\t FirmwareUpgrade: starting download firmware process %d \n",__LINE__);

        if (readLength == 0) 
		{
            moreToDownload = FALSE;
            break;
        }

        qDebug("\t FirmwareUpgrade: starting download firmware process %d \n",__LINE__);
        if ((readLength % sizeof(ULONG)) != 0) 
		{
            qDebug( ("\t FirmwareUpgrade - Read firmware file failed.\n"));
        }

        //
        // Set the download parameters and adjust the offset for this portion of the firmware image
        //
        firmwareDownload->Version = 1;
        firmwareDownload->Size = sizeof(STORAGE_FIRMWARE_DOWNLOAD);
        firmwareDownload->Offset = imageOffset;
        firmwareDownload->BufferSize = readLength;
		
        qDebug("\t FirmwareUpgrade: starting download firmware process %d \n",__LINE__);

        //
        // download this portion of firmware to the device
        //
        result = DeviceIoControl(hIoCtrl,IOCTL_SCSI_MINIPORT,
                                 		buffer,bufferSize,buffer,bufferSize,&returnedLength,NULL);
		

        qDebug("\t FirmwareUpgrade: starting download firmware process %d \n",__LINE__);
        if (result == FALSE) 
		{
           qDebug("\t FirmwareUpgrade - IOCTL - firmware download failed. 0x%X.\n", GetLastError());
           goto Exit;
        }

        if (srbControl->ReturnCode != FIRMWARE_STATUS_SUCCESS) 
		{
            qDebug("\t FirmwareUpgrade - firmware download failed. srbControl->ReturnCode %d.\n", srbControl->ReturnCode);
            goto Exit;
        }

        qDebug("\t FirmwareUpgrade: starting download firmware process %d \n",__LINE__);
        //
        // Update Image Offset for next iteration.
        //
        imageOffset += readLength;
    }


	qDebug (" firmwareupdate: download firmware success ! \n");

    CloseHandle(hIoCtrl);

Exit:
    if (fileHandle != NULL) {
        CloseHandle(fileHandle);
    }

    if (buffer != NULL) {
        free(buffer);
    }

    return;

	

}

void CFirmwareUpdate::DeviceFirmwareWin10Active(INT physicalDriveId,UCHAR Fwslot)
{
    BOOL                    result;
    PUCHAR                  buffer = NULL;
    ULONG                   bufferSize;
    ULONG                   firmwareStructureOffset;
    ULONG                   imageBufferLength;

    PSRB_IO_CONTROL         srbControl;
    PFIRMWARE_REQUEST_BLOCK firmwareRequest;

    PSTORAGE_FIRMWARE_INFO      firmwareInfo;
    PSTORAGE_FIRMWARE_DOWNLOAD  firmwareDownload;
    PSTORAGE_FIRMWARE_ACTIVATE  firmwareActivate;

    ULONG                   slotNumber;
    ULONG                   returnedLength;
    ULONG                   i;

    HANDLE                  fileHandle = NULL;
    ULONG                   imageOffset;
    ULONG                   readLength;
    BOOLEAN                 moreToDownload;

	HANDLE hIoCtrl = NULL;
	TCHAR tbuffer[64];

    // ---------------------------------------------------------------------------
    // ( 3 ) ACTIVATE THE FIRMWARE SLOT ASSIGNED TO THE UPGRADE
    // ---------------------------------------------------------------------------

    //
    // Activate the newly downloaded image with the assigned slot.
    //

	// The STORAGE_FIRMWARE_INFO is located after SRB_IO_CONTROL and FIRMWARE_RESQUEST_BLOCK
    firmwareStructureOffset = ((sizeof(SRB_IO_CONTROL) + sizeof(FIRMWARE_REQUEST_BLOCK) - 1) / sizeof(PVOID) + 1) * sizeof(PVOID);

    //
    // The Max Transfer Length limits the part of buffer that may need to transfer to controller, not the whole buffer.
    //
    //bufferSize = min(DeviceList[Index].AdapterDescriptor.MaximumTransferLength, 2 * 1024 * 1024);
    bufferSize = min(m_storageadptdecripter.MaximumTransferLength, 2 * 1024 * 1024);
    bufferSize += firmwareStructureOffset;
    //bufferSize += FIELD_OFFSET(STORAGE_FIRMWARE_DOWNLOAD, ImageBuffer);
	bufferSize += FIELD_OFFSET(STORAGE_FIRMWARE_ACTIVATE, Reserved0);


    buffer = (PUCHAR)malloc(bufferSize);
	if(buffer ==NULL)
	{
		return ;
	}
    RtlZeroMemory(buffer, bufferSize);

	srbControl = (PSRB_IO_CONTROL)buffer;
	firmwareRequest = (PFIRMWARE_REQUEST_BLOCK)(srbControl + 1);

    //
    // Setup the SRB control with the firmware ioctl control info
    //
    srbControl->HeaderLength = sizeof(SRB_IO_CONTROL);
    srbControl->ControlCode = IOCTL_SCSI_MINIPORT_FIRMWARE;
    RtlMoveMemory(srbControl->Signature, IOCTL_MINIPORT_SIGNATURE_FIRMWARE, 8);
    srbControl->Timeout = 30;
    srbControl->Length = bufferSize - sizeof(SRB_IO_CONTROL);

    //
    // Set firmware request fields for FIRMWARE_FUNCTION_ACTIVATE. This request is to the controller so
    // FIRMWARE_REQUEST_FLAG_CONTROLLER is set in the flags
    //
    firmwareRequest->Version = FIRMWARE_REQUEST_BLOCK_STRUCTURE_VERSION;
    firmwareRequest->Size = sizeof(FIRMWARE_REQUEST_BLOCK);
    firmwareRequest->Function = FIRMWARE_FUNCTION_ACTIVATE;
    firmwareRequest->Flags = FIRMWARE_REQUEST_FLAG_CONTROLLER;
    firmwareRequest->DataBufferOffset = firmwareStructureOffset;
    firmwareRequest->DataBufferLength = bufferSize - firmwareStructureOffset;

    //
    // Initialize the firmware activation structure pointer to the proper position after the request structure
    //
    firmwareActivate = (PSTORAGE_FIRMWARE_ACTIVATE)((PUCHAR)srbControl + firmwareRequest->DataBufferOffset);

    //
    // Set the activation parameters with the available slot selected
    //
    firmwareActivate->Version = 2;
    firmwareActivate->Size = sizeof(STORAGE_FIRMWARE_ACTIVATE);
    firmwareActivate->SlotToActivate = Fwslot;


    //wsprintf(tbuffer, TEXT("\\\\.\\PhysicalDrive%d"), physicalDriveId);
    //hIoCtrl = CreateFile(tbuffer,	GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           //NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);

	hIoCtrl = GetIoCtrlHandle(physicalDriveId);
    if(hIoCtrl == INVALID_HANDLE_VALUE)
    {
		qDebug (" firmwareupdate: create Io control handel failed ! \n");
        goto Exit;
	}

    //
    // Send the activation request
    //
    result = DeviceIoControl(hIoCtrl, IOCTL_SCSI_MINIPORT,
    								buffer,bufferSize,buffer,bufferSize,&returnedLength,NULL);


    if (result == FALSE) 
	{
        qDebug(("\t FirmwareUpgrade - IOCTL - firmware activate failed. error %d., recode %d\n"), GetLastError(),srbControl->ReturnCode);
        goto Exit;
    }

    //
    // Display status result from firmware activation
    //
    switch (srbControl->ReturnCode) 
	{
    case FIRMWARE_STATUS_SUCCESS:
        qDebug(("\t FirmwareUpgrade - firmware activate succeeded.\n"));
        break;

    case FIRMWARE_STATUS_POWER_CYCLE_REQUIRED:
        qDebug(("\t FirmwareUpgrade - firmware activate succeeded. PLEASE REBOOT COMPUTER.\n"));
        break;

    case FIRMWARE_STATUS_ILLEGAL_REQUEST:
    case FIRMWARE_STATUS_INVALID_PARAMETER:
    case FIRMWARE_STATUS_INPUT_BUFFER_TOO_BIG:
        qDebug(("\t FirmwareUpgrade - firmware activate parameter error. srbControl->ReturnCode %d.\n"), srbControl->ReturnCode);
        break;

    case FIRMWARE_STATUS_INVALID_SLOT:
        qDebug(("\t FirmwareUpgrade - firmware activate, slot number invalid.\n"));
        break;

    case FIRMWARE_STATUS_INVALID_IMAGE:
        qDebug(("\t FirmwareUpgrade - firmware activate, invalid firmware image.\n"));
        break;

    case FIRMWARE_STATUS_ERROR:
    case FIRMWARE_STATUS_CONTROLLER_ERROR:
        qDebug(("\t FirmwareUpgrade - firmware activate, error returned.\n"));
        break;

    default:
        qDebug(("\t FirmwareUpgrade - firmware activate, unexpected error. srbControl->ReturnCode %d.\n"), srbControl->ReturnCode);
        break;
   }


	qDebug(("...FirmwareUpgrade - firmware activate succeeded.\n"));

Exit:

    if (fileHandle != NULL) {
        CloseHandle(fileHandle);
    }

    if (buffer != NULL) {
        free(buffer);
    }

    return;
}







#endif


