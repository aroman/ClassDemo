#pragma once

#include "ofMain.h"
#include "OpenFaceModel.h"

struct Space {
  ofRectangle r;

  ofPixels colorPixels;
  ofFloatPixels depthPixels;
  float *depthMap; // raw meters

  float minDepth;
  float maxDepth;
  float avgDepth;
  float avgDepthDiff;

  void doDepthAverage(ofRectangle r);
  void doDepthMinMax(ofRectangle r);

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
  bool isRaisingHand = false;
  bool isConfirmed = false;

  void drawFrontalView() const;
  void drawBirdseyeView() const;
  void recalculateBoundingBox();
  void updateMtcnnBoundingBox(ofRectangle bboxFromMtcnn);
  void update(const ofPixels &newColorPixels, const ofFloatPixels &newDepthPixels);

  Space f; // face
  Space h; // hand-raise area
};
