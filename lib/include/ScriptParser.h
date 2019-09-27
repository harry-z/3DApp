#pragma once
#include "Prereq.h"

struct IScriptParserListener
{
    virtual ~IScriptParserListener() {}
    virtual void OnProcessNode(const String &szParamType, const CArray<String> &arrParam) = 0;
    LinklistNode<IScriptParserListener> m_Node;
};

class DLL_EXPORT CScriptParser
{
public:
    bool Parse(const char *pszScriptContent);
    bool ParseFromFile(const char *pszScriptFile);
    void AddScriptParserListener(IScriptParserListener *pListener);
    void RemoveScriptParserListener(IScriptParserListener *pListener);

private:
    Linklist<IScriptParserListener> m_Listeners;
};