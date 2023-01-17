
#ifndef STORAGEQUERY_H
#define STORAGEQUERY_H

#include <windows.h>
#include <sdkddkver.h>

namespace StorageQuery
{
	//
	// Define the different storage bus types
	// Bus types below 128 (0x80) are reserved for Microsoft use
	//
	
	typedef enum _STORAGE_BUS_TYPE {
		BusTypeUnknown = 0x00,
		BusTypeScsi,
		BusTypeAtapi,
		BusTypeAta,
		BusType1394,
		BusTypeSsa,
		BusTypeFibre,
		BusTypeUsb,
		BusTypeRAID,
		BusTypeiScsi,
		BusTypeSas,
		BusTypeSata,
		BusTypeSd,
		BusTypeMmc,
		BusTypeVirtual,
		BusTypeFileBackedVirtual,
		BusTypeSpaces,
		BusTypeNvme,
		BusTypeSCM,
		BusTypeMax,
		BusTypeMaxReserved = 0x7F
	} STORAGE_BUS_TYPE, *PSTORAGE_BUS_TYPE;

	//
	// Device property descriptor - this is really just a rehash of the inquiry
	// data retrieved from a scsi device
	//
	// This may only be retrieved from a target device.  Sending this to the bus
	// will result in an error
	//

	typedef struct _STORAGE_DEVICE_DESCRIPTOR
	{
    	ULONG Version;
    	ULONG Size;
		UCHAR DeviceType;
		UCHAR DeviceTypeModifier;
    	BOOLEAN RemovableMedia;
    	BOOLEAN CommandQueueing;
    	ULONG VendorIdOffset;
    	ULONG ProductIdOffset;
    	ULONG ProductRevisionOffset;
    	ULONG SerialNumberOffset;
    	STORAGE_BUS_TYPE BusType;
    	ULONG RawPropertiesLength;
    	UCHAR RawDeviceProperties[1];
	} STORAGE_DEVICE_DESCRIPTOR, *PSTORAGE_DEVICE_DESCRIPTOR;


	//
	// Adapter properties
	//
	// This descriptor can be retrieved from a target device object of from the
	// device object for the bus.  Retrieving from the target device object will
	// forward the request to the underlying bus
	//
	
	typedef struct _STORAGE_ADAPTER_DESCRIPTOR 
	{
	
		ULONG Version;
		ULONG Size;
		ULONG MaximumTransferLength;
		ULONG MaximumPhysicalPages;
		ULONG AlignmentMask;
		BOOLEAN AdapterUsesPio;
		BOOLEAN AdapterScansDown;
		BOOLEAN CommandQueueing;
		BOOLEAN AcceleratedTransfer;
#if (NTDDI_VERSION < NTDDI_WINXP)
		BOOLEAN BusType;
#else
		UCHAR BusType;
#endif
		USHORT BusMajorVersion;
		USHORT BusMinorVersion;
#if (NTDDI_VERSION >= NTDDI_WIN8)
		UCHAR SrbType;
		UCHAR AddressType;
#endif
	} STORAGE_ADAPTER_DESCRIPTOR, *PSTORAGE_ADAPTER_DESCRIPTOR;


	// Types of queries
	typedef enum _STORAGE_QUERY_TYPE 
	{
		PropertyStandardQuery = 0,			// Retrieves the descriptor
		PropertyExistsQuery,				// Used to test whether the descriptor is supported
		PropertyMaskQuery,					// Used to retrieve a mask of writeable fields in the descriptor
		PropertyQueryMaxDefined 	// use to validate the value
	} STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;

	// define some initial property id's
	typedef enum _STORAGE_PROPERTY_ID
	{
		StorageDeviceProperty = 0,
		StorageAdapterProperty,
		StorageDeviceIdProperty,
		StorageDeviceUniqueIdProperty,				// See storduid.h for details
		StorageDeviceWriteCacheProperty,
		StorageMiniportProperty,
		StorageAccessAlignmentProperty,
		StorageDeviceSeekPenaltyProperty,
		StorageDeviceTrimProperty,
		StorageDeviceWriteAggregationProperty,
		StorageDeviceDeviceTelemetryProperty,
		StorageDeviceLBProvisioningProperty,
		StorageDevicePowerProperty,
		StorageDeviceCopyOffloadProperty,
		StorageDeviceResiliencyProperty,
		StorageDeviceMediumProductType,
		StorageAdapterRpmbProperty,
	// end_winioctl
		StorageDeviceTieringProperty,
		StorageDeviceFaultDomainProperty,
		StorageDeviceClusportProperty,
	// begin_winioctl
		StorageDeviceIoCapabilityProperty = 48,
		StorageAdapterProtocolSpecificProperty,
		StorageDeviceProtocolSpecificProperty,
		StorageAdapterTemperatureProperty,
		StorageDeviceTemperatureProperty,
		StorageAdapterPhysicalTopologyProperty,
		StorageDevicePhysicalTopologyProperty,
		StorageDeviceAttributesProperty,
		StorageDeviceManagementStatus,
		StorageAdapterSerialNumberProperty,
		StorageDeviceLocationProperty,
	} STORAGE_PROPERTY_ID,*PSTORAGE_PROPERTY_ID;
	
	// Query structure - additional parameters for specific queries can follow the header
	typedef struct _STORAGE_PROPERTY_QUERY 
	{
		// ID of the property being retrieved
		STORAGE_PROPERTY_ID PropertyId;
		// Flags indicating the type of query being performed
		STORAGE_QUERY_TYPE QueryType;
		// Space for additional parameters if necessary
		UCHAR AdditionalParameters[1];
	} STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;
	
	// Standard property descriptor header.  All property pages should use this
	// as their first element or should contain these two elements
	typedef struct _STORAGE_DESCRIPTOR_HEADER 
	{
	  ULONG Version;
	  ULONG Size;
	} STORAGE_DESCRIPTOR_HEADER, *PSTORAGE_DESCRIPTOR_HEADER;



	//
	// Define the different storage command protocols that used between software and hardware.
	// e.g. command protocol software uses to communicate with hardware.
	// Protocol types below 128 (0x80) are reserved for Microsoft use.
	//
	typedef enum _STORAGE_PROTOCOL_TYPE 
	{
		ProtocolTypeUnknown = 0x00,
		ProtocolTypeScsi,
		ProtocolTypeAta,
		ProtocolTypeNvme,
		ProtocolTypeSd,
		ProtocolTypeProprietary = 0x7E,
		ProtocolTypeMaxReserved = 0x7F
	} STORAGE_PROTOCOL_TYPE, *PSTORAGE_PROTOCOL_TYPE;

	typedef enum _STORAGE_PROTOCOL_NVME_DATA_TYPE {
		NVMeDataTypeUnknown = 0,
		NVMeDataTypeIdentify,		// Retrieved by command - IDENTIFY CONTROLLER or IDENTIFY NAMESPACE
		NVMeDataTypeLogPage,		// Retrieved by command - GET LOG PAGE
		NVMeDataTypeFeature,		// Retrieved by command - GET FEATURES
	} STORAGE_PROTOCOL_NVME_DATA_TYPE, *PSTORAGE_PROTOCOL_NVME_DATA_TYPE;


	// Protocol Data should follow this data structure in the same buffer.
	// The offset of Protocol Data from the beginning of this data structure
	// is reported in data field - "ProtocolDataOffset".

	typedef struct _STORAGE_PROTOCOL_SPECIFIC_DATA {
	
		STORAGE_PROTOCOL_TYPE ProtocolType;
		ULONG	DataType;					// The value will be protocol specific, as defined in STORAGE_PROTOCOL_NVME_DATA_TYPE or STORAGE_PROTOCOL_ATA_DATA_TYPE.
	
		ULONG	ProtocolDataRequestValue;
		ULONG	ProtocolDataRequestSubValue;
	
		ULONG	ProtocolDataOffset; 		// The offset of data buffer is from beginning of this data structure.
		ULONG	ProtocolDataLength;
	
		ULONG	FixedProtocolReturnData;	// This is returned data, especially from NVMe feature data that doesn't need separate device data transfer.
		ULONG	Reserved[3];
	
	} STORAGE_PROTOCOL_SPECIFIC_DATA, *PSTORAGE_PROTOCOL_SPECIFIC_DATA;

	//
	// Input parameters for StorageAdapterProtocolSpecificProperty (or StorageDeviceProtocolSpecificProperty) & PropertyStandardQuery
	// will be data structure STORAGE_PROPERTY_QUERY, where the data field "AdditionalParameters" is a buffer
	// in format of STORAGE_PROTOCOL_SPECIFIC_DATA.
	//
	
	//
	// Out parameters for StorageAdapterProtocolSpecificProperty (or StorageDeviceProtocolSpecificProperty) & PropertyStandardQuery
	//
	typedef struct _STORAGE_PROTOCOL_DATA_DESCRIPTOR {
	
		ULONG	Version;
		ULONG	Size;
	
		STORAGE_PROTOCOL_SPECIFIC_DATA ProtocolSpecificData;
	
	} STORAGE_PROTOCOL_DATA_DESCRIPTOR, *PSTORAGE_PROTOCOL_DATA_DESCRIPTOR;



	//
	// Parameter for IOCTL_STORAGE_PROTOCOL_COMMAND
	// Buffer layout: <STORAGE_PROTOCOL_COMMAND> <Command> [Error Info Buffer] [Data-to-Device Buffer] [Data-from-Device Buffer]
	//
#define STORAGE_PROTOCOL_STRUCTURE_VERSION              0x1
	
	typedef struct _STORAGE_PROTOCOL_COMMAND {
	
		ULONG	Version;						// STORAGE_PROTOCOL_STRUCTURE_VERSION
		ULONG	Length; 						// sizeof(STORAGE_PROTOCOL_COMMAND)
	
		STORAGE_PROTOCOL_TYPE  ProtocolType;
		ULONG	Flags;							// Flags for the request
	
		ULONG	ReturnStatus;					// return value
		ULONG	ErrorCode;						// return value, optional
	
		ULONG	CommandLength;					// non-zero value should be set by caller
		ULONG	ErrorInfoLength;				// optional, can be zero
		ULONG	DataToDeviceTransferLength; 	// optional, can be zero. Used by WRITE type of request.
		ULONG	DataFromDeviceTransferLength;	// optional, can be zero. Used by READ type of request.
	
		ULONG	TimeOutValue;					// in unit of seconds
	
		ULONG	ErrorInfoOffset;				// offsets need to be pointer aligned
		ULONG	DataToDeviceBufferOffset;		// offsets need to be pointer aligned
		ULONG	DataFromDeviceBufferOffset; 	// offsets need to be pointer aligned
	
		ULONG	CommandSpecific;				// optional information passed along with Command.
		ULONG	Reserved0;
	
		ULONG	FixedProtocolReturnData;		// return data, optional. Some protocol, such as NVMe, may return a small amount data (DWORD0 from completion queue entry) without the need of separate device data transfer.
		ULONG	Reserved1[3];
	
		_Field_size_bytes_full_(CommandLength) UCHAR Command[ANYSIZE_ARRAY];
	
	} STORAGE_PROTOCOL_COMMAND, *PSTORAGE_PROTOCOL_COMMAND;

//
// Bit-mask values for STORAGE_PROTOCOL_COMMAND - "Flags" field.
//
#define STORAGE_PROTOCOL_COMMAND_FLAG_ADAPTER_REQUEST    0x80000000     // Flag indicates the request targeting to adapter instead of device.

//
// Status values for STORAGE_PROTOCOL_COMMAND - "ReturnStatus" field.
//
#define STORAGE_PROTOCOL_STATUS_PENDING                 0x0
#define STORAGE_PROTOCOL_STATUS_SUCCESS                 0x1
#define STORAGE_PROTOCOL_STATUS_ERROR                   0x2
#define STORAGE_PROTOCOL_STATUS_INVALID_REQUEST         0x3
#define STORAGE_PROTOCOL_STATUS_NO_DEVICE               0x4
#define STORAGE_PROTOCOL_STATUS_BUSY                    0x5
#define STORAGE_PROTOCOL_STATUS_DATA_OVERRUN            0x6
#define STORAGE_PROTOCOL_STATUS_INSUFFICIENT_RESOURCES  0x7

#define STORAGE_PROTOCOL_STATUS_NOT_SUPPORTED           0xFF


// Command Length for Storage Protocols

#define STORAGE_PROTOCOL_COMMAND_LENGTH_NVME            0x40 

// Command Specific Information for Storage protocols - "CommandSpecific" field.

#define STORAGE_PROTOCOL_SPECIFIC_NVME_ADMIN_COMMAND    0x01
#define STORAGE_PROTOCOL_SPECIFIC_NVME_NVM_COMMAND      0x02

#if 1
	typedef struct {
        STORAGE_PROPERTY_QUERY Query;
        STORAGE_PROTOCOL_SPECIFIC_DATA ProtocolSpecific;
        BYTE Buffer[4096];
    } TStorageQueryWithBuffer;
#endif


}





#endif

