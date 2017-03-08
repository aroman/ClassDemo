#include "ofApp.h"
#include <math.h>

void ofApp::setup() {
  visualizer = new ClassVisualizer();
}

void ofApp::update() {
  visualizer->update();
  std::stringstream strm;
  strm << "(" << floor(ofGetFrameRate()) << " FPS)";
  ofSetWindowTitle(strm.str());
}

void ofApp::draw() {
  ofClear(0);
  visualizer->draw();
}

ofApp::~ofApp() {
  ofLogNotice("ofApp", "Shutting down...");
  if (visualizer != NULL) {
    delete visualizer;
  }
}

// ofApp I/O

//use keyReleased instead of keyPressed
//because it will only register once
//otherwise it'll flip as long as it's pressed down
void ofApp::keyReleased(int key) {
  if (visualizer == NULL) return;

  //don't use case 27: //escape
  //  escape will close the app!
  switch(key){
    case 32: //spacebar - toggles between two views
      //switch from one mode to the other
      if(visualizer->mode == ViewAngle::FRONTAL){
          visualizer->mode = ViewAngle::BIRDSEYE;
      }
      else visualizer->mode = ViewAngle::FRONTAL;
      break;
    case 72: //H - same as h
    case 104: //h - enables/disables hand view
      cout << "---------------toggling hands" << endl;
      visualizer->showHands = !visualizer->showHands;
      break;
    case 67: //C - same as c
    case 99: //c - enables/disable cube view
      cout << "---------------toggling cube" << endl;
      visualizer->showPose = !visualizer->showPose;
      break;
    case 76: //L - same as l
    case 108: //l - enables/disable landmarks
      cout << "---------------toggling landmarks" << endl;
      visualizer->showLandmarks = !visualizer->showLandmarks;
      break;
    case 68: //D - same as d
    case 100: //d - resets to default settings
      cout << "---------------reset everything to default" << endl;
      visualizer->showHands = false;
      visualizer->showPose = false;
      visualizer->showLandmarks = false;
      visualizer->showDepth = false;
      visualizer->showPersonInfo = false;
      break;
    case 66: //B - same as b
    case 98: //b - sets to birdseye view
      cout << "---------------set to birdseye" << endl;
      visualizer->mode = ViewAngle::BIRDSEYE;
      break;
    case 88: //X - same as x
    case 120: //x - enables x-ray view
      cout << "---------------toggling depth view" << endl;
      visualizer->showDepth = !visualizer->showDepth;
      break;
    case 48: //0 - show load screen
      cout << "---------------show load screen" << endl;
      visualizer->showLoadScreen = !visualizer->showLoadScreen;
      break;
  }
}

void ofApp::keyPressed(int key) {}
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}
