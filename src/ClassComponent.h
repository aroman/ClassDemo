#pragma once

#include "KinectHelper.h" // Brings in X11, but also can't depend on X11 ()

class ClassComponent  {

public:

  ClassComponent();
  ~ClassComponent();
  virtual void update() = 0;
  virtual void draw() = 0;

protected:
  KinectHelper *kinect = nullptr;

};
