/*---------------------------------------------------------------------------*/
//       Author : Minkyu Kim
//          Web : http://naraeon.net/
//                https://github.com/ebangin127/
//      License : The MIT License
/*---------------------------------------------------------------------------*/

//#include "stdafx.h"
#include <windows.h>
#include "SlotSpeedGetter.h"
//#include <comdef.h>
//#include <ntddkbd.h>
#include "winioctl.h"
#include <setupapi.h>
#include <cfgmgr32.h>
//#include <oleauto.h>
//#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "setupapi.lib")
//#pragma comment(lib, "wbemuuid.lib")
//#pragma comment(lib, "comsuppw.lib")
//#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#include <QMessageBox>

DEFINE_GUID(GUID_DEVCLASS_SCSIADAPTER, 0x4D36E97B, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18);


typedef BOOL(WINAPI *FN_SetupDiGetDeviceProperty)(
    _In_       HDEVINFO DeviceInfoSet,
    _In_       PSP_DEVINFO_DATA DeviceInfoData,
    _In_       const DEVPROPKEY *PropertyKey,
    _Out_      DEVPROPTYPE *PropertyType,
	__out_opt  PBYTE PropertyBuffer,
    _In_       DWORD PropertyBufferSize,
	__out_opt  PDWORD RequiredSize,
    _In_       DWORD Flags
	);
#if 0
QString GetStringValueFromQuery(IWbemServices* pIWbemServices, const QString query)
{
	IEnumWbemClassObject* pEnumCOMDevs = NULL;
	IWbemClassObject* pCOMDev = NULL;
	ULONG uReturned = 0;
    QString	result = "";
    std::string strTmp ="";

	try
	{
        strTmp = query.toStdString();
        if (SUCCEEDED(pIWbemServices->ExecQuery(_bstr_t(L"WQL"),
            _bstr_t(strTmp.c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs)))
		{
			while (pEnumCOMDevs && SUCCEEDED(pEnumCOMDevs->Next(10000, 1, &pCOMDev, &uReturned)) && uReturned == 1)
			{
				VARIANT pVal;
				VariantInit(&pVal);

				if (pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
				{
                    //result = pVal.bstrVal;
                    //char *ptr =const_cast <char*>(_com_util::ConvertBSTRToString(pVal.bstrVal));
                    //strTmp = (_bstr_t)pVal.bstrVal;
                    result = QString::fromStdString(strTmp);
                    //result = QString(ptr);
                    //result = QString::fromWCharArray(pVal.bstrVal);
                    //result = QString::fromUtf16(pVal.bstrVal);
					VariantClear(&pVal);
				}
				VariantInit(&pVal);
			}
		}
	}
	catch (...)
	{
        result = "";
	}

	SAFE_RELEASE(pCOMDev);
	SAFE_RELEASE(pEnumCOMDevs);
	return result;
}

QString GetDeviceIDFromPhysicalDriveID(const INT physicalDriveId, const BOOL IsKernelVerEqualOrOver6)
{
    const QString query2findstorage = "ASSOCIATORS OF {Win32_DiskDrive.DeviceID='\\\\.\\PhysicalDrive%d'} WHERE ResultClass=Win32_PnPEntity";
    const QString query2findcontroller = "ASSOCIATORS OF {Win32_PnPEntity.DeviceID='%s'} WHERE AssocClass=Win32_SCSIControllerDevice";
    QString query;
    QString result;
	IWbemLocator* pIWbemLocator = NULL;
	IWbemServices* pIWbemServices = NULL;
	//BOOL flag = FALSE;

	try
	{
		if (SUCCEEDED(CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
			IID_IWbemLocator, (LPVOID *)&pIWbemLocator)))
		{
			long securityFlag = 0;
			if (IsKernelVerEqualOrOver6) { securityFlag = WBEM_FLAG_CONNECT_USE_MAX_WAIT; }
            if (SUCCEEDED(pIWbemLocator->ConnectServer(_bstr_t(L"\\\\.\\root\\cimv2"),
				NULL, NULL, 0L, securityFlag, NULL, NULL, &pIWbemServices)))
			{
				if (SUCCEEDED(CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
					NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE)))
				{
                    query = QString(query2findstorage).arg(physicalDriveId);
                    //query.sprintf(query2findstorage, physicalDriveId);
                    QString StorageID = GetStringValueFromQuery(pIWbemServices, query);
                    if(!StorageID.isNull())
					{
                        query = QString(query2findcontroller).arg(StorageID);
                        //query.Format(query2findcontroller.GetString(), StorageID.GetString());
                        QString ControllerID = GetStringValueFromQuery(pIWbemServices, query);
						result = ControllerID;
					}
					else
					{
                        result = "";
					}
				}
			}
		}
	}
	catch (...)
	{
        result = "";
	}
	SAFE_RELEASE(pIWbemServices);
	SAFE_RELEASE(pIWbemLocator);
	return result;
}
#endif
QString GetDeviceIDFromPhysicalDriveID(const INT physicalDriveId)
{
    TCHAR szBuff[512] = {0};
    DWORD devIdx = 0;
    QString strDevID ="";
    DWORD GetDeviceResult = 0;

    GUID *pDiskGUID = (GUID*)&GUID_DEVINTERFACE_DISK;
    HDEVINFO hDevInfo = SetupDiGetClassDevs(pDiskGUID, NULL, NULL, DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);
    if(INVALID_HANDLE_VALUE == hDevInfo)
        return strDevID;
    BYTE bBuff[1024] = {0};
    PSP_DEVICE_INTERFACE_DETAIL_DATA pspDevDelInfo = (PSP_DEVICE_INTERFACE_DETAIL_DATA)bBuff;
    SP_DEVICE_INTERFACE_DATA spDevDelInfo = {0};
    SP_DEVINFO_DATA spDevInfo = {0};
    spDevDelInfo.cbSize = sizeof (spDevDelInfo);

    while(TRUE)
    {
        BOOL bRet = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, pDiskGUID, devIdx, &spDevDelInfo);
        if(!bRet)
            break;
        DWORD dwSize = 0;
        SetupDiGetDeviceInterfaceDetail(hDevInfo, &spDevDelInfo, NULL, 0, &dwSize, NULL);
        if(dwSize != 0 && dwSize <= sizeof (bBuff))
        {
            pspDevDelInfo->cbSize = sizeof (*pspDevDelInfo);
            ZeroMemory((PVOID)&spDevInfo, sizeof (spDevInfo));
            spDevInfo.cbSize = sizeof (spDevInfo);
            bRet = SetupDiGetDeviceInterfaceDetail(hDevInfo, &spDevDelInfo, pspDevDelInfo, dwSize, &dwSize, &spDevInfo);
            if(bRet)
            {
                HANDLE hDrive = CreateFile(pspDevDelInfo->DevicePath, GENERIC_READ | GENERIC_WRITE,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           NULL,
                                           OPEN_EXISTING,
                                           FILE_ATTRIBUTE_NORMAL,
                                           NULL);
                if(INVALID_HANDLE_VALUE != hDrive)
                {
                    STORAGE_DEVICE_NUMBER storDevNum;
                    DWORD dwBytesReturn = 0;
                    bRet = DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &storDevNum, sizeof (storDevNum), &dwBytesReturn, NULL);
                    if(bRet)
                    {
                        if(physicalDriveId == (INT)storDevNum.DeviceNumber)
                        {
                            CloseHandle(hDrive);
                            SetupDiDestroyDeviceInfoList(hDevInfo);
                            GetDeviceResult = CM_Get_Device_ID(spDevInfo.DevInst, szBuff, 512, 0);
                            if(GetDeviceResult)
                            strDevID = QString::fromWCharArray(szBuff);
                        }
                     }
                    CloseHandle(hDrive);
                  }
               }
            }
        devIdx++;
     }
    SetupDiDestroyDeviceInfoList(hDevInfo);

    return strDevID;
}


/*SlotMaxCurrSpeed ConvertOSResult(const OSSlotMaxCurrSpeed OSLevelResult)
{
	SlotMaxCurrSpeed result{};
	result.Current.LinkWidth = PCIeDataWidth(OSLevelResult.Current.LinkWidth);
	result.Current.SpecVersion = PCIeSpecification(OSLevelResult.Current.SpecVersion);
	result.Maximum.LinkWidth = PCIeDataWidth(OSLevelResult.Maximum.LinkWidth);
	result.Maximum.SpecVersion = PCIeSpecification(OSLevelResult.Maximum.SpecVersion);
	return result;
}*/

//SlotMaxCurrSpeed GetSlotMaxCurrSpeedFromDeviceID(const QString DeviceID)
OSSlotMaxCurrSpeed GetSlotMaxCurrSpeedFromDeviceID(const QString DeviceID)
{
	DWORD CurrentDevice = 0;
	OSSlotMaxCurrSpeed OSLevelResult{};
	GUID SCSIAdapterGUID = GUID_DEVCLASS_SCSIADAPTER;
	HDEVINFO ClassDeviceInformations = SetupDiGetClassDevs(&SCSIAdapterGUID, nullptr, 0, DIGCF_PRESENT);

	BOOL LastResult{};
	do
	{
		SP_DEVINFO_DATA DeviceInfoData = {sizeof(SP_DEVINFO_DATA)};
		LastResult = SetupDiEnumDeviceInfo(ClassDeviceInformations, CurrentDevice, &DeviceInfoData);

		BOOL DeviceIDFound = FALSE;	
		TCHAR DeviceIDBuffer[MAX_PATH] = {};
		BOOL GetDeviceResult = CM_Get_Device_ID(DeviceInfoData.DevInst, DeviceIDBuffer, MAX_PATH, 0);
        DeviceIDFound = (GetDeviceResult == ERROR_SUCCESS) && (DeviceID.compare(DeviceIDBuffer) == 0);
		if (LastResult && DeviceIDFound)
		{
			DEVPROPTYPE PropertyType{};
			BYTE ResultBuffer[1024]{};
			DWORD RequiredSize{};

			const HMODULE hMod = LoadLibrary(TEXT("Setupapi.dll"));
			if (hMod && hMod != INVALID_HANDLE_VALUE) {
				FN_SetupDiGetDeviceProperty SetupDiGetDeviceProperty =
					(FN_SetupDiGetDeviceProperty)GetProcAddress(hMod, "SetupDiGetDevicePropertyW");
				//Compatible with pre-vista era windows.
				if (SetupDiGetDeviceProperty) {
					SetupDiGetDeviceProperty(ClassDeviceInformations, &DeviceInfoData,
						&DEVPKEY_PciDevice_MaxLinkWidth, &PropertyType, ResultBuffer,
						sizeof(ResultBuffer), &RequiredSize, 0);
					OSLevelResult.Maximum.LinkWidth = ResultBuffer[0];
					SetupDiGetDeviceProperty(ClassDeviceInformations, &DeviceInfoData,
						&DEVPKEY_PciDevice_MaxLinkSpeed, &PropertyType, ResultBuffer,
						sizeof(ResultBuffer), &RequiredSize, 0);
					OSLevelResult.Maximum.SpecVersion = ResultBuffer[0];

					SetupDiGetDeviceProperty(ClassDeviceInformations, &DeviceInfoData,
						&DEVPKEY_PciDevice_CurrentLinkWidth, &PropertyType, ResultBuffer,
						sizeof(ResultBuffer), &RequiredSize, 0);
					OSLevelResult.Current.LinkWidth = ResultBuffer[0];
					SetupDiGetDeviceProperty(ClassDeviceInformations, &DeviceInfoData,
						&DEVPKEY_PciDevice_CurrentLinkSpeed, &PropertyType, ResultBuffer,
						sizeof(ResultBuffer), &RequiredSize, 0);
					OSLevelResult.Current.SpecVersion = ResultBuffer[0];
				}
			}
            //QMessageBox::warning(nullptr, "Get Link Statu Error", "Load SetupAPI.dll Fail!!!");
            printf("Load SetupAPI.dll Fail!!\n");
			break;
		}

		++CurrentDevice;
	} while (LastResult);

    //return ConvertOSResult(OSLevelResult);
    return OSLevelResult;
}

OSSlotMaxCurrSpeed GetPCIeSlotSpeed(const INT physicalDriveId)
{
    QString DeviceID = GetDeviceIDFromPhysicalDriveID(physicalDriveId);
	return GetSlotMaxCurrSpeedFromDeviceID(DeviceID);
}

#if 0
QString SlotSpeedToString(SlotSpeed speedtoconv)
{
    QString result = "";
	if (speedtoconv.SpecVersion == 0 || speedtoconv.LinkWidth == 0)
	{
        result = ("----");
	}
	else
	{
        result.sprintf("PCIe %d.0 x%d", speedtoconv.SpecVersion, speedtoconv.LinkWidth);
	}
	return result;
}
#endif
