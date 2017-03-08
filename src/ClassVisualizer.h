#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "KinectHelper.h"
#include "OpenFace.h"
#include <GazeEstimation.h>
#include "FaceDetector.h"
#include "Person.h"


//We're always going to do the computation
//but we'll control what we show!

//We want to minimize the amount of work it takes when
//drawing, not because it affects performance 
//(because it's single threaded)
//but because conceptually we shouldn't have to

//view settings
enum class ViewAngle {FRONTAL, BIRDSEYE};

class ClassVisualizer {

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

private:
  ofPixels colorPixels;
  ofFloatPixels depthPixels; // depth pixels in meters50
  ofTexture colorTexture;




  //visuals
    //screen
      int screenWidth = 1920;
      int screenHeight = 1080;

    //imagery
      ofImage singleHand;
      ofImage fullLogo;

    //for drawing text
      ofTrueTypeFont demoFont;
      ofTrueTypeFont helpFont;
      int lineSpace;
      string helpText;
      vector<string> aboutText;

    //colors
      ofColor lightBlue;


  void drawStringCentered(ofTrueTypeFont font, string s, int x, int y, ofColor boxColor, ofColor textColor);
  void drawStringTopLeft(ofTrueTypeFont font, string s, int x, int y, ofColor boxColor, ofColor textColor);
  void drawStringTopRight(ofTrueTypeFont font, string s, int x, int y, ofColor boxColor, ofColor textColor);
  void drawStringBottomLeft(ofTrueTypeFont font, string s, int x, int y, ofColor boxColor, ofColor textColor);
  void drawStringBottomRight(ofTrueTypeFont font, string s, int x, int y, ofColor boxColor, ofColor textColor);






  bool hasData = false;

  ofMutex peopleAccessMutex;
  vector<Person> people;

  void drawFrontalView();
  void drawBirdseyeView();
  void drawLoadScreen();
  void drawInfoPanel();

  KinectHelper *kinect = NULL;
  FaceDetector *faceDetector = NULL;
  OpenFace *openFace = NULL;

};
