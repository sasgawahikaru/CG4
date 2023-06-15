#pragma once

#include "fbxsdk.h"
#include <d3d12.h>
#include <d3dx12.h>
#include <string>
#include "Model.h"

class FbxLoader
{

public:
	void Initialize(ID3D12Device* device);

	void Finalize();
private:

	ID3D12Device* device = nullptr;

	FbxManager* fbxManager = nullptr;

	FbxImporter* fbxImporter = nullptr;

public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>インスタンス</returns>
	static FbxLoader* GetInstance();

private:
	// privateなコンストラクタ（シングルトンパターン）
	FbxLoader() = default;
	// privateなデストラクタ（シングルトンパターン）
	~FbxLoader() = default;
	// コピーコンストラクタを禁止（シングルトンパターン）
	FbxLoader(const FbxLoader& obj) = delete;
	// コピー代入演算子を禁止（シングルトンパターン）
	void operator=(const FbxLoader& obj) = delete;

private:
	using string = std::string;
public:
	static const string baseDirectory;

	static const string defaultTextureFileName;

	Model* LoadModelFromFile(const string& modelName);

	void ParseNodeRecursive(Model* model, FbxNode* fbxNode, Node* parent = nullptr);

	void ParseMesh(Model* model, FbxNode* fbxNode);

	void ParseMeshVertices(Model* model, FbxMesh* fbxMesh);
	void ParseMeshFaces(Model* model, FbxMesh* fbxMesh);
	void ParseMaterial(Model* model, FbxNode* fbxNode);
	void LoadTexture(Model* model, const std::string& fullpath);
	std::string ExtractFileName(const std::string& path);
	static void ConvertMatrixFromFbx(DirectX::XMMATRIX* dst, const FbxAMatrix& src);
	void ParseSkin(Model* model, FbxMesh* fbxMesh);
};