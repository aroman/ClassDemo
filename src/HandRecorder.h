#pragma once

#include "ClassComponent.h"
#include "KinectHelper.h" // Brings in X11, but also can't depend on X11 ()
#include "OpenFaceModelPool.h" // Must be included before FaceDetector (python + dlib #define clash), also brings in X11
#include "FaceDetector.h" // Brings in OpenCV, must happen before X11
#include "ofImage.h"
#include "ofTrueTypeFont.h"
#include "ofGraphics.h"
#include "Person.h"
#include "ofxGui.h"

enum class HandLabel {RAISED, NOT_RAISED};

class HandRecorder : public ClassComponent {

public:
  HandRecorder();
  ~HandRecorder();
  void update();
  void draw();
  void onFaceDetectionResults(const vector<ofRectangle> &bboxes);

private:
  HandLabel currentLabel = HandLabel::NOT_RAISED;
  bool hasData = false;

  ofxFloatSlider radius;
  ofxPanel gui;

  ofDirectory *raisedImagesDir;
  ofDirectory *notRaisedImagesDir;

  ofPixels colorPixels;
  ofFloatPixels depthPixels; // depth pixels in meters

  ofTexture colorTexture;
  ofTexture paintedPixelsTexture;

  ofTrueTypeFont openSansBold;
  ofMutex peopleAccessMutex;

  vector<Person> people;
  FaceDetector *faceDetector = nullptr;
};
