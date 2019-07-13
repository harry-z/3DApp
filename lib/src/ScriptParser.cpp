#include "ScriptParser.h"

constexpr char c_ChunkTokenStart = '{';
constexpr char c_ChunkTokenEnd = '}';
constexpr char c_StringSplit = ' ';

struct ParserParamNode {
    String m_szType;
    String m_szParam1;
    String m_szParam2;
    ParserParamNode(const String &szType, const String &szParam1, const String &szParam2)
    : m_szType(szType), m_szParam1(szParam1), m_szParam2(szParam2) {}
};

struct ParserChunkNode {
    String m_szType;
    String m_szParam;
    CArray<ParserChunkNode> m_ChildChunkNodes;
    CArray<ParserParamNode> m_ParamNodes;
    ParserChunkNode(const String &szType, const String &szParam)
    : m_szType(szType), m_szParam(szParam) {}
};

ParserChunkNode* MakeChunkNode(ParserChunkNode *pParentNode, const String &szType, const String &szParam)
{
    if (pParentNode == nullptr)
    {
        ParserChunkNode *pNewNode = NEW_TYPE(ParserChunkNode)(szType, szParam);
        return pNewNode;
    }
    else
    {
        pParentNode->m_ChildChunkNodes.Emplace(szType, szParam);
        return &(pParentNode->m_ChildChunkNodes[pParentNode->m_ChildChunkNodes.Num() - 1]);
    }
}

bool IsAvailableChar(const char c)
{
    return (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            (c == '-' || c == '.');
}

const char *SkipToNextLine(const char *&pszContent)
{
    char c = *pszContent;
    while (c != 0) {
        if (c == '\n')
            return ++pszContent;
        else
            c = *(++pszContent);
    }
    return nullptr;
}

const char* FindFirstAvailableChar(const char *&pszContent)
{
    char c = *pszContent;
    while (c != 0) {
        if (IsAvailableChar(c))
            return pszContent;
        else
            c = *(++pszContent);
    }
    return nullptr;
}

const char* FindChunkToken(const char *&pszContent)
{
    char c = *pszContent;
    while (c != 0) {
        if (c == c_ChunkTokenStart)
            return ++pszContent;
        else
            c = *(++pszContent);
    }
    return nullptr;
}

const char* FindChunkCloseToken(const char *pszContent)
{
    char c = *pszContent;
    while (c != 0) {
        if (c == c_ChunkTokenEnd)
            return ++pszContent;
        else
            c = *(++pszContent);
    }
    return nullptr;
}

String ExtractString(const char *&pszContent)
{
    const char *pszBegin = pszContent;
    char c = *pszContent;
    while (!IsAvailableChar(c)) {
        ++pszContent;
    }
    ptrdiff_t nSize = (ptrdiff_t)(pszContent) - (ptrdiff_t)(pszBegin);
    if (nSize == 0)
        return String();
    String str; str.Reserve((String::size_type)nSize);
    for (int i = 0; i < (int)nSize; ++i)
        str[i] = *pszBegin++;
    return std::move(str);
}

void NotifyListner(const ParserChunkNode &Node, IScriptParserListener *pListener)
{
    pListener->OnProcessChunkTitle(Node.m_szType, Node.m_szParam);
    if (!Node.m_ParamNodes.Empty())
    {
        for (const auto &ParamNode : Node.m_ParamNodes)
        {
            pListener->OnProcessParam(ParamNode.m_szType, ParamNode.m_szParam1, ParamNode.m_szParam2);
        }
    }

    if (!Node.m_ChildChunkNodes.Empty())
    {
        for (const auto &ChildNode : Node.m_ChildChunkNodes)
        {
            NotifyListner(ChildNode, pListener);
        }
    }
}

bool CScriptParser::Parse(const char *pszScriptContent)
{
    enum class EState {
        EState_None,
        EState_FoundChunk,
        EState_FoundName,
        EState_FoundParam1,
        EState_FoundParam2,
        EState_Idle
    };

    bool bSucceeded = true;

    CStack<ParserChunkNode*> ChunkNodeStack;
    ParserChunkNode *pRootChunkNode = nullptr;
    ParserChunkNode *pCurrentChunkNode = nullptr;

    EState eCurrentState = EState::EState_None;
    String szType, szParam1, szParam2;

    char c = *pszScriptContent;
    const char *pszSentinel = pszScriptContent;
    while (c != 0) {
        if (FindFirstAvailableChar(pszSentinel) != nullptr) // Found available char
        {
            switch (eCurrentState)
            {
                case EState::EState_None:
                case EState::EState_FoundChunk:
                case EState::EState_Idle:
                {
                    szType = ExtractString(pszSentinel);
                    eCurrentState = EState::EState_FoundName;
                    break;
                }
                case EState::EState_FoundName:
                {
                    szParam1 = ExtractString(pszSentinel);
                    eCurrentState = EState::EState_FoundParam1;
                    break;
                }
                case EState::EState_FoundParam1:
                {
                    szParam2 = ExtractString(pszSentinel);
                    eCurrentState = EState::EState_FoundParam2;
                    break;
                }
                case EState::EState_FoundParam2:
                {
                    // 在FoundParam2后马上又遇到了有效字符，上一次解析的是一行参数
                    pCurrentChunkNode->m_ParamNodes.Emplace(szType, szParam1, szParam2);
                    szType = ExtractString(pszSentinel);
                    eCurrentState = EState::EState_FoundName;
                    break;
                }
            }
        }
        else if (FindChunkToken(pszSentinel) != nullptr) { // Found {
            if (eCurrentState == EState::EState_FoundChunk) // Skip duplicated {
                continue;
            else if (eCurrentState == EState::EState_FoundName)
            {
                // 在解析完类型后直接遇到了{，是没有参数的Type
                szParam1.clear();
                szParam2.clear();
            }
            else if (eCurrentState == EState::EState_FoundParam2 || eCurrentState == EState::EState_None)
            {
                // 解析完第二个参数后又遇到了{，属于格式错误
                // 没有遇到任何有效字符就先遇到了{，属于格式错误
                bSucceeded = false;
                break;
            }
            
            if (ChunkNodeStack.Num() == 0) // Creating root node
            {
                pRootChunkNode = pCurrentChunkNode = MakeChunkNode(nullptr, szType, szParam1);
                ChunkNodeStack.Emplace(pRootChunkNode);
            }
            else // Creating child node
            {
                CStack<ParserChunkNode*>::_ConstPointerType pParentChunkNode = ChunkNodeStack.Last();
                pCurrentChunkNode = MakeChunkNode(const_cast<ParserChunkNode*>(*pParentChunkNode), szType, szParam1);
                ChunkNodeStack.Emplace(pCurrentChunkNode);
            }

            eCurrentState = EState::EState_FoundChunk;
        }
        else if (pszSentinel = FindChunkCloseToken(pszSentinel)) { // Found }
            if (eCurrentState == EState::EState_FoundParam2)
                pCurrentChunkNode->m_ParamNodes.Emplace(szType, szParam1, szParam2);
            else if (eCurrentState == EState::EState_None || 
                eCurrentState == EState::EState_Idle ||
                eCurrentState == EState::EState_FoundName || 
                eCurrentState == EState::EState_FoundParam1)
            {
                bSucceeded = false;
                break;
            }

            ChunkNodeStack.Pop();
            eCurrentState = EState::EState_Idle;
        }
    }

    bSucceeded = bSucceeded && ChunkNodeStack.IsEmpty() && pRootChunkNode != nullptr;

    if (bSucceeded) {
        LinklistNode<IScriptParserListener*> *pTemp = m_Listeners.m_pRoot;
        while (pTemp != nullptr)
        {
            NotifyListener(*pRootChunkNode, pTemp->m_pOwner);
            pTemp = pTemp->m_pNext;
        }
    }

    if (pRootChunkNode != nullptr)
    {
        DELETE_TYPE(pRootChunkNode, ParserChunkNode);
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
    pListener->m_Node->m_pOwner = pListener;
    m_Listeners.PushBack(&pListener->m_Node);
}

void CScriptParser::RemoveScriptParserListener(IScriptParserListener *pListener)
{
    m_Listeners.Remove(&pListener->m_Node);
}