
/**
 * File: NvmeCmd.cpp
 */ 

#include "NvmeCmd.h"
#include "StorQuery.h"
//#include "Utils.h"

#ifdef WIN32
#include <intsafe.h>
#define SPT_CDB_LENGTH 32
#define SPT_SENSE_LENGTH 32
#define SPTWB_DATA_LENGTH 512

#define FILE_DEVICE_CONTROLLER          0x00000004
#define IOCTL_SCSI_BASE                 FILE_DEVICE_CONTROLLER
#define METHOD_BUFFERED                 0
#define FILE_READ_ACCESS          ( 0x0001 )
#define FILE_WRITE_ACCESS         ( 0x0002 ) 
#define CTL_CODE( DeviceType, Function, Method, Access )  (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))

#define IOCTL_SCSI_PASS_THROUGH_EX          CTL_CODE(IOCTL_SCSI_BASE, 0x0411, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_PASS_THROUGH_DIRECT_EX   CTL_CODE(IOCTL_SCSI_BASE, 0x0412, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


#if defined(_X86_)
#define PAGE_SIZE  0x1000
#define PAGE_SHIFT 12L
#elif defined(_AMD64_)
#define PAGE_SIZE  0x1000
#define PAGE_SHIFT 12L
#elif defined(_IA64_)
#define PAGE_SIZE 0x2000
#define PAGE_SHIFT 13L
#else
// undefined platform?
#define PAGE_SIZE  0x1000
#define PAGE_SHIFT 12L
#endif


// 10-byte scsi commands
#define SCSIOP_READ	0x28
#define SCSIOP_WRITE 0x2A

// 12-byte sici commands
#define SCSIOP_READ12  0xA8
#define SCSIOP_WRITE12	0xAA

// 16-byte sici commands
#define SCSIOP_READ16   0x88
#define SCSIOP_WRITE16	0x8A

LPCSTR BusTypeStrings[] = {
    "Unknown",
    "Scsi",
    "Atapi",
    "Ata",
    "1394",
    "Ssa",
    "Fibre",
    "Usb",
    "RAID",
    "Not Defined",
};

#define NUMBER_OF_BUS_TYPE_STRINGS (sizeof(BusTypeStrings)/sizeof(BusTypeStrings[0]))


typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS
{
	SCSI_PASS_THROUGH 	spt;
	ULONG	Filler; 
	UCHAR	ucSenseBuf[SPT_SENSE_LENGTH];
	UCHAR	ucDataBuf[SPTWB_DATA_LENGTH];
}SCSI_PASS_THROUGH_WITH_BUFFERS,*PSCSI_PASS_THROUGH_WITH_BUFFERS;

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER
{
	SCSI_PASS_THROUGH_DIRECT	sptd;
	ULONG	Filler;
	UCHAR	ucSenseBuf[SPT_SENSE_LENGTH];
}SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

typedef struct _SCSI_PASS_THROUGH_EX
{
	ULONG Version;
	ULONG Length;		// size of the structure
	ULONG CdbLength;	// non-zero value should be set by caller
	ULONG StorAddressLength;	// non-zero value should be set by caller
	ULONG ScsiStatus;
	UCHAR SenseInfoLength;	//optional, can be zero
	UCHAR DataDirection; // data transfer direction
	UCHAR Reserved;		// padding
	ULONG TimeOutValue;
	ULONG StorAddressOffset;	//a value bigger than (structure size + CdbLength) should be set by caller
	ULONG SenseInfoOffset;
	ULONG DataOutTransferLength;	// optional, can be zero
	ULONG DataInTransferLength;		// optional, can be zero
	ULONG_PTR DataOutBufferOffset;
	ULONG_PTR DataInBufferOffset;
	_Field_size_bytes_full_(CdbLength) UCHAR Cdb[ANYSIZE_ARRAY];
}SCSI_PASS_THROUGH_EX, *PSCSI_PASS_THROUGH_EX;

// define different storage address types
#define STOR_ADDRESS_TYPE_UNKNOWN	0x0
#define STOR_ADDRESS_TYPE_BTL8	0x1
#define STOR_ADDRESS_TYPE_MAX	0xffff

#if defined (_WIN64) || defined(_M_ALPHA)
#define STOR_ADDRESS_ALIGN	DECLSPEC_ALIGN(8)
#else
#define STOR_ADDRESS_ALIGN				// now it is.
#endif

// define 8 bit bus, target and LUN address scheme
#define STOR_ADDR_BTL8_ADDRESS_LENGTH	4
//typedef struct STOR_ADDRESS_ALIGH _STOR_ADDR_BTL8
typedef struct _STOR_ADDR_BTL8
{
    _Field_range_(STOR_ADDRESS_TYPE_BTL8, STOR_ADDRESS_TYPE_BTL8)
	unsigned short Type;
	unsigned short Port;
	_Field_range_(STOR_ADDR_BTL8_ADDRESS_LENGTH,STOR_ADDR_BTL8_ADDRESS_LENGTH)
	ULONG AddressLength;
	UCHAR Path;
	UCHAR Target;
	UCHAR Lun;
	UCHAR Reserved;
} STOR_ADDR_BTL8, *PSTOR_ADDR_BTL8;

typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS_EX
{
	SCSI_PASS_THROUGH_EX spt;
	UCHAR	ucCdbBuf[SPT_CDB_LENGTH-1];
	ULONG	Filler;
	STOR_ADDR_BTL8	StorAddress;
	UCHAR	ucSenseBuf[SPT_SENSE_LENGTH];
	UCHAR   ucDataBuf[SPTWB_DATA_LENGTH];
}SCSI_PASS_THROUGH_WITH_BUFFERS_EX, *PSCSI_PASS_THROUGH_WITH_BUFFERS_EX;


typedef struct _SCSI_PASS_THROUGH_DIRECT_EX
{
	ULONG Version;
	ULONG Length;	// size of the structure
	ULONG CdbLength;	// non-zero value should be set by caller
	ULONG StorAddressLength;	// non-zero value should be set by caller
	UCHAR ScsiStatus;
	UCHAR SenseInfoLength;	// optional, can be zero
	UCHAR DataDirection;	// data transfer direction
	UCHAR Reserved;		// padding
	ULONG TimeOutValue;
	ULONG StorAddressOffset;	// a value bigger than (structure size + CdbLength) should be set by caller
	ULONG SenseInfoOffset;
	ULONG DataOutTransferLength;	//optional, can be zero
	ULONG DataInTransferLength;		//optional, can be zero
	_Field_size_bytes_full_(DataOutTransferLength) VOID * DataOutBuffer;
	_Field_size_bytes_full_opt_(DataInTransferLength) VOID * DataInBuffer;
	_Field_size_bytes_full_(CdbLength) UCHAR Cdb[ANYSIZE_ARRAY];
} SCSI_PASS_THROUGH_DIRECT_EX, *PSCSI_PASS_THROUGH_DIRECT_EX;


typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER_EX
{
	SCSI_PASS_THROUGH_DIRECT_EX sptd;
	UCHAR	ucCdbBuf[SPT_CDB_LENGTH-1];
	ULONG	Filler;
	STOR_ADDR_BTL8	StorAddress;
	UCHAR	ucSenseBuf[SPT_SENSE_LENGTH];
}SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER_EX, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER_EX;

#define CDB6GENERIC_LENGTH	6
#define CDB10GENERIC_LENGTH	10
#define CDB12GENERIC_LENGTH	12
#define SETBITON	1
#define SETBITOFF	0

#define BOOLEAN_TO_STRING(_b_) ((_b_) ? "True":"False")

#endif 

#define IOCTL_STORAGE_PROTOCOL_COMMAND              CTL_CODE(IOCTL_STORAGE_BASE, 0x04F0, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

NVMeCmd::NVMeCmd()
{
	;//nothing to do
}

NVMeCmd::NVMeCmd(int pdNum)
{
	TCHAR   buffer[64];
	Drive_index = pdNum;
    wsprintf(buffer, (L"\\\\.\\PhysicalDrive%d"), pdNum);
	m_pdHandle = CreateFile(buffer,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
    if (m_pdHandle != INVALID_HANDLE_VALUE)
    {
		printf(("Find %s \n"), buffer);
	}
}

NVMeCmd::~NVMeCmd()
{
	//nothing to do?
}

static PUCHAR AllocateAlignedBuffer(ULONG size, ULONG AlignmentMask, PUCHAR *pUnAlignedBuffer)
{
	PUCHAR ptr=NULL;
	UINT_PTR align64=(UINT_PTR)AlignmentMask;

	if(AlignmentMask==0)
	{
		ptr=(PUCHAR)malloc(size);
		*pUnAlignedBuffer=ptr;
	}
	else
	{
		ULONG totalSize;
		(void)ULongAdd(size,AlignmentMask,&totalSize);
		ptr=(PUCHAR)malloc(totalSize);
		*pUnAlignedBuffer=ptr;
		ptr=(PUCHAR)(((UINT_PTR)ptr+align64)&~align64);
	}

	if(ptr==NULL)
	{
		printf("malloc error %d\n",GetLastError());
	}
	return ptr;
}
static void PrintSenseInfo(PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb)
{
	UCHAR i;

    fprintf(stderr, "Scsi status: %02Xh\n\n", psptwb->spt.ScsiStatus);
    if (psptwb->spt.SenseInfoLength == 0) return;

    fprintf(stderr, "Sense Info -- consult SCSI spec for details\n");
    fprintf(stderr, "-------------------------------------------------------------\n");
    for (i = 0; i < psptwb->spt.SenseInfoLength; i++)
    {
        fprintf(stderr, "%02X ", psptwb->ucSenseBuf[i]);
    }
    fprintf(stderr, "\n\n");

	return;
}

static void PrintSenseInfoEx(PSCSI_PASS_THROUGH_WITH_BUFFERS_EX psptwb_ex)
{
	ULONG i;

	fprintf(stderr, "Scsi status: %02Xh\n", psptwb_ex->spt.ScsiStatus);
	if(psptwb_ex->spt.SenseInfoLength==0)
	{
		fprintf(stderr, "No sense information is available.\n");
		return;
	}

	fprintf(stderr, "Sense info -- consult SCSI spec for details\n");
	fprintf(stderr, "------------------------------------------------------------\n");

	for (i=0;i<psptwb_ex->spt.SenseInfoLength;i++)
	{
		fprintf(stderr, "%02X ", psptwb_ex->ucSenseBuf[i]);
	}
	fprintf(stderr,"\n");

	return;
}


static void PrintStatusResultsExDIn(BOOL status, DWORD returned, PSCSI_PASS_THROUGH_WITH_BUFFERS_EX psptwbex, ULONG length)
{
	if(!status)
	{
		//add something to do.
		return;
	}

	if(psptwbex->spt.ScsiStatus)
	{
		PrintSenseInfoEx(psptwbex);
		return;
	}
	else
	{
		printf("Scsi status: %02Xh (Succeeded), Bytes returned: %Xh \n", psptwbex->spt.ScsiStatus, returned);
		printf("DataOut buffer length: %Xh, DataIn buffer length: %Xh \n", psptwbex->spt.DataOutTransferLength,psptwbex->spt.DataInTransferLength);
        //PrintDataBuffer((PUCHAR)(psptwbex->ucDataBuf), length);
	}

	return;
}

static void PrintStatusResultsExDOut(BOOL status, DWORD returned, PSCSI_PASS_THROUGH_WITH_BUFFERS_EX psptwb_ex, ULONG length)
{
    if (!status)
    {
        printf("PrintStatusResultsExDOut (Unknown) error",GetLastError());
        return;
    }

    if (psptwb_ex->spt.ScsiStatus)
    {
        PrintSenseInfoEx(psptwb_ex);
        return;
    }
    else
    {
        printf("Scsi status: %02Xh (Succeeded), Bytes returned: %Xh, ", psptwb_ex->spt.ScsiStatus, returned);
        printf("DataOut buffer length: %Xh, DataIn buffer length: %Xh\n", psptwb_ex->spt.DataOutTransferLength, psptwb_ex->spt.DataInTransferLength);
    }
}


static void PrintStatusResultsDIn(BOOL status, DWORD returned,PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb, ULONG length)
{
	if(!status)
		return;

	if (psptwb->spt.ScsiStatus)
	{
		PrintSenseInfo(psptwb);
		return;
	}
	else
	{
		printf("Scsi status: %02Xh (Succeeded), Bytes returned: %Xh, ", psptwb->spt.ScsiStatus, returned);
		printf("Data buffer length: %Xh\n\n\n", psptwb->spt.DataTransferLength);
        //PrintDataBuffer((PUCHAR)(psptwb->ucDataBuf), length);
	}
}

static void PrintStatusResultsDOut(BOOL status, DWORD returned, PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb, ULONG length)
{
    if (!status)
    {
        printf("PrintStatusResultsDOut(Unknown) error",GetLastError());
        return;
    }


    if (psptwb->spt.ScsiStatus)
    {
        PrintSenseInfo(psptwb);
        return;
    }
    else
    {
        printf("Scsi status: %02Xh (Succeeded), Bytes returned: %Xh, ", psptwb->spt.ScsiStatus, returned);
        printf("Data buffer length: %Xh\n\n\n", psptwb->spt.DataTransferLength);
    }
}


BOOL NVMeCmd::QueryPropertyForDevice(HANDLE DeviceHandle,StorageQuery::PSTORAGE_ADAPTER_DESCRIPTOR pStgAdpDsp)
{
	StorageQuery::PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor = NULL;
	StorageQuery::STORAGE_DESCRIPTOR_HEADER header = { 0 };

	BOOL ok = TRUE;
	BOOL failed = TRUE;

	//get size required for storage adapter descriptor
	PVOID buffer = NULL;
	ULONG bufferSize = 0;
	ULONG returnedData;

	StorageQuery::STORAGE_PROPERTY_QUERY query;// = { 0 };

	query.QueryType = StorageQuery::PropertyStandardQuery;
	query.PropertyId = StorageQuery::StorageAdapterProperty;
	bufferSize = sizeof(StorageQuery::STORAGE_DESCRIPTOR_HEADER);
	buffer = &header;

	ZeroMemory(buffer, bufferSize);

	ok = DeviceIoControl(DeviceHandle,
					IOCTL_STORAGE_QUERY_PROPERTY,
					&query,sizeof(StorageQuery::STORAGE_PROPERTY_QUERY),
					buffer,bufferSize,
					&returnedData,FALSE);

    if (!ok)
    {
        printf("\t Get Storage Adapter Property  Header Failed: 0x%X\n", GetLastError());
		return FALSE;
	}

	//allocate and retrieve storage adapter descriptor
	query.QueryType = StorageQuery::PropertyStandardQuery;
	query.PropertyId = StorageQuery::StorageAdapterProperty;
	bufferSize = header.Size;

	if (bufferSize != 0) 
	{
		buffer = (StorageQuery::PSTORAGE_ADAPTER_DESCRIPTOR)malloc(bufferSize);
		if (buffer == NULL) 
		{
			free(buffer);
			return FALSE;
		}
	}

	ZeroMemory(buffer, bufferSize);

	// all setup, do the ioctl
	ok = DeviceIoControl(DeviceHandle,IOCTL_STORAGE_QUERY_PROPERTY,
							&query,sizeof(StorageQuery::STORAGE_PROPERTY_QUERY),
							buffer,bufferSize,
							&returnedData,FALSE);

    if (!ok)
    {
        printf("\t Get Storage Adapter Property Failed: 0x%X\n", GetLastError());
		free(buffer);
		return FALSE;
	}

	// adapterDescriptor is now allocated and full of data.

    if (returnedData == 0)
    {
        printf("   ***** No adapter descriptor supported on the device *****\n");
	}
	else {
		*pStgAdpDsp = *(StorageQuery::PSTORAGE_ADAPTER_DESCRIPTOR)buffer;
	}

	free(buffer);
	return TRUE;
}

BOOL NVMeCmd::QueryPropertyForDevice(HANDLE DeviceHandle, PULONG AlignmentMask, PUCHAR SrbType)
{
	StorageQuery::PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor = NULL;
	StorageQuery::PSTORAGE_DEVICE_DESCRIPTOR  deviceDescriptor = NULL;
	StorageQuery::STORAGE_DESCRIPTOR_HEADER header = {0};
	int ng=0;
	BOOL status=false;
	ULONG i;
	BOOL Ret = 0;

	*AlignmentMask = 0;  //default to no alignment
    *SrbType = 0;

	for(i=0;i<4;i++)
	{
		PVOID buffer=NULL;
		ULONG bufferSize =0;
		ULONG returnedData;
		StorageQuery::STORAGE_PROPERTY_QUERY query;
		ZeroMemory((void*)(&query), sizeof(StorageQuery::STORAGE_PROPERTY_QUERY));

		switch(i)
		{
		case 0:				// get size required for storage adapter descriptor
			{
				query.QueryType = StorageQuery::PropertyStandardQuery;
				query.PropertyId = StorageQuery::StorageAdapterProperty;
				bufferSize = sizeof(StorageQuery::STORAGE_DESCRIPTOR_HEADER);
				buffer = &header;
				break;
			}
		case 1:			// allocate and retrieve storage adapter descriptor
			{
				query.QueryType = StorageQuery::PropertyStandardQuery;
				query.PropertyId = StorageQuery::StorageAdapterProperty;
				bufferSize = header.Size;
				if(bufferSize!=0)
				{
					adapterDescriptor = (StorageQuery::PSTORAGE_ADAPTER_DESCRIPTOR)LocalAlloc(LPTR,bufferSize);
					if(adapterDescriptor == NULL)
					{
						goto Cleanup;
					}
				}
				buffer = adapterDescriptor;
				break;
			}
		case 2:			//get size required for storage device descriptor
			{
				query.QueryType =StorageQuery::PropertyStandardQuery;
				query.PropertyId = StorageQuery::StorageDeviceProperty;
				bufferSize = sizeof(StorageQuery::STORAGE_DESCRIPTOR_HEADER);
				buffer = &header;
				break;
			}
		case 3:			// allocate and retrieve storage device descriptor
			{
				query.QueryType = StorageQuery::PropertyStandardQuery;
				query.PropertyId = StorageQuery::StorageDeviceProperty;
				bufferSize = header.Size;
				if(bufferSize!=0)
				{
					deviceDescriptor = (StorageQuery::PSTORAGE_DEVICE_DESCRIPTOR)LocalAlloc(LPTR,bufferSize);
					if(deviceDescriptor == NULL)
					{
						goto Cleanup;
					}
				}
				buffer = deviceDescriptor;
				break;

			}
		default:
			 //nothing to do
			 break;
		}

		if(buffer !=NULL)
		{
			RtlZeroMemory(buffer,bufferSize);
			
			Ret = DeviceIoControl(DeviceHandle,IOCTL_STORAGE_QUERY_PROPERTY,&query,sizeof(StorageQuery::STORAGE_PROPERTY_QUERY),
										buffer,bufferSize,&returnedData,FALSE);
			
			if(!Ret)
			{
				if((GetLastError() == ERROR_MORE_DATA)||(GetLastError()== ERROR_INVALID_FUNCTION)||(GetLastError()==ERROR_NOT_SUPPORTED))
				{
					printf("it is ok ? DeviceIoControl error code=%d\n ",GetLastError());// it is ok?
				}
				else
				{
					printf("DeviceIoControl error code=%d\n",GetLastError());
					goto Cleanup;
				}

				RtlZeroMemory(buffer, bufferSize);
			}
		}
	} //end i loop

	// adapterDescriptor(deviceDescriptor) is now allocated and full of data
	if(adapterDescriptor == NULL)
	{
		printf("No adapter descriptor supported on the device\n");
	}
	else
	{
        PrintAdapterDecriptor(adapterDescriptor);
		*AlignmentMask = adapterDescriptor -> AlignmentMask;
#if (NTDDI_VERSION >= NTDDI_WIN8)		
		*SrbType = adapterDescriptor->SrbType;
#endif
		status = true;
	}

	if(deviceDescriptor == NULL)
	{
		printf("No device descriptor supported on the device\n");
	}
	else
	{
		PrintDeviceDescriptor(deviceDescriptor);
		status = true;

	}

Cleanup:

	if(adapterDescriptor !=NULL)
	{
		LocalFree(adapterDescriptor);
	}

	if(deviceDescriptor != NULL)
	{
		LocalFree(deviceDescriptor);
	}
	
	return status;
}


void NVMeCmd::PrintAdapterDecriptor(StorageQuery::PSTORAGE_ADAPTER_DESCRIPTOR AdapterDescriptor)
{
	ULONG trueMaximumTransferLength;
	LPCSTR busType;

	if(AdapterDescriptor->BusType < NUMBER_OF_BUS_TYPE_STRINGS)
	{
		busType = BusTypeStrings[AdapterDescriptor->BusType];
	}
	else
	{
		busType = BusTypeStrings[NUMBER_OF_BUS_TYPE_STRINGS-1];
	}

	if(AdapterDescriptor->MaximumPhysicalPages >1)
	{
		trueMaximumTransferLength = AdapterDescriptor->MaximumPhysicalPages -1;
	}
	else
	{
		trueMaximumTransferLength =1;
	}

	// make it into a byte value
	trueMaximumTransferLength <<= PAGE_SHIFT;

	if(trueMaximumTransferLength >AdapterDescriptor->MaximumTransferLength)
	{
		trueMaximumTransferLength = AdapterDescriptor->MaximumTransferLength;
	}

	if(trueMaximumTransferLength<PAGE_SIZE)
	{
		trueMaximumTransferLength=PAGE_SIZE;
	}

	printf("\n	STORAGE ADAPTER DESCRIPTOR DATA		\n");
    printf("              Version: %08x\n"
        "            TotalSize: %08x\n"
        "MaximumTransferLength: %08x (bytes)\n"
        " MaximumPhysicalPages: %08x\n"
        "  TrueMaximumTransfer: %08x (bytes)\n"
        "        AlignmentMask: %08x\n"
        "       AdapterUsesPio: %s\n"
        "     AdapterScansDown: %s\n"
        "      CommandQueueing: %s\n"
        "  AcceleratedTransfer: %s\n"
        "             Bus Type: %s\n"
        "    Bus Major Version: %04x\n"
        "    Bus Minor Version: %04x\n",
        AdapterDescriptor->Version,
        AdapterDescriptor->Size,
        AdapterDescriptor->MaximumTransferLength,
        AdapterDescriptor->MaximumPhysicalPages,
        trueMaximumTransferLength,
        AdapterDescriptor->AlignmentMask,
        BOOLEAN_TO_STRING(AdapterDescriptor->AdapterUsesPio),
        BOOLEAN_TO_STRING(AdapterDescriptor->AdapterScansDown),
        BOOLEAN_TO_STRING(AdapterDescriptor->CommandQueueing),
        BOOLEAN_TO_STRING(AdapterDescriptor->AcceleratedTransfer),
        busType,
        AdapterDescriptor->BusMajorVersion,
        AdapterDescriptor->BusMinorVersion);

	printf("\n");

	return;
}


void NVMeCmd::PrintDeviceDescriptor(StorageQuery::PSTORAGE_DEVICE_DESCRIPTOR DeviceDescriptor)
{
    LPCSTR vendorId = "";
    LPCSTR productId = "";
    LPCSTR productRevision = "";
    LPCSTR serialNumber = "";
    LPCSTR busType;

    if ((ULONG)DeviceDescriptor->BusType < NUMBER_OF_BUS_TYPE_STRINGS)
    {
        busType = BusTypeStrings[DeviceDescriptor->BusType];
    }
    else
    {
        busType = BusTypeStrings[NUMBER_OF_BUS_TYPE_STRINGS - 1];
    }

    if ((DeviceDescriptor->ProductIdOffset != 0) &&	(DeviceDescriptor->ProductIdOffset != -1))
    {
        productId = (LPCSTR)(DeviceDescriptor);
        productId += (ULONG_PTR)DeviceDescriptor->ProductIdOffset;
    }

    if ((DeviceDescriptor->VendorIdOffset != 0) && (DeviceDescriptor->VendorIdOffset != -1))
    {
        vendorId = (LPCSTR)(DeviceDescriptor);
        vendorId += (ULONG_PTR)DeviceDescriptor->VendorIdOffset;
    }

    if ((DeviceDescriptor->ProductRevisionOffset != 0) && (DeviceDescriptor->ProductRevisionOffset != -1))
    {
        productRevision = (LPCSTR)(DeviceDescriptor);
        productRevision += (ULONG_PTR)DeviceDescriptor->ProductRevisionOffset;
    }

    if ((DeviceDescriptor->SerialNumberOffset != 0) && (DeviceDescriptor->SerialNumberOffset != -1))
    {
        serialNumber = (LPCSTR)(DeviceDescriptor);
        serialNumber += (ULONG_PTR)DeviceDescriptor->SerialNumberOffset;
    }

    printf("\n           STORAGE DEVICE DESCRIPTOR DATA ");
    printf("              Version: %08x\n"
        "            TotalSize: %08x\n"
        "           DeviceType: %08x\n"
        "   DeviceTypeModifier: %08x\n"
        "       RemovableMedia: %s\n"
        "      CommandQueueing: %s\n"
        "            Vendor Id: %s\n"
        "           Product Id: %s\n"
        "     Product Revision: %s\n"
        "        Serial Number: %s\n"
        "             Bus Type: %s\n"
        "       Raw Properties: %s\n",
        DeviceDescriptor->Version,
        DeviceDescriptor->Size,
        DeviceDescriptor->DeviceType,
        DeviceDescriptor->DeviceTypeModifier,
        BOOLEAN_TO_STRING(DeviceDescriptor->RemovableMedia),
        BOOLEAN_TO_STRING(DeviceDescriptor->CommandQueueing),
        vendorId,
        productId,
        productRevision,
        serialNumber,
        busType,
        (DeviceDescriptor->RawPropertiesLength ? "Follows" : "None"));

    if (DeviceDescriptor->RawPropertiesLength != 0)
    {
        //PrintDataBuffer(DeviceDescriptor->RawDeviceProperties, DeviceDescriptor->RawPropertiesLength);
    }

    printf("\n\n");

	return;
}
ULONG NVMeCmd::NVMeLogPageQueryProperty(HANDLE handle, DWORD ProtocolDataReqVal, DWORD ProtocolDataLength, LPVOID lpOutBuffer)
{
	ULONG	status = ERROR_SUCCESS;
	BOOL	result;
	PVOID	buffer = NULL;
	ULONG	bufferLength = 0;
	ULONG	returnedLength = 0;

	StorageQuery::PSTORAGE_PROPERTY_QUERY query = NULL;
	StorageQuery::PSTORAGE_PROTOCOL_SPECIFIC_DATA protocolData = NULL;
	StorageQuery::PSTORAGE_PROTOCOL_DATA_DESCRIPTOR protocolDataDescr = NULL;

	//
	// Allocate buffer for use.
	//
	bufferLength = FIELD_OFFSET(StorageQuery::STORAGE_PROPERTY_QUERY, AdditionalParameters) 
							+ sizeof(StorageQuery::STORAGE_PROTOCOL_SPECIFIC_DATA) + NVME_MAX_LOG_SIZE;
	buffer = malloc(bufferLength);

    if (buffer == NULL)
    {
        printf("\tNVMEQueryProperty: allocate buffer failed, exit.\n");
		status = ERROR_NOT_ENOUGH_MEMORY;
		goto exit;
	}

	//
	// Initialize query data structure to get SMART/Health Information Log.
	//
	ZeroMemory(buffer, bufferLength);

	query = (StorageQuery::PSTORAGE_PROPERTY_QUERY)buffer;
	protocolDataDescr = (StorageQuery::PSTORAGE_PROTOCOL_DATA_DESCRIPTOR)buffer;

	protocolData = (StorageQuery::PSTORAGE_PROTOCOL_SPECIFIC_DATA)query->AdditionalParameters;

	query->PropertyId = StorageQuery::StorageDeviceProtocolSpecificProperty;
	query->QueryType = StorageQuery::PropertyStandardQuery;

	protocolData->ProtocolType = StorageQuery::ProtocolTypeNvme;
	protocolData->DataType = StorageQuery::NVMeDataTypeLogPage;
	protocolData->ProtocolDataRequestValue = ProtocolDataReqVal;
	protocolData->ProtocolDataRequestSubValue = 0;
	protocolData->ProtocolDataOffset = sizeof(StorageQuery::STORAGE_PROTOCOL_SPECIFIC_DATA);
	protocolData->ProtocolDataLength = ProtocolDataLength;

	//
	// Send request down.
	//

	result = DeviceIoControl(handle,IOCTL_STORAGE_QUERY_PROPERTY,
								buffer,bufferLength,
								buffer,bufferLength,
								&returnedLength,NULL);

    if (!result || (returnedLength == 0))
    {
		status = GetLastError();
        printf("\tNVMeLogPageQueryProperty:  failed. Error Code %d.\n", status);
		goto exit;
	}

	//
	// Validate the returned data.
	//
	if ((protocolDataDescr->Version != sizeof(StorageQuery::STORAGE_PROTOCOL_DATA_DESCRIPTOR)) ||
		(protocolDataDescr->Size != sizeof(StorageQuery::STORAGE_PROTOCOL_DATA_DESCRIPTOR))) {

		status = ERROR_INVALID_DATA;
        printf("\tNVMeLogPageQueryProperty:  data descriptor header not valid.\n");
		goto exit;
	}

	protocolData = &protocolDataDescr->ProtocolSpecificData;

	if ((protocolData->ProtocolDataOffset < sizeof(StorageQuery::STORAGE_PROTOCOL_SPECIFIC_DATA)) ||
		(protocolData->ProtocolDataLength < ProtocolDataLength)) 
	{
		status = ERROR_INVALID_DATA;
		printf("\tNVMeLogPageQueryProperty: ProtocolData Offset/Length not valid.\n");
		goto exit;
	}

	//PNVME_HEALTH_INFO_LOG smartInfo = (PNVME_HEALTH_INFO_LOG)((PCHAR)protocolData + protocolData->ProtocolDataOffset);

	memcpy(lpOutBuffer, ((PCHAR)protocolData + protocolData->ProtocolDataOffset), protocolData->ProtocolDataLength);

exit:

	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}

	return status;

}

ULONG NVMeCmd::NVMeIdentifyQueryProperty(HANDLE handle,LPVOID lpOutBuffer)
{

	TCHAR tPath[256] = {0};
	BOOL bRet = 0;

	ULONG	status = ERROR_SUCCESS;

	StorQuery::TStorageQueryWithBuffer nptwb;
	ZeroMemory(&nptwb, sizeof(nptwb));

	nptwb.ProtocolSpecific.ProtocolType = StorQuery::ProtocolTypeNvme;
	nptwb.ProtocolSpecific.DataType = StorQuery::NVMeDataTypeIdentify;
	nptwb.ProtocolSpecific.ProtocolDataOffset = sizeof(StorQuery::TStorageProtocolSpecificData);
    nptwb.ProtocolSpecific.ProtocolDataLength = 4096;
    nptwb.ProtocolSpecific.ProtocolDataRequestValue = NVME_IDENTIFY_CNS_CONTROLLER; /*NVME_IDENTIFY_CNS_CONTROLLER*/
    nptwb.ProtocolSpecific.ProtocolDataRequestSubValue = 0;
	nptwb.Query.PropertyId = StorQuery::StorageAdapterProtocolSpecificProperty;
	nptwb.Query.QueryType = StorQuery::PropertyStandardQuery;
	DWORD dwReturned = 0;

	bRet = DeviceIoControl(m_pdHandle, IOCTL_STORAGE_QUERY_PROPERTY,
		&nptwb, sizeof(nptwb), &nptwb, sizeof(nptwb), &dwReturned, NULL);

		
    if (!bRet || (dwReturned == 0))
    {
		status = GetLastError();
        printf("\tNVMeIdentifyQueryProperty:  failed. Error Code %d.\n", status);
		return status;
	}

	printf("\tNVMeIdentifyQueryProperty:  success \n");

	memcpy_s(lpOutBuffer, sizeof(NVME_IDENTIFY_CONTROLLER_DATA), nptwb.Buffer, sizeof(NVME_IDENTIFY_CONTROLLER_DATA));
	return status;

}

ULONG NVMeCmd::NVMeIdentifyNamespaceQueryProperty(HANDLE handle,LPVOID lpOutBuffer)
{

    TCHAR tPath[256] = {0};
    BOOL bRet = 0;

    ULONG	status = ERROR_SUCCESS;

    StorQuery::TStorageQueryWithBuffer nptwb;
    ZeroMemory(&nptwb, sizeof(nptwb));

    nptwb.ProtocolSpecific.ProtocolType = StorQuery::ProtocolTypeNvme;
    nptwb.ProtocolSpecific.DataType = StorQuery::NVMeDataTypeIdentify;
    nptwb.ProtocolSpecific.ProtocolDataOffset = sizeof(StorQuery::TStorageProtocolSpecificData);
    nptwb.ProtocolSpecific.ProtocolDataLength = 4096;
    nptwb.ProtocolSpecific.ProtocolDataRequestValue = NVME_IDENTIFY_CNS_SPECIFIC_NAMESPACE; /*NVME_IDENTIFY_CNS_CONTROLLER*/
    nptwb.ProtocolSpecific.ProtocolDataRequestSubValue = 1;
    nptwb.Query.PropertyId = StorQuery::StorageAdapterProtocolSpecificProperty;
    nptwb.Query.QueryType = StorQuery::PropertyStandardQuery;
    DWORD dwReturned = 0;

    bRet = DeviceIoControl(m_pdHandle, IOCTL_STORAGE_QUERY_PROPERTY,
        &nptwb, sizeof(nptwb), &nptwb, sizeof(nptwb), &dwReturned, NULL);

    if (!bRet || (dwReturned == 0))
    {
        status = GetLastError();
        printf("\tNVMeIdentifyQueryProperty:  failed. Error Code %d.\n", status);
        return status;
    }

    printf("\tNVMeIdentifyQueryProperty:  success \n");

    memcpy_s(lpOutBuffer, sizeof(NVME_IDENTIFY_NAMESPACE_DATA), nptwb.Buffer, sizeof(NVME_IDENTIFY_NAMESPACE_DATA));
    return status;

}

ULONG NVMeCmd::NVMePassThroughDataIn(HANDLE Handle, LPVOID lpNvmcmd, LONG DataTransferLength, LPVOID lpOutBuffer)
{
    ULONG    status = ERROR_SUCCESS;
    BOOL	 result;
    PUCHAR   buffer = NULL;
    ULONG    bufferLength = 0;
    ULONG    returnedLength = 0;

	StorageQuery::PSTORAGE_PROTOCOL_COMMAND protocolCommand = NULL;
	PNVME_COMMAND command = NULL;
	//UINT NVME_MAX_LOG_SIZE = 4096;

	//
	// Allocate buffer for use.
	//
	DWORD dwDataTransferLength = DataTransferLength < 0 ? (-DataTransferLength) : DataTransferLength;
	bufferLength = FIELD_OFFSET(StorageQuery::STORAGE_PROTOCOL_COMMAND, Command) + 
					STORAGE_PROTOCOL_COMMAND_LENGTH_NVME + dwDataTransferLength + sizeof(NVME_ERROR_INFO_LOG);
    buffer = (PUCHAR)malloc(bufferLength);

	if (buffer == NULL) {
		printf("GetDebugLog: allocate buffer failed, exit.\n");
		return 1;
	}

	//
	// Initialize query data structure to get Identify Controller Data.
	//
	ZeroMemory(buffer, bufferLength);

	protocolCommand = (StorageQuery::PSTORAGE_PROTOCOL_COMMAND)buffer;
	protocolCommand->Version = STORAGE_PROTOCOL_STRUCTURE_VERSION;
	protocolCommand->Length = sizeof(StorageQuery::STORAGE_PROTOCOL_COMMAND);
	protocolCommand->ProtocolType =StorageQuery::ProtocolTypeNvme;
	protocolCommand->Flags = STORAGE_PROTOCOL_COMMAND_FLAG_ADAPTER_REQUEST;
	protocolCommand->CommandLength = STORAGE_PROTOCOL_COMMAND_LENGTH_NVME;
	protocolCommand->ErrorInfoLength = sizeof(NVME_ERROR_INFO_LOG);

	//protocolCommand->DataToDeviceTransferLength = 0;

	protocolCommand->DataFromDeviceTransferLength = DataTransferLength;
	protocolCommand->TimeOutValue = 50;

	protocolCommand->ErrorInfoOffset = FIELD_OFFSET(StorageQuery::STORAGE_PROTOCOL_COMMAND, Command) + STORAGE_PROTOCOL_COMMAND_LENGTH_NVME;

	//protocolCommand->DataToDeviceBufferOffset = protocolCommand->ErrorInfoOffset + protocolCommand->ErrorInfoLength;

	protocolCommand->DataFromDeviceBufferOffset = protocolCommand->ErrorInfoOffset + protocolCommand->ErrorInfoLength;
	protocolCommand->CommandSpecific = STORAGE_PROTOCOL_SPECIFIC_NVME_ADMIN_COMMAND;

	command = (PNVME_COMMAND)protocolCommand->Command;
	memcpy(command, lpNvmcmd, STORAGE_PROTOCOL_COMMAND_LENGTH_NVME);

	//
	// Send request down.
	//
    result = DeviceIoControl(Handle,IOCTL_STORAGE_PROTOCOL_COMMAND,	buffer,
                             bufferLength, buffer, bufferLength, &returnedLength, NULL);

	//
	// Validate the returned data.
	//

	if (!result || (returnedLength == 0) || (STORAGE_PROTOCOL_STATUS_SUCCESS != protocolCommand->ReturnStatus)) 
	{
		status = GetLastError();
		printf("\nNVMEPassThrough:  failed. Error Code %d.\n", status);
		if (STORAGE_PROTOCOL_STATUS_SUCCESS != protocolCommand->ReturnStatus)
		{
			printf("\tNVMEPassThrough:  protocolCommand->ReturnStatus: %d.\n", protocolCommand->ReturnStatus);
		}	
		goto exit;
	}

	printf("\tNVMEPassThrough:  Success\n");
	//PNVME_ERROR_INFO_LOG errinfo = (PNVME_ERROR_INFO_LOG)((PCHAR)protocolCommand + protocolCommand->ErrorInfoOffset);

	if (DataTransferLength > 0)
		memcpy(lpOutBuffer, ((PCHAR)protocolCommand + protocolCommand->DataFromDeviceBufferOffset), protocolCommand->DataFromDeviceTransferLength);
exit:

	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}

	return status;
}

ULONG NVMeCmd::NVMePassThroughDataOut(HANDLE Handle, LPVOID lpNvmcmd, LONG DataTransferLength, LPVOID lpOutBuffer)
{
    ULONG    status = ERROR_SUCCESS;
    BOOL	 result;
    PUCHAR   buffer = NULL;
    ULONG    bufferLength = 0;
    ULONG    returnedLength = 0;

    StorageQuery::PSTORAGE_PROTOCOL_COMMAND protocolCommand = NULL;
    PNVME_COMMAND command = NULL;
    //UINT NVME_MAX_LOG_SIZE = 4096;

    //
    // Allocate buffer for use.
    //
    DWORD dwDataTransferLength = DataTransferLength < 0 ? (-DataTransferLength) : DataTransferLength;
    bufferLength = FIELD_OFFSET(StorageQuery::STORAGE_PROTOCOL_COMMAND, Command) +
                    STORAGE_PROTOCOL_COMMAND_LENGTH_NVME + dwDataTransferLength + sizeof(NVME_ERROR_INFO_LOG);
    buffer = (PUCHAR)malloc(bufferLength);

    if (buffer == NULL) {
        printf("GetDebugLog: allocate buffer failed, exit.\n");
        return 1;
    }

    //
    // Initialize query data structure to get Identify Controller Data.
    //
    ZeroMemory(buffer, bufferLength);

    protocolCommand = (StorageQuery::PSTORAGE_PROTOCOL_COMMAND)buffer;
    protocolCommand->Version = STORAGE_PROTOCOL_STRUCTURE_VERSION;
    protocolCommand->Length = sizeof(StorageQuery::STORAGE_PROTOCOL_COMMAND);
    protocolCommand->ProtocolType =StorageQuery::ProtocolTypeNvme;
    protocolCommand->Flags = STORAGE_PROTOCOL_COMMAND_FLAG_ADAPTER_REQUEST;
    protocolCommand->CommandLength = STORAGE_PROTOCOL_COMMAND_LENGTH_NVME;
    protocolCommand->ErrorInfoLength = sizeof(NVME_ERROR_INFO_LOG);

    protocolCommand->DataToDeviceTransferLength = DataTransferLength;

    //protocolCommand->DataFromDeviceTransferLength = DataTransferLength;
    protocolCommand->TimeOutValue = 50;

    protocolCommand->ErrorInfoOffset = FIELD_OFFSET(StorageQuery::STORAGE_PROTOCOL_COMMAND, Command) + STORAGE_PROTOCOL_COMMAND_LENGTH_NVME;

    //protocolCommand->DataToDeviceBufferOffset = protocolCommand->ErrorInfoOffset + protocolCommand->ErrorInfoLength;

    protocolCommand->DataFromDeviceBufferOffset = protocolCommand->ErrorInfoOffset + protocolCommand->ErrorInfoLength;
    protocolCommand->CommandSpecific = STORAGE_PROTOCOL_SPECIFIC_NVME_ADMIN_COMMAND;

    command = (PNVME_COMMAND)protocolCommand->Command;
    memcpy(command, lpNvmcmd, STORAGE_PROTOCOL_COMMAND_LENGTH_NVME);

    //
    // Send request down.
    //
    result = DeviceIoControl(Handle,IOCTL_STORAGE_PROTOCOL_COMMAND,	buffer,
                             bufferLength, buffer, bufferLength, &returnedLength, NULL);

    //
    // Validate the returned data.
    //

    if (!result || (returnedLength == 0) || (STORAGE_PROTOCOL_STATUS_SUCCESS != protocolCommand->ReturnStatus))
    {
        status = GetLastError();
        printf("\nNVMEPassThrough:  failed. Error Code %d.\n", status);
        if (STORAGE_PROTOCOL_STATUS_SUCCESS != protocolCommand->ReturnStatus)
        {
            printf("\tNVMEPassThrough:  protocolCommand->ReturnStatus: %d.\n", protocolCommand->ReturnStatus);
        }
        goto exit;
    }

    printf("\tNVMEPassThrough:  Success\n");
    //PNVME_ERROR_INFO_LOG errinfo = (PNVME_ERROR_INFO_LOG)((PCHAR)protocolCommand + protocolCommand->ErrorInfoOffset);

    if (DataTransferLength > 0)
        memcpy(lpOutBuffer, ((PCHAR)protocolCommand + protocolCommand->DataFromDeviceBufferOffset), protocolCommand->DataFromDeviceTransferLength);
exit:

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    return status;
}

ULONG NVMeCmd::NVMePassThroughNonData(HANDLE Handle, LPVOID lpNvmcmd)
{
    ULONG    status = ERROR_SUCCESS;
    BOOL	 result;
    PUCHAR   buffer = NULL;
    ULONG    bufferLength = 0;
    ULONG    returnedLength = 0;

    StorageQuery::PSTORAGE_PROTOCOL_COMMAND protocolCommand = NULL;
    PNVME_COMMAND command = NULL;
    //UINT NVME_MAX_LOG_SIZE = 4096;

    //
    // Allocate buffer for use.
    //
    //DWORD dwDataTransferLength = DataTransferLength < 0 ? (-DataTransferLength) : DataTransferLength;
    bufferLength = FIELD_OFFSET(StorageQuery::STORAGE_PROTOCOL_COMMAND, Command) +
                    STORAGE_PROTOCOL_COMMAND_LENGTH_NVME + sizeof(NVME_ERROR_INFO_LOG);
    buffer = (PUCHAR)malloc(bufferLength);

    if (buffer == NULL) {
        printf("GetDebugLog: allocate buffer failed, exit.\n");
        return 1;
    }

    //
    // Initialize query data structure to get Identify Controller Data.
    //
    ZeroMemory(buffer, bufferLength);

    protocolCommand = (StorageQuery::PSTORAGE_PROTOCOL_COMMAND)buffer;
    protocolCommand->Version = STORAGE_PROTOCOL_STRUCTURE_VERSION;
    protocolCommand->Length = sizeof(StorageQuery::STORAGE_PROTOCOL_COMMAND);
    protocolCommand->ProtocolType =StorageQuery::ProtocolTypeNvme;
    protocolCommand->Flags = STORAGE_PROTOCOL_COMMAND_FLAG_ADAPTER_REQUEST;
    protocolCommand->CommandLength = STORAGE_PROTOCOL_COMMAND_LENGTH_NVME;
    protocolCommand->ErrorInfoLength = sizeof(NVME_ERROR_INFO_LOG);

    //protocolCommand->DataToDeviceTransferLength = 0;

    //protocolCommand->DataFromDeviceTransferLength = DataTransferLength;
    protocolCommand->TimeOutValue = 50;

    protocolCommand->ErrorInfoOffset = FIELD_OFFSET(StorageQuery::STORAGE_PROTOCOL_COMMAND, Command) + STORAGE_PROTOCOL_COMMAND_LENGTH_NVME;

    //protocolCommand->DataToDeviceBufferOffset = protocolCommand->ErrorInfoOffset + protocolCommand->ErrorInfoLength;

    protocolCommand->DataFromDeviceBufferOffset = protocolCommand->ErrorInfoOffset + protocolCommand->ErrorInfoLength;
    protocolCommand->CommandSpecific = STORAGE_PROTOCOL_SPECIFIC_NVME_ADMIN_COMMAND;

    command = (PNVME_COMMAND)protocolCommand->Command;
    memcpy(command, lpNvmcmd, STORAGE_PROTOCOL_COMMAND_LENGTH_NVME);

    //
    // Send request down.
    //
    result = DeviceIoControl(Handle,IOCTL_STORAGE_PROTOCOL_COMMAND,	buffer,
                             bufferLength, buffer, bufferLength, &returnedLength, NULL);

    //
    // Validate the returned data.
    //

    if (!result || (returnedLength == 0) || (STORAGE_PROTOCOL_STATUS_SUCCESS != protocolCommand->ReturnStatus))
    {
        status = GetLastError();
        printf("\nNVMEPassThrough:  failed. Error Code %d.\n", status);
        if (STORAGE_PROTOCOL_STATUS_SUCCESS != protocolCommand->ReturnStatus)
        {
            printf("\tNVMEPassThrough:  protocolCommand->ReturnStatus: %d.\n", protocolCommand->ReturnStatus);
        }
        goto exit;
    }

    printf("\tNVMEPassThrough:  Success\n");
    //PNVME_ERROR_INFO_LOG errinfo = (PNVME_ERROR_INFO_LOG)((PCHAR)protocolCommand + protocolCommand->ErrorInfoOffset);

    /*if (DataTransferLength > 0)
        memcpy(lpOutBuffer, ((PCHAR)protocolCommand + protocolCommand->DataFromDeviceBufferOffset), protocolCommand->DataFromDeviceTransferLength);*/
exit:

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    return status;
}

int  NVMeCmd::NVMeReadViaSCSIPassThrough(HANDLE Handle, LPVOID lpOutBuffer)
{
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	SCSI_PASS_THROUGH_WITH_BUFFERS_EX sptwb_ex;
	ULONG length=0, errorCode=0, returned=0, sectorSize=512;
	ULONG alignmentMask = 0; // default == no alignment requirement
	UCHAR srbType = 0;
	BOOL ret;

    if(TestViaSCSIPassThrough(Handle, &alignmentMask, &srbType) == false)
	{
		return -1;
	}

	if(srbType)
	{
		ZeroMemory(&sptwb_ex,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS_EX));
		sptwb_ex.spt.Version = 0;
		sptwb_ex.spt.Length = sizeof(SCSI_PASS_THROUGH_EX);
		sptwb_ex.spt.ScsiStatus = 0;
		sptwb_ex.spt.CdbLength = CDB10GENERIC_LENGTH;
		sptwb_ex.spt.StorAddressLength = sizeof(STOR_ADDR_BTL8);
		sptwb_ex.spt.SenseInfoLength = SPT_SENSE_LENGTH;
		sptwb_ex.spt.DataOutTransferLength = 0;
		sptwb_ex.spt.DataInTransferLength = 512;
		sptwb_ex.spt.DataDirection = SCSI_IOCTL_DATA_IN;
		sptwb_ex.spt.TimeOutValue = 2;
		sptwb_ex.spt.StorAddressOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS_EX,StorAddress);
		sptwb_ex.StorAddress.Type = STOR_ADDRESS_TYPE_BTL8;
		sptwb_ex.StorAddress.Port = 0;
		sptwb_ex.StorAddress.AddressLength = STOR_ADDR_BTL8_ADDRESS_LENGTH;
		sptwb_ex.StorAddress.Path = 0;
		sptwb_ex.StorAddress.Target = 0;
		sptwb_ex.StorAddress.Lun = 0;
		sptwb_ex.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS_EX,ucSenseBuf);
		sptwb_ex.spt.DataOutBufferOffset = 0;
		sptwb_ex.spt.DataInBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS_EX,ucDataBuf);
		sptwb_ex.spt.Cdb[0] = SCSIOP_READ;
		sptwb_ex.spt.Cdb[5] = 0; //	Starting LBA
		sptwb_ex.spt.Cdb[8] = 1;  // Transfer length
		length = sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS_EX);

        ret = DeviceIoControl(Handle, IOCTL_SCSI_PASS_THROUGH_EX, &sptwb_ex, length, &sptwb_ex, length, &returned, FALSE);
		if(!ret)
		{
			printf(" Read LBA through IOCTL_SCSI_PASS_THROUGH_EX error code %d\n",GetLastError());
			return -1;
		}

		PrintStatusResultsExDIn(ret, returned, &sptwb_ex, sptwb_ex.spt.DataInTransferLength);
		if(lpOutBuffer!=NULL)
	    {
            memcpy(lpOutBuffer, (PUCHAR)(sptwb_ex.ucDataBuf), sptwb_ex.spt.DataInTransferLength);
		}	
	}
	else
	{
		ZeroMemory(&sptwb, sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
		sptwb.spt.PathId = 0;
		sptwb.spt.TargetId = 0;
		sptwb.spt.Lun = 0;
		sptwb.spt.CdbLength = CDB10GENERIC_LENGTH;
		sptwb.spt.SenseInfoLength = SPT_SENSE_LENGTH;
		sptwb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, ucSenseBuf);
		sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
		sptwb.spt.DataTransferLength = 512;
		sptwb.spt.TimeOutValue = 5;
		sptwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, ucDataBuf);
		sptwb.spt.Cdb[0] = SCSIOP_READ;
		sptwb.spt.Cdb[5] = 0; // Starting LBA
		sptwb.spt.Cdb[8] = 1; // TRANSFER LENGTH

		length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, ucDataBuf) + sptwb.spt.DataTransferLength;

		ret = DeviceIoControl(Handle, IOCTL_SCSI_PASS_THROUGH, &sptwb, length, &sptwb, length, &returned, FALSE);
		
		if(!ret)
		{
			printf(" Read LBA through IOCTL_SCSI_PASS_THROUGH error code %d\n",GetLastError());
			return -1;
		}

		PrintStatusResultsDIn(ret, returned, &sptwb, sptwb.spt.DataTransferLength);		
		if(lpOutBuffer!=NULL)
		{
			memcpy(lpOutBuffer, (PUCHAR)(sptwb.ucDataBuf), sptwb.spt.DataTransferLength);
		}
	}

	return 0;
}


int NVMeCmd::NVMeWriteViaSCSIPassThrough(HANDLE Handle, LPVOID lpOutBuffer)
{
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER_EX sptdwb_ex;
	ULONG length=0, errorCode=0, returned=0, sectorSize=512;
	ULONG alignmentMask = 0; // default == no alignment requirement
	UCHAR srbType = 0;
	BOOL ret;
	
    if(TestViaSCSIPassThrough(Handle, &alignmentMask, &srbType) == false)
	{
		return -1;
	}

	if(srbType)
	{
		PUCHAR pUnAlignedBuffer = NULL;
		PUCHAR databuffer =(PUCHAR)AllocateAlignedBuffer(sectorSize, alignmentMask, &pUnAlignedBuffer); 
		if(databuffer==NULL)
			return -1;

		ZeroMemory(databuffer,512);
		for(int i=0; i<512/8; i++)
		{
			databuffer[i * 8] = 'D';
			databuffer[i * 8 + 1] = 'E';
			databuffer[i * 8 + 2] = 'A';
			databuffer[i * 8 + 3] = 'D';
			databuffer[i * 8 + 4] = 'B';
			databuffer[i * 8 + 5] = 'E';
			databuffer[i * 8 + 6] = 'E';
			databuffer[i * 8 + 7] = 'F';
		}
		
		ZeroMemory(&sptdwb_ex, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER_EX));
		sptdwb_ex.sptd.Version = 0;
		sptdwb_ex.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT_EX);
		sptdwb_ex.sptd.ScsiStatus= 0;
		sptdwb_ex.sptd.CdbLength= CDB10GENERIC_LENGTH;
		sptdwb_ex.sptd.StorAddressLength = sizeof(STOR_ADDR_BTL8);
		sptdwb_ex.sptd.SenseInfoLength = SPT_SENSE_LENGTH;
		sptdwb_ex.sptd.DataOutBuffer = (PVOID)databuffer;
		sptdwb_ex.sptd.DataOutTransferLength = sectorSize;
		sptdwb_ex.sptd.DataInTransferLength = 0;
		sptdwb_ex.sptd.DataDirection = SCSI_IOCTL_DATA_OUT;
		sptdwb_ex.sptd.TimeOutValue = 5;
		sptdwb_ex.sptd.StorAddressOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER_EX, StorAddress);
		sptdwb_ex.StorAddress.Type	= STOR_ADDRESS_TYPE_BTL8;
		sptdwb_ex.StorAddress.Port	= 0;
		sptdwb_ex.StorAddress.AddressLength = STOR_ADDR_BTL8_ADDRESS_LENGTH;
		sptdwb_ex.StorAddress.Path = 0;
		sptdwb_ex.StorAddress.Target	= 0;
		sptdwb_ex.StorAddress.Lun	= 0;
		sptdwb_ex.sptd.SenseInfoOffset	= offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER_EX, ucSenseBuf);
		sptdwb_ex.sptd.Cdb[0]	= SCSIOP_WRITE;
		sptdwb_ex.sptd.Cdb[5]	= 0; // Starting LBA
		sptdwb_ex.sptd.Cdb[8]	= 1; // TRANSFER LENGTH
		
		length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER_EX);
		ret = DeviceIoControl(Handle,IOCTL_SCSI_PASS_THROUGH_DIRECT_EX, &sptdwb_ex, length, &sptdwb_ex, length, &returned, FALSE);
		if(!ret)
		{
			printf(" Write LBA through IOCTL_SCSI_PASS_THROUGH_EX error code %d\n",GetLastError());
		}
		PrintStatusResultsExDOut(ret, returned, (PSCSI_PASS_THROUGH_WITH_BUFFERS_EX)&sptdwb_ex, sptdwb_ex.sptd.DataInTransferLength);
		free(pUnAlignedBuffer);
	}
	else
	{
        UCHAR databuffer[512];
        ZeroMemory(databuffer, 512);

        for (int i = 0; i < 512 / 8; i++)
        {
            databuffer[i * 8] = 'q';
            databuffer[i * 8 + 1] = 'q';
            databuffer[i * 8 + 2] = 'q';
            databuffer[i * 8 + 3] = 'q';
            databuffer[i * 8 + 4] = 'q';
            databuffer[i * 8 + 5] = 'q';
            databuffer[i * 8 + 6] = 'q';
            databuffer[i * 8 + 7] = 'q';
        }

        ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
        sptdwb.sptd.Length              = sizeof(SCSI_PASS_THROUGH_DIRECT);
        sptdwb.sptd.PathId              = 0;
        sptdwb.sptd.TargetId            = 0;
        sptdwb.sptd.Lun                 = 0;
        sptdwb.sptd.CdbLength           = CDB10GENERIC_LENGTH;
        sptdwb.sptd.SenseInfoLength     = SPT_SENSE_LENGTH;
        sptdwb.sptd.SenseInfoOffset     = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);
        sptdwb.sptd.DataIn              = SCSI_IOCTL_DATA_OUT;
        sptdwb.sptd.DataTransferLength  = sectorSize;
        sptdwb.sptd.TimeOutValue        = 5;
        sptdwb.sptd.DataBuffer          = databuffer;
        sptdwb.sptd.Cdb[0]              = SCSIOP_WRITE;
        sptdwb.sptd.Cdb[5]              = 0; // Starting LBA
        sptdwb.sptd.Cdb[8]              = 1; // TRANSFER LENGTH

        length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
		
		ret = DeviceIoControl(Handle,IOCTL_SCSI_PASS_THROUGH_DIRECT, &sptdwb, length, &sptdwb, length, &returned, FALSE);
		if(!ret)
		{
			printf(" Write LBA through IOCTL_SCSI_PASS_THROUGH error code %d\n",GetLastError());
		}

		PrintStatusResultsDOut(ret, returned, (PSCSI_PASS_THROUGH_WITH_BUFFERS)&sptdwb, length);
	}

	return 0;
}

BOOL NVMeCmd::TestViaSCSIPassThrough(HANDLE DeviceHandle, PULONG _alignmentMask, PUCHAR _srbType)
{
	BOOL status = false;
	ULONG errorCode = 0;

	status = QueryPropertyForDevice(DeviceHandle,_alignmentMask,_srbType);
	if(!status)
	{
		printf(" Error %d getting device and /or adapter properties. \n", GetLastError());
	}
    return status;

}


ULONG NVMeCmd::DeviceQueryProperty(HANDLE  Handle, LPVOID lpOutBuffer)
{
	ULONG   status = ERROR_SUCCESS;
	BOOL	result;

	StorageQuery::STORAGE_PROPERTY_QUERY query;
	DWORD cbBytesReturned = 0;

    ZeroMemory((void *)& query, sizeof(query));
	query.PropertyId = StorageQuery::StorageDeviceProperty;
	query.QueryType = StorageQuery::PropertyStandardQuery;

    char local_buff[4096];
	ZeroMemory(local_buff,sizeof(local_buff));

	result = DeviceIoControl(Handle, IOCTL_STORAGE_QUERY_PROPERTY, 
					&query, sizeof(query), &local_buff[0],sizeof(local_buff),
					&cbBytesReturned,NULL);
	
	if (!result || (cbBytesReturned == 0) )
	{
		status = GetLastError();
		printf("\nNVMeDeviceQueryProperty:  failed. Error Code %d.\n", status);
		return status ;
	}

    printf("\nNVMeDeviceQueryProperty: success \n");

    memcpy(lpOutBuffer, (LPVOID)&local_buff, 4096);
	return status;
}

