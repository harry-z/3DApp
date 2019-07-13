#pragma once
#include "Prereq.h"

struct IScriptParserListener
{
    virtual ~IScriptParserListener() {}
    void OnProcessChunkTitle(const String &szChunkType, const String &szChunkParam) {}
    void OnProcessParam(const String &szParamType, const String &szParam1, const String &szParam2) {}
    LinklistNode<IScriptParserListener*> m_Node;
};

class DLL_EXPORT CScriptParser
{
public:
    bool Parse(const char *pszScriptContent);
    bool ParseFromFile(const char *pszScriptFile);
    void AddScriptParserListener(IScriptParserListener *pListener);
    void RemoveScriptParserListener(IScriptParserListener *pListener);

private:
    Linklist<IScriptParserListener*> m_Listeners;
};