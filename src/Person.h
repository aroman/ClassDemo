#pragma once

#include "ofGraphics.h"
#include "ofPixels.h"
#include "ofTrueTypeFont.h"
#include "OpenFaceModel.h"

static const char * const possibleNames[] = {
  "cat",
  "rabbit",
  "fox",
  "dolphin",
  "zebra",
  "penguin",
  "lion",
  "eagle",
  "tiger"
};

struct DepthStat {
  float min;
  float max;
  float avg;
  float mode;
  bool valid;
};

struct Space {
  ofRectangle r; //location in original image coordinates

  ofPixels colorPixels;
  ofFloatPixels depthPixels;

  float *depthMap; // raw meters

  DepthStat doDepthMathAt(float x, float y, float radius);
  DepthStat doDepthMath(ofRectangle r);

  void updateDepthPixels(const ofFloatPixels &newDepthPixels);
  void updateColorPixels(const ofPixels &newColorPixels);
};

struct Person {
  explicit Person(ofRectangle bbox);
  bool operator==(const Person& other) const;
  friend std::ostream& operator<<(std::ostream &strm, const Person &person);

  ofRectangle currentBoundingBox() const;

  std::shared_ptr<OpenFaceModel> openFaceModel;
  ofRectangle mtcnnBoundingBox;
  // ofRectangle openFace


  //void drawFrontalView() const;
  //void drawBirdseyeView() const;

  void drawFrontDepth() const;

  int topBoxWidth = 150;
  void drawTopColor() const;

  void drawFrontDepthPoints(ofColor c) const;

  ofFloatPixels thresholdDepthPixels;
  void drawFrontHandThresholded() const;
  int callThresholdPixels(float low, float high);
  int thresholdPixels(ofFloatPixels* depthPixels, float low, float high);
  void drawFrontHandbox(ofColor c) const;
  void drawFrontPose(ofColor c) const;
  void drawFrontBBox(ofColor c) const;
  void drawFrontLandmarks(ofColor c) const;
  void drawTopHandbox(ofColor c) const;
  void drawTopLandmarks(ofColor c) const;

  void drawFrontPersonInfo(ofTrueTypeFont font) const;
  void drawTopPersonInfo(ofTrueTypeFont font) const;
  void drawPersonInfo(ofTrueTypeFont font, int x, int y) const;

  string getName() const;
  void recalculateBoundingBox();
  void updateMtcnnBoundingBox(ofRectangle bboxFromMtcnn);
  void update(const ofPixels &newColorPixels, const ofFloatPixels &newDepthPixels);

  Space f; // face
  Space h; // hand-raise area

  vector<ofPoint> depthLandmarks;

  float dotRadius = 1;

  float depth;
  int y_depth;

  float offsetFront = 0.075;
  float offsetBack = 0.035;
  float tempBack = offsetBack;
  float threshold = 0.05;

  float test1 = -2.0;
  float test2 = -2.0;
  float test3 = -2.0;
  float test4 = -2.0;
  float test5 = 1.0;


  bool isConfirmed = false;

  //don't do depth work if we have bad depth
  bool hasGoodDepth = false;
  bool isRaisingHand = false;
  unsigned int timestampHandRaised = 0;
};
