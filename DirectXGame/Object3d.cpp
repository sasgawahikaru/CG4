#include "Object3d.h"
#include "FbxLoader.h"

#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

using namespace Microsoft::WRL;
using namespace DirectX;

ID3D12Device* Object3d::device = nullptr;
Camera* Object3d::camera = nullptr;

ComPtr<ID3D12RootSignature> Object3d::rootsignature;
ComPtr<ID3D12PipelineState> Object3d::pipelinestate;

void Object3d::Initialize()
{
	HRESULT result;
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataTransform) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffTransform));

	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataSkin) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffSkin));

	frameTime.SetTime(0, 0, 0, 1, 0, FbxTime::EMode::eFrames60);

	ConstBufferDataSkin* constMapSkin = nullptr;
	result = constBuffSkin->Map(0, nullptr, (void**) & constMapSkin);
	for (int i = 0; i < MAX_BONES; i++) {
		constMapSkin->bones[i] = XMMatrixIdentity();
	}
	constBuffSkin->Unmap(0, nullptr);

}

void Object3d::CreateGraphicsPipeline()
{
		HRESULT result = S_FALSE;
		ComPtr<ID3DBlob> vsBlob; // 頂点シェーダオブジェクト
		ComPtr<ID3DBlob> psBlob;    // ピクセルシェーダオブジェクト
		ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト

		assert(device);

		// 頂点シェーダの読み込みとコンパイル
		result = D3DCompileFromFile(
			L"Resources/shaders/FBXVS.hlsl",    // シェーダファイル名
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
			"main", "vs_5_0",    // エントリーポイント名、シェーダーモデル指定
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
			0,
			&vsBlob, &errorBlob);
		if (FAILED(result)) {
			// errorBlobからエラー内容をstring型にコピー
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());

			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += "\n";
			// エラー内容を出力ウィンドウに表示
			OutputDebugStringA(errstr.c_str());
			exit(1);
		}

		// ピクセルシェーダの読み込みとコンパイル
		result = D3DCompileFromFile(
			L"Resources/shaders/FBXPS.hlsl",    // シェーダファイル名
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
			"main", "ps_5_0",    // エントリーポイント名、シェーダーモデル指定
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
			0,
			&psBlob, &errorBlob);
		if (FAILED(result)) {
			// errorBlobからエラー内容をstring型にコピー
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());

			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += "\n";
			// エラー内容を出力ウィンドウに表示
			OutputDebugStringA(errstr.c_str());
			exit(1);
		}

		// 頂点レイアウト
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ // xy座標(1行で書いたほうが見やすい)
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{ // 法線ベクトル(1行で書いたほうが見やすい)
				"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{ // uv座標(1行で書いたほうが見やすい)
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{
			"BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{
			"BONEWEIGHTS",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
			},
		};

		// グラフィックスパイプラインの流れを設定
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
		gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
		gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

		// サンプルマスク
		gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
		// ラスタライザステート
		gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		//gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		//gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		// デプスステンシルステート
		gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

		// レンダーターゲットのブレンド設定
		D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
		blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;    // RBGA全てのチャンネルを描画
		blenddesc.BlendEnable = true;
		blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
		blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

		// ブレンドステートの設定
		gpipeline.BlendState.RenderTarget[0] = blenddesc;

		// 深度バッファのフォーマット
		gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

		// 頂点レイアウトの設定
		gpipeline.InputLayout.pInputElementDescs = inputLayout;
		gpipeline.InputLayout.NumElements = _countof(inputLayout);

		// 図形の形状設定（三角形）
		gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		gpipeline.NumRenderTargets = 1;    // 描画対象は1つ
		gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0〜255指定のRGBA(SRGB版)
		gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

		// デスクリプタレンジ
		CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
		descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

		// ルートパラメータ
		CD3DX12_ROOT_PARAMETER rootparams[3];
		// CBV（座標変換行列用）
		rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
		// SRV（テクスチャ）
		rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);
		//
		rootparams[2].InitAsConstantBufferView(3, 0, D3D12_SHADER_VISIBILITY_ALL);


		// スタティックサンプラー
		CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

		// ルートシグネチャの設定
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> rootSigBlob;
		// バージョン自動判定のシリアライズ
		result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
		// ルートシグネチャの生成
		result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(rootsignature.ReleaseAndGetAddressOf()));
		if (FAILED(result)) { assert(0); }

		gpipeline.pRootSignature = rootsignature.Get();

		// グラフィックスパイプラインの生成
		result = device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(pipelinestate.ReleaseAndGetAddressOf()));
		if (FAILED(result)) { assert(0); }

	result = D3DCompileFromFile(
		L"Resources/shaders/FBXVS.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vsBlob, &errorBlob);

}

void Object3d::Update()
{

	XMMATRIX matScale, matRot, matTrans;

	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
	matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	matWorld = XMMatrixIdentity();
	matWorld *= matScale;
	matWorld *= matRot;
	matWorld *= matTrans;

	const XMMATRIX& matViewProjection =
		camera->GetViewProjectionMatrix();

	const XMMATRIX& modelTransform = model->GetModelTransform();

	const XMFLOAT3& cameraPos = camera->GetEye();

	HRESULT result;
	if (isPlay)
	{
		currentTime += frameTime;
		if (currentTime > endTime)
		{
			currentTime = startTime;
		}
	}
	ConstBufferDataTransform* constMap = nullptr;
	result = constBuffTransform->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(result))
	{
		constMap->viewproj = matViewProjection;
		constMap->world = modelTransform * matWorld;
		constMap->cameraPos = cameraPos;
		constBuffTransform->Unmap(0, nullptr);
	}

	std::vector<Model::Bone>& bones = model->GetBones();

	ConstBufferDataSkin* constMapSkin = nullptr;
	result = constBuffSkin->Map(0, nullptr, (void**)&constMapSkin);
	for (int i = 0; i < bones.size(); i++)
	{
		XMMATRIX matCurrentPose;
		FbxAMatrix fbxCurrentPose =
			bones[i].fbxCluster->GetLink()->EvaluateGlobalTransform(currentTime);

		FbxLoader::ConvertMatrixFromFbx(&matCurrentPose, fbxCurrentPose);
		constMapSkin->bones[i] = bones[i].invInitialPose * matCurrentPose;
	}
	constBuffSkin->Unmap(0, nullptr);
}

void Object3d::Draw(ID3D12GraphicsCommandList* cmdList)
{
	if (model == nullptr) {
		return;
	}
	cmdList->SetPipelineState(pipelinestate.Get());

	cmdList->SetGraphicsRootSignature(rootsignature.Get());

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cmdList->SetGraphicsRootConstantBufferView(0,constBuffTransform->GetGPUVirtualAddress());

	cmdList->SetGraphicsRootConstantBufferView(2,constBuffSkin->GetGPUVirtualAddress());

	model->Draw(cmdList);
}

void Object3d::PlayAnimation()
{
	FbxScene* fbxScene = model->GetFbxScene();

	FbxAnimStack* animstack = fbxScene->GetSrcObject<FbxAnimStack>(0);

	const char* animstackname = animstack->GetName();

	FbxTakeInfo* takeinfo = fbxScene->GetTakeInfo(animstackname);

	startTime = takeinfo->mLocalTimeSpan.GetStart();

	endTime = takeinfo->mLocalTimeSpan.GetStop();

	currentTime = startTime;

	isPlay = true;

}
