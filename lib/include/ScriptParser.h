#pragma once
#include "Prereq.h"

struct IScriptParserListener
{
    void OnProcessChunkTitle(const char *pszChunkType, const char *pszChunkParam) {}
    void OnProcessParam(const char *pszParamType, const char *pszParam1, const char *pszParam2) {}
};

class DLL_EXPORT CScriptParser
{
public:
    bool Parse(const String &szScriptContent);
    bool Parse(const char *pszScriptFile);

private:
    void NotifyProcessChunkTitle(const char *pszChunkType, const char *pszChunkParam);
    void NotifyProcessParam(const char *pszParamType, const char *pszParam1, const char *pszParam2);

private:
    CArray<IScriptParserListener> m_Listeners;
};