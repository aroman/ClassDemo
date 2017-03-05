#pragma once

#include "ofxCv.h"
#include "ofMain.h"

struct rect {
  double x;
  double y;
  double width;
  double height;

  void scaleAboutCenter(double scale);
};


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

struct Person {

  Person(ofRectangle bbox);

  region f; // face
  region h; // hand-raise area

  bool raisedHand = false;

  void drawFrontView() const;
  void drawTopView() const;
  void init(ofPixels pRGB, ofFloatPixels pBigDepth);
  void update(ofPixels pRGB, ofFloatPixels pBigDepth);
  void free();

};
