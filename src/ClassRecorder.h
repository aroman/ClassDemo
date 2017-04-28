#pragma once

#include "ClassComponent.h"
#include "ofImage.h"

class ClassRecorder : public ClassComponent {

public:
  ClassRecorder();
  ~ClassRecorder();

  void update();
  void draw();

private:
  ofDirectory *colorDirectory;
  ofDirectory *depthDirectory;
  ofFloatImage depthImage;
  ofImage colorImage;

  void readDepthFile(std::string path);

};
