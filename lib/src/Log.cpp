#include "Log.h"
#include "Str.h"

void CLog::Log(ELogType eLogType, ELogFlag eLogFlag, const char *pszLogString) {
	if ((dword)m_eLogLv < (dword)eLogFlag)
		return;
	String msg;
	switch (eLogType) {
	case ELogType::eLogType_Error:
		msg = "Error: ";
		break;
	case ELogType::eLogType_Warning:
		msg = "Warning: ";
		break;
	case ELogType::eLogType_Info:
		msg = "Information: ";
		break;
	}
	msg += pszLogString;
	if (IsLogToConsole())
		puts(msg.c_str());
	msg += "\n";
	if (IsLogToFile()) {
		fputs(msg.c_str(), m_LogFile.GetHandle());
		m_LogFile.Flush();
	}
	if (IsLogToDebugger()) {
#if COMPILER == COMPILER_MSVC
		OutputDebugStringA(msg.c_str());
#endif
	}
}