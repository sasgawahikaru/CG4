#pragma once
#include <string>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <vector>

struct Node
{
	std::string name;


	DirectX::XMVECTOR scaling = { 1,1,1,0 };
	DirectX::XMVECTOR rotation = { 0,0,0,0 };
	DirectX::XMVECTOR translation = { 0,0,0,1 };
	DirectX::XMMATRIX transform;
	DirectX::XMMATRIX globalTransform;
	Node* parent = nullptr;
};
class Model
{
public:
	friend class FbxLoader;
private:
	std::string name;
	std::vector<Node>nodes;
public:
	struct VertexPosNormalUv
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
	};
	Node* meshNode = nullptr;
	std::vector<VertexPosNormalUv>vertices;
	std::vector<unsigned short>indices;

	DirectX::XMFLOAT3 ambient = { 1,1,1 };
	DirectX::XMFLOAT3 diffuse = { 1,1,1 };
	DirectX::TexMetadata metadata = {};
	DirectX::ScratchImage scratchImg = {};
};