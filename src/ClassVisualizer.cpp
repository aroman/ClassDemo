#include "ClassVisualizer.h"
#include "ofxTimeMeasurements.h"

static const int DIST_THRESH = 150;

ClassVisualizer::ClassVisualizer() {
  hasData = false;

  //load imagery
    //singleHand.load("images/logo_hand.png");
    fullLogo.load("images/logo_wide.png");

  //load font

    demoFont.load("fonts/OpenSans-Regular.ttf", 20, true);
    helpFont.load("fonts/OpenSans-Regular.ttf", 16, true);
    peopleFont.load("fonts/OpenSans-Regular.ttf", 12, true);

    lineSpace = ((int)(demoFont.getLineHeight()*1.25));

  //load texts
    helpText = "spacebar   - toggles between top and front\n"
    "I or i     - enables/disables person info boxes\n"
    "H or h     - enables/disables hand view\n"
    "C or c     - enables/disables cube view\n"
    "L or l     - enables/disable landmarks\n"
    "V or v     - toggles between top and front view\n"
    "X or x     - toggles between depth and rgb view\n"
    "D or d     - toggles debug view\n"
    "0          - show load/help screen, resets view\n"
    "PG DWN     - toggles timer screen\n";

    aboutText.push_back("v0.1");
    aboutText.push_back("Carnegie Mellon University");
    aboutText.push_back("Global Education & Skills Forum 2017");

  //set colors
    lightBlue = ofColor(28,154,255);

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
  colorTexture.draw(0, 0);
  // Draw people
  for (auto const &person : people) {
    if(showDebug){
      person.drawFrontDepth();
      person.drawFrontHandbox(ofColor::red);
      person.drawFrontPose(ofColor::yellow);
      person.drawFrontBBox(ofColor::orange);

      person.drawFrontLandmarks(ofColor::red);
      person.drawFrontDepthPoints(ofColor::white);


      person.drawFrontPersonInfo(peopleFont);

    } else{
      if(showDepth){
        person.drawFrontDepth();
      }
      if(showHands){
        person.drawFrontHandbox(ofColor::red);
      }
      if(showPose){
        person.drawFrontPose(ofColor::yellow);
      }
      else{
        person.drawFrontBBox(ofColor::orange);
      }
      if(showLandmarks){
        person.drawFrontLandmarks(ofColor::red);
      }
      if(showPersonInfo){
        person.drawFrontPersonInfo(peopleFont);
      }
    }
  }
}

void ClassVisualizer::drawBirdseyeView() {
  depthTexture.draw(0, 0);
  for (auto const &person : people) {
    if(showDebug){
      person.drawTopColor();
      person.drawTopHandbox(ofColor::black);
      person.drawTopLandmarks(ofColor::red);
      person.drawTopPersonInfo(peopleFont);
    } else{
      if(showDepth){
        person.drawTopColor();
      }
      if(showHands){
        person.drawTopHandbox(ofColor::black);
      }
      if(showLandmarks){
        person.drawTopLandmarks(ofColor::red);
      }
      if(showPersonInfo){
        person.drawTopPersonInfo(peopleFont);
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
    ofDrawRectangle(0,0,screenWidth, screenHeight);

    ofSetColor(255,255,255); //white
    ofFill();
    ofDrawRectangle(0,trim,screenWidth, screenHeight-(2*trim));

  //draw main logo
    int logoWidth = 800;
    int logoHeight = 277;

    x = 100;
    y = (screenHeight/2) - (logoHeight/2);

    fullLogo.draw(x,y,logoWidth,logoHeight);

  //draw text about program version
    x = 20;
    y = 30;

    drawStringTopLeft(demoFont, aboutText[0], x, y,ofColor(0,0,0,0),ofColor(255,255,255,100));

  //draw help text
    x = 1100;
    y = 400;

    //ofSetColor(150, 150, 150);
    //ofDrawRectangle(x-15, y - 20, 1, 330);
    //helpFont.drawString(helpText, x, y);
    drawStringCentered(helpFont,helpText,1100,400,ofColor(0,0,0,0),ofColor(150,150,150));
  }

void ClassVisualizer::drawInfoPanel() {
  int x = 50;
  int y = screenHeight - 50;

  if(mode == ViewAngle::FRONTAL){
    drawStringBottomLeft(demoFont,"Mode: Front View", x, y, ofColor(255,255,255,100), ofColor::white);
  }
  else{
    drawStringBottomLeft(demoFont,"Mode: Top View", x, y, ofColor(255,255,255,100), ofColor::white);
  }

  y += 40;

  string numStudents = "Number of Students: " + ofToString(people.size());
  drawStringBottomLeft(demoFont, numStudents, x, y, ofColor(255,255,255,100), ofColor::white);

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


void ClassVisualizer::drawStringCentered(ofTrueTypeFont font, string s, int xc, int yc, ofColor boxColor, ofColor textColor){
  //do some voodoo
  int sw = (font.stringWidth(s));
  int sh = (font.stringHeight(s));

  int sw1 = sw * 1.05;
  int sh1 = sh * 1.8;

  int x1 = xc - (sw1/2);
  int y1 = yc - (sh1/2);

  ofSetColor(boxColor);
  ofFill();
  ofDrawRectangle(x1,y1,sw1,sh1);

  int sw2 = sw;
  int sh2 = sh;

  int x2 = xc - (sw2/2);
  int y2 = yc - ((sh2*1.3)/2);

  ofSetColor(textColor);
  font.drawString(s, x2, y2 + (sh2));}

void ClassVisualizer::drawStringTopLeft(ofTrueTypeFont font, string s, int xl, int yl, ofColor boxColor, ofColor textColor){
  //do some voodoo
  int sw = (font.stringWidth(s));
  int sh = (font.stringHeight(s));

  int sw1 = sw * 1.05;
  int sh1 = sh * 1.8;

  int x1 = xl;
  int y1 = yl;

  ofSetColor(boxColor);
  ofFill();
  ofDrawRectangle(x1,y1,sw1,sh1);

  int sw2 = sw;
  int sh2 = sh;

  int xc = x1 + (sw1/2);
  int yc = y1 + (sh1/2);

  int x2 = xc - (sw2/2);
  int y2 = yc - ((sh2*1.3)/2);

  ofSetColor(textColor);
  font.drawString(s, x2, y2 + (sh2));}

void ClassVisualizer::drawStringTopRight(ofTrueTypeFont font, string s, int xr, int yr, ofColor boxColor, ofColor textColor){

  //do some voodoo
  int sw = (font.stringWidth(s));
  int sh = (font.stringHeight(s));

  int sw1 = sw * 1.05;
  int sh1 = sh * 1.8;

  xr -= sw1;

  int x1 = xr;
  int y1 = yr;

  ofSetColor(boxColor);
  ofFill();
  ofDrawRectangle(x1,y1,sw1,sh1);

  int sw2 = sw;
  int sh2 = sh;

  int xc = x1 + (sw1/2);
  int yc = y1 + (sh1/2);

  int x2 = xc - (sw2/2);
  int y2 = yc - ((sh2*1.3)/2);

  ofSetColor(textColor);
  font.drawString(s, x2, y2 + (sh2));}

void ClassVisualizer::drawStringBottomLeft(ofTrueTypeFont font, string s, int xl, int yl, ofColor boxColor, ofColor textColor){

  //do some voodoo
  int sw = (font.stringWidth(s));
  int sh = (font.stringHeight(s));

  int sw1 = sw * 1.05;
  int sh1 = sh * 1.8;

  yl -= sh1;

  int x1 = xl;
  int y1 = yl;

  ofSetColor(boxColor);
  ofFill();
  ofDrawRectangle(x1,y1,sw1,sh1);

  int sw2 = sw;
  int sh2 = sh;

  int xc = x1 + (sw1/2);
  int yc = y1 + (sh1/2);

  int x2 = xc - (sw2/2);
  int y2 = yc - ((sh2*1.3)/2);

  ofSetColor(textColor);
  font.drawString(s, x2, y2 + (sh2));}

void ClassVisualizer::drawStringBottomRight(ofTrueTypeFont font, string s, int xr, int yr, ofColor boxColor, ofColor textColor){

  //do some voodoo
  int sw = (font.stringWidth(s));
  int sh = (font.stringHeight(s));

  int sw1 = sw * 1.05;
  int sh1 = sh * 1.8;

  xr -= sw1;
  yr -= sh1;

  int x1 = xr;
  int y1 = yr;

  ofSetColor(boxColor);
  ofFill();
  ofDrawRectangle(x1,y1,sw1,sh1);

  int sw2 = sw;
  int sh2 = sh;

  int xc = x1 + (sw1/2);
  int yc = y1 + (sh1/2);

  int x2 = xc - (sw2/2);
  int y2 = yc - ((sh2*1.3)/2);

  ofSetColor(textColor);
  font.drawString(s, x2, y2 + (sh2));}
