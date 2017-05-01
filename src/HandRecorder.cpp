#include "HandRecorder.h"

#include <cstdlib>
#include "ofxTimeMeasurements.h"
#include "drawUtils.h"

static const int DIST_THRESH = 150;

#define SCREEN_HEIGHT (1080)
#define SCREEN_WIDTH (1920)

HandRecorder::HandRecorder() {
  hasData = false;

  openSansBold.load("fonts/OpenSans-Bold.ttf", 20, true);

  faceDetector = new FaceDetector(6.5);
  faceDetector->startThread(true);
  // Might help performance a bit, we don't want it stealing CPU time
  // from the main/GL/draw thread!
  (&faceDetector->getPocoThread())->setPriority(Poco::Thread::PRIO_LOWEST);
  ofAddListener(
    faceDetector->onDetectionResults,
    this,
    &HandRecorder::onFaceDetectionResults
  );
}

HandRecorder::~HandRecorder() {
  people.clear();

  faceDetector->waitForThread(true);
  delete faceDetector;
}

void HandRecorder::update() {
  if (!this->kinect->isConnected) return;

  TS_START("[Kinect] update frames");
  colorPixels = this->kinect->getColorPixels();
  depthPixels = this->kinect->getAlignedDepthPixels();
  TS_STOP("[Kinect] update frames");
  hasData = (colorPixels.size() > 0);

  if (!hasData) return;

  TS_START("update depth texture");
  ofFloatPixels scaledDepth = depthPixels;
  scaleDepthPixelsForDrawing(&scaledDepth);
  scaledDepthTexture.loadData(scaledDepth);
  TS_STOP("update depth texture");

  TS_START("update color texture");
  colorTexture.loadData(colorPixels);
  TS_STOP("update color texture");

  faceDetector->updateImage(colorPixels);
}

void HandRecorder::draw() {
  if (!hasData){
    return;
  }

  colorTexture.draw(0, 0);

  // Render depth as semi-transparent overlay
  ofEnableAlphaBlending();
  ofSetColor(255,255,255,170);
  scaledDepthTexture.draw(0,0);
  ofDisableAlphaBlending();

  // Draw people
  for (auto const &person : people) {
    // person.drawFrontBBox(ofColor::orange);
    ofRectangle bb = person.currentBoundingBox();
    bb.scaleFromCenter(4.5);

    float heightOffset = bb.height * 0.4;
    bb.height += heightOffset;
    bb.y -= heightOffset;

    float widthOffset = bb.width * 0.4;
    bb.width += widthOffset;
    bb.x -= (widthOffset / 2);

    drawBoundBox(bb, ofColor::orange);
    ofSetColor(255,255,255);
  }
}

void HandRecorder::onFaceDetectionResults(const vector<ofRectangle> &bboxes) {
  ofLogNotice("HandRecorder") << "onFaceDetectionResults " << bboxes.size();

  peopleAccessMutex.lock();

  for (int i = 0; i < people.size(); i++) {
    people.at(i).isConfirmed = false;
  }

  // Sort the boxes by area. Make a copy because we have a const reference.
  auto sortedBBoxes = bboxes;
  std::sort(
    sortedBBoxes.begin(),
    sortedBBoxes.end(),
    [] (const ofRectangle& a, const ofRectangle& b) { return a.getArea() > b.getArea(); }
  );

  // Only consider the first (largest) person
  if (sortedBBoxes.size() > 0) {
    auto bbox = sortedBBoxes[0];
    int closestPersonIndex = -1;

    float closestDistance = DIST_THRESH;
    for (int i = 0; i < people.size(); i++) {
      float dist = people.at(i).currentBoundingBox().getCenter().distance(bbox.getCenter());
      if (dist < closestDistance) {
        closestPersonIndex = i;
        closestDistance = dist;
      }
    }

    ofLogNotice("HandRecorder") << "closest distance: " << closestDistance;
    if (closestDistance < DIST_THRESH) {
      assert (closestPersonIndex != -1);
      people.at(closestPersonIndex).updateMtcnnBoundingBox(bbox);
    } else {
      people.push_back(Person(bbox));
    }

  }

  for (int i = 0; i < people.size(); i++) {
    if (!people.at(i).isConfirmed) {
      people.erase(people.begin() + i);
    }
  }

  for (auto &person : people) {
    ofLogNotice("HandRecorder") << person;
  }

  peopleAccessMutex.unlock();

}
