#pragma once
#include "Model.h"
#include <string>
#include <map>

class ModelManager {
public:

	Model* loadModel(const char* pahtName,const char* name,UploadBatch* batch = nullptr);
	Model* getModelByPath(const char* path) {
		if (auto item = modelsByPath.find(path); item != modelsByPath.end()) {
			return item->second.get();
		}
		return nullptr;
	}
	Model* getModelByName(const char* name) {
		if (auto item = modelsByName.find(name);item != modelsByName.end()) {
			return item->second;
		}
		return nullptr;
	}

	SkinnedModel* loadSkinnedModel(const char* pahtName, const char* name, UploadBatch* batch = nullptr);
	SkinnedModel* getSkinnedModelByPath(const char* path) {
		if (auto item = skinnedModelByPath.find(path);item != skinnedModelByPath.end()) {
			return item->second.get();
		}
		return nullptr;
	}
	SkinnedModel* getSkinnedModelByName(const char* name) {
		if (auto item = skinnedModelsByName.find(name);item != skinnedModelsByName.end()) {
			return item->second;
		}
		return nullptr;
	}

private:
	Model* loadInOBJFormat(const char* pathname,const char* name,UploadBatch* batch);
	Model* loadByAssimp(const char* pathname,const char* name,UploadBatch* batch);
	Model* loadInM3DFormat(const char* pathname,const char* name,UploadBatch* batch);

	SkinnedModel* loadSkinnedModelByAssimp(const char* pathname,const char* name,UploadBatch* batch);
	SkinnedModel* loadSkinnedModelByM3DFormat(const char* pathname,const char* name,UploadBatch* batch);

	std::map<std::string, std::unique_ptr<Model>> modelsByPath;
	std::map<std::string, Model*> modelsByName;

	std::map<std::string, std::unique_ptr<SkinnedModel>> skinnedModelByPath;
	std::map<std::string, SkinnedModel*> skinnedModelsByName;
};

inline ModelManager gModelManager;