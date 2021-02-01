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

private:
	Model* loadInOBJFormat(const char* pathname,const char* name,UploadBatch* batch);
	Model* loadByAssimp(const char* pathname,const char* name,UploadBatch* batch);

	std::map<std::string, std::unique_ptr<Model>> modelsByPath;
	std::map<std::string, Model*> modelsByName;
};

inline ModelManager gModelManager;