#pragma once

#include "ofMain.h"
#include "OpenFaceModel.h"


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


  void recalculateBoundingBox();
  void updateMtcnnBoundingBox(ofRectangle bboxFromMtcnn);
  void update(const ofPixels &newColorPixels, const ofFloatPixels &newDepthPixels);

  Space f; // face
  Space h; // hand-raise area


  string name;

  bool isRaisingHand = false;
  bool isConfirmed = false;

  //don't do depth work if we have bad depth
  bool hasGoodDepth = false;
  float depth;
};
