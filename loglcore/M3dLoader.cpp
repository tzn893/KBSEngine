#include "M3dLoader.h"


/*
bool M3DLoader::LoadM3d(const std::string& filename,
	std::vector<Vertex>& vertices,
	std::vector<USHORT>& indices,
	std::vector<Subset>& subsets,
	std::vector<M3dMaterial>& mats)
{
	std::ifstream fin(filename);

	UINT numMaterials = 0;
	UINT numVertices = 0;
	UINT numTriangles = 0;
	UINT numBones = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if (fin)
	{
		fin >> ignore; // file header text
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;

		ReadMaterials(fin, numMaterials, mats);
		ReadSubsetTable(fin, numMaterials, subsets);
		ReadVertices(fin, numVertices, vertices);
		ReadTriangles(fin, numTriangles, indices);

		return true;
	}
	return false;
}

bool M3DLoader::LoadM3d(const std::string& filename,
	std::vector<SkinnedVertex>& vertices,
	std::vector<USHORT>& indices,
	std::vector<Subset>& subsets,
	std::vector<M3dMaterial>& mats,
	SkinnedData& skinInfo)
{
	std::ifstream fin(filename);

	UINT numMaterials = 0;
	UINT numVertices = 0;
	UINT numTriangles = 0;
	UINT numBones = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if (fin)
	{
		fin >> ignore; // file header text
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;

		std::vector<XMFLOAT4X4> boneOffsets;
		std::vector<int> boneIndexToParentIndex;
		std::unordered_map<std::string, AnimationClip> animations;

		ReadMaterials(fin, numMaterials, mats);
		ReadSubsetTable(fin, numMaterials, subsets);
		ReadSkinnedVertices(fin, numVertices, vertices);
		ReadTriangles(fin, numTriangles, indices);
		ReadBoneOffsets(fin, numBones, boneOffsets);
		ReadBoneHierarchy(fin, numBones, boneIndexToParentIndex);
		ReadAnimationClips(fin, numBones, numAnimationClips, animations);

		skinInfo.Set(boneIndexToParentIndex, boneOffsets, animations);

		return true;
	}
	return false;
}*/

void M3DLoader::ReadMaterials(std::ifstream& fin, size_t numMaterials, std::vector<M3dMaterial>& mats)
{
	std::string ignore;
	mats.resize(numMaterials);

	std::string diffuseMapName;
	std::string normalMapName;

	fin >> ignore; // materials header text
	for (UINT i = 0; i < numMaterials; ++i)
	{
		fin >> ignore >> mats[i].Name;
		fin >> ignore >> mats[i].DiffuseAlbedo.x >> mats[i].DiffuseAlbedo.y >> mats[i].DiffuseAlbedo.z;
		fin >> ignore >> mats[i].FresnelR0.x >> mats[i].FresnelR0.y >> mats[i].FresnelR0.z;
		fin >> ignore >> mats[i].Roughness;
		fin >> ignore >> mats[i].AlphaClip;
		fin >> ignore >> mats[i].MaterialTypeName;
		fin >> ignore >> mats[i].DiffuseMapName;
		fin >> ignore >> mats[i].NormalMapName;
	}
}

void M3DLoader::ReadSubsetTable(std::ifstream& fin, size_t numSubsets, std::vector<Subset>& subsets)
{
	std::string ignore;
	subsets.resize(numSubsets);

	fin >> ignore; // subset header text
	for (UINT i = 0; i < numSubsets; ++i)
	{
		fin >> ignore >> subsets[i].Id;
		fin >> ignore >> subsets[i].VertexStart;
		fin >> ignore >> subsets[i].VertexCount;
		fin >> ignore >> subsets[i].FaceStart;
		fin >> ignore >> subsets[i].FaceCount;
	}
}

void M3DLoader::ReadVertices(std::ifstream& fin, size_t numVertices, std::vector<MeshVertexNormal>& vertices)
{
	std::string ignore;
	vertices.resize(numVertices);
	float trash;
	fin >> ignore; // vertices header text
	for (UINT i = 0; i < numVertices; ++i)
	{
		fin >> ignore >> vertices[i].Position.x >> vertices[i].Position.y >> vertices[i].Position.z;
		fin >> ignore >> vertices[i].Tangent.x >> vertices[i].Tangent.y >> vertices[i].Tangent.z >> trash;
		fin >> ignore >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
		fin >> ignore >> vertices[i].TexCoord.x >> vertices[i].TexCoord.y;
	}
}

void M3DLoader::ReadSkinnedVertices(std::ifstream& fin, size_t numVertices, std::vector<SkinnedNormalVertex>& vertices)
{
	std::string ignore;
	vertices.resize(numVertices);

	fin >> ignore; // vertices header text
	int boneIndices[4];
	float weights[4];
	float trash;
	for (UINT i = 0; i < numVertices; ++i)
	{
		float blah;
		fin >> ignore >> vertices[i].data.Position.x >> vertices[i].data.Position.y >> vertices[i].data.Position.z;
		fin >> ignore >> vertices[i].data.Tangent.x >> vertices[i].data.Tangent.y >> vertices[i].data.Tangent.z >> trash;
		fin >> ignore >> vertices[i].data.Normal.x >> vertices[i].data.Normal.y >> vertices[i].data.Normal.z;
		fin >> ignore >> vertices[i].data.TexCoord.x >> vertices[i].data.TexCoord.y;
		fin >> ignore >> weights[0] >> weights[1] >> weights[2] >> weights[3];
		fin >> ignore >> boneIndices[0] >> boneIndices[1] >> boneIndices[2] >> boneIndices[3];

		vertices[i].boneWeights.x = weights[0];
		vertices[i].boneWeights.y = weights[1];
		vertices[i].boneWeights.z = weights[2];
		vertices[i].boneWeights.w = weights[3];

		vertices[i].boneIndices[0] = (BYTE)boneIndices[0];
		vertices[i].boneIndices[1] = (BYTE)boneIndices[1];
		vertices[i].boneIndices[2] = (BYTE)boneIndices[2];
		vertices[i].boneIndices[3] = (BYTE)boneIndices[3];
	}
}

void M3DLoader::ReadTriangles(std::ifstream& fin, size_t numTriangles, std::vector<uint16_t>& indices)
{
	std::string ignore;
	indices.resize(numTriangles * 3);

	fin >> ignore; // triangles header text
	for (size_t i = 0; i < numTriangles; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
}

void M3DLoader::ReadBoneOffsets(std::ifstream& fin, size_t numBones, std::vector<Game::Mat4x4>& boneOffsets)
{
	std::string ignore;
	boneOffsets.resize(numBones);

	fin >> ignore; // BoneOffsets header text
	for (size_t i = 0; i < numBones; ++i)
	{
		fin >> ignore >>
			boneOffsets[i].a[0][0] >> boneOffsets[i].a[0][1] >> boneOffsets[i].a[0][2] >> boneOffsets[i].a[0][3] >>
			boneOffsets[i].a[1][0] >> boneOffsets[i].a[1][1] >> boneOffsets[i].a[1][2] >> boneOffsets[i].a[1][3] >>
			boneOffsets[i].a[2][0] >> boneOffsets[i].a[2][1] >> boneOffsets[i].a[2][2] >> boneOffsets[i].a[2][3] >>
			boneOffsets[i].a[3][0] >> boneOffsets[i].a[3][1] >> boneOffsets[i].a[3][2] >> boneOffsets[i].a[3][3];
	}
}

void M3DLoader::ReadBoneHierarchy(std::ifstream& fin, size_t numBones, std::vector<int>& boneIndexToParentIndex)
{
	std::string ignore;
	boneIndexToParentIndex.resize(numBones);

	fin >> ignore; // BoneHierarchy header text
	for (UINT i = 0; i < numBones; ++i)
	{
		fin >> ignore >> boneIndexToParentIndex[i];
	}
}
/*
void M3DLoader::ReadAnimationClips(std::ifstream& fin, size_t numBones, size_t numAnimationClips,
	std::unordered_map<std::string, AnimationClip>& animations)
{
	std::string ignore;
	fin >> ignore; // AnimationClips header text
	for (UINT clipIndex = 0; clipIndex < numAnimationClips; ++clipIndex)
	{
		std::string clipName;
		fin >> ignore >> clipName;
		fin >> ignore; // {

		AnimationClip clip;
		clip.BoneAnimations.resize(numBones);

		for (UINT boneIndex = 0; boneIndex < numBones; ++boneIndex)
		{
			ReadBoneKeyframes(fin, numBones, clip.BoneAnimations[boneIndex]);
		}
		fin >> ignore; // }

		animations[clipName] = clip;
	}
}
*/
void M3DLoader::ReadBoneKeyframes(std::ifstream& fin, size_t numBones,std::vector<BoneAnimationNode>& boneAnimation)
{
	std::string ignore;
	UINT numKeyframes = 0;
	fin >> ignore >> ignore >> numKeyframes;
	fin >> ignore; // {
	/*
	boneAnimation.Keyframes.resize(numKeyframes);
	for (UINT i = 0; i < numKeyframes; ++i)
	{
		float t = 0.0f;
		XMFLOAT3 p(0.0f, 0.0f, 0.0f);
		XMFLOAT3 s(1.0f, 1.0f, 1.0f);
		XMFLOAT4 q(0.0f, 0.0f, 0.0f, 1.0f);
		fin >> ignore >> t;
		fin >> ignore >> p.x >> p.y >> p.z;
		fin >> ignore >> s.x >> s.y >> s.z;
		fin >> ignore >> q.x >> q.y >> q.z >> q.w;

		boneAnimation.Keyframes[i].TimePos = t;
		boneAnimation.Keyframes[i].Translation = p;
		boneAnimation.Keyframes[i].Scale = s;
		boneAnimation.Keyframes[i].RotationQuat = q;
	}
	*/
	fin >> ignore; // }
}