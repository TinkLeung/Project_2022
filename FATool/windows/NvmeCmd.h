
/**
 * File: NvmeCmd.h 
 */ 

#ifndef NVME_CMD_H
#define NVME_CMD_H

#include <QDebug>
#ifdef WIN32
#include <windows.h>
#include <winioctl.h>
#include <ntddscsi.h>
#endif
//#include <ntddstor.h>

#include "Nvmeioctl.h"
#include "StorageQuery.h"

#define NVME_MAX_LOG_SIZE		4096

#define NVME_LOG_PAGE_MSFT_HEALTH_VERSION_0       0x0000
#define NVME_LOG_PAGE_MSFT_HEALTH_VERSION_1       0x0001


class NVMeCmd
{
	public:
		NVMeCmd();
		NVMeCmd(int pdNum);
    //public:
        //~NVMeCmd();
        virtual ~NVMeCmd();

	public:

		//drive index
		int Drive_index = 0;
		//drive handle
		HANDLE m_pdHandle = NULL;
		
		StorageQuery::STORAGE_ADAPTER_DESCRIPTOR m_StgAdpDsp;

	public:

		virtual ULONG NVMeLogPageQueryProperty(HANDLE handle, DWORD ProtocolDataReqVal, DWORD ProtocolDataLength, LPVOID lpOutBuffer);
		virtual ULONG NVMeIdentifyQueryProperty(HANDLE handle, LPVOID lpOutBuffer);
        virtual ULONG NVMeIdentifyNamespaceQueryProperty(HANDLE handle, LPVOID lpOutBuffer);
		virtual ULONG NVMePassThrough(HANDLE  Handle,LPVOID  lpNvmcmd,LONG DataTransferLength,LPVOID lpOutBuffer);
		virtual ULONG DeviceQueryProperty(HANDLE Handle, LPVOID lpOutBuffer);
		virtual int NVMeReadViaSCSIPassThrough(HANDLE Handle, LPVOID lpOutBuffer);
        virtual int NVMeWriteViaSCSIPassThrough(HANDLE Handle, LPVOID lpOutBuffer);
;

	private:
		BOOL QueryPropertyForDevice(HANDLE DeviceHandle, StorageQuery::PSTORAGE_ADAPTER_DESCRIPTOR pStgAdpDsp);
		BOOL QueryPropertyForDevice(HANDLE DeviceHandle, PULONG AlignmentMask, PUCHAR SrbType);
		BOOL TestViaSCSIPassThrough(HANDLE DeviceHandle, PULONG _alignmentMask, PUCHAR _srbType);
		void PrintAdapterDecriptor(StorageQuery::PSTORAGE_ADAPTER_DESCRIPTOR AdapterDescriptor);
		void PrintDeviceDescriptor(StorageQuery::PSTORAGE_DEVICE_DESCRIPTOR DeviceDescriptor);
        //void PrintStatusResultsExDIn(BOOL status, DWORD returned, PSCSI_PASS_THROUGH_WITH_BUFFERS psptwbex, ULONG length);
};

#endif

