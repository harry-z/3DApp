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
    CArray<FParserChunkNode> m_ChildChunkNodes;
    CArray<FParserParamNode> m_ParamNodes;
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
        return &pParentNode->m_ChildChunkNodes[pParentNode->m_ChildChunkNodes.Num() - 1];
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
            c == *(++pszContent);
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
    int nDepth = 0;

    EState eCurrentState = EState::EState_None;
    String szType, szParam1, szParam2;

    bool bFirstChunk = true;
    char c = *pszScriptContent;
    const char *pszSentinel = pszScriptContent;
    while (c != 0) {
        if (pszSentinel = FindFirstAvailableChar(pszSentinel)) // 找到了有效字符
        {
            switch (eCurrentState)
            {
                case EState::EState_None:
                case EState::EState_FoundChunk:
                case EState::EState_Idle:
                    szType = ExtractString(pszSentinel);
                    eCurrentState = EState::EState_FoundName;
                    break;
                case EState::EState_FoundName:
                    szParam1 = ExtractString(pszSentinel);
                    eCurrentState = EState::EState_FoundParam1;
                    break;
                case EState::EState_FoundParam1:
                    szParam2 = ExtractString(pszSentinel);
                    eCurrentState = EState::EState_FoundParam2;
                    break;
                case EState::EState_FoundParam2:
                    // 在FoundParam2后马上又遇到了有效字符，上一次解析的是一行参数
                    pCurrentChunkNode->m_ParamNodes.Emplace(szType, szParam1, szParam2);
                    szType = ExtractString(pszSentinel);
                    eCurrentState = EState::EState_FoundName;
                    break;
            }
        }
        else if (pszSentinel = FindChunkToken(pszSentinel)) { // 找到了{
            if (eCurrentState == EState::EState_FoundChunk) // 如果出现连续多个{，跳过
                continue;
            else if (eCurrentState == EState::EState_FoundName)
            {
                // 在解析完类型后直接遇到了{，是没有参数的Type
                szParam1.clear();
                szParam2.clear();
            }
            else if (eCurrentState == EState::EState_FoundParam2 || eCurrentState == EState_None)
            {
                // 解析完第二个参数后又遇到了{，属于格式错误
                // 没有遇到任何有效字符就先遇到了{，属于格式错误
                bSucceeded = false;
                break;
            }
            
            if (ChunkNodeStack.Num() == 0) // 需要生成根节点
            {
                pRootChunkNode = pCurrentChunkNode = MakeChunkNode(nullptr, szType, szParam);
                ChunkNodeStack.Emplace(pRootChunkNode);
            }
            else // 生成子节点
            {
                CStack<ParserChunkNode*>::_ConstPointerType pParentChunkNode = ChunkNodeStack.Last();
                pCurrentChunkNode = MakeChunkNode(const_cast<ParserChunkNode*>(*pParentChunkNode), szType, szParam);
                ChunkNodeStack.Emplace(pCurrentChunkNode);
            }

            eCurrentState = EState::EState_FoundChunk;
        }
        else if (pszSentinel = FindChunkCloseTokenInLine(pszSentinel)) { // 找到了}
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
        }

    }
}