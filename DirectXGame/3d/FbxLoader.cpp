#include "FbxLoader.h"
#include <cassert>
const std::string FbxLoader::baseDirectory = "Resources/";
using namespace DirectX;

FbxLoader* FbxLoader::GetInstance()
{

    static FbxLoader instance;
    return &instance;
}
void FbxLoader::Initialize(ID3D12Device* device)
{
    assert(fbxManager == nullptr);

    this->device = device;

    fbxManager = FbxManager::Create();

    FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);

    fbxManager->SetIOSettings(ios);

    fbxImporter = FbxImporter::Create(fbxManager, "");
}

void FbxLoader::Finalize()
{
    fbxImporter->Destroy();
    fbxManager->Destroy();
}

void FbxLoader::LoadModelFromFile(const string& modelName)
{
    const string directoryPath = baseDirectory + modelName + "/";

    const string fileName = modelName + ".fbx";

    const string fullpath = directoryPath + fileName;

    if (!fbxImporter->Initialize(fullpath.c_str(),
        -1, fbxManager->GetIOSettings())) {
        assert(0);
    }
    FbxScene* fbxScene =
        FbxScene::Create(fbxManager, "fbxScene");
    fbxImporter->Import(fbxScene);

    Model* model = new Model();
    model->name = modelName;

    int nodeCount = fbxScene->GetNodeCount();

    model->nodes.reserve(nodeCount);

    ParseNodeRecursive(model, fbxScene->GetRootNode());

    fbxScene->Destroy();
}

void FbxLoader::ParseNodeRecursive(Model* model, FbxNode* fbxNode, Node* parent)
{
    model->nodes.emplace_back();
    Node& node = model->nodes.back();

    node.name = fbxNode->GetName();

    FbxDouble3 rotation = fbxNode->LclRotation.Get();
    FbxDouble3 scaling = fbxNode->LclScaling.Get();
    FbxDouble3 translation = fbxNode->LclTranslation.Get();

    node.rotation = { (float)rotation[0],(float)rotation[1],(float)rotation[2],0.0f };
    node.scaling = { (float)scaling[0],(float)scaling[1],(float)scaling[2],0.0f };
    node.translation = { (float)translation[0],(float)translation[1],(float)translation[2],1.0f };

    node.rotation.m128_f32[0] = XMConvertToRadians(node.rotation.m128_f32[0]);
    node.rotation.m128_f32[1] = XMConvertToRadians(node.rotation.m128_f32[1]);
    node.rotation.m128_f32[2] = XMConvertToRadians(node.rotation.m128_f32[2]);

    XMMATRIX matScaling, matRotation, matTranslation;
    matScaling = XMMatrixScalingFromVector(node.scaling);
    matRotation = XMMatrixRotationRollPitchYawFromVector(node.rotation);
    matTranslation = XMMatrixTranslationFromVector(node.translation);

    node.transform = XMMatrixIdentity();
    node.transform *= matScaling;
    node.transform *= matRotation;
    node.transform *= matTranslation;

    string name = fbxNode->GetName();

    node.globalTransform = node.transform;
    if (parent) {
        node.parent = parent;

        node.globalTransform *= parent->globalTransform;

    }

    for (int i = 0; i < fbxNode->GetChildCount(); i++) {
        ParseNodeRecursive(model, fbxNode->GetChild(i), &node);
    }

}
