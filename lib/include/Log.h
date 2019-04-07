#pragma once

#include "File.h"

#define LOGMODE_CONSOLE		0x01
#define LOGMODE_FILE					0x02
#define LOGMODE_DEBUGGER	0x04

enum class ELogType {
	eLogType_Info, 
	eLogType_Warning,
	eLogType_Error
};
enum class ELogLevel {
	eLogLv_Low,
	eLogLv_Medium,
	eLogLv_High
};
enum class ELogFlag {
	eLogFlag_Critical,
	eLogFlag_Normal,
	eLogFlag_Trivial
};

class DLL_EXPORT CLog {
public:
	CLog() : m_eLogLv(ELogLevel::eLogLv_Medium), m_nLogMode(0) {}
	inline void SetLogLevel(ELogLevel eLogLv) { m_eLogLv = eLogLv; }
	inline void SetLogToConsole(bool b) {
		b ? (BIT_ADD(m_nLogMode, LOGMODE_CONSOLE)) : (BIT_REMOVE(m_nLogMode, LOGMODE_CONSOLE));
	}
	inline void SetLogToFile(bool b, const char *pszLogFileName) {
		b ? (BIT_ADD(m_nLogMode, LOGMODE_FILE)) : (BIT_REMOVE(m_nLogMode, LOGMODE_FILE));
		if (b)
			m_LogFile.Open(pszLogFileName, "w+");
		else {
			if (m_LogFile.IsOpen()) {
				m_LogFile.Flush();
				m_LogFile.Close();
			}
		}
	}
	inline void SetLogToDebugger(bool b) {
		b ? (BIT_ADD(m_nLogMode, LOGMODE_DEBUGGER)) : (BIT_REMOVE(m_nLogMode, LOGMODE_DEBUGGER));
	}
	inline bool IsLogToConsole() const {
		return BIT_CHECK(m_nLogMode, LOGMODE_CONSOLE);
	}
	inline bool IsLogToFile() const {
		return BIT_CHECK(m_nLogMode, LOGMODE_FILE);
	}
	inline bool IsLogToDebugger() const {
		return BIT_CHECK(m_nLogMode, LOGMODE_DEBUGGER);
	}
	void Log(ELogType eLogType, ELogFlag eLogFlag, const char *pszLogString);
private:
	ELogLevel m_eLogLv;
	dword m_nLogMode;
	CFile m_LogFile;
};