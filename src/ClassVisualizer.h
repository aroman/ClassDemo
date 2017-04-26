#pragma once

#include "ClassComponent.h"
#include "KinectHelper.h" // Brings in X11, but also can't depend on X11 ()
#include "OpenFaceModelPool.h" // Must be included before FaceDetector (python + dlib #define clash), also brings in X11
#include "FaceDetector.h" // Brings in OpenCV, must happen before X11
#include "ofImage.h"
#include "ofTrueTypeFont.h"
#include "ofGraphics.h"
#include "Person.h"

//We're always going to do the computation
//but we'll control what we show!

//We want to minimize the amount of work it takes when
//drawing, not because it affects performance
//(because it's single threaded)
//but because conceptually we shouldn't have to

//view settings
enum class ViewAngle {FRONTAL, BIRDSEYE};

class ClassVisualizer : public ClassComponent {

public:
  ClassVisualizer();
  ~ClassVisualizer();
  void update();
  void draw();
  void onFaceDetectionResults(const vector<ofRectangle> &bboxes);

  void onOpenFaceResults();

  ViewAngle mode = ViewAngle::FRONTAL;
  bool showHands = false; //press H or h
  bool showPose = false; //press C or c
  bool showLandmarks = false; //press L or l
  bool showDepth = false; //press X or x
  bool showPersonInfo = false; //press I or i
  bool showInfoPanel = true;
  bool showLoadScreen = true;
  bool showDebug = false;

private:

  size_t openFaceModelPoolSize = 1;

  ofPixels colorPixels;
  ofFloatPixels depthPixels; // depth pixels in meters
  ofTexture colorTexture;




  //visuals
    //imagery
      ofImage singleHand;
      ofImage cmuWordmark;
      ofImage cmuWordmarkBlue;
      ofImage fullLogo;
      ofImage classroom;

    //for drawing text
      ofTrueTypeFont demoFont;
      ofTrueTypeFont demoBoldFont;
      ofTrueTypeFont peopleFont;
      ofTrueTypeFont helpFont;
      int lineSpace;
      string helpText;
      vector<string> aboutText;

    //colors
      ofColor lightBlue;

  bool hasData = false;

  ofMutex peopleAccessMutex;
  vector<Person> people;

  void drawFrontalView();
  void drawBirdseyeView();
  void drawLoadScreen();
  void drawInfoPanel();

  FaceDetector *faceDetector = nullptr;
  OpenFaceModelPool *openFaceModelPool = nullptr;

};
