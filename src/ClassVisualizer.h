#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "KinectHelper.h"
#include "OpenFace.h"
#include <GazeEstimation.h>
#include "FaceDetector.h"
#include "Person.h"

enum class VisualizerMode {FRONTAL, BIRDSEYE};

class ClassVisualizer {

public:
  ClassVisualizer();
  ~ClassVisualizer();
  void update();
  void draw();
  void onFaceDetectionResults(const vector<ofRectangle> &bboxes);
  void onOpenFaceResults();
  VisualizerMode mode = VisualizerMode::FRONTAL;

private:
  ofPixels colorPixels;
  ofFloatPixels depthPixels;
  ofTexture colorTexture;

  bool hasData = false;

  ofMutex peopleAccessMutex;
  vector<Person> people;

  void drawFrontalView();
  void drawBirdseyeView();

  KinectHelper *kinect = NULL;
  FaceDetector *faceDetector = NULL;
  OpenFace *openFace = NULL;

};
