#pragma once

#include "ofMain.h"
#include "OpenFaceModel.h"


void drawStringCentered(ofTrueTypeFont font, string s, int x, int y, ofColor boxColor, ofColor textColor);
void drawStringTopLeft(ofTrueTypeFont font, string s, int x, int y, ofColor boxColor, ofColor textColor);
void drawStringTopRight(ofTrueTypeFont font, string s, int x, int y, ofColor boxColor, ofColor textColor);
void drawStringBottomLeft(ofTrueTypeFont font, string s, int x, int y, ofColor boxColor, ofColor textColor);
void drawStringBottomRight(ofTrueTypeFont font, string s, int x, int y, ofColor boxColor, ofColor textColor);



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
  void drawTopColor() const;

  void drawFrontDepthPoints(ofColor c) const;


  void drawFrontHandbox(ofColor c) const;
  void drawFrontPose(ofColor c) const;
  void drawFrontBBox(ofColor c) const;
  void drawFrontLandmarks(ofColor c) const; 
  void drawTopHandbox(ofColor c) const;
  void drawTopLandmarks(ofColor c) const;

  void drawFrontPersonInfo(ofTrueTypeFont font) const;
  void drawTopPersonInfo(ofTrueTypeFont font) const;
  void drawPersonInfo(ofTrueTypeFont font, int x, int y) const;



  void recalculateBoundingBox();
  void updateMtcnnBoundingBox(ofRectangle bboxFromMtcnn);
  void update(const ofPixels &newColorPixels, const ofFloatPixels &newDepthPixels);

  Space f; // face
  Space h; // hand-raise area

  vector<ofPoint> depthLandmarks;
  float depth;
  int y_depth;
  string name;
  
  bool isConfirmed = false;

  //don't do depth work if we have bad depth
  bool hasGoodDepth = false;
  bool isRaisingHand = false;
};
