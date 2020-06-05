#include "ShaderDX11.h"
#include "RenderBackendDX11.h"

CShaderDX11::CShaderDX11() {
    m_Shader.m_pVertexShader = nullptr;
    for (auto &pConstantBuffer : m_ConstantBuffers)
    {
        MEMFREE(pConstantBuffer);
    }
}

CShaderDX11::~CShaderDX11() {
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

bool CShaderDX11::Load(EShaderType eType, const byte *pszShaderByteCode, dword nCodeSize) {
    if (eType != EShaderType::EShaderType_Unknown && pszShaderByteCode != nullptr) {
        m_Type = eType;
        bool bResult = false;
        switch (m_Type) {
            case EShaderType::EShaderType_Vertex:
            {
                HRESULT hr = g_pDevice11->CreateVertexShader(pszShaderByteCode, nCodeSize, nullptr, &m_Shader.m_pVertexShader);
                bResult = SUCCEEDED(hr) && FillVariableMap(pszShaderByteCode, nCodeSize);
            }
            case EShaderType::EShaderType_Pixel:
            {
                HRESULT hr = g_pDevice11->CreatePixelShader(pszShaderByteCode, nCodeSize, nullptr, &m_Shader.m_pPixelShader);
                bResult = SUCCEEDED(hr) && FillVariableMap(pszShaderByteCode, nCodeSize);
            }
            default:
                return false;
        }
        if (bResult) {
            m_pByteCode = pszShaderByteCode;
            m_nByteCodeLen = nCodeSize;
        }
    }
    else
    {
        return false;
    }
}

bool CShaderDX11::FillVariableMap(LPCVOID pFunction, dword nCodeSize)
{
    ID3D11ShaderReflection *pShaderReflection = nullptr;
    HRESULT hr = D3D11Reflect(pFunction, nCodeSize, &pShaderReflection);
    if (SUCCEEDED(hr))
    {
        struct Reflector {
            ID3D11ShaderReflection *m_pReflector = nullptr;
            ~Reflector() {
                SAFE_RELEASE(m_pReflector);
            }
        } Ref;
        Ref.m_pReflector = pShaderReflection;

        D3D11_SHADER_DESC ShaderDesc;
        hr = pShaderReflection->GetDesc(&ShaderDesc);
        if (FAILED(hr))
            return false;
        
        UINT nNumConstBuffer = ShaderDesc.ConstantBuffers;
        for (UINT nConstBuffer = 0; nConstBuffer < nNumConstBuffer; ++nConstBuffer)
        {
            ID3D11ShaderReflectionConstantBuffer *pConstantBuffer = pShaderReflection->GetConstantBufferByIndex(nConstBuffer);
            D3D11_SHADER_BUFFER_DESC ShaderBufferDesc;
            pConstantBuffer->GetDesc(&ShaderBufferDesc);
            UINT nRegisterIndex = 0xFFFFFFFF;
            for (UINT nResource = 0; nResource < ShaderDesc.BoundResources; ++nResource)
            {
                D3D11_SHADER_INPUT_BIND_DESC InputBindDesc;
                if (SUCCEEDED(pShaderReflection->GetResourceBindingDesc(nResource, &InputBindDesc)))
                {
                    if (InputBindDesc.Type == D3D_SIT_CBUFFER && strcmp(InputBindDesc.Name, ShaderBufferDesc.Name) == 0)
                    {
                        nRegisterIndex = InputBindDesc.BindPoint;
                        break;
                    }
                }
            }
            if (nRegisterIndex == 0xFFFFFFFF)
                continue;
            void *pConstantBuffer = MEMALLOC(ShaderBufferDesc.Size);
            m_ConstantBuffers.Add(pConstantBuffer);
            for (UINT nVariable = 0; nVariable < ShaderBufferDesc.Variables; ++nVariable)
            {
                ID3D11ShaderReflectionVariable *pVariable = pConstantBuffer->GetVariableByIndex(nVariable);
                D3D11_SHADER_VARIABLE_DESC ShaderVariableDesc;
                pVariable->GetDesc(&ShaderVariableDesc);
                ID3D11ShaderReflectionType *pVariableType = pVariable->GetType();
                D3D11_SHADER_TYPE_DESC ShaderTypeDesc;
                pVariableType->GetDesc(&ShaderTypeDesc);
                m_VariableMap.Insert(IdString(ShaderVariableDesc.Name), 
                    ShaderVariableInfo(nRegisterIndex, MappingShaderConstantType(ShaderTypeDesc.Type), ShaderVariableDesc.StartOffset, ShaderVariableDesc.Size));
            }
        }
        return true;
    }
    else
        return false;
}

CShaderManagerDX11::~CShaderManagerDX11() {
    m_ShaderMap.Clear();
    for (auto &Shader : m_ShaderArr)
    {
        CShaderDX11 *pShaderDX11 = static_cast<CShaderDX11*>(Shader);
        DELETE_TYPE(pShaderDX11, CShaderDX11);
    }
    m_ShaderArr.Clear();
    if (m_pDefaultVertexShader != nullptr) {
        CShaderDX11 *pDefaultShaderDX11 = static_cast<CShaderDX11*>(m_pDefaultVertexShader);
        DELETE_TYPE(pDefaultShaderDX11, CShaderDX11);
        m_pDefaultVertexShader = nullptr;
    }
    if (m_pDefaultPixelShader != nullptr) {
        CShaderDX11 *pDefaultShaderDX11 = static_cast<CShaderDX11*>(m_pDefaultPixelShader);
        DELETE_TYPE(pDefaultShaderDX11, CShaderDX11);
        m_pDefaultPixelShader = nullptr;
    }
}

bool CShaderManagerDX11::LoadShaders() {
    CLog * __restrict pLog = Global::m_pLog;

    String szDefaultShader =
		"cbuffer cbTransform {\n" \
        "float4x4 WorldViewProjectionMatrix;\n" \
        "};\n" \
		"struct VSOutput {\n" \
		  "float4 oPosition : SV_POSITION;\n" \
		"};\n" \
		"VSOutput VS_Default(float4 iPosition : POSITION) {\n" \
		  "VSOutput output;\n" \
		  "output.oPosition = mul(WorldViewProjectionMatrix, iPosition);\n" \
		  "return output;\n" \
		"}\n" \
		"float4 PS_Default() : SV_TARGET0 {\n" \
		  "return float4(1.0f, 1.0f, 1.0f, 1.0f);\n" \
		"}";

    UINT dwFlag = 0;
#ifdef SHADER_DEBUGGING
    BIT_ADD(nFlag, (D3DCOMPILE_DEBUG | D3DCOMPILE_OPTIMIZATION_LEVEL0));
#endif

    ID3DBlob *pVertexByteCode = nullptr, *pPixelByteCode = nullptr, *pByteError = nullptr;
    HRESULT hr = D3DCompile(szDefaultShader.c_str(), szDefaultShader.length(), nullptr, nullptr, "VS_Default", "vs_5_0", dwFlag, 0, &pVertexByteCode, &pByteError);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pByteError);
        return false;
    }
    hr = D3DCompile(szDefaultShader.c_str(), szDefaultShader.length(), nullptr, nullptr, "PS_Default", "ps_5_0", dwFlag, 0, &pPixelByteCode, &pByteError); 
    if (FAILED(hr))
    {
        SAFE_RELEASE(pByteError);
        return false;
    }

    m_pDefaultVertexShader = NEW_TYPE(CShaderDX11);
    if (!m_pDefaultVertexShader->Load(EShaderType::EShaderType_Vertex, (byte *)pVertexByteCode->GetBufferPointer()))
    {
        SAFE_RELEASE(pVertexByteCode);
        return false;
    }
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create Default Vertex Shader");

    m_pDefaultPixelShader = NEW_TYPE(CShaderDX11);
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
            CShaderDX11 *pShaderDX11 = NEW_TYPE(CShaderDX11);
            if (pShaderDX11->Load((EShaderType)nShaderType, pByteCode, nShaderByteCodeLen)) {
                pShaderDX11->m_nId = nShaderId++;
                m_ShaderMap.Insert(IdString(szShaderName), pShaderDX11);
                m_ShaderArr.Emplace(pShaderDX11);
                LOG_ARGS(pLog, ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Load Shader: %s", szShaderName.c_str());
            }
            else
            {
                MEMFREE(pByteCode);
                DELETE_TYPE(pShaderDX11, CShaderDX11);
                LOG_ARGS(pLog, ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Failed to Load Shader: %s", szShaderName.c_str());
            }

            ++nNumShaderRead;
        }

        MEMFREE(p);

        return true;
    }
    else
        return false;
}

void CRenderBackendDX11::SetShader(CShader *pShader)
{
    switch (pShader->GetShaderType())
    {
        case EShaderType::EShaderType_Vertex:
        {
            CShaderDX11 *pShaderDX11 = static_cast<CShaderDX11*>(pShader);
            m_pD3DContext11->VSSetShader(pShaderDX11->m_Shader.m_pVertexShader);
            break;
        }
        case EShaderType::EShaderType_Pixel:
        {
            CShaderDX11 *pShaderDX11 = static_cast<CShaderDX11*>(pShader);
            m_pD3DContext11->PSSetShader(pShaderDX11->m_Shader.m_pPixelShader);
            break;
        }
    }
}

EShaderConstantType MappingShaderConstantType(D3D_SHADER_VARIABLE_TYPE d3dType)
{
    switch (d3dType)
    {
        case D3D_SVT_FLOAT:
            return EShaderConstantType::EShaderConstantType_Float;
        case D3D_SVT_INT:
            return EShaderConstantType::EShaderConstantType_Int;
        default:
            return EShaderConstantType::EShaderConstantType_Unknown;
    }
}