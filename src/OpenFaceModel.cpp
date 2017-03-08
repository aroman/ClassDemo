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
  model->Reset();
  // Ensures that a wider window is used
  // for initial landmark localisation
  model->detection_success = false;
}

// Try to initialize model at the face located at boundingBox
// Return true if model was successfully, initialized, otherwise false.
bool OpenFaceModel::initializeTracking(ofPixels colorPixels, ofRectangle boundingBox) {
  ofLogNotice("OpenFaceModel::initializeTracking") << boundingBox;
  return LandmarkDetector::DetectLandmarksInVideo(
    colorPixelsToGrayscaleMat(colorPixels),
    ofxCv::toCv(boundingBox),
    *model,
    *parameters
  );
}

bool OpenFaceModel::updateTracking(ofPixels colorPixels) {
  return LandmarkDetector::DetectLandmarksInVideo(
    colorPixelsToGrayscaleMat(colorPixels),
    *model,
    *parameters
  );
}

ofRectangle OpenFaceModel::get2DBoundingBox() const {
  return ofxCv::toOf(model->GetBoundingBox());
}

double OpenFaceModel::getX() const {
  return model->params_global[4];
}

double OpenFaceModel::getY() const {
  return model->params_global[5];
}

bool OpenFaceModel::isActive() const {
  return model->detection_success && model->tracking_initialised;
}
