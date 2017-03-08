#pragma once

#include "ofMain.h"
#include <GazeEstimation.h>
#include "LandmarkCoreIncludes.h"

static const int MAX_MODEL_FAILURES_IN_A_ROW = 3;
static const double MIN_CERTAINTY_FOR_VISUALIZATION = 0.35;

struct CameraIntrinsics {
  float fx, fy, cx, cy;
};

class OpenFaceModel {

public:
  explicit OpenFaceModel(int id, CameraIntrinsics cameraIntrinsics);
  ~OpenFaceModel();
  bool operator==(const OpenFaceModel& other) const;
  friend std::ostream& operator<<(std::ostream &strm, const OpenFaceModel &model);

  bool initializeTracking(ofPixels colorPixels, ofRectangle boundingBox);
  bool updateTracking(ofPixels colorPixels);
  void reset();

  // Methods to access OpenFace model data with OpenFrameworks types
  ofRectangle get2DBoundingBox() const;
  std::vector<std::pair<ofVec2f, ofVec2f>> get3DBoundingBox() const;
  std::pair<ofVec3f, ofVec3f> getGazeDirection() const;
  std::vector<ofVec2f> getLandmarks() const;
  ofPolyline getLandmarksPolyline() const;
  std::vector<double> getPoseCamera() const; // [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
  double getX() const;
  double getY() const;
  bool isActive() const;

  LandmarkDetector::CLNF *model = nullptr;
  LandmarkDetector::FaceModelParameters *parameters = nullptr;

private:
  int id; // used for keeping track in the pool
  CameraIntrinsics cameraIntrinsics;
  float fx, fy, cx, cy;

};
