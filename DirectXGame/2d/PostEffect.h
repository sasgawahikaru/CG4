#pragma once
#include "Sprite.h"


class PostEffect :
    public Sprite
{
public:
    PostEffect();

    void Draw(ID3D12GraphicsCommandList* cmdList);
    void Initialize();
protected:
    ComPtr<ID3D12Resource>texBuff;
    ComPtr<ID3D12DescriptorHeap>descHeapSRV;
};