

/**
 * File: NvmeApp.h 
**/ 

#ifndef NVME_APP_H
#define NVME_APP_H

#include "NvmeCmd.h"
#include "CommonFunc.h"


class NVMeApp : public NVMeCmd 
{

	public:
		NVMeApp();
		NVMeApp(int pdNum);

	public:
		void RefreshDrive();
        ULONG Identify();
        ULONG SmartInfo();
        //ULONG ReadFlashID();
        //ULONG EventLog();
        //ULONG ReadLbainfo();
        //ULONG WriteLbainfo();

	public:
        ULONG Identify(PUCHAR buffer);
        ULONG SmartInfo(PUCHAR buffer);
        ULONG ReadFlashID(PUCHAR buffer);
        ULONG GetRDTResultInfo(PUCHAR buffer, UCHAR chanel, UCHAR ce, UCHAR lun);
        //ULONG EventLog(PUCHAR backBuffer);
        //ULONG ReadLbainfo(PUCHAR backBuffer);
        ULONG IDTableNamespaceInfo(PUCHAR backBuffer);
        ULONG IDTableControllerInfo(PUCHAR backBuffer);


	private:
        //void PrintDataIdentify(PNVME_IDENTIFY_CONTROLLER_DATA idtfyData);
       // void PrintSmartLog(PNVME_HEALTH_INFO_LOG smartInfo);
       void PrintRawDataLog(PUCHAR data, int len);

};




#endif

