#include "ClassVisualizer.h"

#include <cstdlib>
#include "ofxTimeMeasurements.h"
#include "drawUtils.h"

static const int DIST_THRESH = 150;

#define SCREEN_HEIGHT (1080)
#define SCREEN_WIDTH (1920)

ClassVisualizer::ClassVisualizer() {
  hasData = false;

  //load imagery
    //singleHand.load("images/logo_hand.png");
    fullLogo.load("images/logo_wide.png");
    cmuWordmark.load("images/cmu_wordmark.png");
    cmuWordmarkBlue.load("images/cmu_wordmark_blue.png");
    classroom.load("images/classroom.png");

  //load font

    demoFont.load("fonts/OpenSans-Regular.ttf", 24, true);
    helpFont.load("fonts/OpenSans-Regular.ttf", 20, true);
    demoBoldFont.load("fonts/OpenSans-Bold.ttf", 30, true);
    peopleFont.load("fonts/OpenSans-Bold.ttf", 20, true);

    lineSpace = ((int)(demoFont.getLineHeight()*1.25));

  //load texts
    helpText = R"([spacebar] birdseye/frontal view toggle
[I or i] toggle person info boxes
[H or h] hand detection
[C or c] cube / box view
[L or l] face landmarks
[V or v] between top and front view
[X or x] between depth and rgb view
[D or d] all debug features on
[Z or z] toggle full screen view
[0] show load/help screen, resets view
[1-7] presentation modes
[PG DWN] toggles timer screen)";

    aboutText.push_back("v0.1");
    aboutText.push_back("Carnegie Mellon University");
    aboutText.push_back("Global Education & Skills Forum 2017");

  //set colors
    lightBlue = ofColor(28,154,255);

  const char *numModelsEnv = std::getenv("NUM_MODELS");
  if (numModelsEnv != NULL) {
    int numModels = atoi(numModelsEnv);
    if (numModels >= 0) {
      ofLogNotice("ClassVisualizer") << "Using " << numModels << " models (env)";
      openFaceModelPoolSize = numModels;
    }
  }

  openFaceModelPool = new OpenFaceModelPool(openFaceModelPoolSize, CameraIntrinsics{
    this->kinect->fx,
    this->kinect->fy,
    this->kinect->cx,
    this->kinect->cy
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

  delete openFaceModelPool;
}

void ClassVisualizer::update() {
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

  faceDetector->updateImage(colorPixels);

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
  if (!hasData){
    drawLoadScreen();
    return;
  }

  if(showLoadScreen){
    drawLoadScreen();
  }
  else{
    if (mode == ViewAngle::BIRDSEYE) {
      drawBirdseyeView();
    } else {
      drawFrontalView();
    }
    if(showInfoPanel){
      drawInfoPanel();
    }
  }
}

void ClassVisualizer::drawFrontalView() {

  ofSetColor(255,255,255);
  ofDrawRectangle(0,0,1920,1080);

  if(showDepth){
    ofFloatPixels copy = depthPixels;

    ofTexture scaledDepthTexture;
    ofFloatPixels scaledDepth;
    scaledDepth.setFromPixels(reinterpret_cast<float *>(copy.getData()),copy.getWidth(),copy.getHeight(), OF_PIXELS_GRAY);

    scaleDepthPixelsForDrawing(&scaledDepth);
    scaledDepthTexture.loadData(scaledDepth);
    scaledDepthTexture.draw(0,0);
  }else{
    colorTexture.draw(0, 0);
  }

  // Draw people
  for (auto const &person : people) {
    if(showDebug){
      person.drawFrontDepth();
      person.drawFrontHandbox(ofColor::red);
      person.drawFrontPose(ofColor(28,154,255));
      person.drawFrontBBox(ofColor::orange);

      person.drawFrontLandmarks(ofColor(255,255,255,160));
      person.drawFrontDepthPoints(ofColor::white);

      person.drawFrontPersonInfo(peopleFont);

    } else{
      if(showHands){
        person.drawFrontHandbox(ofColor::red);
      }
      if(showPose){
        person.drawFrontPose(ofColor(28,154,255));
      }
      else{
        person.drawFrontBBox(ofColor::orange);
      }
      if(showLandmarks){
        person.drawFrontLandmarks(ofColor(255,255,255,160));
      }
      if(showPersonInfo){
        person.drawFrontPersonInfo(peopleFont);
      }
    }
  }
}

void ClassVisualizer::drawBirdseyeView() {

  ofSetColor(255,255,255);
  ofDrawRectangle(0,0,1920,1080);

  classroom.draw(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);

  for (auto const &person : people) {
    if(showDebug){
      person.drawTopColor();
      person.drawTopHandbox(ofColor::black);
      //person.drawTopLandmarks(ofColor::red);
      person.drawTopPersonInfo(peopleFont);
    } else{
      if(person.openFaceModel != nullptr){
        person.drawTopColor();

        if(showHands){
          person.drawTopHandbox(ofColor::black);
        }
        /*
        if(showLandmarks){
          person.drawTopLandmarks(ofColor::red);
        }
        */

        if(showPersonInfo){
          person.drawTopPersonInfo(peopleFont);
        }
      }
    }
  }

  //drawStringTopLeft(demoFont,"back",540,15,ofColor(0,0,0,0),ofColor::white);
  //drawStringTopLeft(demoFont,"back",540,1065,ofColor(0,0,0,0),ofColor::white);
}

void ClassVisualizer::drawLoadScreen(){
  int x, y;

  //draw background
    int trim = 100;
    ofSetColor(28,154,255); //sensei light blue
    ofFill();
    ofDrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);

    ofSetColor(255,255,255); //white
    ofFill();
    ofDrawRectangle(0,trim,SCREEN_WIDTH, SCREEN_HEIGHT-(2*trim));

  //draw main logo
    int logoWidth = 800;
    int logoHeight = 277;
    int wordMarkHeight = 40 * 1.25;
    int wordMarkWidth = 450 * 1.25;

    x = 300;
    y = (SCREEN_HEIGHT/2) - ((logoHeight + wordMarkHeight) /2) - 50 ;

    fullLogo.draw(x, y, logoWidth, logoHeight);
    cmuWordmarkBlue.draw(x + ((logoWidth - wordMarkWidth)/2), y + logoHeight + (wordMarkHeight / 2), wordMarkWidth, wordMarkHeight);

  //draw text about program version
    x = 20;
    y = 30;

    // drawStringTopRight(demoBoldFont, aboutText[0], 50, 120, ofColor(0,0,0,0), ofColor::white);

  //draw help text
    x = 1200;
    y = 380;

    ofSetColor(150, 150, 150);
    ofDrawRectangle(x-30, y , 1, 350);
    helpFont.drawString(helpText, x, y);
  }

void ClassVisualizer::drawInfoPanel() {
  int height = 180;
  int radius = 15;
  int width = 400;
  int x = SCREEN_WIDTH - width -(radius);
  int y = SCREEN_HEIGHT - height + (radius);

  ofColor backgroundColor = ofColor(255, 255, 255, 255);

  ofFill();
  ofSetColor(ofColor(255, 255, 255, 140));
  ofDrawRectRounded(x, y, width, height, radius);
  ofNoFill();

  x += radius*1.5;

  y = SCREEN_HEIGHT - height + radius + 12;

  if (mode == ViewAngle::FRONTAL) {
    drawStringTopLeft(demoFont,"Mode: Front View", x, y, ofColor(0,0,0,0), ofColor::black);
  }
  else {
    drawStringTopLeft(demoFont,"Mode: Top View", x, y, ofColor(0,0,0,0), ofColor::black);
  }

  y += 45;

  string numStudents = "Number of Students: " + ofToString(people.size());
  drawStringTopLeft(demoFont, numStudents, x, y, ofColor(0,0,0,0), ofColor::black);

  int numHandsRaised = 0;
  for (auto const &person : people) {
    if(person.isRaisingHand){
      numHandsRaised++;
    }
  }

  y += 40;

  string handsRaised  = "Number of questions: " + ofToString(numHandsRaised);
  drawStringTopLeft(demoFont, handsRaised , x, y, ofColor(0,0,0,0), ofColor::black);

}

void ClassVisualizer::onFaceDetectionResults(const vector<ofRectangle> &bboxes) {
  ofLogNotice("ClassVisualizer") << "onFaceDetectionResults " << bboxes.size();

  peopleAccessMutex.lock();

  for (int i = 0; i < people.size(); i++) {
    people.at(i).isConfirmed = false;
  }

  for (auto bbox : bboxes) {
    int closestPersonIndex = -1;

    float closestDistance = DIST_THRESH;
    for (int i = 0; i < people.size(); i++) {
      float dist = people.at(i).currentBoundingBox().getCenter().distance(bbox.getCenter());
      if (dist < closestDistance) {
        closestPersonIndex = i;
        closestDistance = dist;
      }
    }

    ofLogNotice("ClassVisualizer") << "closest distance: " << closestDistance;
    if (closestDistance < DIST_THRESH) {
      assert (closestPersonIndex != -1);
      people.at(closestPersonIndex).updateMtcnnBoundingBox(bbox);
    } else {
      people.push_back(Person(bbox));
    }
  }

  for (int i = 0; i < people.size(); i++) {
    if (!people.at(i).isConfirmed) {
      openFaceModelPool->returnModel(people.at(i).openFaceModel);
      people.erase(people.begin() + i);
    }
  }

  for (auto &person : people) {
    ofLogNotice("ClassVisualizer") << person;
  }

  peopleAccessMutex.unlock();

}
