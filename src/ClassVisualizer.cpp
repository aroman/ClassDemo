#include "ClassVisualizer.h"
#include "ofxTimeMeasurements.h"

ClassVisualizer::ClassVisualizer() {
  hasData = false;

  kinect = new KinectHelper();

  bool didConnectSuccessfully = kinect->connect();
  if (!didConnectSuccessfully) {
      std::exit(1);
  }

  openFaceModelPool = new OpenFaceModelPool(openFaceModelPoolSize, CameraIntrinsics{
    kinect->fx,
    kinect->fy,
    kinect->cx,
    kinect->cy
  });

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

  delete openFaceModelPool;
}

void ClassVisualizer::update() {
  if (!kinect->isConnected) return;

  TS_START("[Kinect] update frames");
  colorPixels = kinect->getColorPixels();
  depthPixels = kinect->getAlignedDepthPixels();
  TS_STOP("[Kinect] update frames");
  hasData = (colorPixels.size() > 0);

  if (!hasData) return;

  TS_START("update color texture");
  colorTexture.loadData(colorPixels);
  TS_STOP("update color texture");

  faceDetector->updateImage(&colorPixels);

  peopleAccessMutex.lock();
  for (auto &person : people) {
    person.update(colorPixels, depthPixels);

    // no active OpenFace model associated with this person
    if (person.openFaceModel == nullptr) {
      auto model = openFaceModelPool->getModel();
      if (model == nullptr) {
        ofLogWarning("Could not get model from pool!");
        continue;
      }
      ofLogNotice("update get new model: ") << *model;
      ofRectangle biasedAdjustedBBox = person.mtcnnBoundingBox;
      biasedAdjustedBBox.x += (biasedAdjustedBBox.width * -0.0075);
      biasedAdjustedBBox.y += (biasedAdjustedBBox.height * 0.2459);
      biasedAdjustedBBox.width *= 1.0323;
      biasedAdjustedBBox.height *= 0.7751;
      bool initSuccess = model->initializeTracking(colorPixels, biasedAdjustedBBox);
      if (initSuccess) {
        person.openFaceModel = model;
      } else {
        openFaceModelPool->returnModel(model);
      }
    } else {
      // existing model association
      bool updateSuccess = person.openFaceModel->updateTracking(colorPixels);
      if (!updateSuccess && person.openFaceModel->model->failures_in_a_row > MAX_MODEL_FAILURES_IN_A_ROW) {
        openFaceModelPool->returnModel(person.openFaceModel);
        person.openFaceModel = nullptr;
      }
    }
  }
  peopleAccessMutex.unlock();
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
  colorTexture.draw(0, 0);

  // Draw people
  for (auto const &person : people) {
    person.drawFrontalView();
  }

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

static const int DIST_THRESH = 150;

void ClassVisualizer::onFaceDetectionResults(const vector<ofRectangle> &bboxes) {
  ofLogNotice("ClassVisualizer") << "onFaceDetectionResults " << bboxes.size();


  // TODO(avi) remove up people that mtcnn can't find anymore
  peopleAccessMutex.lock();

  for (int i = 0; i < people.size(); i++) {
    people[i].isConfirmed = false;
  }

  for (auto bbox : bboxes) {
    int closestPersonIndex = -1;

    float closestDistance = DIST_THRESH;
    for (int i = 0; i < people.size(); i++) {
      float dist = people[i].currentBoundingBox().getCenter().distance(bbox.getCenter());
      if (dist < closestDistance) {
        closestPersonIndex = i;
        closestDistance = dist;
      }
    }

    printf("closest distance: %f\n", closestDistance);
    if (closestDistance < DIST_THRESH) {
      assert (closestPersonIndex != -1);
      // printf("updating existing person at index: %d\n", closestPersonIndex);
      people[closestPersonIndex].updateMtcnnBoundingBox(bbox);
    } else {
      // printf("creating new person\n");
      people.push_back(Person(bbox));
    }
  }

  for (int i = 0; i < people.size(); i++) {
    if (!people[i].isConfirmed) {
      openFaceModelPool->returnModel(people[i].openFaceModel);
      people.erase(people.begin() + i);
    }
  }

  for (auto &person : people) {
    ofLogNotice("ClassVisualizer") << "Person: " << person;
  }

  peopleAccessMutex.unlock();

}
