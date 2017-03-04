#pragma once

#include "ofMain.h"
#include "ClassVisualizer.h"

class ofApp : public ofBaseApp {

private:
  ClassVisualizer *visualizer;
  void updateKinect();
  void detectLandmarks();
  void detectMxnet();

public:
  void setup();
  void update();
  void draw();
  ~ofApp();

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y );
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void mouseEntered(int x, int y);
  void mouseExited(int x, int y);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);

};
