#include "FaceDetector.h"
#include "ofxTimeMeasurements.h"

FaceDetector::FaceDetector() {
  detector = new MtcnnDetector();
  isImageDirty = false;
}

FaceDetector::~FaceDetector() {
  stopThread();
  if (detector != NULL) delete detector;
}

void FaceDetector::updateImage(ofPixels *newImage) {
  // ofLogNotice("FaceDetector", "updateImage");
  mutex.lock();
  image = newImage;
  mutex.unlock();
  isImageDirty = true;
}

void FaceDetector::threadedFunction() {
  while(isThreadRunning()) {
    if (!isImageDirty) {
      yield();
      continue;
    }

    // resize the imageScaled
    ofPixels imageScaled;
    mutex.lock();
    ofPixels imageCopy = *image;
    mutex.unlock();
    yield();

    imageScaled.allocate(
      imageCopy.getWidth() / DOWNSCALE_FACTOR,
      imageCopy.getHeight() / DOWNSCALE_FACTOR,
      imageCopy.getPixelFormat()
    );
    yield();
    imageCopy.resizeTo(imageScaled);

    cv::Mat imageScaledMat = ofxCv::toCv(imageScaled);

    // XXX: When I rewrote the kinect integration, this conversion became necessary...
    cv::cvtColor(imageScaledMat, imageScaledMat, CV_RGBA2RGB);

    yield();
    TS_START("detector->detectFaces");
    detectedFaces = detector->detectFaces(imageScaledMat);
    TS_STOP("detector->detectFaces");

  }
}
