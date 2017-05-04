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

struct Space {
  ofRectangle r; // location in original image coordinates

  ofPixels colorPixels;
  ofFloatPixels depthPixels;

  void updatePixels(const ofPixels &newColorPixels, const ofFloatPixels &newDepthPixels);
};

struct Person {
  explicit Person(ofRectangle bbox);
  bool operator==(const Person& other) const;
  friend std::ostream& operator<<(std::ostream &strm, const Person &person);

  // Methods
  string getName() const;
  void recalculateSpaces();
  void updateMtcnnBoundingBox(ofRectangle bboxFromMtcnn);
  void update(const ofPixels &newColorPixels, const ofFloatPixels &newDepthPixels);
  ofRectangle currentBoundingBox() const;

  // Instance variables
  ofPixels depthPaintedPixels;
  std::shared_ptr<OpenFaceModel> openFaceModel;
  ofRectangle mtcnnBoundingBox;
  int topBoxWidth = 150;
  Space f; // face
  Space h; // hand-raise area
  vector<ofPoint> depthLandmarks;
  float depth;
  int y_depth;
  bool isConfirmed = false; // person has been re-confirmed during tracking update
  bool isRaisingHand = false;
  bool hasGoodDepth = false;
  unsigned int timestampHandRaised = 0;

  // Drawing methods
  void drawFrontHandThresholded() const;
  void drawFrontDepthPoints(ofColor c) const;
  void drawFrontDepth() const;
  void drawTopColor() const;
  void drawFrontHandbox(ofColor c) const;
  void drawFrontPose(ofColor c) const;
  void drawFrontBBox(ofColor c) const;
  void drawFrontLandmarks(ofColor c) const;
  void drawTopHandbox(ofColor c) const;
  void drawTopLandmarks(ofColor c) const;
  void drawFrontPersonInfo(ofTrueTypeFont font) const;
  void drawTopPersonInfo(ofTrueTypeFont font) const;
  void drawPersonInfo(ofTrueTypeFont font, int x, int y) const;
};
