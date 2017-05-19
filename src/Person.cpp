#include "Person.h"

#include <math.h>
#include "drawUtils.h"
#include "ofAppRunner.h"

static const double FACE_SCALE_RATIO = 0.75;
static const double HANDSPACE_X_RATIO = 7;
static const double HANDSPACE_Y_RATIO = 5.5;

float mmToFeet(float millimeters) { return millimeters * 0.00328084; }
float feetToMm(float feet) { return feet * 304.8; }

void Space::updatePixels(const ofPixels &newColorPixels, const ofFloatPixels &newDepthPixels) {
  if (r.x < 0 || r.y < 0) return;
  newDepthPixels.cropTo(depthPixels, r.x, r.y, r.width, r.height);
  newColorPixels.cropTo(colorPixels, r.x, r.y, r.width, r.height);
  colorPixels.swapRgb();
}

// Construct a person from a bounding box
Person::Person(ofRectangle bbox) {
  updateMtcnnBoundingBox(bbox);
}

std::ostream& operator<<(std::ostream &strm, const Person &person) {
  ofRectangle bb = person.currentBoundingBox();
  if (person.openFaceModel != nullptr) {
    return strm << "Person(x=" << bb.x << ", " << "y=" << bb.y << ", " << *person.openFaceModel << ")";
  } else {
    return strm << "Person(x=" << bb.x << ", " << "y=" << bb.y << ")";
  }
}

string Person::getName() const {
  if (openFaceModel == nullptr) {
    return "unknown";
  } else {
    int numPossibleNames = sizeof(possibleNames) / sizeof(char*);
    return possibleNames[openFaceModel->getId() % numPossibleNames];
  }
}

ofRectangle Person::currentBoundingBox() const {
  if (openFaceModel != nullptr && openFaceModel->isActive()) {
    return openFaceModel->get2DBoundingBox();
  }
  return mtcnnBoundingBox;
}

void Person::update(const ofPixels &newColorPixels, const ofFloatPixels &newDepthPixels) {
  isRaisingHand = false;

  // update spaces using mtcnn or OpenFace (as available)
  recalculateSpaces();

  // update face region
  f.updatePixels(newColorPixels, newDepthPixels);

  // update hand region
  h.updatePixels(newColorPixels, newDepthPixels);

  if (timestampHandRaised == 0 && isRaisingHand) {
    timestampHandRaised = ofGetUnixTime();
  }
  if (!isRaisingHand) {
    timestampHandRaised = 0;
  }
}

// returns distance from front in millimeters
float Person::distanceFromFront() const {
  auto numPixels = f.depthPixels.size();
  float totalPixelValue = 0;
  const float *pixelData = f.depthPixels.getData();
  for (size_t i = 0; i < numPixels; i++) {
    auto currentPixel = pixelData[i];
    auto normalizedPixelValue = max(min(currentPixel, MAX_DEPTH_MILLIMETERS), MIN_DEPTH_MILLIMETERS);
    totalPixelValue += normalizedPixelValue;
  }
  auto average = totalPixelValue / numPixels;
  return average;
}

void Person::recalculateSpaces() {

  f.r = currentBoundingBox();
  f.r.scaleFromCenter(FACE_SCALE_RATIO);

  ofLogNotice("Person") << "distanceFromFront: " << distanceFromFront();

  // auto hWidthScaled = HANDSPACE_X_RATIO * f.r.width;
  // auto hHeightScaled = HANDSPACE_Y_RATIO * f.r.height;

  // auto scaleBasedOnDistance = (distanceFromFront() / MAX_DEPTH_MILLIMETERS) * 10;
  auto scaleBasedOnDistance = ofMap(distanceFromFront(),
    MIN_DEPTH_MILLIMETERS, MAX_DEPTH_MILLIMETERS * 0.75,
    1, 0, true
  );

  auto hWidth = 800 * scaleBasedOnDistance;
  auto hHeight = 550 * scaleBasedOnDistance;

  h.r = ofRectangle(
    f.r.x - (hWidth / 2) + (f.r.width / 2),
    f.r.y - (hHeight / 1.8),
    hWidth,
    hHeight
  );

  ofRectangle window = ofRectangle(0, 0, ofGetWidth(), ofGetHeight());

  if (!window.inside(h.r)) {
    h.r = ofRectangle(0,0,0,0);
    // if (!window.intersects(h.r)) {
    //   h.r = ofRectangle(0,0,0,0);
    // } else {
    //   auto x0 = h.r.x;
    //   auto y0 = h.r.y;
    //
    //   auto x1 = h.r.x + h.r.width;
    //   auto y1 = h.r.y + h.r.height;
    //
    //   x0 = MAX(0.0, x0);
    //   y0 = MAX(0.0, y0);
    //
    //   x1 = MIN(x1, window.width);
    //   y1 = MIN(y1, window.height);
    //
    //   h.r.x = x0;
    //   h.r.y = y0;
    //   h.r.width = x1 - x0;
    //   h.r.height = y1 - y0;
    // }
   }
}

void Person::updateMtcnnBoundingBox(ofRectangle bboxFromMtcnn) {
  mtcnnBoundingBox = bboxFromMtcnn;
  recalculateSpaces();
  isConfirmed = true;
}
