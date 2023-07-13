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
    static const float clearColor[4];
private:
    ComPtr<ID3D12Resource>texBuff;
    ComPtr<ID3D12Resource>depthBuff;
    ComPtr<ID3D12DescriptorHeap>descHeapSRV;
    ComPtr<ID3D12DescriptorHeap>descHeapRTV;
    ComPtr<ID3D12DescriptorHeap>descHeapDSV;
};