#include "HandRecorder.h"

#include <cstdlib>
#include <cmath>
#include "ofxTimeMeasurements.h"
#include "drawUtils.h"
#include "Serializers.h"

static const int DIST_THRESH = 150;
static const string OUTPUT_PATH_ROOT = "../hand-images";

static const float MIN_DEPTH_MILLIMETERS = 250;
static const float MAX_DEPTH_MILLIMETERS = 10000;

static const string UI_BANNER_RAISE = "images/record-hand/raise.png";
static const string UI_BANNER_DONT_RAISE = "images/record-hand/dont-raise.png";
static const string UI_BANNER_TROUBLE = "images/record-hand/lost-tracking.png";
static const string UI_BANNER_WAIT = "images/record-hand/wait.png";
static const string UI_BANNER_CENTER = "images/record-hand/move-center.png";
static const string UI_BANNER_NO_FACE = "images/record-hand/no-face.png";

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

ofPixels paintPixelsWithinRange(ofColor inColor, ofColor outColor, const ofFloatPixels &pixels, float low, float high) {
  ofPixels paintedPixels;
  paintedPixels.allocate(pixels.getWidth(), pixels.getHeight(), OF_IMAGE_COLOR_ALPHA);
  const float *data = pixels.getData();

  for (int i = 0; i < pixels.getWidth() * pixels.getHeight(); i += 1) {
    auto normalizedPixelValue = max(min(data[i], MAX_DEPTH_MILLIMETERS), MIN_DEPTH_MILLIMETERS);
    bool isPixelWithinThreshold = (normalizedPixelValue > low) && (normalizedPixelValue < high);
    paintedPixels.setColor(i*4, isPixelWithinThreshold ? inColor : outColor);
  }

  return paintedPixels;
}

ofPixels scaleDepthWithinRange(const ofFloatPixels &pixels, float low, float high) {
  ofPixels paintedPixels;
  paintedPixels.allocate(pixels.getWidth(), pixels.getHeight(), OF_IMAGE_COLOR_ALPHA);
  const float *data = pixels.getData();

  for (int i = 0; i < pixels.getWidth() * pixels.getHeight(); i += 1) {
    auto normalizedPixelValue = max(min(data[i], MAX_DEPTH_MILLIMETERS), MIN_DEPTH_MILLIMETERS);
    bool isPixelWithinThreshold = (normalizedPixelValue > low) && (normalizedPixelValue < high);
    if (isPixelWithinThreshold) {
      float scaledDepthValue = ofMap(normalizedPixelValue, low, high, 0, 255, true);
      ofColor grayscalePixel = ofColor(
        scaledDepthValue,
        scaledDepthValue,
        scaledDepthValue
      );
      paintedPixels.setColor(i*4, grayscalePixel);
    } else {
      paintedPixels.setColor(i*4, ofColor::black);
    }
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
  gui.setPosition(100, 1000);
  gui.add(radius.setup("radius", 255, 25, 500));

  faceDetector = new FaceDetector(10);
  faceDetector->startThread(true);
  // Might help performance a bit, we don't want it stealing CPU time
  // from the main/GL/draw thread!
  (&faceDetector->getPocoThread())->setPriority(Poco::Thread::PRIO_LOWEST);
  ofAddListener(faceDetector->onDetectionResults, this, &HandRecorder::onFaceDetectionResults);

  setState(RecordState::WAIT);
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


  paintedPixelsTexture.clear();
  bool isRecording = state == RecordState::RAISE || state == RecordState::DONT_RAISE;

  peopleAccessMutex.lock();

  if (people.size() == 0) {
    setState(RecordState::NO_FACE);
  } else {
    auto &person = people[0];
    person.update(colorPixels, depthPixels);
    TS_START("paint depth pixels");
    if (person.h.r.getArea() == 0) {
      setState(RecordState::MOVE_CENTER);
    } else {
      if (state != RecordState::RAISE && state != RecordState::DONT_RAISE) {
        setState(RecordState::WAIT);
      }
      ofPoint faceCenter = person.f.r.getCenter();
      auto averageFaceDepth = averagePixelValue(person.f.depthPixels);
      ofPixels depthPaintedPixels = paintPixelsWithinRange(
        isRecording ? (ofColor(255,0,0,127)) : (ofColor(255,255,255,127)),
        ofColor(0,0,0,0),
        person.h.depthPixels,
        averageFaceDepth - (radius * 2),
        averageFaceDepth + radius
      );
      paintedPixelsTexture.loadData(depthPaintedPixels);
    }
    // ofLogNotice("Person") << "average face depth: " << mmToFeet(averageFaceDepth) << "ft";
    TS_STOP("paint depth pixels");
  }

  peopleAccessMutex.unlock();

  faceDetector->updateImage(colorPixels);
}

void HandRecorder::draw() {
  if (!hasData) return;

  colorTexture.draw(0, 0);

  if (people.size() == 0) {
    if (stateImage.isAllocated()) {
      stateImage.draw(0, 0);
    }
    return;
  }

  auto const &person = people[0];

  if (paintedPixelsTexture.isAllocated()) {
    ofEnableAlphaBlending();
    paintedPixelsTexture.draw(person.h.r.x, person.h.r.y);
    ofDisableAlphaBlending();
  }

  bool isRecording = state == RecordState::RAISE || state == RecordState::DONT_RAISE;

  ofNoFill();
  ofSetLineWidth(10.0);
  ofSetColor(isRecording ? (ofColor::red) : (ofColor::white));
  ofDrawRectangle(person.h.r);

  if (person.h.r.getArea() == 0) {
    ofSetColor(ofColor::white);
    ofSetLineWidth(5.0);
    ofDrawRectangle(person.f.r);
  }
  ofSetColor(ofColor::white);

  gui.draw();

  if (stateImage.isAllocated()) {
    stateImage.draw(0, 0);
  }

  if (parsedDepthTexture.isAllocated()) {
    parsedDepthTexture.draw(0, 0);
  }
}

void HandRecorder::setState(RecordState newState) {
  // If the new state is the same as the old state, no-op
  if (newState == state) return;

  state = newState;

  stateImage.clear();
  if (state == RecordState::MOVE_CENTER) {
    stateImage.load(UI_BANNER_CENTER);
  }
  else if (state == RecordState::RAISE) {
    stateImage.load(UI_BANNER_RAISE);
  }
  else if (state == RecordState::DONT_RAISE) {
    stateImage.load(UI_BANNER_DONT_RAISE);
  }
  else if (state == RecordState::NO_FACE) {
    stateImage.load(UI_BANNER_NO_FACE);
  }
  else {
    stateImage.load(UI_BANNER_WAIT);
  }
}

void HandRecorder::saveFramesToDisk() {
  if (people.size() == 0) return;
  auto const &person = people[0];
  if (state == RecordState::RAISE || state == RecordState::DONT_RAISE) {
    ofDirectory *baseDir = (state == RecordState::RAISE) ? raisedImagesDir : notRaisedImagesDir;
    string baseFilePath = ofFilePath::join(
      baseDir->getAbsolutePath(),
      ofToString(ofGetElapsedTimeMicros())
    );
    ofLogNotice("HandRecorder") << baseFilePath;
    serializeColor(baseFilePath + ".color.jpg", person.h.colorPixels);
    ofPoint faceCenter = person.f.r.getCenter();
    auto averageFaceDepth = averagePixelValue(person.f.depthPixels);
    // ofPixels filteredDepthPixels = paintPixelsWithinRange(
    //   ofColor::white,
    //   ofColor::black,
    //   person.h.depthPixels,
    //   averageFaceDepth - (radius * 2),
    //   averageFaceDepth + radius
    // );
    ofPixels scaledDepth = scaleDepthWithinRange(
      person.h.depthPixels,
      averageFaceDepth - (radius * 2),
      averageFaceDepth + radius
    );
    serializeGrayscaleDepth(baseFilePath + ".depth.png", scaledDepth);
  }
}

void HandRecorder::onFaceDetectionResults(const vector<ofRectangle> &bboxes) {
  // ofLogNotice("HandRecorder") << "onFaceDetectionResults " << bboxes.size();

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

    // ofLogNotice("HandRecorder") << "closest distance: " << closestDistance;
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

  peopleAccessMutex.unlock();
}
