#pragma once

#include "ClassComponent.h"
#include "KinectHelper.h" // Brings in X11, but also can't depend on X11 ()
#include "OpenFaceModelPool.h" // Must be included before FaceDetector (python + dlib #define clash), also brings in X11
#include "FaceDetector.h" // Brings in OpenCV, must happen before X11
#include "ofImage.h"
#include "ofTrueTypeFont.h"
#include "ofGraphics.h"
#include "Person.h"

class HandRecorder : public ClassComponent {

public:
  HandRecorder();
  ~HandRecorder();
  void update();
  void draw();
  void onFaceDetectionResults(const vector<ofRectangle> &bboxes);

private:
  bool hasData = false;

  ofPixels colorPixels;
  ofFloatPixels depthPixels; // depth pixels in meters

  ofTexture colorTexture;
  ofTexture scaledDepthTexture;

  ofTrueTypeFont openSansBold;
  ofMutex peopleAccessMutex;

  vector<Person> people;
  FaceDetector *faceDetector = nullptr;
};
