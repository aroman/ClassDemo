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

void FaceDetector::updateImage(ofPixels newImage) {
  image = newImage;
  isImageDirty = true;
}

void FaceDetector::threadedFunction() {
  while(isThreadRunning()) {
    if (!isImageDirty) {
      sleep(10);
      continue;
    }

    // mutex.lock();

    // resize the imageScaled
    ofPixels imageScaled;
    imageScaled.allocate(
      image.getWidth() / DOWNSCALE_FACTOR,
      image.getHeight() / DOWNSCALE_FACTOR,
      image.getPixelFormat()
    );
    image.resizeTo(imageScaled);

    cv::Mat imageScaledMat = ofxCv::toCv(imageScaled);

    // XXX: When I rewrote the kinect integration, this conversion became necessary...
    cv::cvtColor(imageScaledMat, imageScaledMat, CV_RGBA2RGB);

    TS_START("mtcnn detect");
    detectedFaces = detector->detectFaces(imageScaledMat);
    TS_STOP("mtcnn detect");

    // mutex.unlock();
  }
}
