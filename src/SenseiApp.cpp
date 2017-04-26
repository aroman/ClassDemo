#include "SenseiApp.h"

#include <math.h>
#include "ClassVisualizer.h"
#include "ClassRecorder.h"
#include "ofAppRunner.h"
#include "ofxCv/Utilities.h"

void SenseiApp::setup() {
  // Parse environment variable and set app mode
  const char *senseiModeEnv = std::getenv("SENSEI_MODE");
  if (senseiModeEnv != NULL) {
    std::string senseiMode(senseiModeEnv);
    if (senseiMode == "record") {
      mode = SenseiAppMode::RECORD;
    }
  }

  if (mode == SenseiAppMode::RECORD) {
    component = new ClassRecorder();
  } else {
    component = new ClassVisualizer();
  }
}

void SenseiApp::update() {
  component->update();
  std::stringstream strm;
  strm << "(" << floor(ofGetFrameRate()) << " FPS)";
  ofSetWindowTitle(strm.str());
}

void SenseiApp::draw() {
  ofClear(0);
  component->draw();
}

SenseiApp::~SenseiApp() {
  ofLogNotice("SenseiApp", "Shutting down...");
  if (component != NULL) {
    delete component;
  }
}

// SenseiApp I/O

//use keyReleased instead of keyPressed
//because it will only register once
//otherwise it'll flip as long as it's pressed down
void SenseiApp::keyReleased(int key) {
  if (mode != SenseiAppMode::VISUALIZE) {
    return;
  }

  ClassVisualizer *visualizer = static_cast<ClassVisualizer *>(component);

  if (visualizer == NULL) {
    return;
  }

  //don't use case 27: //escape
  //  escape will close the app!
  switch (key) {
    case 86: //V - same as v
    case 118: //v - toggles view
    case 32: //spacebar - toggles between two views
      //switch from one mode to the other
      if (visualizer->mode == ViewAngle::FRONTAL){
          visualizer->mode = ViewAngle::BIRDSEYE;
      }
      else visualizer->mode = ViewAngle::FRONTAL;
      break;
    case 72: //H - same as h
    case 104: //h - enables/disables hand view
      //cout << "---------------toggling hands" << endl;
      visualizer->showHands = !visualizer->showHands;
      break;
    case 67: //C - same as c
    case 99: //c - enables/disable cube view
      //cout << "---------------toggling cube" << endl;
      visualizer->showPose = !visualizer->showPose;
      break;
    case 76: //L - same as l
    case 108: //l - enables/disable landmarks
      //cout << "---------------toggling landmarks" << endl;
      visualizer->showLandmarks = !visualizer->showLandmarks;
      break;
    case 68: //D - same as d
    case 100: //d - enables debug view settings
      //cout << "---------------sets to debug view" << endl;
      visualizer->showDebug = !visualizer->showDebug;
      break;

    case 73: //I - same as i
    case 105: //i - sets person info
      //cout << "---------------set to personInfo" << endl;
      visualizer->showPersonInfo = !visualizer->showPersonInfo;
      break;

    case 88: //X - same as x
    case 120: //x - enables x-ray view
      //cout << "---------------toggling depth view" << endl;
      visualizer->showDepth = !visualizer->showDepth;
      break;
    case 48: //0 - show load screen
      //cout << "---------------show load screen" << endl;
      visualizer->showLoadScreen = !visualizer->showLoadScreen;
      visualizer->showHands = false;
      visualizer->showPose = false;
      visualizer->showLandmarks = false;
      visualizer->showDepth = false;
      visualizer->showPersonInfo = false;
      visualizer->showInfoPanel = !visualizer->showLoadScreen;
      visualizer->showDebug = false;
      break;
    case 49: //1, bounding box
      visualizer->showLoadScreen = false;
      visualizer->showInfoPanel = true;
      visualizer->showDebug = false;
      visualizer->showPose = false;
      visualizer->showPersonInfo = false;
      visualizer->showLandmarks = false;
      visualizer->showHands = false;
      visualizer->showDepth = false;
      visualizer->mode = ViewAngle::FRONTAL;
      break;
    case 50: //2, cube
      visualizer->showLoadScreen = false;
      visualizer->showInfoPanel = true;
      visualizer->showDebug = false;
      visualizer->showPose = true;
      visualizer->showPersonInfo = false;
      visualizer->showLandmarks = false;
      visualizer->showHands = false;
      visualizer->showDepth = false;
      visualizer->mode = ViewAngle::FRONTAL;
      break;
    case 51: //3, info per person
      visualizer->showLoadScreen = false;
      visualizer->showInfoPanel = true;
      visualizer->showDebug = false;
      visualizer->showPose = true;
      visualizer->showPersonInfo = true;
      visualizer->showLandmarks = false;
      visualizer->showHands = false;
      visualizer->showDepth = false;
      visualizer->mode = ViewAngle::FRONTAL;
      break;
    case 52: //4, landmarks
      visualizer->showLoadScreen = false;
      visualizer->showInfoPanel = true;
      visualizer->showDebug = false;
      visualizer->showPose = true;
      visualizer->showPersonInfo = true;
      visualizer->showLandmarks = true;
      visualizer->showHands = false;
      visualizer->showDepth = false;
      visualizer->mode = ViewAngle::FRONTAL;
      break;
    case 53: //5, hand detector
      visualizer->showLoadScreen = false;
      visualizer->showInfoPanel = true;
      visualizer->showDebug = false;
      visualizer->showPose = true;
      visualizer->showPersonInfo = true;
      visualizer->showLandmarks = true;
      visualizer->showHands = true;
      visualizer->showDepth = false;
      visualizer->mode = ViewAngle::FRONTAL;
      break;
    case 54: //6, depth
      visualizer->showLoadScreen = false;
      visualizer->showInfoPanel = true;
      visualizer->showDebug = false;
      visualizer->showPose = true;
      visualizer->showPersonInfo = true;
      visualizer->showLandmarks = true;
      visualizer->showHands = true;
      visualizer->showDepth = true;
      visualizer->mode = ViewAngle::FRONTAL;
      break;
    case 55: //7, top view
      visualizer->showLoadScreen = false;
      visualizer->showInfoPanel = true;
      visualizer->showDebug = false;
      visualizer->showPose = false;
      visualizer->showPersonInfo = true;
      visualizer->showLandmarks = false;
      visualizer->showHands = true;
      visualizer->showDepth = false;
      visualizer->mode = ViewAngle::BIRDSEYE;
      break;
    case 90: //Z - same as z
    case 122: //z - toggles full screen mode
      isFullScreen = !isFullScreen;
      ofSetFullscreen(isFullScreen);
      break;

  }
}

void SenseiApp::keyPressed(int key) {}
void SenseiApp::mouseMoved(int x, int y) {}
void SenseiApp::mouseDragged(int x, int y, int button) {}
void SenseiApp::mousePressed(int x, int y, int button) {}
void SenseiApp::mouseReleased(int x, int y, int button) {}
void SenseiApp::mouseEntered(int x, int y) {}
void SenseiApp::mouseExited(int x, int y) {}
void SenseiApp::windowResized(int w, int h) {}
void SenseiApp::gotMessage(ofMessage msg) {}
void SenseiApp::dragEvent(ofDragInfo dragInfo) {}
