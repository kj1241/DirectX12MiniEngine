#include "stdafx.h"
#include "DirectX12UI.h"

void DirectX12UI::RenderText(ID3D12GraphicsCommandList* directX12_commandList, ID3D12DescriptorHeap* directX12_dsvHeap,BMFontUI font, std::wstring text, XMFLOAT2 pos, XMFLOAT2 scale, XMFLOAT2 padding, XMFLOAT4 color)
{
    directX12_commandList->ClearDepthStencilView(directX12_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);     //������ �׸��� �ֵ��� ���̹��� ���
    directX12_commandList->SetPipelineState(directX12_textPSO.Get());     //�ؽ�Ʈ���������� ���� ��ü ����
    directX12_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);   //�ﰢ�� �������� ����� 6�����ƴ� 4�� �ʿ�
    directX12_commandList->IASetVertexBuffers(0, 1, &directX12_textVertexBufferView[FrameCount]);
    directX12_commandList->SetGraphicsRootDescriptorTable(1, font.GetSrvHandle());

    int numCharacters = 0;

    float topLeftScreenX = (pos.x * 2.0f) - 1.0f;
    float topLeftScreenY = ((1.0f - pos.y) * 2.0f) - 1.0f;

    float x = topLeftScreenX;
    float y = topLeftScreenY;

    float horrizontalPadding = (font.GetLeftpadding() + font.GetRightpadding()) * padding.x;
    float verticalPadding = (font.GetToppadding() + font.GetBottompadding()) * padding.y;

    TextVertex* vert = (TextVertex*)directX12_textVBGPUAddress[FrameCount];

    WCHAR lastChar = -1; //�����Ҷ� ������ ���ڴ� ����

    for (int i = 0; i < text.size(); ++i)
    {
        WCHAR c = text[i];
        BMFontUI::FontChar* fc = font.GetChar(c);
        if (fc == nullptr) //�۲� �� ������
            continue;
        
        if (c == L'\0') // ���ڿ��� ��
            break;

        if (c == L'\n') //����
        {
            x = topLeftScreenX;
            y -= (font.GetLineHeight() + verticalPadding) * scale.y;
            continue;
        }

        if (numCharacters >= maxNumTextCharacters)  // ���� �����÷ο�� ��������
            break;

        float kerning = 0.0f;
        if (i > 0)
            kerning = font.GetKerning(lastChar, c);

        vert[numCharacters].color = color;
        vert[numCharacters].pos = XMFLOAT4(x + ((fc->xoffset + kerning) * scale.x), y - (fc->yoffset * scale.y), fc->width * scale.x, fc->height * scale.y);
        vert[numCharacters].texCoord = XMFLOAT4(fc->u, fc->v, fc->textWidth, fc->textHeight);
        numCharacters++;

        x += (fc->xadvance - horrizontalPadding) * scale.x;      // �����е��� �����ϰ� ���� ���� ��ġ

        lastChar = c;
    }
    directX12_commandList->DrawInstanced(4, numCharacters, 0, 0);     //���ڴ� 4���� ������ 
}

DirectX12UI::DirectX12UI(int frameCount): FrameCount(frameCount)
{
    //����Ʈ ����Ʈ �ʱ�ȭ 
    directX12_textVertexBuffer.reserve(FrameCount);  //���� �ʱ�ȭ ���� resize �����Ұ�������
    directX12_textVertexBufferView.reserve(FrameCount);
    directX12_textVBGPUAddress.reserve(FrameCount);

    BMFont.LoadFont(L"Arial.fnt", 900, 1600);

}

DirectX12UI::~DirectX12UI() //�Ҹ��� //������
{
}

void DirectX12UI::LoadAssets(std::wstring assetsPath, ComPtr<ID3D12Device> directX12_device, ComPtr<ID3D12RootSignature> directX12_rootSignature)
{
    //text pso
    ComPtr<ID3DBlob> textVertexShader; //���ؽ� ���̴�
    ComPtr<ID3DBlob> textPixelShader;  //�Ƚ����̴�

#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif
    ThrowIfFailed(D3DCompileFromFile(assetsPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &textVertexShader, nullptr)); //���ؽ� ���̴��� hlsl VSMain ���
    ThrowIfFailed(D3DCompileFromFile(assetsPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &textPixelShader, nullptr));  //�ȼ� ���̴� PSMain ���

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

    directX12_device->CreateGraphicsPipelineState(&textpsoDesc, IID_PPV_ARGS(&directX12_textPSO));
}
/*
void DirectX12UI::LoadVertextBuffer(ComPtr<ID3D12Device> directX12_device, ComPtr<ID3D12GraphicsCommandList> directX12_commandList, ComPtr<ID3D12DescriptorHeap> directX12_dsvHeap)
{
    // Load the image from file
    D3D12_RESOURCE_DESC fontTextureDesc;
    int fontImageBytesPerRow;
    BYTE* fontImageData;
    int fontImageSize = LoadImageDataFromFile(&fontImageData, fontTextureDesc, BMFont.GetFontImage().c_str(), fontImageBytesPerRow);

    // create the font texture resource
    directX12_device->CreateCommittedResource(
        &keep(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
        D3D12_HEAP_FLAG_NONE,
        &fontTextureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&BMFont.GetTextureBuffer()));
    
    BMFont.GetTextureBuffer()->SetName(L"Font Texture Buffer Resource Heap");

    ID3D12Resource* fontTextureBufferUploadHeap;
    UINT64 fontTextureUploadBufferSize;
    directX12_device->GetCopyableFootprints(&fontTextureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &fontTextureUploadBufferSize);

    // create an upload heap to copy the texture to the gpu
    directX12_device->CreateCommittedResource(
        &keep(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
        D3D12_HEAP_FLAG_NONE, // no flags
        &keep(CD3DX12_RESOURCE_DESC::Buffer(fontTextureUploadBufferSize)),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&fontTextureBufferUploadHeap));
   
    fontTextureBufferUploadHeap->SetName(L"Font Texture Buffer Upload Resource Heap");

    // store font image in upload heap
    D3D12_SUBRESOURCE_DATA fontTextureData = {};
    fontTextureData.pData = &fontImageData[0]; // pointer to our image data
    fontTextureData.RowPitch = fontImageBytesPerRow; // size of all our triangle vertex data
    fontTextureData.SlicePitch = fontImageBytesPerRow * fontTextureDesc.Height; // also the size of our triangle vertex data



    // Now we copy the upload buffer contents to the default heap
    UpdateSubresources(directX12_commandList.Get(), BMFont.GetTextureBuffer().Get(), fontTextureBufferUploadHeap, 0, 0, 1, &fontTextureData);

    // transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
    directX12_commandList->ResourceBarrier(
        1, 
        &keep(CD3DX12_RESOURCE_BARRIER::Transition(BMFont.GetTextureBuffer().Get(),D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)));


    // create an srv for the font
    D3D12_SHADER_RESOURCE_VIEW_DESC fontsrvDesc = {};
    fontsrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    fontsrvDesc.Format = fontTextureDesc.Format;
    fontsrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    fontsrvDesc.Texture2D.MipLevels = 1;

    // we need to get the next descriptor location in the descriptor heap to store this srv
    srvHandleSize = directX12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    BMFont.SetSrvHandle(CD3DX12_GPU_DESCRIPTOR_HANDLE(directX12_dsvHeap->GetGPUDescriptorHandleForHeapStart(), 1, srvHandleSize));

    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(directX12_dsvHeap->GetCPUDescriptorHandleForHeapStart(), 1, srvHandleSize);
    directX12_device->CreateShaderResourceView(BMFont.GetTextureBuffer().Get(), &fontsrvDesc, srvHandle);

    for (int i = 0; i < FrameCount; ++i)
    {
        // ���ε��� ���� �ý�ó ä���
        ID3D12Resource* vBufferUploadHeap;
        directX12_device->CreateCommittedResource(
            &keep(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)), // ���ε���
            D3D12_HEAP_FLAG_NONE, // �÷��׾���
            &keep(CD3DX12_RESOURCE_DESC::Buffer(maxNumTextCharacters * sizeof(TextVertex))), // ���ۿ����� ���ҽ�����
            D3D12_RESOURCE_STATE_GENERIC_READ, // GPU���۸� �а� ������ �⺻������ ����
            nullptr,
            IID_PPV_ARGS(&directX12_textVertexBuffer[i]));

        directX12_textVertexBuffer[i]->SetName(L"���ҽ� ���� ���ε�� �ؽ��� ����");

        CD3DX12_RANGE readRange(0, 0);    //cpu���� ���� ���� ����

        //���ҽ� �����Ͽ� GPU�����ּ� ������
        directX12_textVertexBuffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&directX12_textVBGPUAddress[i]));
    }
}
*/
void DirectX12UI::LoadVertextBufferView()
{
    //�������ӿ����� ���ؽ� ���ۺ�
    for (int i = 0; i < FrameCount; ++i)
    {
        directX12_textVertexBufferView[i].BufferLocation = directX12_textVertexBuffer[i]->GetGPUVirtualAddress();
        directX12_textVertexBufferView[i].StrideInBytes = sizeof(TextVertex);
        directX12_textVertexBufferView[i].SizeInBytes = maxNumTextCharacters * sizeof(TextVertex);
    }
}

void DirectX12UI::UpdatePipline(ComPtr<ID3D12GraphicsCommandList> directX12_commandList, ComPtr<ID3D12DescriptorHeap> directX12_dsvHeap)
{
    RenderText(directX12_commandList.Get(), directX12_dsvHeap.Get(), BMFont, std::wstring(L"FPS: ") + std::to_wstring(4), XMFLOAT2(0.02f, 0.01f), XMFLOAT2(2.0f, 2.0f));
}


