#include "M3dLoader.h"
#include "TextureManager.h"
#include "graphic.h"

/*
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
SkinnedModel* M3DLoader::ReadSkinnedModel(const char* filename,const char* name,UploadBatch* up) {

	std::ifstream fi(filename);

	size_t matNum, vNum, tNum, bNum, aNum;
	std::vector<M3dMaterial> mats;
	std::vector<Subset> subsets;
	std::vector<SkinnedNormalVertex> vertices;
	std::vector<uint16_t> indices;
	std::vector<int> boneToParentIndex;
	std::vector<BoneAnimationClip*> animations;
	BoneHeirarchy* boneHeirarchy = new BoneHeirarchy();

	std::string ignore;
	if (fi) {
		fi >> ignore;
		fi >> ignore >> matNum;
		fi >> ignore >> vNum;
		fi >> ignore >> tNum;
		fi >> ignore >> bNum;
		fi >> ignore >> aNum;

		ReadMaterials(fi, matNum, mats);
		ReadSubsetTable(fi, matNum, subsets);
		ReadSkinnedVertices(fi, vNum, vertices);
		ReadTriangles(fi, tNum, indices, subsets);
		ReadBoneHierarchy(fi, bNum, boneHeirarchy);
		ReadAnimationClips(fi, bNum, aNum,animations, boneHeirarchy);

		SkinnedModel* model = new SkinnedModel(boneHeirarchy, animations, name);
		for (size_t i = 0; i != subsets.size(); i++) {
			model->PushBackSubMesh(new SkinnedSubMesh(mats[i].Name.c_str(), i, gGraphic.GetDevice(),
				subsets[i].FaceCount * 3,
				indices.data() + subsets[i].FaceStart * 3,
				subsets[i].VertexCount,
				vertices.data() + subsets[i].VertexStart,
				up)
			);
			SubMeshMaterial mat;
			mat.diffuse = mats[i].DiffuseAlbedo;
			mat.roughness = mats[i].Roughness;
			mat.metallic = 0.f;
			std::wstring filename = String2WString(mats[i].DiffuseMapName);
			mat.textures[SUBMESH_MATERIAL_TYPE_DIFFUSE] = gTextureManager.loadTexture(filename.c_str(), filename.c_str(), true, up);
			filename = String2WString(mats[i].NormalMapName);
			mat.textures[SUBMESH_MATERIAL_TYPE_BUMP] = gTextureManager.loadTexture(filename.c_str(), filename.c_str(), true, up);
			model->PushBackSubMeshMaterial(mat);
		}

		return model;
	}

	return nullptr;
}

Model* M3DLoader::ReadModel(const char* filename,const char* name,UploadBatch* up) {
	std::ifstream fi(filename);

	size_t matNum, vNum, tNum, bNum, aNum;
	std::vector<M3dMaterial> mats;
	std::vector<Subset> subsets;
	std::vector<MeshVertexNormal> vertices;
	std::vector<uint16_t> indices;

	std::string ignore;
	if(fi) {
		fi >> ignore;
		fi >> ignore >> matNum;
		fi >> ignore >> vNum;
		fi >> ignore >> tNum;
		fi >> ignore >> bNum;
		fi >> ignore >> aNum;

		ReadMaterials(fi, matNum, mats);
		ReadSubsetTable(fi, matNum, subsets);
		ReadVertices(fi, vNum, vertices);
		ReadTriangles(fi, tNum, indices,subsets);
		
		Model* model = new Model(name);
		for (size_t i = 0; i != subsets.size();i++) {
			model->PushBackSubMesh(new SubMesh(mats[i].Name.c_str(), i, gGraphic.GetDevice(),
				subsets[i].FaceCount * 3,
				indices.data() + subsets[i].FaceStart * 3,
				subsets[i].VertexCount,
				vertices.data() + subsets[i].VertexStart,
				up)
			);
			SubMeshMaterial mat;
			mat.diffuse = mats[i].DiffuseAlbedo;
			mat.roughness = mats[i].Roughness;
			mat.metallic = 0.f;
			std::wstring filename = String2WString(mats[i].DiffuseMapName);
			mat.textures[SUBMESH_MATERIAL_TYPE_DIFFUSE] = gTextureManager.loadTexture(filename.c_str(), filename.c_str(), true, up);
			filename = String2WString(mats[i].NormalMapName);
			mat.textures[SUBMESH_MATERIAL_TYPE_BUMP] = gTextureManager.loadTexture(filename.c_str(),filename.c_str(),true,up);
			model->PushBackSubMeshMaterial(mat);
		}

		return model;
	}

	return nullptr;
}

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
		fin >> ignore >> ignore >> ignore >> ignore >> ignore;
		fin >> ignore >> ignore >> ignore >> ignore >> ignore;
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

void M3DLoader::ReadTriangles(std::ifstream& fin, size_t numTriangles, std::vector<uint16_t>& indices,std::vector<M3DLoader::Subset>& subsets)
{
	std::string ignore;
	indices.resize(numTriangles * 3);

	fin >> ignore; // triangles header text
	for (size_t i = 0; i < numTriangles; ++i)
	{
		size_t subsetIndex = 0;
		for (; subsetIndex != subsets.size();subsetIndex++) {
			if ((subsets[subsetIndex].FaceStart + subsets[subsetIndex].FaceCount) > i) {
				break;
			}
		}
		size_t indexBase = subsets[subsetIndex].VertexStart;

		fin >> indices[i * 3 + 0] >> indices[i * 3 + 2] >> indices[i * 3 + 1];
		indices[i * 3 + 0] -= indexBase;
		indices[i * 3 + 1] -= indexBase;
		indices[i * 3 + 2] -= indexBase;
	}

	
}

void M3DLoader::ReadBoneHierarchy(std::ifstream& fin, size_t numBones,BoneHeirarchy* boneHeirarchy)
{
	std::string ignore;
	std::vector<Game::Mat4x4> boneOffsets;
	boneOffsets.resize(numBones);

	fin >> ignore; // BoneOffsets header text
	for (size_t i = 0; i < numBones; ++i)
	{
		fin >> ignore >>
			boneOffsets[i].a[0][0] >> boneOffsets[i].a[0][1] >> boneOffsets[i].a[0][2] >> boneOffsets[i].a[0][3] >>
			boneOffsets[i].a[1][0] >> boneOffsets[i].a[1][1] >> boneOffsets[i].a[1][2] >> boneOffsets[i].a[1][3] >>
			boneOffsets[i].a[2][0] >> boneOffsets[i].a[2][1] >> boneOffsets[i].a[2][2] >> boneOffsets[i].a[2][3] >>
			boneOffsets[i].a[3][0] >> boneOffsets[i].a[3][1] >> boneOffsets[i].a[3][2] >> boneOffsets[i].a[3][3];
		boneOffsets[i] = boneOffsets[i].T();
	}

	fin >> ignore; // BoneHierarchy header text
	for (UINT i = 0; i < numBones; ++i)
	{
		int parentIndex;
		fin >> ignore >> parentIndex;
		std::string boneName = "bone_" + std::to_string(i);
		Bone b(boneName.c_str());
		b.offset = boneOffsets[i];
		if (parentIndex < 0) {
			boneHeirarchy->Pushback(b, nullptr);
		}
		else {
			size_t index = parentIndex;
			boneHeirarchy->Pushback(b, &index);
		}
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

void M3DLoader::ReadAnimationClips(std::ifstream& fin, size_t numBones, size_t numClips, std::vector<BoneAnimationClip*>& boneAnimations, BoneHeirarchy* boneHeirarchy) {
	std::string ignore;
	fin >> ignore; 
	
	for (size_t clipIndex = 0; clipIndex != numClips;clipIndex++) {
		std::string clipName;
		fin >> ignore >> clipName;
		float totalTicks, ticksPersecond;
		fin >> ignore >> totalTicks >> ignore >> ticksPersecond;
		fin >> ignore;
		std::vector<BoneAnimationNode> animationNodes;
		for (size_t boneIndex = 0; boneIndex != numBones; boneIndex++) {
			BoneAnimationNode node;
			ReadBoneKeyframes(fin, node);
			animationNodes.push_back(node);
		}
		BoneAnimationClip* animationClip = new BoneAnimationClip(animationNodes,boneHeirarchy,
			ticksPersecond,totalTicks,clipName.c_str());
		boneAnimations.push_back(animationClip);
	}
}
void M3DLoader::ReadBoneKeyframes(std::ifstream& fin,BoneAnimationNode& boneAnimation)
{
	std::string ignore;
	UINT numKeyframes = 0;
	fin >> ignore >> ignore >> numKeyframes;
	fin >> ignore;
	std::vector<BoneAnimationNode::Position> positions;
	std::vector<BoneAnimationNode::Rotation> rotations;
	std::vector<BoneAnimationNode::Scale> scales;
	for (size_t i = 0; i != numKeyframes;i++) {
		float t = 0.f;
		Game::Vector3 pos;
		Game::Vector4 rot;
		Game::Vector3 scl;
		fin >> ignore >> t;
		fin >> ignore >> pos.x >> pos.y >> pos.z;
		fin >> ignore >> scl.x >> scl.y >> scl.z;
		fin >> ignore >> rot.x >> rot.y >> rot.z >> rot.w;

		rot = Game::normalize(rot);

		positions.push_back({ pos,t });
		rotations.push_back({ Game::Quaterion(rot),t });
		scales.push_back({ scl,t });
	}
	fin >> ignore;
	boneAnimation = BoneAnimationNode(positions, rotations, scales);
}