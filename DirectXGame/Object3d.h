#pragma once

#include"Model.h"
#include"Camera.h"

#include <string>
#include <DirectXMath.h>
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>

class Object3d
{
protected:
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;



	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 position = { 0,0,0 };
	XMMATRIX matWorld;
	Model* model = nullptr;

public:
	static void SetDevice(ID3D12Device* device) { Object3d::device = device; }
	static void SetCamera(Camera* camera) { Object3d::camera = camera; }
	static void CreateGraphicsPipeline();

private:
	static ID3D12Device* device;
	static Camera* camera;

	static ComPtr<ID3D12RootSignature> rootsignature;
	static ComPtr<ID3D12PipelineState> pipelinestate;

public:
	struct ConstBufferDataTransform
	{
		XMMATRIX viewproj;
		XMMATRIX world;
		XMFLOAT3 cameraPos;
	};

public:
	void Initialize();

	void Update();

	void SetModel(Model* model) { this->model = model; }

	void Draw(ID3D12GraphicsCommandList* cmdList);

protected:
	ComPtr<ID3D12Resource>constBuffTransform;

};