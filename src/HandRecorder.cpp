#include "HandRecorder.h"

#include <cstdlib>
#include <cmath>
#include "ofxTimeMeasurements.h"
#include "drawUtils.h"
#include "Serializers.h"

static const int DIST_THRESH = 150;
static const std::string OUTPUT_PATH_ROOT = "../hand-images";

static const float MIN_DEPTH_MILLIMETERS = 1;
static const float MAX_DEPTH_MILLIMETERS = 10000;

float averagePixelValue(const ofFloatPixels &pixels) {
  auto numPixels = pixels.size();
  float totalPixelValue = 0;
  const float *pixelData = pixels.getData();
  for (size_t i = 0; i < numPixels; i++) {
    auto currentPixel = pixelData[i];
    auto normalizedPixelValue = max(min(currentPixel, MAX_DEPTH_MILLIMETERS), MIN_DEPTH_MILLIMETERS);
    totalPixelValue += normalizedPixelValue;
  }
  auto average = totalPixelValue / numPixels;
  return average;
}

ofPixels paintPixelsWithinRange(const ofFloatPixels &pixels, float low, float high) {
  ofPixels paintedPixels;
  paintedPixels.allocate(pixels.getWidth(), pixels.getHeight(), OF_IMAGE_COLOR_ALPHA);
  // ofLogNotice("paintPixelsWithinRange") << pixels.getWidth() << "x" << pixels.getHeight();
  const float *data = pixels.getData();
  auto redPixel = ofColor(255,0,0,127);
  auto transparentPixel = ofColor(0,0,0,0);

  for (int i = 0; i < pixels.getWidth() * pixels.getHeight(); i += 1) {
    auto normalizedPixelValue = max(min(data[i], MAX_DEPTH_MILLIMETERS), MIN_DEPTH_MILLIMETERS);
    bool isPixelWithinThreshold = (normalizedPixelValue > low) && (normalizedPixelValue < high);
    paintedPixels.setColor(i*4, isPixelWithinThreshold ? redPixel : transparentPixel);
  }

  return paintedPixels;
}

HandRecorder::HandRecorder() {
  hasData = false;

  openSansBold.load("fonts/OpenSans-Bold.ttf", 50, true);

  auto outputPath = ofFilePath::join(OUTPUT_PATH_ROOT, ofGetTimestampString());

  raisedImagesDir = new ofDirectory(ofFilePath::join(outputPath, "true"));
  notRaisedImagesDir = new ofDirectory(ofFilePath::join(outputPath, "false"));

  gui.setup();
  gui.add(radius.setup("radius", 255, 25, 500));

  faceDetector = new FaceDetector(10);
  faceDetector->startThread(true);
  // Might help performance a bit, we don't want it stealing CPU time
  // from the main/GL/draw thread!
  (&faceDetector->getPocoThread())->setPriority(Poco::Thread::PRIO_LOWEST);
  ofAddListener(faceDetector->onDetectionResults, this, &HandRecorder::onFaceDetectionResults);
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

  TS_START("update color texture");
  colorTexture.loadData(colorPixels);
  TS_STOP("update color texture");

  peopleAccessMutex.lock();
  for (auto &person : people) {
    person.update(colorPixels, depthPixels);
    TS_START("paint depth pixels");
    ofPoint faceCenter = person.f.r.getCenter();
    // ofLogNotice("Person") << "average face depth: " << mmToFeet(averageFaceDepth) << "ft";
    auto averageFaceDepth = averagePixelValue(person.f.depthPixels);
    ofPixels depthPaintedPixels = paintPixelsWithinRange(
      person.h.depthPixels,
      averageFaceDepth - (radius * 2),
      averageFaceDepth + radius
    );
    paintedPixelsTexture.loadData(depthPaintedPixels);
    TS_STOP("update depth texture");

  }
  peopleAccessMutex.unlock();

  faceDetector->updateImage(colorPixels);
}

void HandRecorder::draw() {
  if (!hasData) return;

  colorTexture.draw(0, 0);

  if (people.size() == 0) return;

  auto const &person = people[0];

  ofEnableAlphaBlending();
  paintedPixelsTexture.draw(person.h.r.x, person.h.r.y);
  ofDisableAlphaBlending();

  drawBoundBox(person.f.r, ofColor::purple);
  drawBoundBox(person.h.r, ofColor::blue);
  ofSetColor(ofColor::white);

  gui.draw();

  auto barHeight = 200;
  ofColor barColor;
  if (currentLabel == HandLabel::RAISED) {
    barColor = ofColor::green;
  } else {
    barColor = ofColor::red;
  }
  ofFill();
  ofSetColor(barColor);
  ofDrawRectangle(0, ofGetHeight() - barHeight, ofGetWidth(), barHeight);
  ofSetColor(ofColor::white);
  ofNoFill();
  ofDrawBitmapStringHighlight((currentLabel == HandLabel::RAISED ? "RAISED" : "NOT RAISED"), 100, 200);
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
      assert(closestPersonIndex != -1);
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

  string baseFilePath;
  int randInt = round(ofRandom(0, 1000000));
  if (currentLabel == HandLabel::RAISED) {
    baseFilePath = ofFilePath::join(raisedImagesDir->getAbsolutePath(), ofToString(randInt));
  } else {
    baseFilePath = ofFilePath::join(notRaisedImagesDir->getAbsolutePath(), ofToString(randInt));
  }

  ofLogNotice("HandRecorder") << baseFilePath;

  // serializeColor(baseFilePath + ".jpg", person.f.colorPixels);

  peopleAccessMutex.unlock();

}
