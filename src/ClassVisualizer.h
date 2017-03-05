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
  void onFaceDetectionResults(vector<ofRectangle> &bboxes);
  void onOpenFaceResults();
  VisualizerMode mode = VisualizerMode::FRONTAL;

private:
  ofPixels pRGB;
  ofFloatPixels pBigDepth;
  ofTexture tRender;

  bool hasData = false;
  bool isDataStale = false;

  vector<Person> people;

  void drawFrontView();
  void drawTopView();

  KinectHelper *kinect = NULL;
  FaceDetector *faceDetector = NULL;
  OpenFace *openFace = NULL;

};
