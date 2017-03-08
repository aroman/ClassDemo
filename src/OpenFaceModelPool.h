#pragma once

#include "OpenFaceModel.h"

using std::shared_ptr;

class OpenFaceModelPool {

public:
  OpenFaceModelPool(size_t poolSize, CameraIntrinsics cameraIntrinsics);
  ~OpenFaceModelPool();
  shared_ptr<OpenFaceModel> getModel();
  void returnModel(shared_ptr<OpenFaceModel> model);

private:
  size_t poolSize;
  vector<shared_ptr<OpenFaceModel>> freeModels;
  vector<shared_ptr<OpenFaceModel>> usedModels;

};
