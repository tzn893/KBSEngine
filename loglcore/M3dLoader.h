#pragma once


#include "Math.h"
#include "ShaderDataStruct.h"
#include <vector>
#include <string>
#include "BoneHeirarchy.h"
#include <unordered_map>
#include <fstream>

class M3DLoader
{
public:
	/*struct Vertex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexC;
		DirectX::XMFLOAT4 TangentU;
	};

	struct SkinnedVertex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexC;
		DirectX::XMFLOAT3 TangentU;
		DirectX::XMFLOAT3 BoneWeights;
		BYTE BoneIndices[4];
	};*/

	struct Subset
	{
		UINT Id = -1;
		UINT VertexStart = 0;
		UINT VertexCount = 0;
		UINT FaceStart = 0;
		UINT FaceCount = 0;
	};

	struct M3dMaterial
	{
		std::string Name;

		Game::Vector4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		Game::Vector3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness = 0.8f;
		bool AlphaClip = false;

		std::string MaterialTypeName;
		std::string DiffuseMapName;
		std::string NormalMapName;
	};

	/*bool LoadM3d(const std::string& filename,
		std::vector<Vertex>& vertices,
		std::vector<uint8_t>& indices,
		std::vector<Subset>& subsets,
		std::vector<M3dMaterial>& mats);
	bool LoadM3d(const std::string& filename,
		std::vector<SkinnedVertex>& vertices,
		std::vector<USHORT>& indices,
		std::vector<Subset>& subsets,
		std::vector<M3dMaterial>& mats,
		SkinnedData& skinInfo);*/

private:
	void ReadMaterials(std::ifstream& fin, size_t numMaterials, std::vector<M3dMaterial>& mats);
	void ReadSubsetTable(std::ifstream& fin, size_t numSubsets, std::vector<Subset>& subsets);
	void ReadVertices(std::ifstream& fin, size_t numVertices, std::vector<MeshVertexNormal>& vertices);
	void ReadSkinnedVertices(std::ifstream& fin, size_t numVertices, std::vector<SkinnedNormalVertex>& vertices);
	void ReadTriangles(std::ifstream& fin, size_t numTriangles, std::vector<uint16_t>& indices);
	void ReadBoneOffsets(std::ifstream& fin, size_t numBones, std::vector<Game::Mat4x4>& boneOffsets);
	void ReadBoneHierarchy(std::ifstream& fin, size_t numBones, std::vector<int>& boneIndexToParentIndex);
	//void ReadAnimationClips(std::ifstream& fin, size_t numBones,size_t numAnimationClips, std::unordered_map<std::string,BoneAnimationNode >& animations);
	void ReadBoneKeyframes(std::ifstream& fin, size_t  numBones, std::vector<BoneAnimationNode>& boneAnimation);
};