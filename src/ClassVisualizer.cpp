#include "ClassVisualizer.h"
#include "ofxTimeMeasurements.h"

ClassVisualizer::ClassVisualizer() {
  openFace = new OpenFace();
  // openFace->doSetup();
  // openFace->startThread(true);

  kinect = new KinectHelper();

  bool didConnectSuccessfully = kinect->connect();
  if (!didConnectSuccessfully) {
      std::exit(1);
  }

  hasData = false;

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

  pRGB = kinect->getRgbPixels();
  pBigDepth = kinect->getBigDepthPixels();
  hasData = (pRGB.size() > 0);

  if (!hasData) return;

  TS_START("load RGB texture");
  tRender.loadData(pRGB);
  TS_STOP("load RGB texture");

  faceDetector->updateImage(&pRGB);

  peopleAccessMutex.lock();
  for (auto &person : people) {
    person.update(pRGB, pBigDepth);
  }
  peopleAccessMutex.unlock();
}

void ClassVisualizer::draw() {
  if (!hasData) return;

  if (mode == VisualizerMode::BIRDSEYE) {
    drawTopView();
  } else {
    drawFrontView();
  }
}

void ClassVisualizer::drawFrontView() {
  tRender.draw(0, 0);

  for (auto const &person : people) {
    person.drawFrontView();
  }

  ofDrawBitmapStringHighlight("Front Facing View", 960, 15);
}

void ClassVisualizer::drawTopView() {
  for (auto const &person : people) {
    person.drawTopView();
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
}

void ClassVisualizer::onOpenFaceResults() {
  return;
  // if (people.size() == 0) return;

  // if (openFace->isSetup) {
  //   openFace->updateFaces(faces);
  //   openFace->updateImage(pRGB);
  //   cv::Mat mat;
  //   cv::cvtColor(ofxCv::toCv(pRGB), mat, CV_BGRA2RGB);
  //   // openFace->drawTo(mat);
  //   ofxCv::toOf(mat, pRGB);
  // }
}
