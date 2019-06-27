#include "ShaderPrecacher.h"
#include "RendererTypes.h"
#include "Str.h"
#include "Array.h"
#include "File.h"
#include "Hashmap.h"
#include "Memory.h"
#include "Buffer.h"

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
    else
        szFullPath = szPath1 + "/" + szPath2;
}

bool ReadAllFile(const String &szGlobalPath, const String &szCurrentPath, OUT CHashmap<String, char*> &Files)
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
            FILE *pFile = fopen(szFullPath.c_str(), "r+");
            if (pFile)
            {
                fseek(pFile, 0, SEEK_END);
                long len = ftell(pFile);
                fseek(pFile, 0, SEEK_SET);
                char *pShaderCodeBuffer = (char *)MEMALLOC(len);
                fread(pShaderCodeBuffer, 1, len, pFile);
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
                FILE *pFile = fopen(szFullPath.c_str(), "r+");
                if (pFile)
                {
                    fseek(pFile, 0, SEEK_END);
                    long len = ftell(pFile);
                    fseek(pFile, 0, SEEK_SET);
                    char *pShaderCodeBuffer = (char *)MEMALLOC(len);
                    fread(pShaderCodeBuffer, 1, len, pFile);
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

bool ReadShaderIndexFile(const String &szShaderIndexFile, OUT CArray<ShaderEntry> &ShaderEntries)
{
    FILE *pFile = fopen(szShaderIndexFile.c_str(), "r+");
    if (pFile != nullptr) {
        char szLine[1024];
        dword nIndex = 0;
        char *p = nullptr;
        while (!feof(pFile)) {
            if (fgets(szLine, 1024, pFile)) {
                if (szLine[0] == '[') {
                    nIndex = ShaderEntries.Num();
                    ShaderEntries.Emplace(ShaderEntry());
                    char *pszMatchedBrace = strchr(szLine + 1, ']');
                    if (pszMatchedBrace == nullptr)
                        continue;
                    *pszMatchedBrace = 0;
                    ShaderEntries[nIndex].m_ShaderName = szLine + 1;
                }
                else if (p = strstr(szLine, "Type")) {
                    p = FindFirstCharOrNumber(p + strlen("Type"));
                    if (p != nullptr)
                    {
                        RemoveEnter(p);
                        if (strcmp(p, "Vertex") == 0)
                            ShaderEntries[nIndex].m_ShaderType = EShaderType::EShaderType_Vertex;
                        else if (strcmp(p, "Pixel") == 0)
                            ShaderEntries[nIndex].m_ShaderType = EShaderType::EShaderType_Pixel;
                    }
                }
                else if (p = strstr(szLine, "EntryPoint")) {
                    p = FindFirstCharOrNumber(p + strlen("EntryPoint"));
                    if (p != nullptr)
                    {
                        RemoveEnter(p);
                        ShaderEntries[nIndex].m_EntryPoint = p;
                    }
                }
                else if (p = strstr(szLine, "Macros")) {
                    p = FindFirstCharOrNumber(p + strlen("Macros"));
                    if (p != nullptr)
                    {
                        char *pComma = nullptr;
                        while (*p != 0 && (pComma = strchr(p, ','))) {
                            *pComma = 0;
                            ShaderEntries[nIndex].m_ShaderMacros.Emplace(p);
                            p = ++pComma;
                        }
                        if (*p != 0)
                        {
                            RemoveEnter(p);
                            ShaderEntries[nIndex].m_ShaderMacros.Emplace(p);
                        }
                    }
                }
                else if (p = strstr(szLine, "File")) {
                    p = FindFirstCharOrNumber(p + strlen("File"));
                    if (p != nullptr)
                    {
                        RemoveEnter(p);
                        ShaderEntries[nIndex].m_ShaderFile = p;
                    }
                }
            }
        }
        return true;
    }
    else
        return false;
}

#ifdef RENDERAPI_DX9
#include <d3dcompiler.h>
class CD3DIncludeHandler final : public ID3DInclude
{
public:
    CD3DIncludeHandler(const CHashmap<String, char*> *pHashmapShaderFiles) 
    : m_pHashmapShaderFiles(const_cast<CHashmap<String, char*>*>(pHashmapShaderFiles)) {}
    STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) {
		if (m_pHashmapShaderFiles != nullptr) {
            CHashmap<String, char*>::_ValuePointerType ppFileContent = m_pHashmapShaderFiles->Find(String(pFileName));
            if (ppFileContent != nullptr)
            {
                *ppData = *ppFileContent;
                *pBytes = strlen(*ppFileContent);
                return S_OK;
            }
        }
		return S_FALSE;
	}
	STDMETHOD(Close)(THIS_ LPCVOID pData) {
		(void)pData;
		return S_OK;
	}
    CHashmap<String, char*> *m_pHashmapShaderFiles = nullptr;
};
#endif

bool CompileAndCacheAllShaders(const CHashmap<String, char*> &arrShaderFiles, const CArray<ShaderEntry> &arrShaderEntries)
{
    DelFile("./Shader.bundle");
    NewFile("./Shader.bundle");

    CFile file;
    if (!file.Open("./Shader.bundle", "wb"))
        return false;

#ifdef RENDERAPI_DX9
    DWORD dwFlag = 0;
#ifdef SHADER_DEBUGGING
    BIT_ADD(dwFlag, (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION));
#endif

    CD3DIncludeHandler IncludeHandler(&arrShaderFiles);
    ID3DBlob *pByteCode = nullptr, *pByteError = nullptr;

    for (const auto &ShaderEntry : arrShaderEntries) {
        // 准备 Macro
        CArray<D3D_SHADER_MACRO> D3DMacros;
        D3D_SHADER_MACRO D3DMacro;
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
            CHashmap<String, char*>::_ValueConstPointerType ppShaderCode = arrShaderFiles.Find(ShaderEntry.m_ShaderFile);
            if (ppShaderCode != nullptr) {
                HRESULT hr = D3DCompile(*ppShaderCode, 
                    strlen(*ppShaderCode), 
                    nullptr, 
                    D3DMacros.Data(), 
                    &IncludeHandler, 
                    ShaderEntry.m_EntryPoint.c_str(), 
                    szProfile.c_str(), 
                    dwFlag, 0, 
                    &pByteCode, 
                    &pByteError);
                if (SUCCEEDED(hr))
                {
                    // 编译成功，写入缓存文件
                    dword nSize = ShaderEntry.m_ShaderName.length() + 1 + pByteCode->GetBufferSize() + sizeof(dword);
                    byte *pBuffer = (byte*)malloc(nSize);

                    AddString(pBuffer, ShaderEntry.m_ShaderName);
                    dword nByteCodeSize = (dword)pByteCode->GetBufferSize();
                    AddDwords(pBuffer, &nByteCodeSize, 1);
                    AddBytes(pBuffer, (byte *)pByteCode->GetBufferPointer(), pByteCode->GetBufferSize());
                    file.Write(pBuffer, nSize);

                    free(pBuffer);
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
    file.Close();

    return true;
}

bool CShaderPrecacher::Precache()
{
    // 查找目录下所有文件，并读取其内容
    CHashmap<String, char*> ShaderFiles(100);
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

    return true;
}