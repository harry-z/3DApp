#include "ScriptParser.h"

constexpr char c_ChunkTokenStart = '{';
constexpr char c_ChunkTokenEnd = '}';
constexpr char c_Newline = '\n';

struct ParserNode {
    dword m_nDepth = 0;
    String m_szType;
    CArray<String> m_arrParam;
    CArray<ParserNode> m_ChildNodes;
    ParserNode() {}
    ParserNode(dword nDepth, const String &szType, const CArray<String> &arrParam)
    : m_nDepth(nDepth), m_szType(szType), m_arrParam(arrParam) {}
};

ParserNode* MakeNode(ParserNode *pParentNode, dword nDepth, const String &szType, const CArray<String> &arrParam)
{
    if (pParentNode == nullptr)
    {
        ParserNode *pNewNode = NEW_TYPE(ParserNode)(nDepth, szType, arrParam);
        return pNewNode;
    }
    else
    {
        pParentNode->m_ChildNodes.Emplace(nDepth, szType, arrParam);
        return &(pParentNode->m_ChildNodes[pParentNode->m_ChildNodes.Num() - 1]);
    }
}

ParserNode* BuildNode(dword nDepth, const String &szType, const CArray<String> &arrParam, ParserNode *pParentNode)
{
    if (szType.length() == 0)
        return nullptr;
    return MakeNode(pParentNode, nDepth, szType, arrParam);
}

bool IsAvailableChar(const char c)
{
    return (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            (c == '-' || c == '.' || c == '_' || c == '/' || c == '\\');
}

bool IsTokenStart(const char c)
{
    return c == c_ChunkTokenStart;
}

bool IsTokenEnd(const char c)
{
    return c == c_ChunkTokenEnd;
}

bool IsNewline(const char c)
{
    return c == c_Newline;
}

const char* FindFirstAvailableChar(const char *&pszContent)
{
    char c = *pszContent;
    while (c != 0) {
        if (IsAvailableChar(c) || IsTokenStart(c) || IsTokenEnd(c) || IsNewline(c))
            return pszContent;
        else
            c = *(++pszContent);
    }
    return nullptr;
}

String ExtractString(const char *&pszContent)
{
    const char *pszBegin = pszContent;
    char c = *pszContent;
    while (IsAvailableChar(c)) {
        c = *(++pszContent);
    }
    ptrdiff_t nSize = (ptrdiff_t)(pszContent) - (ptrdiff_t)(pszBegin);
    if (nSize == 0)
        return String();
    String str; str.Reserve((String::size_type)nSize);
    for (int i = 0; i < (int)nSize; ++i)
        str.push_back(*pszBegin++);
    return std::move(str);
}

void NotifyListener(const ParserNode &Node, IScriptParserListener *pListener)
{
    pListener->OnProcessNode(Node.m_szType, Node.m_arrParam);

    if (!Node.m_ChildNodes.Empty())
    {
        for (const auto &ChildNode : Node.m_ChildNodes)
        {
            NotifyListener(ChildNode, pListener);
        }
    }
}

bool CScriptParser::Parse(const char *pszScriptContent)
{
    enum EState : dword {
        EState_None = 0,
        EState_FoundChunkStart,
        EState_FoundChunkEnd,
        EState_FoundType,
        EState_FoundParam,
        EState_FoundNewline = 0x80000000
    };

    bool bSucceeded = true;
    dword nDepth = 0;

    CStack<ParserNode*> ChunkNodeStack;
    ParserNode *pRootChunkNode = MakeNode(nullptr, nDepth++, String(), CArray<String>());
    ParserNode *pCurrentChunkNode = pRootChunkNode;
    ChunkNodeStack.Emplace(pCurrentChunkNode);

    dword eCurrentState = EState_None;
    String szType;
    CArray<String> arrParam;

    char c = *pszScriptContent;
    const char *pszSentinel = pszScriptContent;
    while ((*pszSentinel) != 0 && FindFirstAvailableChar(pszSentinel) != nullptr) 
    {
        if (IsAvailableChar(*pszSentinel))
        {
            switch (eCurrentState & 0x7FFFFFFF) 
            {
                case EState_None:
                case EState_FoundChunkStart:
                case EState_FoundChunkEnd:
                {
                    szType = ExtractString(pszSentinel);
                    arrParam.Clear();
                    arrParam.SetNum(0);
                    eCurrentState = EState_FoundType;
                    break;
                }
                case EState_FoundType:
                case EState_FoundParam:
                {
                    if (BIT_CHECK(eCurrentState, EState_FoundNewline))
                    {
                        BuildNode(nDepth, szType, arrParam, pCurrentChunkNode);
                        szType = ExtractString(pszSentinel);
                        arrParam.Clear();
                        arrParam.SetNum(0);
                        eCurrentState = EState_FoundType;
                    }
                    else
                    {
                        arrParam.Emplace(ExtractString(pszSentinel));
                        eCurrentState = EState_FoundParam;
                    }
                    break;
                }
            }
        }
        else if (IsTokenStart(*pszSentinel)) 
        {
            switch (eCurrentState & 0x7FFFFFFF)
            {
                case EState_None:
                case EState_FoundChunkEnd:
                    bSucceeded = false;
                    break;
                case EState_FoundType:
                case EState_FoundParam:
                {
                    ParserNode *pNewNode = BuildNode(nDepth, szType, arrParam, pCurrentChunkNode);
                    if (pNewNode != nullptr)
                    {
                        ChunkNodeStack.Emplace(pCurrentChunkNode);
                        pCurrentChunkNode = pNewNode;
                    }
                    break;
                }
            }

            ++pszSentinel;
            ++nDepth;
            eCurrentState = EState_FoundChunkStart;
        }
        else if (IsTokenEnd(*pszSentinel)) 
        {
            switch (eCurrentState & 0x7FFFFFFF)
            {
                case EState_None:
                    bSucceeded = false;
                    break;
                case EState_FoundType:
                case EState_FoundParam:
                {
                    BuildNode(nDepth, szType, arrParam, pCurrentChunkNode);
                    break;
                }
            }

            ++pszSentinel;
            --nDepth;
            ParserNode *TempNode = *(ChunkNodeStack.Last());
            if (TempNode->m_nDepth == nDepth - 1)
            {
                pCurrentChunkNode = *(ChunkNodeStack.Last());
                ChunkNodeStack.Pop();
            }

            eCurrentState = EState_FoundChunkEnd;
        }
        else if (IsNewline(*pszSentinel))
        {
            switch (eCurrentState & 0x7FFFFFFF)
            {
                case EState_FoundType:
                case EState_FoundParam:
                {
                    BIT_ADD(eCurrentState, EState_FoundNewline);
                    break;
                }
            }

            ++pszSentinel;
        }

        if (!bSucceeded)
            break;
    }

    bSucceeded = bSucceeded && nDepth == 1 && pRootChunkNode != nullptr;

    if (bSucceeded) {
        Linklist<IScriptParserListener>::_NodeType *pTemp = m_Listeners.m_pRoot;
        while (pTemp != nullptr)
        {
            for (const auto &ChildNode : pRootChunkNode->m_ChildNodes)
            {
                NotifyListener(ChildNode, pTemp->m_pOwner);
            }
            pTemp = pTemp->m_pNext;
        }
    }

    if (pRootChunkNode != nullptr)
    {
        DELETE_TYPE(pRootChunkNode, ParserNode);
    }

    return bSucceeded;
}

bool CScriptParser::ParseFromFile(const char *pszScriptFile)
{
    CFile file;
    if (file.Open(pszScriptFile, "r"))
    {
        long len = file.Length();
        char *pFileBuffer = (char*)MEMALLOC(len + 1);
        file.ReadTypeRaw<char>(pFileBuffer, len + 1);
        pFileBuffer[len] = 0;
        file.Close();

        bool bRet = Parse(pFileBuffer);
        MEMFREE(pFileBuffer);
        return bRet;
    }
    else
        return false;
}

void CScriptParser::AddScriptParserListener(IScriptParserListener *pListener)
{
    pListener->m_Node.m_pOwner = pListener;
    m_Listeners.PushBack(&pListener->m_Node);
}

void CScriptParser::RemoveScriptParserListener(IScriptParserListener *pListener)
{
    m_Listeners.Remove(&pListener->m_Node);
}