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
                return SUCCEEDED(hr) && FillConstantMap((const DWORD*)pszShaderByteCode);
            }
            case EShaderType::EShaderType_Pixel:
            {
                HRESULT hr = g_pDevice9->CreatePixelShader((const DWORD*)pszShaderByteCode, &m_Shader.m_pPixelShader);
                return SUCCEEDED(hr) && FillConstantMap((const DWORD*)pszShaderByteCode);
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

bool CShaderDX9::FillConstantMap(const DWORD *pFunction)
{
    ID3DXConstantTable *pConstTable;
    HRESULT hr = D3DXGetShaderConstantTable(pFunction, &pConstTable);
    if (FAILED(hr))
        return false;
    D3DXCONSTANTTABLE_DESC ConstTableDesc;
    hr = pConstTable->GetDesc(&ConstTableDesc);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pConstTable);
        return false;
    }
    for (UINT i = 0; i < ConstTableDesc.Constants; ++i)
    {
        D3DXHANDLE hConst = pConstTable->GetConstant(NULL, i);
        D3DXCONSTANT_DESC ConstDesc;
        UINT n;
        hr = pConstTable->GetConstantDesc(hConst, &ConstDesc, &n);
        if (SUCCEEDED(hr))
        {
            m_ConstantMap.Insert(IdString(ConstDesc.Name), ConstDesc.RegisterIndex);
        }
    }
    SAFE_RELEASE(pConstTable);
    return true;
}

dword CShaderDX9::GetConstantIndexByName(const IdString &szName) const
{
    CMap<IdString, dword>::_MyConstIterType CIter = m_ConstantMap.Find(szName);
    return CIter ? CIter.Value() : 0xFFFFFFFF;
}

CShaderManagerDX9::~CShaderManagerDX9() {
    m_ShaderMap.Clear();
    for (auto &Shader : m_ShaderArr)
    {
        CShaderDX9 *pShaderDX9 = static_cast<CShaderDX9*>(Shader);
        pShaderDX9->~CShaderDX9();
        DELETE_TYPE(pShaderDX9, CShaderDX9);
    }
    m_ShaderArr.Clear();
    if (m_pDefaultVertexShader != nullptr) {
        CShaderDX9 *pDefaultShaderDX9 = static_cast<CShaderDX9*>(m_pDefaultVertexShader);
        pDefaultShaderDX9->~CShaderDX9();
        DELETE_TYPE(pDefaultShaderDX9, CShaderDX9);
        m_pDefaultVertexShader = nullptr;
    }
    if (m_pDefaultPixelShader != nullptr) {
        CShaderDX9 *pDefaultShaderDX9 = static_cast<CShaderDX9*>(m_pDefaultPixelShader);
        pDefaultShaderDX9->~CShaderDX9();
        DELETE_TYPE(pDefaultShaderDX9, CShaderDX9);
        m_pDefaultPixelShader = nullptr;
    }
}

bool CShaderManagerDX9::LoadShaders() {
    CLog * __restrict pLog = Global::m_pLog;

    String szDefaultShader =
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

    ID3DXBuffer *pVertexByteCode = nullptr, *pPixelByteCode = nullptr, *pByteError = nullptr;
    HRESULT hr = D3DXCompileShader(szDefaultShader.c_str(), szDefaultShader.length(), nullptr, nullptr, "VSMain", "vs_3_0", dwFlag, &pVertexByteCode, &pByteError, nullptr);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pByteError);
        return false;
    }
    hr = D3DXCompileShader(szDefaultShader.c_str(), szDefaultShader.length(), nullptr, nullptr, "PSMain", "ps_3_0", dwFlag, &pPixelByteCode, &pByteError, nullptr); 
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
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create Default Vertex Shader");

    m_pDefaultPixelShader = NEW_TYPE(CShaderDX9);
    if (!m_pDefaultPixelShader->Load(EShaderType::EShaderType_Pixel, (byte *)pPixelByteCode->GetBufferPointer()))
    {
        SAFE_RELEASE(pPixelByteCode);
        return false;
    }
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create Default Pixel Shader");

    SAFE_RELEASE(pVertexByteCode);
    SAFE_RELEASE(pPixelByteCode);

    CFile file;
    if (file.Open("./Shader.bundle", "r")) {
        long nFileLen = file.Length();
        void *p = MEMALLOC(nFileLen);
        file.ReadRaw(p, nFileLen);
        file.Close();

        
        word nShaderId = 1;

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
            byte *pByteCode = static_cast<byte *>(MEMALLOC(nShaderByteCodeLen));
            GetBytes(pBuffer, pByteCode, nShaderByteCodeLen);
            CShaderDX9 *pShaderDX9 = NEW_TYPE(CShaderDX9);
            if (pShaderDX9->Load((EShaderType)nShaderType, pByteCode)) {
                pShaderDX9->m_nId = nShaderId++;
                m_ShaderMap.Insert(IdString(szShaderName), pShaderDX9);
                m_ShaderArr.Emplace(pShaderDX9);
                LOG_ARGS(pLog, ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Load Shader: %s", szShaderName.c_str());
            }
            else
            {
                DELETE_TYPE(pShaderDX9, CShaderDX9);
                LOG_ARGS(pLog, ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Failed to Load Shader: %s", szShaderName.c_str());
            }
            MEMFREE(pByteCode);

            ++nNumShaderRead;
        }

        MEMFREE(p);

        return true;
    }
    else
        return false;
}