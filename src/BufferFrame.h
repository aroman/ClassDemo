#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "KinectHelper.h"
#include "OpenFace.h"
#include <GazeEstimation.h>
#include "FaceDetector.h"

struct region {
  rect r;

  ofImage imRGB;

  ofImage imDepth;
  ofFloatPixels ofpDepth;
  float* fDepth;

  int numPixels;

  float minDepth;
  float maxDepth;
  float avgDepth;
  float avgDepthDiff;

  void doDepthStats(rect r);
  void doDepthAverage(rect r);
  void doDepthVariance(rect r);
  void doDepthMinMax(rect r);
  void updateDepth(ofFloatPixels pDepth);
  void updateRGB(ofPixels pRGB);
  void free();
};

struct person {

  person();

  region f;
  region h;

  bool raisedHand = false;

  ofTrueTypeFont font;

  void drawFrontView();
  void drawTopView();
  void init(ofPixels pRGB, ofFloatPixels pBigDepth);
  void update(ofPixels pRGB, ofFloatPixels pBigDepth);
  void free();
};

class BufferFrame {

public:
  BufferFrame();
  ~BufferFrame();
  bool toggleView;
  void update();
  void draw();

private:
  ofPixels pRGB;
  ofFloatPixels pBigDepth;
  // ofPixels render;
  ofTexture tRender;
  ofTrueTypeFont font;

  bool hasData = false;
  bool isDataStale = false;

  vector<person> people;
  vector<faceData> faces;

  void drawFrontView();
  void drawTopView();

  void initPeople(FaceDetector *faceDetector);
  void findPeople(FaceDetector *faceDetector);
  void updateOpenFace(OpenFace *openFace);

  KinectHelper *kinect = NULL;
  FaceDetector *faceDetector = NULL;
  OpenFace *openFace = NULL;

};
