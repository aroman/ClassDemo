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

  double getX() const;
  double getY() const;
};

class OpenFace : public ofThread  {

public:
  OpenFace(float fx, float fy, float cx, float cy);
  ~OpenFace();
  void updateFaces(vector<ofRectangle> newFaces);
  void updateImage(ofPixels rgb);
  void drawTo(cv::Mat mat);
  bool isSetup;
  void doSetup();
  void updateTrackers();

private:
  void threadedFunction();

  bool isFaceAtIndexAlreadyBeingTracked(int face_ind);
  void alreadyTrackingFaceWithin(ofRectangle bbox);

  float fx, fy, cx, cy;

  bool isMatDirty;
  cv::Mat matGrayscale;
  cv::Mat matGrayscaleBack;
  cv::Mat matGrayscaleFront;

  vector<cv::Rect> faces;
  vector<FaceTracker> trackers;

};
