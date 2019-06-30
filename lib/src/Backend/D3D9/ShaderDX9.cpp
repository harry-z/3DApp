#include "ShaderDX9.h"

CShaderDX9::CShaderDX9() {
    m_Shader.m_pVertexShader = nullptr;
}

CShaderDX9::~CShaderDX9() {
    switch (m_Type) {
        case EShaderType::EShaderType_Vertex:
        {
            SAFE_RELEASE(m_Shader.m_pVertexShader);
            break;
        }
        case EShaderType::EShaderType_Pixel:
        {
            SAFE_RELEASE(m_Shader.m_pPixelShader);
            break;
        }
    }
}

bool CShaderDX9::Load(EShaderType eType, const byte *pszShaderByteCode) {
    if (eType != EShaderType::EShaderType_Unknown && pszShaderByteCode != nullptr) {
        m_Type = eType;
        switch (m_Type) {
            case EShaderType::EShaderType_Vertex:
            {
                HRESULT hr = g_pDevice9->CreateVertexShader((const DWORD*)pszShaderByteCode, &m_Shader.m_pVertexShader);
                return SUCCEEDED(hr);
            }
            case EShaderType::EShaderType_Pixel:
            {
                HRESULT hr = g_pDevice9->CreatePixelShader((const DWORD*)pszShaderByteCode, &m_Shader.m_pPixelShader);
                return SUCCEEDED(hr);
            }
            default:
                return false;
        }
    }
    else
    {
        return false;
    }
}

CShaderManagerDX9::~CShaderManagerDX9() {
    m_ShaderMap.Clear();
    for (auto &Shader : m_ShaderArr)
    {
        CShaderDX9 *pShaderDX9 = static_cast<CShaderDX9*>(Shader);
        DELETE_TYPE(pShaderDX9, CShaderDX9);
    }
    m_ShaderArr.Clear();
    if (m_pDefaultVertexShader != nullptr) {
        CShaderDX9 *pDefaultShaderDX9 = static_cast<CShaderDX9*>(m_pDefaultVertexShader);
        DELETE_TYPE(pDefaultShaderDX9, CShaderDX9);
        m_pDefaultVertexShader = nullptr;
    }
    if (m_pDefaultPixelShader != nullptr) {
        CShaderDX9 *pDefaultShaderDX9 = static_cast<CShaderDX9*>(m_pDefaultPixelShader);
        DELETE_TYPE(pDefaultShaderDX9, CShaderDX9);
        m_pDefaultPixelShader = nullptr;
    }
}

bool CShaderManagerDX9::LoadShaders() {
    String szDefaultShader =
#ifdef RENDERAPI_DX9
		"float4x4 WorldViewProj : register(c0);\n" \
		"struct VSOutput {\n" \
		"float4 oPosition : POSITION;\n" \
		"};\n" \
		"VSOutput VSMain(float4 iPosition : POSITION) {\n" \
		"VSOutput output;\n" \
		"output.oPosition = mul(WorldViewProj, iPosition);\n" \
		"return output;\n" \
		"}\n" \
		"float4 PSMain() : COLOR0 {\n" \
		"return float4(1.0f, 1.0f, 1.0f, 1.0f);\n" \
		"}";

    DWORD dwFlag = 0;
#ifdef SHADER_DEBUGGING
    BIT_ADD(dwFlag, (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION));
#endif

    ID3DBlob *pVertexByteCode, *pPixelByteCode, *pByteError;
    HRESULT hr = D3DCompile(szDefaultShader.c_str(), szDefaultShader.length() + 1, nullptr, nullptr, nullptr, "VSMain", "vs_3_0", dwFlag, 0, &pVertexByteCode, &pByteError);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pByteError);
        return false;
    }
    hr = D3DCompile(szDefaultShader.c_str(), szDefaultShader.length() + 1, nullptr, nullptr, nullptr, "PSMain", "ps_3_0", dwFlag, 0, &pPixelByteCode, &pByteError); 
    if (FAILED(hr))
    {
        SAFE_RELEASE(pByteError);
        return false;
    }

    m_pDefaultVertexShader = NEW_TYPE(CShaderDX9);
    if (!m_pDefaultVertexShader->Load(EShaderType::EShaderType_Vertex, (byte *)pVertexByteCode->GetBufferPointer()))
    {
        SAFE_RELEASE(pVertexByteCode);
        return false;
    }

    m_pDefaultPixelShader = NEW_TYPE(CShaderDX9);
    if (!m_pDefaultPixelShader->Load(EShaderType::EShaderType_Pixel, (byte *)pPixelByteCode->GetBufferPointer()))
    {
        SAFE_RELEASE(pPixelByteCode);
        return false;
    }

    SAFE_RELEASE(pVertexByteCode);
    SAFE_RELEASE(pPixelByteCode);
#endif

    CFile file;
    if (file.Open("./Shader.bundle", "r")) {
        long nFileLen = file.Length();
        void *p = malloc(nFileLen);
        file.ReadRaw(p, nFileLen);
        file.Close();

        
        word nShaderId = 0;

        byte *pBuffer = static_cast<byte*>(p);
        byte *pBufferStart = pBuffer;

        dword nNumShader = 0;
        dword nNumShaderRead = 0;
        GetDwords(pBuffer, &nNumShader, 1);

        while (nNumShaderRead < nNumShader) {
            String szShaderName = GetString(pBuffer);
            byte nShaderType;
            GetBytes(pBuffer, &nShaderType, 1);
            dword nShaderByteCodeLen;
            GetDwords(pBuffer, &nShaderByteCodeLen, 1);
            byte *pByteCode = static_cast<byte *>(malloc(nShaderByteCodeLen));
            GetBytes(pBuffer, pByteCode, nShaderByteCodeLen);
            CShaderDX9 *pShaderDX9 = NEW_TYPE(CShaderDX9);
            if (pShaderDX9->Load((EShaderType)nShaderType, pByteCode)) {
                pShaderDX9->m_nId = nShaderId++;
                m_ShaderMap.Insert(IdString(szShaderName), pShaderDX9);
                m_ShaderArr.Emplace(pShaderDX9);
            }
            else
            {
                DELETE_TYPE(pShaderDX9, CShaderDX9);
            }
            free(pByteCode);

            ++nNumShaderRead;
        }

        free(p);

        return true;
    }
    else
        return false;
}