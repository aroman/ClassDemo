#pragma once

#include "ofxCv.h"
#include "ofMain.h"

struct Space {
  ofRectangle r;

  ofImage imColor;
  ofFloatImage imDepth;
  float *depthMap;

  float minDepth;
  float maxDepth;
  float avgDepth;
  float avgDepthDiff;

  void doDepthAverage(ofRectangle r);
  void doDepthMinMax(ofRectangle r);

  void updateDepth(const ofFloatPixels &depthPixels);
  void updateColor(const ofPixels &colorPixels);
};

struct Person {
  explicit Person(ofRectangle bbox);

  Space f;  // face
  Space h;  // hand-raise area

  bool raisedHand = false;

  void drawFrontalView() const;
  void drawBirdseyeView() const;
  void update(const ofPixels &colorPixels, const ofFloatPixels &depthPixels);
};
