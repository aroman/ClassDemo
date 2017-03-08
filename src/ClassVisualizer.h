#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "KinectHelper.h"
#include <GazeEstimation.h>
#include "FaceDetector.h"
#include "OpenFaceModelPool.h"
#include "Person.h"

enum class VisualizerMode {FRONTAL, BIRDSEYE};

class ClassVisualizer {

public:
  ClassVisualizer();
  ~ClassVisualizer();
  void update();
  void draw();
  void onFaceDetectionResults(const vector<ofRectangle> &bboxes);
  VisualizerMode mode = VisualizerMode::FRONTAL;

private:
  static const size_t openFaceModelPoolSize = 1;
  ofPixels colorPixels;
  ofFloatPixels depthPixels; // depth pixels in meters
  ofTexture colorTexture;

  bool hasData = false;

  ofMutex peopleAccessMutex;
  vector<Person> people;

  void drawFrontalView();
  void drawBirdseyeView();

  KinectHelper *kinect = NULL;
  FaceDetector *faceDetector = NULL;
  OpenFaceModelPool *openFaceModelPool = NULL;

};
