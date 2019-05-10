#include "ShaderPrecacher.h"
#include "RendererTypes.h"
#include "Str.h"
#include "Hashmap.h"
#include "Memory.h"
// #include <stdio>

#ifdef RENDERAPI_DX9
#include <d3dcompiler.h>
// class CD3DIncludeHandler final : public ID3DInclude
// {
// public:
//     STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) {
// 		FILE *pFile = fopen(pFileName, "r+");
//         if (pFile)
//         {
//             fseek(pFile, 0, SEEK_END);
//             long len = ftell(pFile);
//             char *pShaderCodeBuffer = (char *)MEMALLOC(len);
//             size_t bytesRead = fread(pFile, 1, len, pFile);
//             fclose(pFile);
//             if (bytesRead) {

//             }
//         }
// 		return S_OK;
// 	}
// 	STDMETHOD(Close)(THIS_ LPCVOID pData) {
// 		(void)pData;
// 		return S_OK;
// 	}
// };
#endif

char* FindFirstCharOrNumber(char *pStart)
{
    char c = *pStart;
    do
    {
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9'))
            break;
        ++pStart;
    } while (c != 0);
    
    return c != 0 ? pStart : nullptr;
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

bool CShaderPrecacher::Precache()
{
    // 查找目录下所有文件，并读取其内容
    CHashmap<String, char*> ShaderFiles(32);
#ifdef RENDERAPI_DX9
    ReadAllFile("ShaderSource/D3D9", "", ShaderFiles);
#endif

    return true;

    // FILE *pFile = fopen("Shader.config", "r+");
    // if (pFile != nullptr)
    // {
    //     struct Shader {
    //         EShaderType m_ShaderType;
    //         FString m_ShaderName;
    //         CArray<FString> m_ShaderMacros;
    //         FString m_ShaderFile;

    //         Shader() : m_ShaderType(EShaderType::EShaderType_Unknown) {}
    //     };
    //     CArray<Shader*> Shaders;

    //     Shader *pShader = nullptr;
    //     char line[MAX_PATH];
    //     while (!feof(pFile))
    //     {
    //         if (fgets(line, MAX_PATH, pFile))
    //         {
    //             if (line[0] == '[')
    //             {
    //                 pShader = NEW_TYPE(Shader);
    //                 Shaders.Add(pShader);
    //             }
    //             else if (char *p = strstr(line, "type"))
    //             {
    //                 p = FindFirstCharOrNumber(p + strlen("type"));
    //                 if (strcmp(p, "vertex") == 0)
    //                     pShader->m_ShaderType = EShaderType::EShaderType_Vertex;
    //                 else if (strcmp(p, "pixel") == 0)
    //                     pShader->m_ShaderType = EShaderType::EShaderType_Pixel;
    //             }
    //             else if (char *p = strstr(line, "macro"))
    //             {
    //                 p = FindFirstCharOrNumber(p + strlen("macro"));
    //                 pShader->m_ShaderMacros.Add(String(p));
    //             }
    //             else if (char *p = strstr(line, "file"))
    //             {
    //                 p = FindFirstCharOrNumber(p + strlen("file"));
    //                 pShader->m_ShaderFile = p;
    //             }
    //         }
    //     }
    //     fclose(pFile);

    //     for (const auto & Shader : Shaders)
    //     {
    //         FILE *pShaderFile = fopen(Shader->m_ShaderFile.c_str());
    //         if (pShaderFile)
    //         {
    //             long len = fseek(pShaderFile, 0, SEEK_END);
    //             fseek(pShaderFile, 0, SEEK_SET);
    //             char *pShaderCodeBuffer = (char *)MEMALLOC(len);
    //             size_t bytesRead = fread(pShaderCodeBuffer, 1, len, pShaderFile)
    //             fclose(pShaderFile);
    //             if (bytesRead == len)
    //             {
    //             #ifdef RENDERAPI_DX9
    //                 DWORD dwFlag = 0;
    //             #ifdef _DEBUG
    //                 dwFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    //             #endif
    //                 String szEntryPoint, szProfile;
    //                 switch (Shader->m_ShaderType)
    //                 {
    //                     case EShaderType::EShaderType_Vertex:
    //                         szEntryPoint = "VSMain";
    //                         szProfile = "vs_3_0";
    //                         break;
    //                     case EShaderType::EShaderType_Pixel:
    //                         szEntryPoint = "PSMain";
    //                         szProfile = "ps_3_0";
    //                         break;
    //                 }

    //                 CArray<D3D_SHADER_MACRO> D3DMacros;
    //                 for (const auto & Macro : Shader->m_ShaderMacros)
    //                 {
    //                     D3D_SHADER_MACRO D3DMacro;
    //                     D3DMacro.Name = Macro.c_str();
    //                     D3DMacro.Definition = "1";
    //                 }
    //                 D3DMacro EndMacro;
    //                 EndMacro.Name = 0;
    //                 EndMacro.Definition = 0;

    //                 ID3DBlob *pByteCode, *pErrorMsg;
    //                 HRESULT hr = D3DCompile(pShaderCodeBuffer, len, D3DMacros.Data(), nullptr, nullptr, szEntryPoint.c_str(), szProfile.c_str(), dwFlag, &pByteCode, &pErrorMsg);
    //             #endif
    //             }
    //         }
    //     }
    // }
}