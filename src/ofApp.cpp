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
void ofApp::keyPressed(int key) {
  if (visualizer == NULL) return;
  if (key == 32) { // spacebar
    visualizer->mode = VisualizerMode::BIRDSEYE;
  }
}

void ofApp::keyReleased(int key) {
  if (visualizer == NULL) return;
  if (key == 32) { // spacebar
    visualizer->mode = VisualizerMode::FRONTAL;
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
