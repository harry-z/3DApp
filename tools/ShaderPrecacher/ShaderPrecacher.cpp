#include "ShaderPrecacher.h"
#include "RendererTypes.h"
#include "Str.h"
#include "Array.h"
#include "File.h"
#include "Hashmap.h"
#include "Memory.h"
#include "Buffer.h"
#include "ScriptParser.h"

#include <errno.h>

struct ShaderEntry {
    String m_ShaderName;
    String m_EntryPoint;
    String m_ShaderFile;
    CArray<String> m_ShaderMacros;
    EShaderType m_ShaderType = EShaderType::EShaderType_Unknown;
};

char* FindFirstCharOrNumber(char *pStart)
{
    char c = *pStart;
    while (c != 0) {
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9'))
            break;
        ++pStart;
        c = *pStart;
    }
    
    return c != 0 ? pStart : nullptr;
}

void RemoveEnter(char *p)
{
    char c = *p;
    while (c != 0) {
        if (c == '\n')
        {
            *p = 0;
            return;
        }
        ++p;
        c = *p;
    }
}

void ConcatenatePath(const String &szPath1, const String &szPath2, OUT String &szFullPath)
{
    if (szPath1.empty())
        szFullPath = szPath2;
    else if (szPath2.empty())
        szFullPath = szPath1;
    else
        szFullPath = szPath1 + "/" + szPath2;
}

bool ReadAllFile(const String &szGlobalPath, const String &szCurrentPath, OUT CMap<String, char*> &Files)
{
#if TARGET_PLATFORM == PLATFORM_WINDOWS
    WIN32_FIND_DATA FindFileData;
    String szOriginalPath;
    ConcatenatePath(szGlobalPath, szCurrentPath, szOriginalPath);
    String szPath(szOriginalPath);
    szPath += "/*";
    HANDLE hFind = FindFirstFile(szPath.c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;
    else
    {
        if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0)
        {
            String szFullPath;
            ConcatenatePath(szOriginalPath, FindFileData.cFileName, szFullPath);
            FILE *pFile = fopen(szFullPath.c_str(), "rb");
            if (pFile)
            {
                fseek(pFile, 0, SEEK_END);
                long len = ftell(pFile);
                fseek(pFile, 0, SEEK_SET);
                char *pShaderCodeBuffer = (char *)malloc(len + 1);
                fread(pShaderCodeBuffer, 1, len, pFile);
                pShaderCodeBuffer[len] = 0;
                fclose(pFile);

                String szEntry;
                ConcatenatePath(szCurrentPath, FindFileData.cFileName, szEntry);
                Files.Insert(szEntry, pShaderCodeBuffer);
            }
        }
        else if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && 
                strcmp(FindFileData.cFileName, ".") != 0 &&
                strcmp(FindFileData.cFileName, "..") != 0)
        {
            String szSubPath;
            ConcatenatePath(szCurrentPath, FindFileData.cFileName, szSubPath);
            ReadAllFile(szGlobalPath.c_str(), szSubPath.c_str(), Files);
        }

        while (FindNextFile(hFind, &FindFileData) != 0)
        {
            if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0)
            {
                String szFullPath;
                ConcatenatePath(szOriginalPath, FindFileData.cFileName, szFullPath);
                FILE *pFile = fopen(szFullPath.c_str(), "rb");
                if (pFile)
                {
                    fseek(pFile, 0, SEEK_END);
                    long len = ftell(pFile);
                    fseek(pFile, 0, SEEK_SET);
                    char *pShaderCodeBuffer = (char *)MEMALLOC(len + 1);
                    fread(pShaderCodeBuffer, 1, len, pFile);
                    pShaderCodeBuffer[len] = 0;
                    fclose(pFile);

                    String szEntry;
                    ConcatenatePath(szCurrentPath, FindFileData.cFileName, szEntry);
                    Files.Insert(szEntry, pShaderCodeBuffer);
                }
            }
            else if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && 
                    strcmp(FindFileData.cFileName, ".") != 0 &&
                    strcmp(FindFileData.cFileName, "..") != 0)
            {
                String szSubPath;
                ConcatenatePath(szCurrentPath, FindFileData.cFileName, szSubPath);
                ReadAllFile(szGlobalPath.c_str(), szSubPath.c_str(), Files);
            }
        }

        return true;
    }
#else
    return false;
#endif
}

class CShaderIndexFileListener final : public IScriptParserListener
{
public:
    virtual void OnProcessNode(const String &szParamType, const CArray<String> &arrParam) override
    {
        if (szParamType == "Shader" && arrParam.Num() > 0) 
        {
            m_nIndex = m_pShaderEntries->Num();
            m_pShaderEntries->Emplace(ShaderEntry());
            m_pShaderEntries->operator[](m_nIndex).m_ShaderName = arrParam[0];
        }
        else if (szParamType == "Type" && arrParam.Num() > 0)
        {
            if (arrParam[0] == "Vertex")
                m_pShaderEntries->operator[](m_nIndex).m_ShaderType = EShaderType::EShaderType_Vertex;
            else if (arrParam[0] == "Pixel")
                m_pShaderEntries->operator[](m_nIndex).m_ShaderType = EShaderType::EShaderType_Pixel;
        }
        else if (szParamType == "EntryPoint" && arrParam.Num() > 0)
        {
            m_pShaderEntries->operator[](m_nIndex).m_EntryPoint = arrParam[0];
        }
        else if (szParamType == "Macros")
        {
            for (const auto &Param : arrParam)
            {
                m_pShaderEntries->operator[](m_nIndex).m_ShaderMacros.Emplace(Param);
            }
        }
        else if (szParamType == "File" && arrParam.Num() > 0)
        {
            m_pShaderEntries->operator[](m_nIndex).m_ShaderFile = arrParam[0];
        }
    }

    CArray<ShaderEntry> *m_pShaderEntries;
    dword m_nIndex = 0;
};

bool ReadShaderIndexFile(const String &szShaderIndexFile, OUT CArray<ShaderEntry> &ShaderEntries)
{
    CShaderIndexFileListener Listener;
    Listener.m_pShaderEntries = &ShaderEntries;

    CScriptParser Parser;
    Parser.AddScriptParserListener(&Listener);
    return Parser.ParseFromFile(szShaderIndexFile.c_str());
}

#ifdef RENDERAPI_DX9
#include "d3dx9shader.h"
class CD3DIncludeHandler final : public ID3DXInclude
{
public:
    CD3DIncludeHandler(const CMap<String, char*> *pMapShaderFiles) 
    : m_pMapShaderFiles(const_cast<CMap<String, char*>*>(pMapShaderFiles)) {}

    STDMETHOD(Open)(THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) {
		if (m_pMapShaderFiles != nullptr) {
            CMap<String, char*>::_MyIterType FileContentConstIter = m_pMapShaderFiles->Find(String(pFileName));
            if (FileContentConstIter)
            {
                *ppData = FileContentConstIter.Value();
                *pBytes = strlen(FileContentConstIter.Value());
                return S_OK;
            }
        }
		return S_FALSE;
	}

	STDMETHOD(Close)(THIS_ LPCVOID pData) {
		(void)pData;
		return S_OK;
	}
    CMap<String, char*> *m_pMapShaderFiles = nullptr;
};
#endif

bool CompileAndCacheAllShaders(const CMap<String, char*> &arrShaderFiles, const CArray<ShaderEntry> &arrShaderEntries)
{
    CFile file;
    if (!file.Open("./Shader.bundle", "w+"))
        return false;

    struct ShaderBuffer {
        void *buffer;
        dword size;
        ShaderBuffer(void *buf, dword nz) : buffer(buf), size(nz) {}
        ~ShaderBuffer() {
            if (buffer != nullptr)
                free(buffer);
        }
    };
    CArray<ShaderBuffer> arrShaderBuffer;

#ifdef RENDERAPI_DX9
    DWORD dwFlag = 0;

#ifdef SHADER_DEBUGGING
    BIT_ADD(dwFlag, (D3DXSHADER_DEBUG | D3DXSHADER_OPTIMIZATION_LEVEL0));
#endif

    CD3DIncludeHandler IncludeHandler(&arrShaderFiles);
    ID3DXBuffer *pByteCode = nullptr, *pByteError = nullptr;

    for (const auto &ShaderEntry : arrShaderEntries) {
        // 准备 Macro
        CArray<D3DXMACRO> D3DMacros;
        D3DXMACRO D3DMacro;
        for (const auto &ShaderMacro : ShaderEntry.m_ShaderMacros) {
            String::size_type pos = ShaderMacro.find_first_of("=");
            if (pos != String::npos)
                D3DMacro.Name = ShaderMacro.substr(0, pos).c_str();
            else
                D3DMacro.Name = ShaderMacro.c_str();
            D3DMacro.Definition = "1";
            D3DMacros.Emplace(std::move(D3DMacro));
        }
        D3DMacro.Name = 0;
        D3DMacro.Definition = 0;
        D3DMacros.Emplace(std::move(D3DMacro));

        bool bShaderCached = false;
        if (ShaderEntry.m_ShaderType != EShaderType::EShaderType_Unknown) {
            String szProfile;
            if (ShaderEntry.m_ShaderType == EShaderType::EShaderType_Vertex)
                szProfile = "vs_3_0";
            else if (ShaderEntry.m_ShaderType == EShaderType::EShaderType_Pixel)
                szProfile = "ps_3_0";

            // 找到相应的Shader文件
            CMap<String, char*>::_MyConstIterType ShaderCodeConstIter = arrShaderFiles.Find(ShaderEntry.m_ShaderFile);
            if (ShaderCodeConstIter) {
                HRESULT hr = D3DXCompileShader(ShaderCodeConstIter.Value(), 
                    strlen(ShaderCodeConstIter.Value()), 
                    D3DMacros.Data(), 
                    &IncludeHandler, 
                    ShaderEntry.m_EntryPoint.c_str(), 
                    szProfile.c_str(), 
                    dwFlag,  
                    &pByteCode, 
                    &pByteError,
                    nullptr);
                if (SUCCEEDED(hr))
                {
                    // 编译成功，写入缓存文件
                    dword nSize = ShaderEntry.m_ShaderName.length() + 1 + sizeof(byte) + pByteCode->GetBufferSize() + sizeof(dword);
                    byte *pBuffer = (byte*)malloc(nSize);
                    byte *pBufferStart = pBuffer;

                    // 写入Shader名称
                    AddString(pBuffer, ShaderEntry.m_ShaderName);
                    // 写入Shader类型
                    byte nShaderType = (byte)ShaderEntry.m_ShaderType;
                    AddBytes(pBuffer, &nShaderType, 1);
                    // 写入ShaderByteCode长度
                    dword nByteCodeSize = (dword)pByteCode->GetBufferSize();
                    AddDwords(pBuffer, &nByteCodeSize, 1);
                    // 写入ShaderByteCode
                    AddBytes(pBuffer, (byte *)pByteCode->GetBufferPointer(), pByteCode->GetBufferSize());

                    arrShaderBuffer.Emplace(pBufferStart, nSize);

                    SAFE_RELEASE(pByteCode);

                    bShaderCached = true;
                }
                else
                {
                    std::cout << "Shader " << ShaderEntry.m_ShaderName.c_str() << " compile error: " << (char *)pByteError->GetBufferPointer() << std::endl;
                }
            }
        }

        if (bShaderCached)
            std::cout << "Shader cached: " << ShaderEntry.m_ShaderName.c_str() << std::endl;
        else
            std::cout << "Shader not cached: " << ShaderEntry.m_ShaderName.c_str() << std::endl;
    }
    
#endif

    dword nNumCachedShader = arrShaderBuffer.Num();
    file.Write(&nNumCachedShader, sizeof(dword));
    for (const auto &ShaderBuffer : arrShaderBuffer) {
        file.Write(ShaderBuffer.buffer, ShaderBuffer.size);
    }
    file.Close();

    arrShaderBuffer.Empty();

    return true;
}

bool CShaderPrecacher::Precache()
{
    // 查找目录下所有文件，并读取其内容
    CMap<String, char*> ShaderFiles;
    CArray<ShaderEntry> ShaderEntries;

    String ShaderFilePath, ShaderIndexFilePath;
#ifdef RENDERAPI_DX9
    ShaderFilePath = "ShaderSource/D3D9";
    ShaderIndexFilePath = "ShaderSource/D3D9/Shaders.idx";
#endif

    std::cout << "Start reading all shader files..." << std::endl;
    if (!ReadAllFile(ShaderFilePath, "", ShaderFiles))
    {
        std::cout << "Failed to read shader files." << std::endl;
        return false;
    }
    std::cout << "Finish reading all shader files." << std::endl;
    std::cout << "Start reading shader index file..." << std::endl;
    if (!ReadShaderIndexFile(ShaderIndexFilePath, ShaderEntries))
    {
        std::cout << "Failed to read shader index file." << std::endl;
        return false;
    }
    std::cout << "Finish reading shader index file." << std::endl;
    if (!CompileAndCacheAllShaders(ShaderFiles, ShaderEntries))
    {
        std::cout << "Failed to cache shaders." << std::endl;
    }
    std::cout << "Finish caching shaders." << std::endl;

    CMap<String, char*>::_MyIterType Iter = ShaderFiles.CreateIterator();
    for (; Iter; ++Iter) {
        free(Iter.Value());
    }

    return true;
}