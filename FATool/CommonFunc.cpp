#include "CommonFunc.h"
#ifndef WIN32
#include "linux/nvme_cpp.h"
#else
#include "windows/Nvmeioctl.h"
#endif



uint128b_t le128_to_cpu(unsigned char *data)
{
    uint128b_t tmp;
    uint128b_t u;
    memcpy(tmp.bytes,data,16);
    u.words[0] = tmp.words[3];
    u.words[1] = tmp.words[2];
    u.words[2] = tmp.words[1];
    u.words[3] = tmp.words[0];

    return u;
}

char *uint128b_to_string(uint128b_t val)
{
    static char str[40];
    int idx = 40;
    uint64_t div,rem;
    str[--idx] = '\0';
    do{
        rem = val.words[0];

        div = rem/10;
        rem = ((rem -div *10) << 32) + val.words[1];
        val.words[0] = div;

        div = rem/10;
        rem = ((rem -div *10) << 32) + val.words[2];
        val.words[1] = div;

        div = rem/10;
        rem = ((rem -div *10) << 32) + val.words[3];
        val.words[2] = div;

        div = rem /10;
        rem = rem - div*10;
        val.words[3] = div;

        str[--idx] = '0'+rem;

    }while(val.words[0] || val.words[1] || val.words[2] || val.words[3]);

    return str+idx;
}

char *flipAndCodeBytes(const char * str, int pos, int flip, char * buf)
{
    int i;
    int j = 0;
    int k = 0;

    buf[0] = '\0';
    if (pos <= 0)
        return buf;

    if (!j)
    {
        char p = 0;

        // First try to gather all characters representing hex digits only.
        j = 1;
        k = 0;
        buf[k] = 0;
        for (i = pos; j && str[i] != '\0'; ++i)
        {
            char c = tolower(str[i]);

            if (isspace(c))
                c = '0';

            ++p;
            buf[k] <<= 4;

            if (c >= '0' && c <= '9')
                buf[k] |= (unsigned char)(c - '0');
            else if (c >= 'a' && c <= 'f')
                buf[k] |= (unsigned char)(c - 'a' + 10);
            else
            {
                j = 0;
                break;
            }

            if (p == 2)
            {
                if ((unsigned char)buf[k] != '\0' && !isprint((unsigned char)buf[k]))
                {
                    j = 0;
                    break;
                }
                ++k;
                p = 0;
                buf[k] = 0;
            }

        }
    }

    if (!j)
    {
        // There are non-digit characters, gather them as is.
        j = 1;
        k = 0;
        for (i = pos; j && str[i] != '\0'; ++i)
        {
            char c = str[i];

            if (!isprint(c))
            {
                j = 0;
                break;
            }

            buf[k++] = c;
        }
    }

    if (!j)
    {
        // The characters are not there or are not printable.
        k = 0;
    }

    buf[k] = '\0';

    if (flip)
        // Flip adjacent characters
        for (j = 0; j < k; j += 2)
        {
            char t = buf[j];
            buf[j] = buf[j + 1];
            buf[j + 1] = t;
        }

    // Trim any beginning and end space
    i = j = -1;
    for (k = 0; buf[k] != '\0'; ++k)
    {
        if (!isspace(buf[k]))
        {
            if (i < 0)
                i = k;
            j = k;
        }
    }

    if ((i >= 0) && (j >= 0))
    {
        for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
            buf[k - i] = buf[k];
        buf[k - i] = '\0';
    }

    return buf;
}

long double int128_to_double(unsigned char *data)
{
    long double result = 0;
    for (int i = 0; i<16; i++)
    {
        result *= 256;
        result += data[15-i];
    }
    return result;
}

int char_to_int(char data)
{
    int ret =0;
    if(data >= '0' && data <= '9')
    {
        ret = data -'0';
    }
    else if(data >='a' && data <= 'z' )
    {
        ret = data -'a' +10;
    }
    else if(data >='A' && data <= 'Z')
    {
        ret = data -'A' +10;
    }
    return ret;
}


QList<SPEC_ITEM>* ParseSmartData(unsigned char *data)
{
    QString dataStr;
    QList<SPEC_ITEM> *itemList = new QList<SPEC_ITEM> ();
    SPEC_ITEM item;
    uint16_t temp;


#ifdef WIN32
     PNVME_HEALTH_INFO_LOG SmartData = reinterpret_cast<PNVME_HEALTH_INFO_LOG>(data);
     item.Description = "Critical Warning";
     item.RawValue = dataStr.sprintf("%#x ",SmartData->CriticalWarning.AsUchar);
     itemList->append(item);

     temp = SmartData->Temperature[1]<< 8|SmartData->Temperature[0];
     item.Description = "Composite Temperature";
     item.RawValue = dataStr.sprintf("%d ℃(%u Kelvin) ",temp-273, temp);
     itemList->append(item);

     item.Description = "Available Spare";
     item.RawValue = dataStr.sprintf("%u %%", SmartData->AvailableSpare);
     itemList->append(item);

     item.Description = "Available Spare Threshold";
     item.RawValue = dataStr.sprintf("%u %%", SmartData->AvailableSpareThreshold);
     itemList->append(item);

     item.Description = "Percentage Used";
     item.RawValue = dataStr.sprintf("%u %%", SmartData->PercentageUsed);
     itemList->append(item);

     /*item.Description = "Endurance Group Critical Warning Summary";
     item.RawValue = dataStr.sprintf("%#x", SmartData->);
     itemList->append(item);*/

     item.Description = "Data Units Read";

     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->DataUnitRead)));
     itemList->append(item);

     item.Description = "Data Units Write";

     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->DataUnitWritten)));
     itemList->append(item);

     item.Description = "Host Read Commands";

     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->HostReadCommands)));
     itemList->append(item);

     item.Description = "Host Write Commands";

     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->HostWrittenCommands)));
     itemList->append(item);

     item.Description = "Controller Busy Time";

     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->ControllerBusyTime)));
     itemList->append(item);

     item.Description = "Power Cycles";

     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->PowerCycle)));
     itemList->append(item);

     item.Description = "Power On Hours";

     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->PowerOnHours)));
     itemList->append(item);

     item.Description = "Unsafe Shutdowns";
     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->UnsafeShutdowns)));
     itemList->append(item);

     item.Description = "Media and Data Integrity Errors";

     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->MediaErrors)));
     itemList->append(item);

     item.Description = "Number of Error Information Log Entries";

     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->ErrorInfoLogEntryCount)));
     itemList->append(item);

     item.Description = "Warning Composite Temperature Time";
     item.RawValue = dataStr.sprintf("%u",SmartData->WarningCompositeTemperatureTime);
     itemList->append(item);

     item.Description = "Critical Composite Temperature Time";
     item.RawValue = dataStr.sprintf("%u",SmartData->CriticalCompositeTemperatureTime);
     itemList->append(item);

     temp = SmartData->TemperatureSensor1;
     if(temp != 0)
     {
         item.Description =dataStr.sprintf("Temperature Sensor %d",1);
         item.RawValue = dataStr.sprintf("%d ℃ (%u Kelvin) ",temp-273, temp);
         itemList->append(item);
     }
     temp = SmartData->TemperatureSensor2;
     if(temp != 0)
     {
         item.Description =dataStr.sprintf("Temperature Sensor %d",2);
         item.RawValue = dataStr.sprintf("%d ℃ (%u Kelvin) ",temp-273, temp);
         itemList->append(item);
     }
     temp = SmartData->TemperatureSensor3;
     if(temp != 0)
     {
         item.Description =dataStr.sprintf("Temperature Sensor %d",3);
         item.RawValue = dataStr.sprintf("%d ℃ (%u Kelvin) ",temp-273, temp);
         itemList->append(item);
     }
     temp = SmartData->TemperatureSensor4;
     if(temp != 0)
     {
         item.Description =dataStr.sprintf("Temperature Sensor %d",4);
         item.RawValue = dataStr.sprintf("%d ℃ (%u Kelvin) ",temp-273, temp);
         itemList->append(item);
     }
     temp = SmartData->TemperatureSensor5;
     if(temp != 0)
     {
         item.Description =dataStr.sprintf("Temperature Sensor %d",5);
         item.RawValue = dataStr.sprintf("%d ℃ (%u Kelvin) ",temp-273, temp);
         itemList->append(item);
     }
     temp = SmartData->TemperatureSensor6;
     if(temp != 0)
     {
         item.Description =dataStr.sprintf("Temperature Sensor %d",6);
         item.RawValue = dataStr.sprintf("%d ℃ (%u Kelvin) ",temp-273, temp);
         itemList->append(item);
     }
     temp = SmartData->TemperatureSensor7;
     if(temp != 0)
     {
         item.Description =dataStr.sprintf("Temperature Sensor %d",7);
         item.RawValue = dataStr.sprintf("%d ℃ (%u Kelvin) ",temp-273, temp);
         itemList->append(item);
     }
     temp = SmartData->TemperatureSensor8;
     if(temp != 0)
     {
         item.Description =dataStr.sprintf("Temperature Sensor %d",8);
         item.RawValue = dataStr.sprintf("%d ℃ (%u Kelvin) ",temp-273, temp);
         itemList->append(item);
     }

     /*item.Description = "Thermal Management Temperature 1 Transition Count";
     item.RawValue =dataStr.sprintf("%u",SmartData->);
     itemList->append(item);

     item.Description = "Thermal Management Temperature 2 Transition Count";
     item.RawValue =dataStr.sprintf("%u",SmartData->thm_temp2_trans_count);
     itemList->append(item);

     item.Description = "Total Time For Thermal Management Temperature 1";
     item.RawValue =dataStr.sprintf("%u",SmartData->thm_temp1_total_time);
     itemList->append(item);

     item.Description = "Total Time For Thermal Management Temperature 2";
     item.RawValue =dataStr.sprintf("%u",SmartData->thm_temp2_total_time);
     itemList->append(item);*/
#else
    struct nvme_smart_log *SmartData = reinterpret_cast<struct nvme_smart_log*>(data);


    //fmtStr = "********************************SMART / Health Information Log********************************\r\n\n\n";

    item.Description = "Critical Warning";
    item.RawValue = dataStr.sprintf("%#x ",SmartData->critical_warning);
    itemList->append(item);

    temp = SmartData->temperature[1] << 8|SmartData->temperature[0];
    item.Description = "Composite Temperature";
    item.RawValue = dataStr.sprintf("%d ℃(%u Kelvin) ",temp-273, temp);
    itemList->append(item);

    item.Description = "Available Spare";
    item.RawValue = dataStr.sprintf("%u %%", SmartData->avail_spare);
    itemList->append(item);

    item.Description = "Available Spare Threshold";
    item.RawValue = dataStr.sprintf("%u %%", SmartData->spare_thresh);
    itemList->append(item);

    item.Description = "Percentage Used";
    item.RawValue = dataStr.sprintf("%u %%", SmartData->percent_used);
    itemList->append(item);    

    item.Description = "Endurance Group Critical Warning Summary";
    item.RawValue = dataStr.sprintf("%#x", SmartData->endu_grp_crit_warn_sumry);
    itemList->append(item);   

    item.Description = "Data Units Read";

    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->data_units_read)));
    itemList->append(item);

    item.Description = "Data Units Write";

    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->data_units_written)));
    itemList->append(item);

    item.Description = "Host Read Commands";

    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->host_reads)));
    itemList->append(item);

    item.Description = "Host Write Commands";

    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->host_writes)));
    itemList->append(item);

    item.Description = "Controller Busy Time";

    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->ctrl_busy_time)));
    itemList->append(item);

    item.Description = "Power Cycles";

    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->power_cycles)));
    itemList->append(item);

    item.Description = "Power On Hours";

    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->power_on_hours)));
    itemList->append(item);

    item.Description = "Unsafe Shutdowns";
    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->unsafe_shutdowns)));
    itemList->append(item);

    item.Description = "Media and Data Integrity Errors";

    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->media_errors)));
    itemList->append(item);

    item.Description = "Number of Error Information Log Entries";

    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(SmartData->num_err_log_entries)));
    itemList->append(item);

    item.Description = "Warning Composite Temperature Time";
    item.RawValue = dataStr.sprintf("%u",SmartData->warning_temp_time);
    itemList->append(item);

    item.Description = "Critical Composite Temperature Time";
    item.RawValue = dataStr.sprintf("%u",SmartData->critical_comp_time);
    itemList->append(item);

    for (int c = 0; c< 8; c++)
    {
        temp = SmartData->temp_sensor[c];
        if(temp == 0)
            continue;
        item.Description =dataStr.sprintf("Temperature Sensor %d",c+1);
        item.RawValue = dataStr.sprintf("%d ℃ (%u Kelvin) ",temp-273, temp);
        itemList->append(item);
    }

    item.Description = "Thermal Management Temperature 1 Transition Count";
    item.RawValue =dataStr.sprintf("%u",SmartData->thm_temp1_trans_count);
    itemList->append(item);

    item.Description = "Thermal Management Temperature 2 Transition Count";
    item.RawValue =dataStr.sprintf("%u",SmartData->thm_temp2_trans_count);
    itemList->append(item);

    item.Description = "Total Time For Thermal Management Temperature 1";
    item.RawValue =dataStr.sprintf("%u",SmartData->thm_temp1_total_time);
    itemList->append(item);

    item.Description = "Total Time For Thermal Management Temperature 2";
    item.RawValue =dataStr.sprintf("%u",SmartData->thm_temp2_total_time);
    itemList->append(item);
#endif
    return itemList;
}

QList<SPEC_ITEM>* ParseIdyCtrlData(unsigned char *data)
{
    QString dataStr;    
    SPEC_ITEM item;
    QList<SPEC_ITEM> *itemList = new QList<SPEC_ITEM> ();

#ifdef WIN32
    PNVME_IDENTIFY_CONTROLLER_DATA IDCtrlData = reinterpret_cast<PNVME_IDENTIFY_CONTROLLER_DATA>(data);

    item.Description = "VID";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->VID);
    itemList->append(item);

    item.Description = "SSVID";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->SSVID);
    itemList->append(item);

    item.Description = "SN";
    item.RawValue = (dataStr.sprintf("%s",IDCtrlData->SN)).mid(0,20);
    itemList->append(item);

    item.Description = "MN";
    item.RawValue = (dataStr.sprintf("%s",IDCtrlData->MN)).mid(0,40);
    itemList->append(item);

    item.Description = "FR";
    item.RawValue = (dataStr.sprintf("%s",IDCtrlData->FR)).mid(0,8);
    itemList->append(item);

    item.Description = "RAB";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->RAB);
    itemList->append(item);

    item.Description = "IEEE";
    item.RawValue = dataStr.sprintf("%X",IDCtrlData->IEEE[2]<<16 |
                                         IDCtrlData->IEEE[1]<<8  |
                                         IDCtrlData->IEEE[0]);
    itemList->append(item);

    item.Description = "CMIC";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->CMIC.AsULong);
    itemList->append(item);

    item.Description = "MDTS";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->MDTS);
    itemList->append(item);

    item.Description = "CNTLID";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->CNTLID);
    itemList->append(item);

    item.Description = "VER";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->VER);
    itemList->append(item);

    item.Description = "RTD3R";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->RTD3R);
    itemList->append(item);

    item.Description = "RTD3E";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->RTD3E);
    itemList->append(item);

    item.Description = "OAES";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->OAES.AsULong);
    itemList->append(item);

    /*item.Description = "CTRATT";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData);
    itemList->append(item);

    item.Description = "RRLS";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->rrls);
    itemList->append(item);

    item.Description = "CNTRLTYPE";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->cntrltype);
    itemList->append(item);

    item.Description = "FGUID";
    item.RawValue = dataStr.sprintf("%s",IDCtrlData->fguid);//
    itemList->append(item);

    item.Description = "CRDT1";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->crdt1);
    itemList->append(item);

    item.Description = "CRDT2";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->crdt2);
    itemList->append(item);

    item.Description = "CRDT3";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->crdt3);
    itemList->append(item);*/

    item.Description = "OACS";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->OACS.AsUShort);
    itemList->append(item);

    item.Description = "ACL";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->ACL);
    itemList->append(item);

    item.Description = "AERL";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->AERL);
    itemList->append(item);

    item.Description = "FRMW";
    item.RawValue = dataStr.sprintf("%X",IDCtrlData->FRMW.AsUChar);//
    itemList->append(item);

    item.Description = "LPA";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->LPA.AsUChar);//
    itemList->append(item);

    item.Description = "ELPE";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->ELPE);
    itemList->append(item);

    item.Description = "NPSS";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->NPSS);
    itemList->append(item);

    item.Description = "AVSCC";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->AVSCC.AsUChar);
    itemList->append(item);

    item.Description = "APSTA";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->APSTA.AsUChar);
    itemList->append(item);

    item.Description = "WCTEMP";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->WCTEMP);
    itemList->append(item);

    item.Description = "CCTEMP";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->CCTEMP);
    itemList->append(item);

    item.Description = "MTFA";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->MTFA);
    itemList->append(item);

    item.Description = "HMPRE";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->HMPRE);
    itemList->append(item);

    item.Description = "HMMIN";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->HMMIN);
    itemList->append(item);

    item.Description = "TNVMCAP";

    item.RawValue = dataStr.sprintf("%s", uint128b_to_string(le128_to_cpu(IDCtrlData->TNVMCAP)));//
    itemList->append(item);

    item.Description = "UNVMCAP";

    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(IDCtrlData->UNVMCAP)));//
    itemList->append(item);

    item.Description = "RPMBS";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->RPMBS.AsULong);
    itemList->append(item);

    /*item.Description = "EDSTT";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->EDSTT);
    itemList->append(item);

    item.Description = "DSTO";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->dsto);
    itemList->append(item);

    item.Description = "FWUG";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->fwug);
    itemList->append(item);

    item.Description = "KAS";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->kas);
    itemList->append(item);

    item.Description = "HCTMA";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->hctma);
    itemList->append(item);

    item.Description = "MNTMT";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->mntmt);
    itemList->append(item);

    item.Description = "MXTMT";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->mxtmt);
    itemList->append(item);

    item.Description = "SANICAP";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->sanicap);
    itemList->append(item);

    item.Description = "HMMINDS";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->hmminds);
    itemList->append(item);

    item.Description = "HMMAXD";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->hmmaxd);
    itemList->append(item);

    item.Description = "NSETIDMAX";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->nsetidmax);
    itemList->append(item);

    item.Description = "ENDGIDMAX";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->endgidmax);
    itemList->append(item);

    item.Description = "ANATT";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->anatt);
    itemList->append(item);

    item.Description = "ANACAP";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->anacap);
    itemList->append(item);

    item.Description = "ANAGRPMAX";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->anagrpmax);
    itemList->append(item);

    item.Description = "NANAGRPID";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->nanagrpid);
    itemList->append(item);

    item.Description = "PELS";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->pels);
    itemList->append(item);*/

    item.Description = "SQES";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->SQES.AsUChar);
    itemList->append(item);

    item.Description = "CQES";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->CQES.AsUChar);
    itemList->append(item);

    item.Description = "MAXCMD";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->MAXCMD);
    itemList->append(item);

    item.Description = "NN";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->NN);
    itemList->append(item);

    item.Description = "ONCS";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->ONCS.AsUShort);
    itemList->append(item);

    item.Description = "FUSES";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->FUSES.AsUShort);
    itemList->append(item);

    item.Description = "FNA";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->FNA.AsUChar);
    itemList->append(item);

    item.Description = "VWC";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->VWC.AsUChar);
    itemList->append(item);

    item.Description = "AWUN";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->AWUN);
    itemList->append(item);

    item.Description = "AWUPF";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->AWUPF);
    itemList->append(item);

    item.Description = "NVSCC";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->NVSCC.AsUChar);
    itemList->append(item);

    item.Description = "NWPC";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->NWPC);
    itemList->append(item);

    item.Description = "ACWU";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->ACWU);
    itemList->append(item);

    item.Description = "SGLS";
    item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->SGLS.ASUlong);
    itemList->append(item);

    item.Description = "MNAN";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->MNAN);
    itemList->append(item);

    item.Description = "SUBNQN";
    item.RawValue = (dataStr.sprintf("%s",IDCtrlData->SUBNQN)).mid(0,256);
    itemList->append(item);

    /*item.Description = "IOCCSZ";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->ioccsz);
    itemList->append(item);

    item.Description = "IORCSZ";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->iorcsz);
    itemList->append(item);

    item.Description = "ICDOFF";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->icdoff);
    itemList->append(item);

    item.Description = "FCATT";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->ctrattr);
    itemList->append(item);

    item.Description = "MSDBD";
    item.RawValue = dataStr.sprintf("%d",IDCtrlData->msdbd);
    itemList->append(item);*/

    for(int idx = 0; idx <= IDCtrlData->NPSS;idx++)
    {
       double Scale;
       double Power;
       item.Description = dataStr.sprintf("PS %d:",idx);
       Scale = IDCtrlData->PDS[idx].MPS ? (0.0001):(0.01);//Max Power Scale
       Power = Scale * IDCtrlData->PDS[idx].MP;
       item.RawValue = dataStr.sprintf("  mp:%0.2f W",Power);
       dataStr = IDCtrlData->PDS[idx].NOPS?("  Non-Operational"):("  Operational");
       item.RawValue += dataStr;
       item.RawValue += dataStr.sprintf("  enlat:%d", IDCtrlData->PDS[idx].ENLAT);
       item.RawValue += dataStr.sprintf("  exlat:%d", IDCtrlData->PDS[idx].EXLAT);
       item.RawValue += dataStr.sprintf("  rrt:%d",IDCtrlData->PDS[idx].RRT);
       item.RawValue += dataStr.sprintf("  rrl:%d\n",IDCtrlData->PDS[idx].RRL);
       item.RawValue += dataStr.sprintf("  rwt:%d",IDCtrlData->PDS[idx].RWT);
       item.RawValue += dataStr.sprintf("  rwl:%d",IDCtrlData->PDS[idx].RWL);
       if(IDCtrlData->PDS[idx].IDLP == 0)
       {
           item.RawValue +="  idle power:-";
       }
       else
       {
           if(IDCtrlData->PDS[idx].IPS & BIT0) Scale = 0.0001;
           else if(IDCtrlData->PDS[idx].IPS & BIT1) Scale = 0.01;
           Power = Scale * IDCtrlData->PDS[idx].IDLP;
           item.RawValue += dataStr.sprintf("  idle power:%0.2f",Power);
       }
       if(IDCtrlData->PDS[idx].ACTP == 0)
       {
           item.RawValue += "  active power:-";
       }
       else
       {
           if(IDCtrlData->PDS[idx].APS & BIT0) Scale = 0.0001;
           else if(IDCtrlData->PDS[idx].APS & BIT1) Scale = 0.01;
           Power = Scale * IDCtrlData->PDS[idx].APW;
           item.RawValue += dataStr.sprintf("  active power:%0.2f",Power);
       }
       itemList->append(item);
    }

#else
     nvme_id_ctrl *IDCtrlData = reinterpret_cast<struct nvme_id_ctrl*>(data);

     item.Description = "VID";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->vid);
     itemList->append(item);

     item.Description = "SSVID";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->ssvid);
     itemList->append(item);

     item.Description = "SN";     
     item.RawValue = (dataStr.sprintf("%s",IDCtrlData->sn)).mid(0,20);
     itemList->append(item);

     item.Description = "MN";
     item.RawValue = (dataStr.sprintf("%s",IDCtrlData->mn)).mid(0,40);
     itemList->append(item);

     item.Description = "FR";
     item.RawValue = (dataStr.sprintf("%s",IDCtrlData->fr)).mid(0,8);
     itemList->append(item);

     item.Description = "RAB";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->rab);
     itemList->append(item);

     item.Description = "IEEE";
     item.RawValue = dataStr.sprintf("%X",IDCtrlData->ieee[2]<<16 |
                                          IDCtrlData->ieee[1]<<8  |
                                          IDCtrlData->ieee[0]);
     itemList->append(item);

     item.Description = "CMIC";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->cmic);
     itemList->append(item);

     item.Description = "MDTS";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->mdts);
     itemList->append(item);

     item.Description = "CNTLID";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->cntlid);
     itemList->append(item);

     item.Description = "VER";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->ver);
     itemList->append(item);

     item.Description = "RTD3R";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->rtd3r);
     itemList->append(item);

     item.Description = "RTD3E";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->rtd3e);
     itemList->append(item);

     item.Description = "OAES";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->oaes);
     itemList->append(item);

     item.Description = "CTRATT";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->ctratt);
     itemList->append(item);

     item.Description = "RRLS";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->rrls);
     itemList->append(item);

     item.Description = "CNTRLTYPE";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->cntrltype);
     itemList->append(item);

     item.Description = "FGUID";
     item.RawValue = dataStr.sprintf("%s",IDCtrlData->fguid);//
     itemList->append(item);

     item.Description = "CRDT1";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->crdt1);
     itemList->append(item);

     item.Description = "CRDT2";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->crdt2);
     itemList->append(item);

     item.Description = "CRDT3";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->crdt3);
     itemList->append(item);

     item.Description = "OACS";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->oacs);
     itemList->append(item);

     item.Description = "ACL";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->acl);
     itemList->append(item);

     item.Description = "AERL";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->aerl);
     itemList->append(item);

     item.Description = "FRMW";
     item.RawValue = dataStr.sprintf("%X",IDCtrlData->frmw);
     itemList->append(item);

     item.Description = "LPA";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->lpa);
     itemList->append(item);

     item.Description = "ELPE";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->elpe);
     itemList->append(item);

     item.Description = "NPSS";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->npss);
     itemList->append(item);

     item.Description = "AVSCC";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->avscc);
     itemList->append(item);

     item.Description = "APSTA";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->apsta);
     itemList->append(item);

     item.Description = "WCTEMP";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->wctemp);
     itemList->append(item);

     item.Description = "CCTEMP";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->cctemp);
     itemList->append(item);

     item.Description = "MTFA";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->mtfa);
     itemList->append(item);

     item.Description = "HMPRE";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->hmpre);
     itemList->append(item);

     item.Description = "HMMIN";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->hmmin);
     itemList->append(item);

     item.Description = "TNVMCAP";

     item.RawValue = dataStr.sprintf("%s", uint128b_to_string(le128_to_cpu(IDCtrlData->tnvmcap)));//
     itemList->append(item);

     item.Description = "UNVMCAP";

     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(IDCtrlData->unvmcap)));//
     itemList->append(item);

     item.Description = "RPMBS";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->rpmbs);
     itemList->append(item);

     item.Description = "EDSTT";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->edstt);
     itemList->append(item);

     item.Description = "DSTO";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->dsto);
     itemList->append(item);

     item.Description = "FWUG";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->fwug);
     itemList->append(item);

     item.Description = "KAS";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->kas);
     itemList->append(item);

     item.Description = "HCTMA";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->hctma);
     itemList->append(item);

     item.Description = "MNTMT";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->mntmt);
     itemList->append(item);

     item.Description = "MXTMT";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->mxtmt);
     itemList->append(item);

     item.Description = "SANICAP";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->sanicap);
     itemList->append(item);

     item.Description = "HMMINDS";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->hmminds);
     itemList->append(item);

     item.Description = "HMMAXD";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->hmmaxd);
     itemList->append(item);

     item.Description = "NSETIDMAX";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->nsetidmax);
     itemList->append(item);

     item.Description = "ENDGIDMAX";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->endgidmax);
     itemList->append(item);

     item.Description = "ANATT";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->anatt);
     itemList->append(item);

     item.Description = "ANACAP";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->anacap);
     itemList->append(item);

     item.Description = "ANAGRPMAX";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->anagrpmax);
     itemList->append(item);

     item.Description = "NANAGRPID";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->nanagrpid);
     itemList->append(item);

     item.Description = "PELS";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->pels);
     itemList->append(item);

     item.Description = "SQES";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->sqes);
     itemList->append(item);

     item.Description = "CQES";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->cqes);
     itemList->append(item);

     item.Description = "MAXCMD";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->maxcmd);
     itemList->append(item);

     item.Description = "NN";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->nn);
     itemList->append(item);

     item.Description = "ONCS";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->oncs);
     itemList->append(item);

     item.Description = "FUSES";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->fuses);
     itemList->append(item);

     item.Description = "FNA";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->fna);
     itemList->append(item);

     item.Description = "VWC";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->vwc);
     itemList->append(item);

     item.Description = "AWUN";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->awun);
     itemList->append(item);

     item.Description = "AWUPF";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->awupf);
     itemList->append(item);

     item.Description = "NVSCC";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->nvscc);
     itemList->append(item);

     item.Description = "NWPC";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->nwpc);
     itemList->append(item);

     item.Description = "ACWU";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->acwu);
     itemList->append(item);

     item.Description = "SGLS";
     item.RawValue = dataStr.sprintf("0x%X",IDCtrlData->sgls);
     itemList->append(item);

     item.Description = "MNAN";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->mnan);
     itemList->append(item);

     item.Description = "SUBNQN";
     item.RawValue = (dataStr.sprintf("%s",IDCtrlData->subnqn)).mid(0,256);
     itemList->append(item);

     item.Description = "IOCCSZ";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->ioccsz);
     itemList->append(item);

     item.Description = "IORCSZ";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->iorcsz);
     itemList->append(item);

     item.Description = "ICDOFF";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->icdoff);
     itemList->append(item);

     item.Description = "FCATT";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->ctrattr);
     itemList->append(item);

     item.Description = "MSDBD";
     item.RawValue = dataStr.sprintf("%d",IDCtrlData->msdbd);
     itemList->append(item);

     for(int idx = 0; idx <= IDCtrlData->npss;idx++)
     {
        double Scale = 0.0;
        double Power = 0.0;
        item.Description = dataStr.sprintf("PS %d:",idx);

        if(IDCtrlData->psd[idx].flags & BIT0)
        {
            Scale = 0.0001;
            Power = Scale * IDCtrlData->psd[idx].max_power;
            item.RawValue = dataStr.sprintf("  mp:%0.4f W",Power);
        }
        else {
            Scale = 0.01;
            Power = Scale * IDCtrlData->psd[idx].max_power;
            item.RawValue = dataStr.sprintf("  mp:%0.2f W",Power);
        }
        dataStr = IDCtrlData->psd[idx].flags & BIT1 ?("  Non-Operational"):("  Operational");
        item.RawValue += dataStr;
        item.RawValue += dataStr.sprintf("  enlat:%d", IDCtrlData->psd[idx].entry_lat);
        item.RawValue += dataStr.sprintf("  exlat:%d", IDCtrlData->psd[idx].exit_lat);
        item.RawValue += dataStr.sprintf("  rrt:%d",IDCtrlData->psd[idx].read_tput);
        item.RawValue += dataStr.sprintf("  rrl:%d",IDCtrlData->psd[idx].read_lat);
        item.RawValue += dataStr.sprintf("  rwt:%d",IDCtrlData->psd[idx].write_tput);
        item.RawValue += dataStr.sprintf("  rwl:%d",IDCtrlData->psd[idx].write_lat);
        if(IDCtrlData->psd[idx].idle_power == 0)
        {
            item.RawValue +="  idle power:-";
        }
        else
        {
            if(IDCtrlData->psd[idx].idle_scale & BIT0)
            {
                Scale = 0.0001;
                Power = Scale * IDCtrlData->psd[idx].idle_power;
                item.RawValue += dataStr.sprintf("  idle power:%0.4f",Power);
            }
            else if(IDCtrlData->psd[idx].idle_scale & BIT1)
            {
                Scale = 0.01;
                Power = Scale * IDCtrlData->psd[idx].idle_power;
                item.RawValue += dataStr.sprintf("  idle power:%0.2f",Power);
            }
        }
        if(IDCtrlData->psd[idx].active_power == 0)
        {
            item.RawValue += "  active power:-";
        }
        else
        {
            if(IDCtrlData->psd[idx].active_work_scale & BIT0)
            {
                Scale = 0.0001;
                Power = Scale * IDCtrlData->psd[idx].active_power;
                item.RawValue += dataStr.sprintf("  active power:%0.4f",Power);
            }
            else if(IDCtrlData->psd[idx].active_work_scale & BIT1)
            {
                Scale = 0.01;
                Power = Scale * IDCtrlData->psd[idx].active_power;
                item.RawValue += dataStr.sprintf("  active power:%0.2f",Power);
            }
        }
        itemList->append(item);
     }
#endif
    return itemList;
}


QList<SPEC_ITEM>* ParseIdyCtrNSlData(unsigned char *data)
{
    QString dataStr;
    SPEC_ITEM item;
    int idx =0;
    QList<SPEC_ITEM> *itemList = new QList<SPEC_ITEM> ();

#ifdef WIN32
     PNVME_IDENTIFY_NAMESPACE_DATA IDNsData = reinterpret_cast<PNVME_IDENTIFY_NAMESPACE_DATA>(data);

     item.Description = "NVMe Identify Namespace 1:";
     item.RawValue = " ";
     itemList->append(item);

     item.Description = "NSZE";
     item.RawValue = dataStr.sprintf("%#llx",IDNsData->NSZE);
     itemList->append(item);

     item.Description = "NCAP";
     item.RawValue = dataStr.sprintf("%#llx",IDNsData->NCAP);
     itemList->append(item);

     item.Description = "NUSE";
     item.RawValue = dataStr.sprintf("%#llx",IDNsData->NUSE);
     itemList->append(item);

     item.Description = "NSFEAT";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NSFEAT.AsUChar);//
     itemList->append(item);

     item.Description = "NLBAF";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NLBAF);
     itemList->append(item);

     item.Description = "FLBAS";
     item.RawValue = dataStr.sprintf("%d",IDNsData->FLBAS.AsUChar);//
     itemList->append(item);

     item.Description = "MC";
     item.RawValue = dataStr.sprintf("%d",IDNsData->MC.AsUChar);//
     itemList->append(item);

     item.Description = "DPC";
     item.RawValue = dataStr.sprintf("%d",IDNsData->DPC.AsUChar);
     itemList->append(item);

     item.Description = "DPS";
     item.RawValue = dataStr.sprintf("%d",IDNsData->DPS.AsUChar);
     itemList->append(item);

     item.Description = "NMIC";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NMIC.AsUChar);
     itemList->append(item);

     item.Description = "RESSCAP";
     item.RawValue = dataStr.sprintf("%d",IDNsData->RESCAP.AsUChar);
     itemList->append(item);

     item.Description = "FPI";
     item.RawValue = dataStr.sprintf("%d",IDNsData->FPI.AsUChar);
     itemList->append(item);

     item.Description = "DLFEAT";
     item.RawValue = dataStr.sprintf("%d",IDNsData->DLFEAT);
     itemList->append(item);

     item.Description = "NAWUN";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NAWUN);
     itemList->append(item);

     item.Description = "NAWUPF";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NAWUPF);
     itemList->append(item);

     item.Description = "NACWU";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NACWU);
     itemList->append(item);

     item.Description = "NABSN";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NABSN);
     itemList->append(item);

     item.Description = "NABO";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NABO);
     itemList->append(item);

     item.Description = "NABSPF";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NABSPF);
     itemList->append(item);

     item.Description = "NOIOB";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NOIOB);
     itemList->append(item);

     item.Description = "NVMCAP";
     item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(IDNsData->NVMCAP)));
     itemList->append(item);

     item.Description = "NPWG";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NPWG);
     itemList->append(item);

     item.Description = "NPWA";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NPWA);
     itemList->append(item);

     item.Description = "NPDA";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NPDA);
     itemList->append(item);

     item.Description = "NOWS";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NOWS);
     itemList->append(item);

     item.Description = "ANAGRPID";
     item.RawValue = dataStr.sprintf("%d",IDNsData->ANAGRPID);
     itemList->append(item);

     item.Description = "NSATTR";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NSATTR);
     itemList->append(item);

     item.Description = "NVMSETID";
     item.RawValue = dataStr.sprintf("%d",IDNsData->NVMSETID);
     itemList->append(item);

     item.Description = "ENDGID";
     item.RawValue = dataStr.sprintf("%d",IDNsData->ENDGID);
     itemList->append(item);

     item.Description = "NGUID";
     item.RawValue = "";
     for (idx = 0; idx<16 ;idx++)
     {
         item.RawValue += dataStr.sprintf("%02x",IDNsData->NGUID[idx]);
     }
     itemList->append(item);

     item.Description = "EUI64";
    item.RawValue = "";
     for (idx = 0; idx<8 ;idx++)
     {
         item.RawValue += dataStr.sprintf("%02x",IDNsData->EUI64[idx]);
     }
     itemList->append(item);

     for (int idx = 0; idx <= IDNsData->NLBAF; idx++)
     {
         item.Description = dataStr.sprintf("lbaf %d :", idx);
         dataStr.sprintf("ms: %d   lbads:%d   rp:%d",IDNsData->LBAF[idx].MS,IDNsData->LBAF[idx].LBADS,IDNsData->LBAF[idx].RP);
         item.RawValue = dataStr;
         itemList->append(item);
     }
#else
    nvme_id_ns *IDNsData = reinterpret_cast<nvme_id_ns*>(data);

    item.Description = "NVMe Identify Namespace 1:";
    item.RawValue = " ";
    itemList->append(item);

    item.Description = "NSZE";
    item.RawValue = dataStr.sprintf("%#llx",IDNsData->nsze);
    itemList->append(item);

    item.Description = "NCAP";
    item.RawValue = dataStr.sprintf("%#llx",IDNsData->ncap);
    itemList->append(item);

    item.Description = "NUSE";
    item.RawValue = dataStr.sprintf("%#llx",IDNsData->nuse);
    itemList->append(item);

    item.Description = "NSFEAT";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nsfeat);
    itemList->append(item);

    item.Description = "NLBAF";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nlbaf);
    itemList->append(item);

    item.Description = "FLBAS";
    item.RawValue = dataStr.sprintf("%d",IDNsData->flbas);
    itemList->append(item);

    item.Description = "MC";
    item.RawValue = dataStr.sprintf("%d",IDNsData->mc);
    itemList->append(item);

    item.Description = "DPC";
    item.RawValue = dataStr.sprintf("%d",IDNsData->dpc);
    itemList->append(item);

    item.Description = "DPS";
    item.RawValue = dataStr.sprintf("%d",IDNsData->dps);
    itemList->append(item);

    item.Description = "NMIC";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nmic);
    itemList->append(item);

    item.Description = "RESSCAP";
    item.RawValue = dataStr.sprintf("%d",IDNsData->rescap);
    itemList->append(item);

    item.Description = "FPI";
    item.RawValue = dataStr.sprintf("%d",IDNsData->fpi);
    itemList->append(item);

    item.Description = "DLFEAT";
    item.RawValue = dataStr.sprintf("%d",IDNsData->dlfeat);
    itemList->append(item);

    item.Description = "NAWUN";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nawun);
    itemList->append(item);

    item.Description = "NAWUPF";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nawupf);
    itemList->append(item);

    item.Description = "NACWU";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nacwu);
    itemList->append(item);

    item.Description = "NABSN";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nabsn);
    itemList->append(item);

    item.Description = "NABO";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nabo);
    itemList->append(item);

    item.Description = "NABSPF";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nabspf);
    itemList->append(item);

    item.Description = "NOIOB";
    item.RawValue = dataStr.sprintf("%d",IDNsData->noiob);
    itemList->append(item);

    item.Description = "NVMCAP";
    item.RawValue = dataStr.sprintf("%s",uint128b_to_string(le128_to_cpu(IDNsData->nvmcap)));
    itemList->append(item);

    item.Description = "NPWG";
    item.RawValue = dataStr.sprintf("%d",IDNsData->npwg);
    itemList->append(item);

    item.Description = "NPWA";
    item.RawValue = dataStr.sprintf("%d",IDNsData->npwa);
    itemList->append(item);

    item.Description = "NPDA";
    item.RawValue = dataStr.sprintf("%d",IDNsData->npda);
    itemList->append(item);

    item.Description = "NOWS";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nows);
    itemList->append(item);

    item.Description = "ANAGRPID";
    item.RawValue = dataStr.sprintf("%d",IDNsData->anagrpid);
    itemList->append(item);

    item.Description = "NSATTR";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nsattr);
    itemList->append(item);

    item.Description = "NVMSETID";
    item.RawValue = dataStr.sprintf("%d",IDNsData->nvmsetid);
    itemList->append(item);

    item.Description = "ENDGID";
    item.RawValue = dataStr.sprintf("%d",IDNsData->endgid);
    itemList->append(item);

    item.Description = "NGUID";
    item.RawValue = "";
    for (idx = 0; idx<16 ;idx++)
    {
        item.RawValue += dataStr.sprintf("%02x",IDNsData->nguid[idx]);
    }
    itemList->append(item);

    item.Description = "EUI64";
   item.RawValue = "";
    for (idx = 0; idx<8 ;idx++)
    {
        item.RawValue += dataStr.sprintf("%02x",IDNsData->eui64[idx]);
    }
    itemList->append(item);

    for (int idx = 0; idx <= IDNsData->nlbaf; idx++)
    {
        item.Description = dataStr.sprintf("lbaf %d :", idx);
        dataStr.sprintf("ms: %d   lbads:%d   rp:%d",IDNsData->lbaf[idx].ms,IDNsData->lbaf[idx].ds,IDNsData->lbaf[idx].rp);
        item.RawValue = dataStr;
        itemList->append(item);
    }
#endif
    return itemList;
}

QList<SPEC_ITEM>* ParseErrorLog(unsigned char *data, int entries)
{
    QString dataStr;
    QString trType;
    SPEC_ITEM item;
    QList<SPEC_ITEM> *itemList = new QList<SPEC_ITEM> ();

#ifdef WIN32
    PNVME_ERROR_INFO_LOG *ErrorLogData = reinterpret_cast<PNVME_ERROR_INFO_LOG*>(data);
    if(entries == 0)
    {
        item.Description = ("No Error");
        item.RawValue = "";
        itemList->append(item);
        return itemList;
    }

    for(int i=0; i<entries; i++)
    {
        item.Description = dataStr.sprintf("Error Entries  %x:", i+1);
        item.RawValue = "";
        itemList->append(item);

        item.Description = "Error Count";
        item.RawValue = dataStr.sprintf("%llu", ErrorLogData[i]->ErrorCount);
        itemList->append(item);

        item.Description = "Submission Queue ID";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i]->SQID);
        itemList->append(item);

        item.Description = "Command ID";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i]->CMDID);
        itemList->append(item);

        item.Description = "Status Field";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i]->Status.AsUshort);
        itemList->append(item);

        item.Description = "Phase Tag";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i]->Status.P);
        itemList->append(item);

        item.Description = "Parameter Error Location";
        item.RawValue = dataStr.sprintf("Byte:%x,Bit:%x", ErrorLogData[i]->ParameterErrorLocation.Byte, ErrorLogData[i]->ParameterErrorLocation.Bit);//
        itemList->append(item);

        item.Description = "LBA";
        item.RawValue = dataStr.sprintf("%#llx", ErrorLogData[i]->Lba);
        itemList->append(item);

        item.Description = "Namespace";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i]->NameSpace);
        itemList->append(item);

        item.Description = "Vendor Specific";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i]->VendorInfoAvailable);
        itemList->append(item);

        item.Description = "Transport Type";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i]->Reserved0[0]);
        itemList->append(item);

        item.Description = "Command Specific Info";
        item.RawValue = dataStr.sprintf("%#llx", ErrorLogData[i]->CommandSpecificInfo);
        itemList->append(item);

        trType = dataStr.sprintf("%d",ErrorLogData[i]->Reserved1[0]<<8|ErrorLogData[i]->Reserved1[0]);
        if(trType.toInt() == 0x3)
        {
            item.Description = "Transport Type Specific Info";
            item.RawValue = dataStr.sprintf("%x", ErrorLogData[i]->Reserved1[0]<<8|ErrorLogData[i]->Reserved1[0]);
            itemList->append(item);
        }
    }

#else
    nvme_error_log_page *ErrorLogData = reinterpret_cast<nvme_error_log_page*>(data);

    if(entries == 0)
    {
        item.Description = ("No Error");
        item.RawValue = "";
        itemList->append(item);
        return itemList;
    }

    for(int i=0; i<entries; i++)
    {
        item.Description = dataStr.sprintf("Error Entries  %x:", i+1);
        item.RawValue = "";
        itemList->append(item);

        item.Description = "Error Count";
        item.RawValue = dataStr.sprintf("%llu", ErrorLogData[i].error_count);
        itemList->append(item);

        item.Description = "Submission Queue ID";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i].sqid);
        itemList->append(item);

        item.Description = "Command ID";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i].cmdid);
        itemList->append(item);

        item.Description = "Status Field";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i].status_field);
        itemList->append(item);

        item.Description = "Phase Tag";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i].status_field&0x1);
        itemList->append(item);

        item.Description = "Parameter Error Location";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i].parm_error_location);
        itemList->append(item);

        item.Description = "LBA";
        item.RawValue = dataStr.sprintf("%#llx", ErrorLogData[i].lba);
        itemList->append(item);

        item.Description = "Namespace";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i].nsid);
        itemList->append(item);

        item.Description = "Vendor Specific";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i].vs);
        itemList->append(item);

        item.Description = "Transport Type";
        item.RawValue = dataStr.sprintf("%x", ErrorLogData[i].trtype);
        itemList->append(item);

        item.Description = "Command Specific Info";
        item.RawValue = dataStr.sprintf("%#llx", ErrorLogData[i].cs);
        itemList->append(item);

        trType = dataStr.sprintf("%d",ErrorLogData[i].trtype);
        if(trType.toInt() == 0x3)
        {
            item.Description = "Transport Type Specific Info";
            item.RawValue = dataStr.sprintf("%x", ErrorLogData[i].trtype_spec_info);
            itemList->append(item);
        }
    }
#endif
    return itemList;
}
