#include "ofApp.h"
#include <math.h>

void ofApp::setup() {
  frame = new BufferFrame();
}

void ofApp::update() {
  frame->update();
  std::stringstream strm;
  strm << "(" << floor(ofGetFrameRate()) << " FPS)";
  ofSetWindowTitle(strm.str());
}

void ofApp::draw() {
  // ofClear(0);
  frame->draw();
}

ofApp::~ofApp() {
  ofLogNotice("ofApp", "Shutting down...");
  if (frame != NULL) {
    delete frame;
  }
}

// ofApp I/O
void ofApp::keyPressed(int key) {
  if (frame == NULL) return;
  if (key == 32) { // spacebar
    frame->toggleView = true;
  }
}

void ofApp::keyReleased(int key) {
  if (frame == NULL) return;
  if (key == 32) { // spacebar
    frame->toggleView = false;
  }
}

void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}
