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

  region f; // face
  region h; // hand-raise area

  bool raisedHand = false;

  void drawFrontView() const;
  void drawTopView() const;
  void init(ofPixels pRGB, ofFloatPixels pBigDepth);
  void update(ofPixels pRGB, ofFloatPixels pBigDepth);
  void free();
};

class BufferFrame {

public:
  BufferFrame();
  ~BufferFrame();
  bool toggleView = false;
  void update();
  void draw();
  void onFaceDetectionResults(mtcnn_detect_results &results);

private:
  ofPixels pRGB;
  ofFloatPixels pBigDepth;
  ofTexture tRender;

  bool hasData = false;
  bool isDataStale = false;

  vector<person> people;
  vector<faceData> faces;

  void drawFrontView();
  void drawTopView();

  void updateOpenFace();

  KinectHelper *kinect = NULL;
  FaceDetector *faceDetector = NULL;
  OpenFace *openFace = NULL;

};
