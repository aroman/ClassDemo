#include "ClassComponent.h"

ClassComponent::ClassComponent() {
  kinect = new KinectHelper();

  bool didConnectSuccessfully = kinect->connect();
  if (!didConnectSuccessfully) {
      std::exit(1);
  }
}


ClassComponent::~ClassComponent() {
  kinect->waitForThread(true);
  kinect->disconnect();
}
