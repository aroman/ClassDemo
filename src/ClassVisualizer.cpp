#include "ClassVisualizer.h"
#include "ofxTimeMeasurements.h"

#define USE_CREEPYFACES false

using namespace std;

// ClassVisualizer
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

  TS_START("getRgbPixels");
  pRGB = kinect->getRgbPixels();
  TS_STOP("getRgbPixels");
  pBigDepth = kinect->getBigDepthPixels();
  hasData = (pRGB.size() > 0);

  if (!hasData) return;

  TS_START("loadData");
  tRender.loadData(pRGB);
  TS_STOP("loadData");

  faceDetector->updateImage(&pRGB);
  // updateOpenFace();
}

void ClassVisualizer::draw() {
  if (!hasData) return;

  if (toggleView) {
    drawTopView();
  } else {
    drawFrontView();
  }
}

void ClassVisualizer::drawFrontView() {
  tRender.draw(0, 0);

  // draw people on top
  for (auto const &person : people) {
    person.drawFrontView();
  }

  ofDrawBitmapString("Front Facing View", 960, 15);

  // if (USE_CREEPYFACES) {
  //   for (int i = 0; i < faces.size(); i++) {
  //       rect r = faces[i].r;
  //
  //       ofPath p;
  //       p.setFillColor(ofColor::blue);
  //       p.rectangle(r.x,
  //                   r.y,
  //                   r.width,
  //                   r.height);
  //
  //       p.rectangle(r.x + (r.width*0.1),
  //                   r.y + (r.height*0.1),
  //                   (r.width*0.8),
  //                   (r.height*0.8));
  //
  //       p.draw();
  //
  //       ofPath p1;
  //       p1.setFillColor(ofColor(0, 255, 255, 200));
  //       p1.circle(faces[i].points[0], faces[i].points[5], r.width*0.05);
  //       p1.close();
  //       p1.circle(faces[i].points[1], faces[i].points[6], r.width*0.05);
  //       p1.close();
  //       p1.circle(faces[i].points[2], faces[i].points[7], r.width*0.05);
  //       p1.close();
  //       p1.circle(faces[i].points[3], faces[i].points[8], r.width*0.05);
  //       p1.close();
  //       p1.circle(faces[i].points[4], faces[i].points[9], r.width*0.05);
  //       p1.close();
  //       p1.draw();
  //
  //       int eye_size = 14;
  //
  //       tRGB.drawSubsection(r.x, r.y, 50, 50, faces[i].points[0] - (eye_size / 2), faces[i].points[5] - (eye_size / 2), eye_size, eye_size);
  //       tRGB.drawSubsection(r.x + (r.width/2) + (r.width/2), r.y, 50, 50, faces[i].points[1] - (eye_size / 2), faces[i].points[6] - (eye_size / 2), eye_size, eye_size);
  //       tRGB.drawSubsection(
  //         r.x + (r.width/4),
  //         r.y + (r.height/2),
  //         100,
  //         50,
  //         faces[i].points[3], // x
  //         faces[i].points[8] - (r.height *0.1), // y
  //         faces[i].points[4] - faces[i].points[3],
  //         r.height * 0.20);
  // }
  // }
}

void ClassVisualizer::drawTopView() {
  // exit if not ready to draw additional:
  // if(!doRender) return;

  // draw people
  for (int i = 0; i < people.size(); i++) {
    people[i].drawTopView();
  }

  ofDrawBitmapString("Bird's Eye View", 960, 15);
  ofDrawBitmapString("back", 10, 15);
  ofDrawBitmapString("front", 10, 1065);
}

void ClassVisualizer::onFaceDetectionResults(mtcnn_detect_results &results) {
  ofLogNotice("ClassVisualizer") << "onFaceDetectionResults" << results.bboxes.size();

  for (auto &person : people) {
    person.free();
  }
  people.clear();

  people.reserve(results.bboxes.size());

  for (auto bbox : results.bboxes) {
    Person p;
    p.f.r.x = bbox.x1;
    p.f.r.y = bbox.y1;
    p.f.r.width = abs(bbox.x2 - bbox.x1);
    p.f.r.height = abs(bbox.y2 - bbox.y1);
    people.push_back(p);
  }

  for (auto &person : people) {
    person.init(pRGB, pBigDepth);
  }
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
