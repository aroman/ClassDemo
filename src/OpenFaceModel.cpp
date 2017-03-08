#include "OpenFaceModel.h"
#include "ofxCv.h"

cv::Mat colorPixelsToGrayscaleMat(ofPixels colorPixels) {
  cv::Mat grayscaleMat;
  cv::cvtColor(ofxCv::toCv(colorPixels), grayscaleMat, CV_BGRA2GRAY);
  return grayscaleMat;
}

OpenFaceModel::OpenFaceModel(int id, CameraIntrinsics cameraIntrinsics) {
  this->id = id;
  this->cameraIntrinsics = cameraIntrinsics;

  this->parameters = new LandmarkDetector::FaceModelParameters();
  // maybe try turning it off, doesn't make much difference
  parameters->use_face_template = true;
  // Model should not try to re-initialising itself
  parameters->reinit_video_every = -1;
  // TODO(avi) read from relative directory?
  parameters->model_location = "/opt/sensei/model/main_clnf_general.txt";
  parameters->track_gaze = true;
  this->model = new LandmarkDetector::CLNF(parameters->model_location);
  reset();
}

OpenFaceModel::~OpenFaceModel() {
  ofLogWarning("OpenFaceModel") << "destructor called. Shouldn't happen until end!";
  delete parameters;
  delete model;
}

bool OpenFaceModel::operator==(const OpenFaceModel& other) const {
  return id == other.id;
}

std::ostream& operator<<(std::ostream &strm, const OpenFaceModel &model) {
  return strm << "OpenFaceModel(id=" << model.id << ")";
}

void OpenFaceModel::reset() {
  assert(model != nullptr);
  model->Reset();
  // Ensures that a wider window is used
  // for initial landmark localisation
  model->detection_success = false;
}

// Try to initialize model at the face located at boundingBox
// Return true if model was successfully, initialized, otherwise false.
bool OpenFaceModel::initializeTracking(ofPixels colorPixels, ofRectangle boundingBox) {
  assert(model != nullptr);
  assert(parameters != nullptr);
  ofLogNotice("OpenFaceModel::initializeTracking") << boundingBox;
  return LandmarkDetector::DetectLandmarksInVideo(
    colorPixelsToGrayscaleMat(colorPixels),
    ofxCv::toCv(boundingBox),
    *model,
    *parameters
  );
}

bool OpenFaceModel::updateTracking(ofPixels colorPixels) {
  assert(model != nullptr);
  assert(parameters != nullptr);
  return LandmarkDetector::DetectLandmarksInVideo(
    colorPixelsToGrayscaleMat(colorPixels),
    *model,
    *parameters
  );
}

ofRectangle OpenFaceModel::get2DBoundingBox() const {
  assert(model != nullptr);
  return ofxCv::toOf(model->GetBoundingBox());
}

std::vector<std::pair<ofVec2f, ofVec2f>> OpenFaceModel::get3DBoundingBox() const {
  assert(model != nullptr);
  auto poseWorld = LandmarkDetector::GetCorrectedPoseWorld(
    *model,
    cameraIntrinsics.fx,
    cameraIntrinsics.fy,
    cameraIntrinsics.cx,
    cameraIntrinsics.cy
  );
  vector<std::pair<cv::Point2d, cv::Point2d>> box = LandmarkDetector::CalculateBox(
    poseWorld,
    cameraIntrinsics.fx,
    cameraIntrinsics.fy,
    cameraIntrinsics.cx,
    cameraIntrinsics.cy
  );
  std::vector<std::pair<ofVec2f, ofVec2f>> toReturn(box.size());
  for (int i = 0; i < box.size(); i++) {
    toReturn.at(i) = std::make_pair(
      ofxCv::toOf(
        std::get<0>(box.at(i))
      ),
      ofxCv::toOf(
        std::get<1>(box.at(i))
      )
    );
  }
  return toReturn;
}

ofPolyline OpenFaceModel::getLandmarksPolyline() const {
  assert(model != nullptr);
  return ofxCv::toOf(CalculateLandmarks(*model));
}

std::pair<ofVec3f, ofVec3f> OpenFaceModel::getGazeDirection() const {
  assert(model != nullptr);
  cv::Point3f leftEyeGaze(0, 0, -1);
  cv::Point3f rightEyeGaze(0, 0, -1);
  FaceAnalysis::EstimateGaze(
    *model,
    leftEyeGaze,
    cameraIntrinsics.fx,
    cameraIntrinsics.fy,
    cameraIntrinsics.cx,
    cameraIntrinsics.cy,
    true // isLeftEye -> true
  );
  FaceAnalysis::EstimateGaze(
    *model,
    rightEyeGaze,
    cameraIntrinsics.fx,
    cameraIntrinsics.fy,
    cameraIntrinsics.cx,
    cameraIntrinsics.cy,
    false // isLeftEye -> false
  );
  return std::make_pair(ofxCv::toOf(leftEyeGaze), ofxCv::toOf(rightEyeGaze));
}

std::vector<ofVec2f> OpenFaceModel::getLandmarks() const {
  assert(model != nullptr);
  std::vector<cv::Point2d> landmarks = CalculateLandmarks(*model);
  std::vector<ofVec2f> toReturn(landmarks.size());
  for (int i = 0; i < landmarks.size(); i++) {
      toReturn.at(i) = ofxCv::toOf(landmarks.at(i));
  }
  return toReturn;
}

std::vector<double> OpenFaceModel::getPoseCamera() const {
  assert(model != nullptr);
  std::vector<double> toReturn(6);
  auto vec6d = LandmarkDetector::GetCorrectedPoseCamera(
    *model,
    cameraIntrinsics.fx,
    cameraIntrinsics.fy,
    cameraIntrinsics.cx,
    cameraIntrinsics.cy
  );
  for (int i = 0; i < 6; i++) {
    toReturn.at(i) = vec6d[i];
  }
  return toReturn;
}

double OpenFaceModel::getX() const {
  assert(model != nullptr);
  return model->params_global[4];
}

double OpenFaceModel::getY() const {
  assert(model != nullptr);
  return model->params_global[5];
}

bool OpenFaceModel::isActive() const {
  assert(model != nullptr);
  return model->detection_success && model->tracking_initialised;
}
