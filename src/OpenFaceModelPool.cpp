#include "OpenFaceModelPool.h"
#include "LandmarkCoreIncludes.h"

using std::shared_ptr;

void printModels(const std::vector<shared_ptr<OpenFaceModel>> &vec) {
  std::cout << "[";
  for (auto model: vec) {
    if (model == nullptr) {
      std::cout << "nullptr";
    } else {
      std::cout << *model;
    }
    std::cout << ", ";
  }
  std::cout << "]";
  std::cout << endl;
}

OpenFaceModelPool::OpenFaceModelPool(size_t poolSize, CameraIntrinsics cameraIntrinsics) {
  this->poolSize = poolSize;

  freeModels.reserve(poolSize);
  usedModels.reserve(poolSize);

  ofLogNotice("OpenFaceModelPool") << "Allocating models...";
  for (int i = 0; i < poolSize; ++i) {
    shared_ptr<OpenFaceModel> model(new OpenFaceModel(i, cameraIntrinsics));
    freeModels.push_back(std::move(model));
    ofLogNotice("OpenFaceModelPool") << "Allocated model " << i+1 << " of " << poolSize;
  }

  ofLogNotice("OpenFaceModelPool") << "free models";
  printModels(freeModels);
  ofLogNotice("OpenFaceModelPool") << "used models";
  printModels(usedModels);
}

OpenFaceModelPool::~OpenFaceModelPool() {
  // TODO(avi)
}

shared_ptr<OpenFaceModel> OpenFaceModelPool::getModel() {
  if (freeModels.empty()) {
    ofLogNotice("OpenFaceModelPool") << "getModel(): No free models! Models in use: " << usedModels.size();
    return nullptr;
  }
  shared_ptr<OpenFaceModel> freeModel = freeModels.back();
  usedModels.push_back(freeModel);
  freeModels.pop_back();
  return usedModels.back();
}

void OpenFaceModelPool::returnModel(shared_ptr<OpenFaceModel> model) {
  if (model == nullptr) return;
  auto copyToPush = model;
  for (int i = 0; i < usedModels.size(); i++) {
    auto otherModel = usedModels[i];
    if (*otherModel == *model) {
      usedModels.erase(usedModels.begin() + i);
      ofLogNotice("OpenFaceModelPool") << "removed returned model from used models";
    }
  }
  model.reset();
  freeModels.push_back(copyToPush);
}
