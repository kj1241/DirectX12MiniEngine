#include "stdafx.h"
#include "DirectX12UI.h"

void FontUI::RenderText(ComPtr<ID3D12GraphicsCommandList> directX12_commandList)
{

}

FontUI FontUI::LoadFont(LPCWSTR filename, int windowWidth, int windowHeight)
{
 
}

float FontUI::GetKerning(wchar_t first, wchar_t second)
{
    for (int i = 0; i < numKernings; ++i)
    {
        if ((wchar_t)KerningsList[i].firstid == first && (wchar_t)KerningsList[i].secondid == second)
            return KerningsList[i].amount;
    }
    return 0.0f;
}

FontChar* FontUI::GetChar(wchar_t c)
{
    for (int i = 0; i < numCharacters; ++i)
    {
        if (c == (wchar_t)CharList[i].id)
            return &CharList[i];
    }
    return nullptr;
}

void FontUI::LoadAssets(std::wstring assetsPath, ComPtr<ID3D12Device> directX12_device, ComPtr<ID3D12RootSignature> directX12_rootSignature)
{//text pso
    ComPtr<ID3DBlob> textVertexShader; //���ؽ� ���̴�
    ComPtr<ID3DBlob> textPixelShader;  //�Ƚ����̴�

#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif
    ThrowIfFailed(D3DCompileFromFile((assetsPath + L"../../TextShaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &textVertexShader, nullptr)); //���ؽ� ���̴��� hlsl VSMain ���
    ThrowIfFailed(D3DCompileFromFile((assetsPath + L"../../TextShaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &textPixelShader, nullptr));  //�ȼ� ���̴� PSMain ���

    D3D12_SHADER_BYTECODE textVertexShaderBytecode = {};     // ���ý����̴� ����Ʈ �ڵ�� ũ��
    textVertexShaderBytecode.BytecodeLength = textVertexShader->GetBufferSize();
    textVertexShaderBytecode.pShaderBytecode = textVertexShader->GetBufferPointer();

    D3D12_SHADER_BYTECODE textPixelShaderBytecode = {};      //  �ȼ����̴� ����Ʈ �ڵ�� ũ��
    textPixelShaderBytecode.BytecodeLength = textPixelShader->GetBufferSize();
    textPixelShaderBytecode.pShaderBytecode = textPixelShader->GetBufferPointer();

    D3D12_INPUT_ELEMENT_DESC textInputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
    };

    // �Է� ���̾ƿ� �����ۼ�
    D3D12_INPUT_LAYOUT_DESC textInputLayoutDesc = {};
    textInputLayoutDesc.NumElements = sizeof(textInputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    textInputLayoutDesc.pInputElementDescs = textInputLayout;

    //���������� PSO ��ä ����
    D3D12_GRAPHICS_PIPELINE_STATE_DESC textpsoDesc = {};
    textpsoDesc.InputLayout = textInputLayoutDesc;
    textpsoDesc.pRootSignature = directX12_rootSignature.Get();
    textpsoDesc.VS = textVertexShaderBytecode;
    textpsoDesc.PS = textPixelShaderBytecode;
    textpsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    textpsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    //textpsoDesc.SampleDesc = sampleDesc;
    textpsoDesc.SampleMask = 0xffffffff;
    textpsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

    D3D12_BLEND_DESC textBlendStateDesc = {};
    textBlendStateDesc.AlphaToCoverageEnable = FALSE;
    textBlendStateDesc.IndependentBlendEnable = FALSE;
    textBlendStateDesc.RenderTarget[0].BlendEnable = TRUE;

    textBlendStateDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    textBlendStateDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    textBlendStateDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

    textBlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
    textBlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    textBlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    textBlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    textpsoDesc.BlendState = textBlendStateDesc;
    textpsoDesc.NumRenderTargets = 1;
    D3D12_DEPTH_STENCIL_DESC textDepthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    textDepthStencilDesc.DepthEnable = false;
    textpsoDesc.DepthStencilState = textDepthStencilDesc;

    directX12_device->CreateGraphicsPipelineState(&textpsoDesc, IID_PPV_ARGS(&textPSO));
}

void FontUI::LoadVertextBuffer(ComPtr<ID3D12Device> directX12_device)
{
    for (int i = 0; i < frameBufferCount; ++i)
    {
        // ���ε��� ���� �ý�ó ä���
        ID3D12Resource* vBufferUploadHeap;
        directX12_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // ���ε���
            D3D12_HEAP_FLAG_NONE, // �÷��׾���
            &CD3DX12_RESOURCE_DESC::Buffer(maxNumTextCharacters * sizeof(TextVertex)), // ���ۿ����� ���ҽ�����
            D3D12_RESOURCE_STATE_GENERIC_READ, // GPU���۸� �а� ������ �⺻������ ����
            nullptr,
            IID_PPV_ARGS(&textVertexBuffer[i]));
      
        textVertexBuffer[i]->SetName(L"Text Vertex Buffer Upload Resource Heap");

        CD3DX12_RANGE readRange(0, 0);    //cpu���� ���� ���� ����

        //���ҽ� �����Ͽ� GPU�����ּ� ������
       textVertexBuffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&textVBGPUAddress[i]));
    }
}

void FontUI::LoadVertextBufferView()
{
    //�������ӿ����� ���ؽ� ���ۺ�
    for (int i = 0; i < frameBufferCount; ++i)
    {
        textVertexBufferView[i].BufferLocation = textVertexBuffer[i]->GetGPUVirtualAddress();
        textVertexBufferView[i].StrideInBytes = sizeof(TextVertex);
        textVertexBufferView[i].SizeInBytes = maxNumTextCharacters * sizeof(TextVertex);
    }
}

void FontUI::UpdatePipline()
{
    RenderText(arialFont, std::wstring(L"FPS: ") + std::to_wstring(4), XMFLOAT2(0.02f, 0.01f), XMFLOAT2(2.0f, 2.0f));
}


