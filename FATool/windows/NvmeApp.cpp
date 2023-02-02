
/**
 * File: NvmeApp.cpp
**/ 

#include "NvmeApp.h"
#include "CommonFunc.h"

NVMeApp::NVMeApp():
	NVMeCmd()
{

}

NVMeApp::NVMeApp(int pdNum):
	NVMeCmd(pdNum)
{
	return;
}

ULONG NVMeApp::Identify()
{
    PUCHAR buffer;
    ULONG buflength = IDY_CONTROLLER_INFO_LENGTH;
    buffer = (PUCHAR)malloc(buflength);
	ULONG status = ERROR_SUCCESS;

	status = NVMeIdentifyQueryProperty(m_pdHandle, (LPVOID)buffer);
	if(status == ERROR_SUCCESS)
	{
        //PrintDataIdentify((PNVME_IDENTIFY_CONTROLLER_DATA)buffer);
	}
	else
	{
        printf("Not support identify, because it is not NVMe drive.\n");
	}

    free(buffer);
	return status;
}

ULONG NVMeApp::SmartInfo()
{	
    PUCHAR buffer;
    ULONG buflength = SMART_DATA_LENGTH; //512;

    ULONG status = ERROR_SUCCESS;
    buffer =(PUCHAR)malloc(buflength);

    status = NVMeLogPageQueryProperty(m_pdHandle, NVME_LOG_PAGE_HEALTH_INFO, sizeof(NVME_HEALTH_INFO_LOG), buffer);

	if(status == ERROR_SUCCESS)
	{
        //PrintSmartLog((PNVME_HEALTH_INFO_LOG)buffer);
	}
	else
	{
		printf("Not support smartlog\n");
	}

    free(buffer);
	return status;
	
}

/*
ULONG NVMeApp::ReadFlashID()
{

    ULONG status = ERROR_SUCCESS;
    NVME_COMMAND nvm_cmd = { 0 };
    PUCHAR buffer;

    nvm_cmd.CDW0.OPC = 0x000000FC;
    nvm_cmd.u.GENERAL.CDW13 = 0x0050FFFF;


    nvm_cmd.CDW0.AsUlong = 0x000000C2;
    nvm_cmd.u.GENERAL.CDW10 = 0x00000800;
    nvm_cmd.u.GENERAL.CDW11 = 0x0;
    nvm_cmd.u.GENERAL.CDW12 = 0x00000040;
    nvm_cmd.u.GENERAL.CDW13 = 0x00000001;
    nvm_cmd.u.GENERAL.CDW14 = 0x0;
    nvm_cmd.u.GENERAL.CDW15 = 0x0;


    //ULONG buflength = FLASH_ID_DATA_LENGTH;

    buffer = (PUCHAR)malloc(FLASH_ID_DATA_LENGTH);

    if (buffer == NULL)
    {
        printf("\tPanicDump: allocate buffer failed, exit.\n");
        status = GetLastError();
        return status;
    }

    ZeroMemory(buffer, FLASH_ID_DATA_LENGTH);
    status = NVMePassThroughDataIn(m_pdHandle, &nvm_cmd, FLASH_ID_DATA_LENGTH, buffer);
    if(status == ERROR_SUCCESS)
    {
        PrintRawDataLog(buffer,0x800);
    }
    free(buffer);
    return status;
}*/

#if 0
ULONG NVMeApp::EventLog()
{

	ULONG status = 0;
	NVME_COMMAND nvm_cmd = { 0 };
	nvm_cmd.CDW0.AsUlong = 0x000900C2;
	nvm_cmd.u.GENERAL.CDW10 = 0x00001000;
	nvm_cmd.u.GENERAL.CDW11 = 0x0;
    nvm_cmd.u.GENERAL.CDW12 = 0x00000057;
	nvm_cmd.u.GENERAL.CDW13 = 0x0;
	nvm_cmd.u.GENERAL.CDW14 = 0x0;
	nvm_cmd.u.GENERAL.CDW15 = 0x0;

    PUCHAR buffer;
    ULONG buflength = EVENT_LOG_DATA_LENGTH;

    buffer = (PUCHAR)malloc(buflength);
	
    if (buffer == NULL)
	{
		printf("\tPanicDump: allocate buffer failed, exit.\n");
		status = GetLastError();
		return status;
	}
	
	ZeroMemory(buffer, buflength);
	status = NVMePassThrough(m_pdHandle, &nvm_cmd, buflength, buffer);

	if(status == ERROR_SUCCESS)
	{
        PrintRawDataLog(buffer,EVENT_LOG_DATA_LENGTH);
	}
	free(buffer);
	return status;
}

#endif


void NVMeApp::RefreshDrive()
{
    int i;
    int drvnum = 0;
    TCHAR   ucbuffer[256] = {0};
    HANDLE  drvHandle;
    //CHAR ssdname[100] = { 0 };

    for (i = 0; i < MAX_DRIVE_NUM; i++)
    {
        wsprintf(ucbuffer, (L"\\\\.\\PhysicalDrive%d"), i);
        drvHandle = CreateFile(ucbuffer,GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);

        if (drvHandle != INVALID_HANDLE_VALUE) 
		{
			char local_buffer[4096];			
			char serialNumber[64];
			char modelNumber[64];
			//char vendorId[64];
			char productRevision[64];

			
            memset(local_buffer, 0, sizeof(local_buffer));
            DeviceQueryProperty(drvHandle,(LPVOID)&local_buffer);

            STORAGE_DEVICE_DESCRIPTOR *descrip = (STORAGE_DEVICE_DESCRIPTOR *)&local_buffer;

			flipAndCodeBytes(local_buffer,descrip->ProductIdOffset,0, modelNumber);
			flipAndCodeBytes(local_buffer,descrip->ProductRevisionOffset,0, productRevision);
			flipAndCodeBytes(local_buffer,descrip->SerialNumberOffset,1, serialNumber);

			printf("\n**** STORAGE_DEVICE_DESCRIPTOR for drive %d ****\n"
				   "Product Id = [%s]\n"
				   "Product Revision = [%s]\n"
				   "Serial Number = [%s]\n",
				   i,modelNumber,productRevision,serialNumber);

            drvnum = drvnum + 1;
         }
		
         CloseHandle(drvHandle);
        }

    printf("\nThere are total %d physical drive in current system! \n", drvnum);
}
/*
void NVMeApp::PrintDataIdentify(PNVME_IDENTIFY_CONTROLLER_DATA idtfyData)
{
	printf("vid     : 0x%x\n", idtfyData->VID);
	printf( "ssvid   : 0x%x\n", idtfyData->SSVID);
	//printf( "sn      : %-.*S\n", (int)sizeof(idtfyData->SN), idtfyData->SN);
	//printf( "mn      : %-.*S\n", (int)sizeof(idtfyData->MN), idtfyData->MN);
	//printf( "fr      : %-.*S\n", (int)sizeof(idtfyData->FR), idtfyData->FR);
	printf( "sn      : %s\n",  idtfyData->SN);
	printf( "mn      : %s\n",  idtfyData->MN);
	printf( "fr      : %s\n",  idtfyData->FR);
	printf( "rab     : %d\n", idtfyData->RAB);
	printf( "ieee    : 0x%02x%02x%02x\n",
    idtfyData->IEEE[2], idtfyData->IEEE[1], idtfyData->IEEE[0]);
	printf( "cmic    : 0x%x\n", idtfyData->CMIC);

	printf( "mdts    : 0x%x\n", idtfyData->MDTS);
	printf( "cntlid  : 0x%x\n", idtfyData->CNTLID);
	printf( "ver     : 0x%I32x\n", idtfyData->VER);
	printf( "rtd3r   : 0x%I32x\n", idtfyData->RTD3R);
	printf( "rtd3e   : 0x%I32x\n", idtfyData->RTD3E);
    printf( "oaes    : 0x%x\n", idtfyData->OAES.S.NamespaceAttributeChanged);
	printf( "oacs    : 0x%x\n", idtfyData->OACS);
	printf( "acl     : 0x%x\n", idtfyData->ACL);
	printf( "aerl    : 0x%x\n", idtfyData->AERL);
	printf( "frmw    : 0x%x\n", idtfyData->FRMW);
	printf( "lpa     : 0x%x\n", idtfyData->LPA);
	printf( "elpe    : 0x%x\n", idtfyData->ELPE);
	printf( "npss    : 0x%x\n", idtfyData->NPSS);
	printf( "avscc   : 0x%x\n", idtfyData->AVSCC);

	printf( "apsta   : 0x%x\n", idtfyData->APSTA);

	printf( "wctemp  : 0x%x\n", idtfyData->WCTEMP);
	printf( "cctemp  : 0x%x\n", idtfyData->CCTEMP);
	printf( "mtfa    : 0x%x\n", idtfyData->MTFA);
	printf( "hmpre   : 0x%I32x\n", idtfyData->HMPRE);
	printf( "hmmin   : 0x%x\n", idtfyData->HMMIN);
    printf( "tnvmcap : 0x");
	for (int i = 0; i<16; i++)
        printf( "%x", idtfyData->TNVMCAP[i]);

    printf( "\nunvmcap : 0x");
	for (int i = 0; i<16; i++)
        printf( "%x", idtfyData->UNVMCAP[i]);

	printf( "\nrpmbs   : 0x%I32x\n", idtfyData->RPMBS);

	printf( "sqes    : 0x%x\n", idtfyData->SQES);

	printf( "cqes    : 0x%x\n", idtfyData->CQES);

	printf( "nn      : 0x%I32x\n", idtfyData->NN);
	printf( "oncs    : 0x%x\n", idtfyData->ONCS);
	printf( "fuses   : 0x%x\n", idtfyData->FUSES);
	printf( "fna     : 0x%x\n", idtfyData->FNA);
	printf( "vwc     : 0x%x\n", idtfyData->VWC);
	printf( "awun    : 0x%x\n", idtfyData->AWUN);
	printf( "awupf   : 0x%x\n", idtfyData->AWUPF);
	printf( "nvscc   : 0x%x\n", idtfyData->NVSCC);
	printf( "acwu    : 0x%x\n", idtfyData->ACWU);
	printf( "sgls    : 0x%I32x\n", idtfyData->SGLS);

}

void NVMeApp::PrintSmartLog(PNVME_HEALTH_INFO_LOG smartInfo)
{
	if(smartInfo==nullptr)
	{
		return;
	}

	BYTE RawValue[16];
	int i;

	//ULONG temperature = 0;

	printf( "\n\tNVMe - Info from Health/Smart Log:\n");

	ZeroMemory(&RawValue, sizeof(RawValue));
	RawValue[0] = smartInfo->CriticalWarning.AsUchar;
	printf("\t Critical Warning:\t\t %02X %02X %02X %02X %02X %02X\n",
						RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

	ZeroMemory(&RawValue, sizeof(RawValue));
	RawValue[0] = smartInfo->Temperature[0];
	RawValue[1] = smartInfo->Temperature[1];
	printf("\t Composite Temperature:\t\t %02X %02X %02X %02X %02X %02X\n",
						RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

	ZeroMemory(&RawValue, sizeof(RawValue));
	RawValue[0] = smartInfo->AvailableSpare;
	printf("\t Available Spare:\t\t %02X %02X %02X %02X %02X %02X\n",
						RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

	ZeroMemory(&RawValue, sizeof(RawValue));
	RawValue[0] = smartInfo->AvailableSpareThreshold;
	printf("\t Available Spare Threshold:\t %02X %02X %02X %02X %02X %02X\n",
						RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

	ZeroMemory(&RawValue, sizeof(RawValue));
	RawValue[0] = smartInfo->PercentageUsed;
	printf("\t Endurance Percentage Used:\t %02X %02X %02X %02X %02X %02X\n",
						RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

						
	ZeroMemory(&RawValue, sizeof(RawValue));
	for(i=15;i>=0;i--)
	{
		RawValue[i] = (UCHAR)smartInfo->DataUnitRead[i];
	}

	printf("\t DataUnitRead: \t\t\t %02X %02X %02X %02X %02X %02X\n",							               
	          //RawValue[15],RawValue[14],RawValue[13],RawValue[12]
	          //RawValue[11],RawValue[10],RawValue[9],RawValue[8],RawValue[7],RawValue[6],
	          RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);


	ZeroMemory(&RawValue, sizeof(RawValue));
	for(i=15;i>=0;i--)
	{
		RawValue[i] = (UCHAR)smartInfo->DataUnitWritten[i];
	}
	printf("\t DataUnitWritten: \t\t %02X %02X %02X %02X %02X %02X\n",							               
	          RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

	ZeroMemory(&RawValue, sizeof(RawValue));
	for(i=15;i>=0;i--)
	{
		RawValue[i] = (UCHAR)smartInfo->HostReadCommands[i];			
	}
	printf("\t HostReadCommands: \t\t %02X %02X %02X %02X %02X %02X\n",							               
	          RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

	ZeroMemory(&RawValue, sizeof(RawValue));
	for(i=15;i>=0;i--)
	{
		RawValue[i] = (UCHAR)smartInfo->HostWrittenCommands[i];			
	}
	printf("\t HostWrittenCommands: \t\t %02X %02X %02X %02X %02X %02X\n",							               
	          RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

	ZeroMemory(&RawValue, sizeof(RawValue));
	for(i=15;i>=0;i--)
	{
		RawValue[i] = (UCHAR)smartInfo->ControllerBusyTime[i];			
	}
	printf("\t ControllerBusyTime: \t\t %02X %02X %02X %02X %02X %02X\n",							               
	          RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

	ZeroMemory(&RawValue, sizeof(RawValue));
	for(i=15;i>=0;i--)
	{
		RawValue[i] = (UCHAR)smartInfo->PowerCycle[i];			
	}
	printf("\t PowerCycle: \t\t\t %02X %02X %02X %02X %02X %02X\n",							               
	          RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

	ZeroMemory(&RawValue, sizeof(RawValue));
	for(i=15;i>=0;i--)
	{
		RawValue[i] = (UCHAR)smartInfo->PowerOnHours[i];			
	}
	printf("\t PowerOnHours: \t\t\t %02X %02X %02X %02X %02X %02X\n",							               
	          RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

			  
	ZeroMemory(&RawValue, sizeof(RawValue));
	for(i=15;i>=0;i--)
	{
		RawValue[i] = (UCHAR)smartInfo->UnsafeShutdowns[i];			
	}
	printf("\t UnsafeShutdowns: \t\t %02X %02X %02X %02X %02X %02X\n",							               
	          RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

	ZeroMemory(&RawValue, sizeof(RawValue));
	for(i=15;i>=0;i--)
	{
		RawValue[i] = (UCHAR)smartInfo->MediaErrors[i];			
	}
	printf("\t MediaErrors: \t\t\t %02X %02X %02X %02X %02X %02X\n",							               
	          RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);

	ZeroMemory(&RawValue, sizeof(RawValue));
	for(i=15;i>=0;i--)
	{
		RawValue[i] = (UCHAR)smartInfo->ErrorInfoLogEntryCount[i];			
	}
	printf("\t ErrorInfoLogEntryCount: \t %02X %02X %02X %02X %02X %02X\n",							               
	          RawValue[5],RawValue[4],RawValue[3],RawValue[2],RawValue[1],RawValue[0]);
			  
}

#if 0
ULONG NVMeApp::ReadLbainfo()
{
	ULONG status = 0;
	status= NVMeReadViaSCSIPassThrough(m_pdHandle,NULL);

	return status;
}
#endif
*/
void NVMeApp::PrintRawDataLog(PUCHAR data, int len)
{
    printf("\t00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    UCHAR buf[16];

    for(int i=0; i<len;++i)
	{
        if(i%16 == 0)
        {  
           printf("\n");
           printf("%6x",i);
        }

        printf(" %2x",*(data+i));
		if((*(data+i)>=0x21)&&(*(data+i)<=0x7e))
		{
			buf[i%16] = *(data+i);
		}	
		else
		{
			buf[i%16] = 0x2e;
		}
		
        if(i%16 == 15)
        {
            printf("\t");
            for(int j=0;j<16;++j)
                printf("%c",buf[j]);
        }
	}
    printf("\n");

	return;
}

//For UI Data Display
#if 0
ULONG NVMeApp::Identify(PUCHAR backBuffer)
{
    PUCHAR buffer;
    ULONG buflength = IDENTIFY_DATA_LENGTH;
    buffer = (PUCHAR)malloc(buflength);
    ULONG status = ERROR_SUCCESS;

    status = NVMeIdentifyQueryProperty(m_pdHandle, (LPVOID)buffer);
    if(status == ERROR_SUCCESS)
    {
        memcpy(backBuffer,buffer,buflength);
    }

    free(buffer);
    return status;
}
#endif

ULONG NVMeApp::SmartInfo(PUCHAR backBuffer)
{
    PUCHAR buffer;    

    ULONG status = ERROR_SUCCESS;
    buffer =(PUCHAR)malloc(SMART_DATA_LENGTH);

    status = NVMeLogPageQueryProperty(m_pdHandle, NVME_LOG_PAGE_HEALTH_INFO, sizeof(NVME_HEALTH_INFO_LOG), buffer);
    if(status == ERROR_SUCCESS)
    {
        memcpy(backBuffer,buffer,SMART_DATA_LENGTH);
    }

    free(buffer);
    return status;
}


ULONG NVMeApp::ReadFlashID(PUCHAR backBuffer)
{
    ULONG status = ERROR_SUCCESS;
    NVME_COMMAND nvm_cmd = { 0 };

    nvm_cmd.CDW0.OPC = 0x000000FC;
    nvm_cmd.u.GENERAL.CDW10 = 0x0;
    nvm_cmd.u.GENERAL.CDW11 = 0x0;
    nvm_cmd.u.GENERAL.CDW12 = 0x0;
    nvm_cmd.u.GENERAL.CDW13 = 0x50FFFF;
    nvm_cmd.u.GENERAL.CDW14 = 0x0;
    nvm_cmd.u.GENERAL.CDW15 = 0x0;

    status =NVMePassThroughNonData(m_pdHandle, &nvm_cmd);
    if(status != ERROR_SUCCESS)
    {
        status = GetLastError();
        return status;
    }

    ZeroMemory(&nvm_cmd, sizeof(NVME_COMMAND));
    PUCHAR buffer;

    buffer = (PUCHAR)malloc(FLASH_ID_DATA_LENGTH);

    if (buffer == NULL)
    {
        status = GetLastError();
        return status;
    }

    nvm_cmd.CDW0.OPC = 0x000000FA;
    nvm_cmd.u.GENERAL.CDW10 = 0x80;       //NDT
    nvm_cmd.u.GENERAL.CDW11 = 0x0;
    nvm_cmd.u.GENERAL.CDW12 = 0xa200a020; //virtual address low
    nvm_cmd.u.GENERAL.CDW13 = 0x100;      //subcode
    nvm_cmd.u.GENERAL.CDW14 = 0x0;
    nvm_cmd.u.GENERAL.CDW15 = 0x0;

    ZeroMemory(buffer, FLASH_ID_DATA_LENGTH);
    status = NVMePassThroughDataIn(m_pdHandle, &nvm_cmd, FLASH_ID_DATA_LENGTH, buffer);
    if(status == ERROR_SUCCESS)
    {
        memcpy(backBuffer,buffer,FLASH_ID_DATA_LENGTH);
        PrintRawDataLog(buffer, FLASH_ID_DATA_LENGTH);
    }
    free(buffer);
    return status;
}
#if 0
ULONG NVMeApp::EventLog(PUCHAR backBuffer)
{
    ULONG status = 0;
    NVME_COMMAND nvm_cmd = { 0 };
    nvm_cmd.CDW0.AsUlong = 0x000900C2;
    nvm_cmd.u.GENERAL.CDW10 = 0x00001000;
    nvm_cmd.u.GENERAL.CDW11 = 0x0;
    nvm_cmd.u.GENERAL.CDW12 = 0x00000057;
    nvm_cmd.u.GENERAL.CDW13 = 0x0;
    nvm_cmd.u.GENERAL.CDW14 = 0x0;
    nvm_cmd.u.GENERAL.CDW15 = 0x0;

    PUCHAR buffer;
    ULONG buflength = EVENT_LOG_DATA_LENGTH;

    buffer = (PUCHAR)malloc(buflength);

    if (buffer == NULL)
    {
        status = GetLastError();
        return status;
    }

    ZeroMemory(buffer, buflength);
    status = NVMePassThrough(m_pdHandle, &nvm_cmd, buflength, buffer);

    if(status == ERROR_SUCCESS)
    {
        memcpy(backBuffer,buffer,buflength);
    }
    free(buffer);
    return status;
}

//test code
ULONG NVMeApp::WriteLbainfo()
{
	ULONG status =0;
    status = NVMeWriteViaSCSIPassThrough(m_pdHandle,NULL);

	return status;
}

ULONG NVMeApp::ReadLbainfo(PUCHAR backBuffer)
{
	PUCHAR buffer;
	ULONG buflength = LBA_INFO_LENGTH; //512

	ULONG status = ERROR_SUCCESS;
	buffer = (PUCHAR)malloc(buflength);
	
	status= NVMeReadViaSCSIPassThrough(m_pdHandle,buffer);
	if(status == ERROR_SUCCESS)
	{
		memcpy(backBuffer,buffer,buflength);
	}
	
	free(buffer);
	return status;
}
#endif

ULONG NVMeApp::IDTableNamespaceInfo(PUCHAR backBuffer)
{
    PUCHAR buffer;
    ULONG buflength = IDY_NAMESPACE_INFO_LENGTH;
    buffer = (PUCHAR)malloc(buflength);
    ULONG status = ERROR_SUCCESS;

    status = NVMeIdentifyNamespaceQueryProperty(m_pdHandle, (LPVOID)buffer);
    if(status == ERROR_SUCCESS)
    {
        memcpy(backBuffer,buffer,buflength);
    }

    free(buffer);
    return status;
}

ULONG NVMeApp::IDTableControllerInfo(PUCHAR backBuffer)
{
    PUCHAR buffer;
    ULONG buflength = IDY_CONTROLLER_INFO_LENGTH;
    buffer = (PUCHAR)malloc(buflength);
    ULONG status = ERROR_SUCCESS;

    status = NVMeIdentifyQueryProperty(m_pdHandle, (LPVOID)buffer);
    if(status == ERROR_SUCCESS)
    {
        memcpy(backBuffer,buffer,buflength);
    }

    free(buffer);
    return status;
}

