/**
 * File: CmdInterface.cpp
**/ 

#include "CmdInterface.h"
#include "NvmeApp.h"
#include "stdio.h"
#include "stdlib.h"

char Cmdstr[256] = {0};
char Cmd_substr[7][50] = {0};


CmdInterface* CmdInterface::m_s_pInstance(nullptr);

CmdInterface::CmdInterface()
{
	
}

CmdInterface* CmdInterface::GetInstance()
{
	if(nullptr == m_s_pInstance)
	{
		m_s_pInstance = new CmdInterface();
	}
	return m_s_pInstance;
}

void CmdInterface::ParseCmd()
{
	int length =0;
	int num = 0;
	char *p,*p_next;

	p = strtok_s(Cmdstr," :",&p_next);
	while(p)
	{
		if(num >=7)
		{
			printf("error!! input para less than 7\n");
		}
        //printf("cmd string %d is %s \n", num,p);
		strcpy_s(Cmd_substr[num], p);

		num = num + 1;
		p= strtok_s(NULL, " :", &p_next);
	}
    //printf("input totla para num is %d\n", num);
	for(int i=0; i< num; i++)
	{
        ;//printf("Cmd substr %d is %s \n", i, Cmd_substr[i]);
	}

}

void CmdInterface::PrintHelp()
{
	printf("input command : help\n");
	printf("\n>>>>>>>>>>>>>>>>>>>>>> Answer As Below <<<<<<<<<<<<<<<<<<<\n\n");
    printf("Command : show_ssd\n\teg:  show_ssd\n");
    printf("Command : identify\n\teg:  identify drive:1\n");
    printf("Command : smart\n\teg:  smart drive:1\n");
    printf("Command : readflashid\n\teg:  readflashid drive:1\n");
	printf("Command : eventlog\n\teg:  eventlog drive:1\n");
	printf("Command : readlba\n\teg:  readlba drive:1\n");
    printf("Command : writelba\n\teg:  writelba drive:1\n");

	return;
}

void CmdInterface::DumpIdentify()
{
	int driveid = atoi(Cmd_substr[2]);
	if(0 != strcmp("drive",Cmd_substr[1]))
	{
		printf("input para error !");
		return;
	}

	NVMeApp nvmeapp(driveid);
    nvmeapp.Identify();
    return;
}

void CmdInterface::DumpSmart()
{
    int driveid = atoi(Cmd_substr[2]);
    if(0 !=strcmp("drive",Cmd_substr[1]))
    {
        printf("input para error !");
        return;
    }

    NVMeApp nvmeapp(driveid);
    nvmeapp.SmartInfo();
    return;
}

#if 0
void CmdInterface::DumpFlashId()
{
    int driveid = atoi(Cmd_substr[2]);
    if(0 !=strcmp("drive",Cmd_substr[1]))
    {
        printf("input para error !");
        return;
    }

    NVMeApp nvmeapp(driveid);
    PUCHAR *buffer;
    ULONG buflength = 0xFFF;
    buffer = (PUCHAR*)malloc(buflength);
    nvmeapp.ReadFlashID();
	return;
}

void CmdInterface::DumpEventLog()
{
    int driveid = atoi(Cmd_substr[2]);
    if(0 !=strcmp("drive",Cmd_substr[1]))
    {
        printf("input para error !");
        return;
    }

    NVMeApp nvmeapp(driveid);
    nvmeapp.EventLog();
	return;
}

void CmdInterface::ReadLbaInfo()
{
    int driveid = atoi(Cmd_substr[2]);
    if(0 !=strcmp("drive",Cmd_substr[1]))
    {
        printf("input para error !");
        return;
    }

	NVMeApp nvmeapp(driveid);
	nvmeapp.ReadLbainfo();
	return;
}

void CmdInterface::WriteLbaInfo()
{
    int driveid = atoi(Cmd_substr[2]);
    if(0 !=strcmp("drive",Cmd_substr[1]))
    {
        printf("input para error !");
        return;
    }

	NVMeApp nvmeapp(driveid);
	nvmeapp.WriteLbainfo();
	return;
}
#endif

void CmdInterface::Init()
{
	while(TRUE)
	{
		printf("-----------------------------------------------------------------------------------------------\n");
        //printf("Note: Number of the input command fields less than 7 and totla length less than 255 characters.\n");
		printf("ToolBoxCmd: ");

		memset(Cmdstr, 256, 0);
		memset(Cmd_substr, 350, 0);
		//ZeroMemory(Cmd_substr, 250);

        //gets_s(Cmdstr, 254);
        gets(Cmdstr);
		
		ParseCmd();

		if (0 == strcmp(Cmd_substr[0], "show_ssd"))
		{
			printf("input command : show_ssd\n");
			printf("\n>>>>>>>>>>>>>>>>>>>>>> Answer As Below <<<<<<<<<<<<<<<<<<<\n\n");
			//ListNvmeSsd();
			continue;
		}
		else if (0 == strcmp(Cmd_substr[0], "identify"))
		{
            printf("input command : identify\n");
			printf("\n>>>>>>>>>>>>>>>>>>>>>> Answer As Below <<<<<<<<<<<<<<<<<<<\n\n");
            DumpIdentify();
			continue;
		}
        else if(0 == strcmp(Cmd_substr[0], "smart"))
        {
            printf("input command : smart\n");
            printf("\n>>>>>>>>>>>>>>>>>>>>>> Answer As Below <<<<<<<<<<<<<<<<<<<\n\n");
            DumpSmart();
            continue;
        }
		else if(0 == strcmp(Cmd_substr[0], "readflashid"))
		{
			printf("input command : readflashid\n");
			printf("\n>>>>>>>>>>>>>>>>>>>>>> Answer As Below <<<<<<<<<<<<<<<<<<<\n\n");
            //DumpFlashId();
			continue;
		}
		else if(0 == strcmp(Cmd_substr[0], "eventlog"))
		{
			printf("input command : EventLog\n");
			printf("\n>>>>>>>>>>>>>>>>>>>>>> Answer As Below <<<<<<<<<<<<<<<<<<<\n\n");
            //DumpEventLog();
			continue;
		}
		else if(0 == strcmp(Cmd_substr[0], "readlba"))
		{
            printf("input command : Readlba");
			printf("\n>>>>>>>>>>>>>>>>>>>>>> Answer As Below <<<<<<<<<<<<<<<<<<<\n\n");
            //ReadLbaInfo();
			continue;
		}
		else if(0 == strcmp(Cmd_substr[0], "writelba")) 
		{
            printf("input command : Writelba");
			printf("\n>>>>>>>>>>>>>>>>>>>>>> Answer As Below <<<<<<<<<<<<<<<<<<<\n\n");
            //WriteLbaInfo();
			continue;

		}
		else if (0 == strcmp(Cmd_substr[0], "help"))
		{
			if(0 == strcmp(Cmd_substr[1], "show_ssd"))
			{
                printf("Command: show_ssd\n\teg:	show_ssd\n");
			}
			else if(0== strcmp(Cmd_substr[1], "identify"))
			{
                printf("Command: identify\n\teg:	identify drive:[drive number]\n");
			}
            else if(0 == strcmp(Cmd_substr[1], "smart"))
            {
                printf("Command: smart\n\teg:	smart drive:[drive number]\n");
            }
			else if(0== strcmp(Cmd_substr[1], "readflashid"))
			{
                printf("Command: identify\n\teg:	readflashid drive:[drive number]\n");
			}
            else if(0 == strcmp(Cmd_substr[1], "eventlog"))
            {
                printf("Command: eventlog\n\teg:	eventlog drive:[drive number]\n");
            }
            else if(0 == strcmp(Cmd_substr[1], "readlba"))
            {
                printf("Command: readlba\n\teg:	readlba drive:[drive number]\n");
            }
            else if(0 == strcmp(Cmd_substr[1], "writelba"))
            {
                printf("Command: writelba\n\teg:	writelba drive:[drive number]\n");
            }
            continue;
		}
        else if((0 == strcmp(Cmd_substr[0],"?"))||(0 == strcmp(Cmd_substr[0],"help")))
        {
            PrintHelp();
            continue;
        }
		else
		{
			printf("!!! input error, please check input command. \n");
			PrintHelp();
            continue;
		}

	}

	
}

void CmdInterface::ShowAllDrive()
{
    printf("All physical drive in current system as below:\n");
    printf("-----------------------------------------------------------------------------------------------\n");

    NVMeApp driveapp;
    driveapp.RefreshDrive();

}




