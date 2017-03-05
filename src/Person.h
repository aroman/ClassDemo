#pragma once

#include "ofxCv.h"
#include "ofMain.h"

struct region {
  ofRectangle r;

  ofImage imRGB;
  ofFloatImage imDepth;
  float *depthMap;

  float minDepth;
  float maxDepth;
  float avgDepth;
  float avgDepthDiff;

  void doDepthStats(ofRectangle r);
  void doDepthAverage(ofRectangle r);
  void doDepthVariance(ofRectangle r);
  void doDepthMinMax(ofRectangle r);
  void updateDepth(const ofFloatPixels &pDepth);
  void updateRGB(const ofPixels &pRGB);
  void free();
};

struct Person {
  explicit Person(ofRectangle bbox);

  region f;  // face
  region h;  // hand-raise area

  bool raisedHand = false;

  void drawFrontView() const;
  void drawTopView() const;
  void update(const ofPixels &pRGB, const ofFloatPixels &pBigDepth);
  void free();
};
