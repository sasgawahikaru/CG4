#pragma once
#include "Sprite.h"


class PostEffect :
    public Sprite
{
public:
    PostEffect();

    void Draw(ID3D12GraphicsCommandList* cmdList);
    void Initialize();
    void PreDrawScene(ID3D12GraphicsCommandList* cmdList);
    void PostDrawScene(ID3D12GraphicsCommandList* cmdList);
    void CreateGraphicsPipelineState();
    static const float clearColor[4];
private:
    ComPtr<ID3D12Resource>texBuff[2];
    ComPtr<ID3D12Resource>depthBuff;
    ComPtr<ID3D12DescriptorHeap>descHeapSRV;
    ComPtr<ID3D12DescriptorHeap>descHeapRTV;
    ComPtr<ID3D12DescriptorHeap>descHeapDSV;
    ComPtr<ID3D12PipelineState>pipelineState;
    ComPtr<ID3D12RootSignature>rootSignature;
};