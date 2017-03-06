#include "ClassVisualizer.h"
#include "ofxTimeMeasurements.h"

ClassVisualizer::ClassVisualizer() {
  hasData = false;

  kinect = new KinectHelper();

  bool didConnectSuccessfully = kinect->connect();
  if (!didConnectSuccessfully) {
      std::exit(1);
  }

  openFace = new OpenFace(kinect->fx, kinect->fy, kinect->cx, kinect->cy);
  openFace->doSetup();
  openFace->startThread(true);

  faceDetector = new FaceDetector();
  faceDetector->startThread(true);
  // Might help performance a bit, we don't want it stealing CPU time
  // from the main/GL/draw thread!
  (&faceDetector->getPocoThread())->setPriority(Poco::Thread::PRIO_LOWEST);
  ofAddListener(
    faceDetector->onDetectionResults,
    this,
    &ClassVisualizer::onFaceDetectionResults
  );
}

ClassVisualizer::~ClassVisualizer() {
  people.clear();

  faceDetector->waitForThread(true);
  delete faceDetector;

  kinect->waitForThread(true);
  kinect->disconnect();

  openFace->waitForThread(true);
  delete openFace;
}

void ClassVisualizer::update() {
  if (!kinect->isConnected) return;

  colorPixels = kinect->getColorPixels();
  depthPixels = kinect->getBigDepthPixels();
  hasData = (colorPixels.size() > 0);

  if (!hasData) return;

  TS_START("update color texture");
  // colorTexture.loadData(colorPixels);
  TS_STOP("update color texture");

  faceDetector->updateImage(&colorPixels);

  peopleAccessMutex.lock();
  for (auto &person : people) {
    person.update(colorPixels, depthPixels);
  }
  peopleAccessMutex.unlock();

  if (openFace->isSetup && !people.empty()) {
    TS_START("[OpenFace] update color pixel data");
    openFace->updateImage(colorPixels);
    TS_STOP("[OpenFace] update color pixel data");

    TS_START("[OpenFace] update trackers");
    openFace->updateTrackers();
    TS_STOP("[OpenFace] update trackers");
  }
}

void ClassVisualizer::draw() {
  if (!hasData) return;

  if (mode == VisualizerMode::BIRDSEYE) {
    drawBirdseyeView();
  } else {
    drawFrontalView();
  }
}

void ClassVisualizer::drawFrontalView() {
  // Draw OpenFace
  TS_START("[OpenFace] draw");
  ofPixels colorForMat;
  colorForMat.setFromPixels(colorPixels.getData(), colorPixels.getWidth(), colorPixels.getHeight(), OF_IMAGE_COLOR_ALPHA);
  colorForMat.swapRgb();
  cv::Mat mat = ofxCv::toCv(colorForMat);
  openFace->drawTo(mat);
  ofxCv::drawMat(mat, 0, 0);
  TS_STOP("[OpenFace] draw");

  // Draw people
  for (auto const &person : people) {
    person.drawFrontalView();
  }

  // colorTexture.draw(0, 0);

  ofDrawBitmapStringHighlight("Front Facing View", 960, 15);
}

void ClassVisualizer::drawBirdseyeView() {
  for (auto const &person : people) {
    person.drawBirdseyeView();
  }

  ofDrawBitmapStringHighlight("Bird's Eye View", 960, 15);
  ofDrawBitmapStringHighlight("back", 10, 15);
  ofDrawBitmapStringHighlight("front", 10, 1065);
}

void ClassVisualizer::onFaceDetectionResults(const vector<ofRectangle> &bboxes) {
  ofLogNotice("ClassVisualizer") << "onFaceDetectionResults " << bboxes.size();

  peopleAccessMutex.lock();
  people.clear();
  people.reserve(bboxes.size());
  for (auto bbox : bboxes) {
    people.push_back(Person(bbox));
  }
  peopleAccessMutex.unlock();
  openFace->updateFaces(bboxes);
}
