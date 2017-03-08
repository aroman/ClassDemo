#include "ClassVisualizer.h"
#include "ofxTimeMeasurements.h"

ClassVisualizer::ClassVisualizer() {
  hasData = false;

  //load imagery
    //singleHand.load("images/logo_hand.png");
    fullLogo.load("images/logo_wide.png");

  //load font
    demoFont.load("/opt/sensei/Hack-Regular.ttf", 20, true);
    helpFont.load("/opt/sensei/Hack-Regular.ttf", 16, true);
    lineSpace = ((int)(demoFont.getLineHeight()*1.25));

  //load texts
    helpText = "spacebar   - toggles between top and front\n"
    "H or h     - enables/disables hand view\n"
    "C or c     - enables/disables cube view\n"
    "L or l     - enables/disable landmarks\n"
    "D or d     - resets to default settings\n"
    "                x DON'T show hands\n"
    "                x DON'T show cube\n"
    "                x DON'T show landmarks\n"
    "                o show rgb instead of depth\n"
    "                x DON'T show person info boxes\n"
    "B or b     - manually sets to 'birdseye' view\n"
    "X or x     - toggles between depth and rgb view\n"
    "0          - show load/help screen\n";

    aboutText.push_back("v0.1");
    aboutText.push_back("Carnegie Mellon University");
    aboutText.push_back("Global Education & Skills Forum 2017");
  
  //set colors
    lightBlue = ofColor(28,154,255);

  // kinect = new KinectHelper();

  // bool didConnectSuccessfully = kinect->connect();
  // if (!didConnectSuccessfully) {
  //     std::exit(1);
  // }

  // openFace = new OpenFace(kinect->fx, kinect->fy, kinect->cx, kinect->cy);
  // openFace->doSetup();
  // openFace->startThread(true);

  // faceDetector = new FaceDetector();
  // faceDetector->startThread(true);
  // // Might help performance a bit, we don't want it stealing CPU time
  // // from the main/GL/draw thread!
  // (&faceDetector->getPocoThread())->setPriority(Poco::Thread::PRIO_LOWEST);
  // ofAddListener(
  //   faceDetector->onDetectionResults,
  //   this,
  //   &ClassVisualizer::onFaceDetectionResults
  // );
}

ClassVisualizer::~ClassVisualizer() {
//   people.clear();

//   faceDetector->waitForThread(true);
//   delete faceDetector;

//   kinect->waitForThread(true);
//   kinect->disconnect();

//   openFace->waitForThread(true);
//   delete openFace;
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
  if (!hasData){
    drawLoadScreen();
    drawInfoPanel();
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
  // Draw OpenFace
  TS_START("[OpenFace] draw");
  ofPixels colorForMat;
  colorForMat.setFromPixels(colorPixels.getData(), colorPixels.getWidth(), colorPixels.getHeight(), OF_IMAGE_COLOR_ALPHA);
  colorForMat.swapRgb();
  cv::Mat mat = ofxCv::toCv(colorForMat);
  openFace->drawTo(mat);
  //ofxCv::drawMat(mat, 0, 0);
  TS_STOP("[OpenFace] draw");

  // Draw people
  for (auto const &person : people) {
    person.drawFrontalView();
  }

  colorTexture.draw(0, 0);
}

void ClassVisualizer::drawBirdseyeView() {
  for (auto const &person : people) {
    person.drawBirdseyeView();
  }

  ofDrawBitmapStringHighlight("back", 540, 15);
  ofDrawBitmapStringHighlight("front", 540, 1065);
}


void ClassVisualizer::drawTextboxCentered(string s, int xc, int yc, ofColor boxColor, ofColor textColor){
  //do some voodoo
  int sw = (demoFont.stringWidth(s));
  int sh = (demoFont.stringHeight(s));

  int sw1 = sw * 1.025;
  int sh1 = sh * 1.325;

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
  demoFont.drawString(s, x2, y2 + (sh2));}

void ClassVisualizer::drawTextboxTopLeft(string s, int xl, int yl, ofColor boxColor, ofColor textColor){
  //do some voodoo
  int sw = (demoFont.stringWidth(s));
  int sh = (demoFont.stringHeight(s));

  int sw1 = sw * 1.025;
  int sh1 = sh * 1.325;

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
  demoFont.drawString(s, x2, y2 + (sh2));}

void ClassVisualizer::drawTextboxTopRight(string s, int xr, int yr, ofColor boxColor, ofColor textColor){

  //do some voodoo
  int sw = (demoFont.stringWidth(s));
  int sh = (demoFont.stringHeight(s));

  int sw1 = sw * 1.025;
  int sh1 = sh * 1.325;

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
  demoFont.drawString(s, x2, y2 + (sh2));}

void ClassVisualizer::drawTextboxBottomLeft(string s, int xl, int yl, ofColor boxColor, ofColor textColor){

  //do some voodoo
  int sw = (demoFont.stringWidth(s));
  int sh = (demoFont.stringHeight(s));

  int sw1 = sw * 1.025;
  int sh1 = sh * 1.325;

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
  demoFont.drawString(s, x2, y2 + (sh2));}

void ClassVisualizer::drawTextboxBottomRight(string s, int xr, int yr, ofColor boxColor, ofColor textColor){

  //do some voodoo
  int sw = (demoFont.stringWidth(s));
  int sh = (demoFont.stringHeight(s));

  int sw1 = sw * 1.025;
  int sh1 = sh * 1.325;

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
  demoFont.drawString(s, x2, y2 + (sh2));}

void ClassVisualizer::drawLoadScreen(){
  //ofEnableAlphaBlending();
  //ofDisableAlphaBlending(); 

  ofColor red;
  red.r=255;
  red.g=0;
  red.b=0;
  ofSetColor(red);

  int trim = 100;

  ofSetColor(ofColor::black);
  ofFill();
  ofDrawRectangle(0,0,screenWidth,screenHeight);


  int xc = screenWidth /2;
  int yc = screenHeight /2;

  demoFont.load("/opt/sensei/Hack-Regular.ttf", 40, true);
  drawTextboxCentered(aboutText[1], xc, yc, ofColor::red, ofColor::white);
  drawTextboxTopLeft(aboutText[1], 0, 0, ofColor::red, ofColor::white);
  drawTextboxTopRight(aboutText[1], 1920, 0, ofColor::red, ofColor::white);
  drawTextboxBottomLeft(aboutText[1], 0, 1080, ofColor::red, ofColor::white);
  drawTextboxBottomRight(aboutText[1], 1920, 1080, ofColor::red, ofColor::white);

  /*
  demoFont.load("/opt/sensei/Hack-Regular.ttf", 20, true);
  drawTextboxCentered(aboutText[1], xc, yc, ofColor::blue, ofColor::white);
  drawTextboxTopLeft(aboutText[1], 0, 0, ofColor::blue, ofColor::white);
  drawTextboxTopRight(aboutText[1], 1920, 0, ofColor::blue, ofColor::white);
  drawTextboxBottomLeft(aboutText[1], 0, 1080, ofColor::blue, ofColor::white);
  drawTextboxBottomRight(aboutText[1], 1920, 1080, ofColor::blue, ofColor::white);
  */

  ofSetColor(ofColor::red);
  ofDrawRectangle(xc-1,yc-1,3,3);

  /*
  //draw background
    ofSetColor(28,154,255); //sensei light blue
    ofFill();
    ofDrawRectangle(0,0,screenWidth, screenHeight);

    ofSetColor(255,255,255); //white
    ofFill();
    ofDrawRectangle(0,trim,screenWidth, screenHeight-(2*trim));
  
  //draw main logo
    int logoWidth = 800;
    int logoHeight = 277;

    int x = 100;
    int y = (h/2) - (logoHeight/2);

    fullLogo.draw(x,y,logoWidth,logoHeight);

  //draw text about program version
    x = 50;
    y = 50;

    ofSetColor(255,255,255);
    demoFont.drawString(aboutText[0], x, y);
  

  //draw help text
    x = 1100;
    y = 400;

    ofSetColor(150, 150, 150);
    ofDrawRectangle(x-15, y - 20, 1, 330);
    helpFont.drawString(helpText, x, y);
  */ 
}

void ClassVisualizer::drawInfoPanel() {

  ofSetColor(255,255,255,50); //white
  ofFill();
  int imageHeight = 1080;
  int boxHeight = 100;
  int boxWidth = 350;

  string numStudents = "Number of Students: " + ofToString(people.size());
  boxWidth = (int)demoFont.stringWidth(numStudents);
  
  int padding = 20;
  int x = padding;
  int y = imageHeight - padding - boxHeight;
  ofDrawRectangle(x, y, boxWidth, boxHeight);

  x += 15;
  y += lineSpace;

  ofSetColor(0,0,0,100);

  y += lineSpace;
  if(mode == ViewAngle::FRONTAL){
    demoFont.drawString("Mode: Front View", x, y);
  }
  else{
    demoFont.drawString("Mode: Top View", x, y);
  }
  //ofDrawBitmapStringHighlight("Number of Students: " + ofToString(people.size()), 50, 900);
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
