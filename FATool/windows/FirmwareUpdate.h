
#ifndef FIRMWAREUPDATE_H
#define FIRMWAREUPDATE_H

#ifdef WIN32
#include <windows.h>
#include <winioctl.h>
#include <QtDebug>
#include <QDir>
#include <string>
#include <Tchar.h>

using namespace std;

//#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define NVME_SIG_STR "NvmeMini"
#define NVME_SIG_STR_LEN 8
#define NVME_FROM_DEV_TO_HOST 2
#define NVME_IOCTL_VENDOR_SPECIFIC_DW_SIZE 6
#define NVME_IOCTL_CMD_DW_SIZE 16
#define NVME_IOCTL_COMPLETE_DW_SIZE 4
#define NVME_PT_TIMEOUT 40



#define IOCTL_SCSI_BASE                 FILE_DEVICE_CONTROLLER
#define FILE_DEVICE_SCSI                0x0000001b

/* the following are the NVME driver private IOCTL definitions */

#define NVME_STORPORT_DRIVER 0xE000
#define NVME_PASS_THROUGH_SRB_IO_CODE \
    CTL_CODE( NVME_STORPORT_DRIVER, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NVME_RESET_DEVICE \
    CTL_CODE(NVME_STORPORT_DRIVER, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NVME_HOT_ADD_NAMESPACE \
    CTL_CODE(NVME_STORPORT_DRIVER, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NVME_HOT_REMOVE_NAMESPACE \
    CTL_CODE(NVME_STORPORT_DRIVER, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define STORAGE_FIRMWARE_SLOT_INFO_V2_REVISION_LENGTH   16


#define NVME_SIG_STR          "NvmeMini"
#define NVME_SIG_STR_LEN      8
#define SCSI_SIG_STR          "SCSIDISK"
#define SCSI_SIG_STR_LEN      8
#define NVME_NO_DATA_TX       0 /* No data transfer involved */
#define NVME_FROM_HOST_TO_DEV 1 /* Transfer data from host to device */
#define NVME_FROM_DEV_TO_HOST 2 /* Transfer data from device to host */
#define NVME_BI_DIRECTION     3 /* Tx data from host to device and back */

#define NVME_IOCTL_VENDOR_SPECIFIC_DW_SIZE 6  /* Vendor sp qualifier (DWORDs) */
#define NVME_IOCTL_CMD_DW_SIZE             16 /* NVMe cmd entry size (DWORDs) */
#define NVME_IOCTL_COMPLETE_DW_SIZE        4  /* NVMe cpl entry size (DWORDs) */


#define FIRMWARE_REQUEST_BLOCK_STRUCTURE_VERSION            0x1

#define IOCTL_SCSI_MINIPORT_FIRMWARE          ((FILE_DEVICE_SCSI << 16) + 0x0780)
#define IOCTL_MINIPORT_SIGNATURE_FIRMWARE           "FIRMWARE"


#define FIRMWARE_FUNCTION_GET_INFO                          0x01
#define FIRMWARE_FUNCTION_DOWNLOAD                          0x02
#define FIRMWARE_FUNCTION_ACTIVATE                          0x03

/* FIRMWARE IOCTL status */

#define FIRMWARE_STATUS_SUCCESS                             0x0
#define FIRMWARE_STATUS_ERROR                               0x1
#define FIRMWARE_STATUS_ILLEGAL_REQUEST                     0x2
#define FIRMWARE_STATUS_INVALID_PARAMETER                   0x3
#define FIRMWARE_STATUS_INPUT_BUFFER_TOO_BIG                0x4
#define FIRMWARE_STATUS_OUTPUT_BUFFER_TOO_SMALL             0x5
#define FIRMWARE_STATUS_INVALID_SLOT                        0x6
#define FIRMWARE_STATUS_INVALID_IMAGE                       0x7
#define FIRMWARE_STATUS_CONTROLLER_ERROR                    0x10
#define FIRMWARE_STATUS_POWER_CYCLE_REQUIRED                0x20
#define FIRMWARE_STATUS_DEVICE_ERROR                        0x40



#define FIRMWARE_REQUEST_FLAG_CONTROLLER                    0x00000001
#define STORAGE_FIRMWARE_INFO_INVALID_SLOT              0xFF

typedef struct Bin_Identify_Device
{
	BYTE		Bin[512];
}Bin_IDENTIFY_DEVICE;


typedef struct _DEVICE_LIST
{
    HANDLE    Handle;
    STORAGE_ADAPTER_DESCRIPTOR  AdapterDescriptor;

}DEVICE_LIST, *PDEVICE_LIST;;

typedef struct _STORAGE_FIRMWARE_SLOT_INFO {

    UCHAR   SlotNumber;
    BOOLEAN ReadOnly;
    UCHAR   Reserved[6];

    union {
        UCHAR     Info[8];
        ULONGLONG AsUlonglong;
    } Revision;

} STORAGE_FIRMWARE_SLOT_INFO, *PSTORAGE_FIRMWARE_SLOT_INFO;


typedef struct _STORAGE_FIRMWARE_SLOT_INFO_V2 {

    UCHAR   SlotNumber;
    BOOLEAN ReadOnly;
    UCHAR   Reserved[6];

    UCHAR   Revision[STORAGE_FIRMWARE_SLOT_INFO_V2_REVISION_LENGTH];

} STORAGE_FIRMWARE_SLOT_INFO_V2, *PSTORAGE_FIRMWARE_SLOT_INFO_V2;


//#pragma warning(push)
//#pragma warning(disable:4200) // nonstandard extension used : zero-sized array in struct/union

typedef struct _STORAGE_FIRMWARE_EX_INFO 
{

    ULONG   Version;        // STORAGE_FIRMWARE_INFO_STRUCTURE_VERSION
    ULONG   Size;           // sizeof(STORAGE_FIRMWARE_INFO)

    BOOL UpgradeSupport;
    UCHAR   SlotCount;
    UCHAR   ActiveSlot;
    UCHAR   PendingActivateSlot;
    BOOL IsFirmwareActWithoutRest;
	UCHAR   FirmwareUptGranularity;
    ULONG   Reserved;

    STORAGE_FIRMWARE_SLOT_INFO Slot[0];

} STORAGE_FIRMWARE_EX_INFO, *PSTORAGE_FIRMWARE_EX_INFO;



typedef struct _STORAGE_FIRMWARE_INFO 
{
    ULONG   Version;        // STORAGE_FIRMWARE_INFO_STRUCTURE_VERSION
    ULONG   Size;           // sizeof(STORAGE_FIRMWARE_INFO)
    BOOLEAN UpgradeSupport;
    UCHAR   SlotCount;
    UCHAR   ActiveSlot;
    UCHAR   PendingActivateSlot;
    ULONG   Reserved;
    STORAGE_FIRMWARE_SLOT_INFO Slot[0];
} STORAGE_FIRMWARE_INFO, *PSTORAGE_FIRMWARE_INFO;



typedef struct _FIRMWARE_REQUEST_BLOCK 
{
    ULONG   Version;            // FIRMWARE_REQUEST_BLOCK_STRUCTURE_VERSION
    ULONG   Size;               // Size of the data structure.
    ULONG   Function;           // Function code
    ULONG   Flags;
    ULONG   DataBufferOffset;   // the offset is from the beginning of buffer. e.g. from beginning of SRB_IO_CONTROL. The value should be multiple of sizeof(PVOID); Value 0 means that there is no data buffer.
    ULONG   DataBufferLength;   // length of the buffer
} FIRMWARE_REQUEST_BLOCK, *PFIRMWARE_REQUEST_BLOCK;

typedef struct _STORAGE_FIRMWARE_DOWNLOAD 
{
	ULONG       Version;            // STORAGE_FIRMWARE_DOWNLOAD_STRUCTURE_VERSION
    ULONG       Size;               // sizeof(STORAGE_FIRMWARE_DOWNLOAD)
    ULONGLONG   Offset;             // image file offset, should be aligned to value of "ImagePayloadAlignment" from STORAGE_FIRMWARE_INFO.
    ULONGLONG   BufferSize;         // should be multiple of value of "ImagePayloadAlignment" from STORAGE_FIRMWARE_INFO
    UCHAR       ImageBuffer[0];     // firmware image file. 
} STORAGE_FIRMWARE_DOWNLOAD, *PSTORAGE_FIRMWARE_DOWNLOAD; 


typedef struct _STORAGE_FIRMWARE_ACTIVATE 
{
    ULONG   Version;
    ULONG   Size;
    UCHAR   SlotToActivate;
    UCHAR   Reserved0[3];
} STORAGE_FIRMWARE_ACTIVATE, *PSTORAGE_FIRMWARE_ACTIVATE;



#define IOCTL_STORAGE_FIRMWARE_GET_INFO         CTL_CODE(IOCTL_STORAGE_BASE, 0x0700, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_FIRMWARE_DOWNLOAD         CTL_CODE(IOCTL_STORAGE_BASE, 0x0701, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_STORAGE_FIRMWARE_ACTIVATE         CTL_CODE(IOCTL_STORAGE_BASE, 0x0702, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define STORAGE_HW_FIRMWARE_REQUEST_FLAG_CONTROLLER                     0x00000001
#define STORAGE_HW_FIRMWARE_REQUEST_FLAG_SWITCH_TO_EXISTING_FIRMWARE    0x80000000

#define STG_S_POWER_CYCLE_REQUIRED  0x207				// the device needs to be power cycled.


typedef struct _STORAGE_HW_FIRMWARE_INFO_QUERY 
{

    ULONG   Version;            // sizeof(STORAGE_FIRMWARE_INFO_QUERY)
    ULONG   Size;               // Whole size of the buffer (in case this data structure being extended to be variable length)
    ULONG   Flags;
    ULONG   Reserved;
} STORAGE_HW_FIRMWARE_INFO_QUERY, *PSTORAGE_HW_FIRMWARE_INFO_QUERY;


#define STORAGE_HW_FIRMWARE_REVISION_LENGTH  16

typedef struct _STORAGE_HW_FIRMWARE_SLOT_INFO 
{
  	DWORD Version;
  	DWORD Size;
  	BYTE  SlotNumber;
  	BYTE  ReadOnly  :1;
  	BYTE  Reserved0  :7;
  	BYTE  Reserved1[6];
  	BYTE  Revision[STORAGE_HW_FIRMWARE_REVISION_LENGTH];
} STORAGE_HW_FIRMWARE_SLOT_INFO, *PSTORAGE_HW_FIRMWARE_SLOT_INFO;


#define STORAGE_HW_FIRMWARE_INVALID_SLOT              0xFF


typedef struct _STORAGE_HW_FIRMWARE_INFO 
{
   ULONG                         Version;
  ULONG                         Size;
  UCHAR                         SupportUpgrade : 1;
  UCHAR                         Reserved0 : 7;
  UCHAR                         SlotCount;
  UCHAR                         ActiveSlot;
  UCHAR                         PendingActivateSlot;
  BOOLEAN                       FirmwareShared;
  UCHAR                         Reserved[3];
  ULONG                         ImagePayloadAlignment;
  ULONG                         ImagePayloadMaxSize;
  STORAGE_HW_FIRMWARE_SLOT_INFO 		Slot[ANYSIZE_ARRAY];
} STORAGE_HW_FIRMWARE_INFO, *PSTORAGE_HW_FIRMWARE_INFO;



typedef struct _STORAGE_HW_FIRMWARE_DOWNLOAD 
{
	ULONG       Version;            // sizeof(STORAGE_FIRMWARE_DOWNLOAD)
	ULONG       Size;               // size of the whole buffer include "ImageBuffer"
	ULONG       Flags;
	UCHAR       Slot;               // Slot number that firmware image will be downloaded into.
	UCHAR       Reserved[3];
	ULONGLONG   Offset;             // Image file offset, should be aligned to "ImagePayloadAlignment" value from STORAGE_FIRMWARE_INFO.
	ULONGLONG   BufferSize;         // should be multiple of "ImagePayloadAlignment" value from STORAGE_FIRMWARE_INFO.
	UCHAR       ImageBuffer[ANYSIZE_ARRAY];     // firmware image file.
} STORAGE_HW_FIRMWARE_DOWNLOAD, *PSTORAGE_HW_FIRMWARE_DOWNLOAD;




typedef struct _STORAGE_HW_FIRMWARE_ACTIVATE 
{
    ULONG   Version;
    ULONG   Size;
    ULONG   Flags;
    UCHAR   Slot;                   // Slot with firmware image to be activated.
    UCHAR   Reserved0[3];
} STORAGE_HW_FIRMWARE_ACTIVATE, *PSTORAGE_HW_FIRMWARE_ACTIVATE;


//#if (NTDDI_VERSION < NTDDI_WINTHRESHOLD)
//#error11111
//#endif

//#pragma warning(pop)
class CFirmwareUpdate
{


/*  Constructors and destructors*/
public:
	virtual ~CFirmwareUpdate();
private:
	CFirmwareUpdate();


public:

	static 	CFirmwareUpdate* GetInstance();					// singleton implementation of firmwareupdate.

    BOOL FirmWareUpdateProcess(INT driveNumber,QString FileName, qint64 transize);
	//BOOL FirmWareUpdateActive(INT driveNumber);
    BOOL DeviceStorageFirmwareWin10Upgrade(INT physicalDriveId, INT slotID, ULONG action);
private:
	BOOL DeviceFirmwareHandle(INT driveNumber,PDEVICE_LIST pDeviceList);
	BOOL DeviceQueryProperty(HANDLE hioctrl, PSTORAGE_ADAPTER_DESCRIPTOR pStorageAdpDsp);
	BOOL DeviceGetFirmwareInfo(INT physicalDriveId, PSTORAGE_FIRMWARE_INFO pStorageFirmwareInfo, BOOL DisplayResult);
	string GetScsiPath(const TCHAR* Path);
    HANDLE GetIoCtrlHandle(BYTE index);


	/*Intel mode of firmware upgrade proccess, NVMe Passthrough mode*/
	BOOL DeviceFirmwareIntelUpgrade(INT physicalDriveId, string FileName);
	BOOL DeviceGetFirmwareSlotInfoIntelIdentify(INT physicalDriveId, STORAGE_FIRMWARE_EX_INFO *StoratgeFirmwareInfo);
	BOOL DeviceGetFirmwareSlotInfoIntelLog(INT physicalDriveId, STORAGE_FIRMWARE_EX_INFO *StoratgeFirmwareInfo);


	/*Old Samgsunm mode of firmware update proccess, Scsi passthrough translation NVMe mode,  in windows 8.1, this mode should support*/
	BOOL DeviceGetFirmwareSlotInfoSamsungIdentify(INT physicalDriveId,STORAGE_FIRMWARE_EX_INFO *StoratgeFirmwareInfo);
    BOOL DeviceGetFirmwareSlotInfoSamsungLog(INT physicalDriveId, STORAGE_FIRMWARE_EX_INFO *StoratgeFirmwareInfo);


	/* Windows 10 firmware update proccess, using miniport mode. Note: it seems failed. */
    BOOL DeviceGetFirmwareWin10Info(INT physicalDriveId,PUCHAR Buffer,DWORD BufferLength,BOOL DisplayResult);
    void DeviceFirmwareWin10Upgrade(INT physicalDriveId, string FileName, UCHAR *pFwslot);
    void DeviceFirmwareWin10Active(INT physicalDriveId,UCHAR Fwslot);

	/* Windows 10 firmware update process, using storage mode*/
	BOOL DeviceGetStorageFirmwareWin10Info(INT physicalDriveId,PUCHAR* Buffer, DWORD* BufferLength,BOOL DisplayResult);
    BOOL  DeviceStorageFirmwareWin10Upgrade1(INT physicalDriveId, QString FileName, BOOL VerboseDisplay);
	BOOL DeviceValidateFirmwareUpgradeSupport(PSTORAGE_HW_FIRMWARE_INFO FirmwareInfo);

    BOOL DeviceStorageFirmwareWin10Download(INT physicalDriveId, QString FileName, qint64 transize);

private: 

	static CFirmwareUpdate* m_s_pInstance;

	STORAGE_ADAPTER_DESCRIPTOR m_storageadptdecripter;

	UCHAR m_slot;
	UCHAR m_FirmwareRevision[17];
	
};


#endif

#endif
