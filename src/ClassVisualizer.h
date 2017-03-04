#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "KinectHelper.h"
#include "OpenFace.h"
#include <GazeEstimation.h>
#include "FaceDetector.h"
#include "Person.h"

class ClassVisualizer {

public:
  ClassVisualizer();
  ~ClassVisualizer();
  bool toggleView = false;
  void update();
  void draw();
  void onFaceDetectionResults(mtcnn_detect_results &results);
  void onOpenFaceResults();

private:
  ofPixels pRGB;
  ofFloatPixels pBigDepth;
  ofTexture tRender;

  bool hasData = false;
  bool isDataStale = false;

  vector<Person> people;

  void drawFrontView();
  void drawTopView();

  void updateOpenFace();

  KinectHelper *kinect = NULL;
  FaceDetector *faceDetector = NULL;
  OpenFace *openFace = NULL;

};
