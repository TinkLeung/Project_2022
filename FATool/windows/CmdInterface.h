/**
 * File: CmdInterface.h
**/ 

#ifndef CMD_INTERFACE_H
#define CMD_INTERFACE_H

class CmdInterface
{
	public:
		CmdInterface();

//
//	Operations
//
	public:
		void Init();
		void ShowAllDrive();
        static CmdInterface* GetInstance();

	private:
		void PrintHelp();
        void ParseCmd();
        void DumpIdentify();
        void DumpSmart();
		void DumpFlashId();
		void DumpEventLog();
		void ReadLbaInfo();
		void WriteLbaInfo();
		
	private:
		static CmdInterface *m_s_pInstance;
		
};





#endif


