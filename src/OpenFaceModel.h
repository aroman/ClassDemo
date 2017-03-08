#pragma once

#include "ofMain.h"
#include "LandmarkCoreIncludes.h"

static const int MAX_MODEL_FAILURES_IN_A_ROW = 1;
static const double MIN_CERTAINTY_FOR_VISUALIZATION = 0.35;

class OpenFaceModel {

public:
  explicit OpenFaceModel(int id);
  ~OpenFaceModel();
  bool operator==(const OpenFaceModel& other) const;
  friend std::ostream& operator<<(std::ostream &strm, const OpenFaceModel &model);

  bool initializeTracking(ofPixels colorPixels, ofRectangle boundingBox);
  bool updateTracking(ofPixels colorPixels);
  void reset();

  ofRectangle get2DBoundingBox() const;
  double getX() const;
  double getY() const;
  bool isActive() const;

  LandmarkDetector::CLNF *model = nullptr;
  LandmarkDetector::FaceModelParameters *parameters = nullptr;

private:
  int id; // used for keeping track in the pool
  float fx, fy, cx, cy;

};