#pragma once

#include <vector>
#include "ofThread.h"
#include "ofxCv.h"
#include "LandmarkCoreIncludes.h"
#include <FaceAnalyser.h>
#include "MtcnnDetector.h"
#include "tbb/tbb.h"
#include "Person.h"

struct FaceTracker {
  int faceIndex;
  LandmarkDetector::CLNF model;
  LandmarkDetector::FaceModelParameters parameters;
  bool isActive;
};

class OpenFace : public ofThread  {

public:
  OpenFace();
  ~OpenFace();
  void updateFaces(vector<ofRectangle> newFaces);
  void updateImage(ofPixels rgb);
  void drawTo(cv::Mat mat);
  bool isSetup;
  void doSetup();

private:
  void threadedFunction();
  void updateTrackers();

  bool isFaceAtIndexAlreadyBeingTracked(int face_ind);

  float fx, fy, cx, cy;

  bool isMatDirty;
  cv::Mat matGrayscale;

  vector<cv::Rect> faces;
  vector<FaceTracker> trackers;

};
