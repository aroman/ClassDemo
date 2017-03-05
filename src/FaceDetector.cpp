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
  while (isThreadRunning()) {
    if (!isImageDirty) {
      yield();
      continue;
    }

    // resize the imageCopy to the size of imageScaled
    ofPixels imageScaled;
    mutex.lock();
    ofPixels imageCopy = *image;
    mutex.unlock();
    yield();

    imageScaled.allocate(
      imageCopy.getWidth() / scaleFactor,
      imageCopy.getHeight() / scaleFactor,
      imageCopy.getPixelFormat()
    );
    yield();
    imageCopy.resizeTo(imageScaled);

    cv::Mat imageScaledMat = ofxCv::toCv(imageScaled);

    // XXX: When I rewrote the kinect integration, this conversion became necessary...
    cv::cvtColor(imageScaledMat, imageScaledMat, CV_BGRA2BGR);

    yield();
    mtcnn_detect_results results;
    TS_START("detector->detectFaces");
    results = detector->detectFaces(imageScaledMat);
    TS_STOP("detector->detectFaces");
    yield();

    vector<ofRectangle> bboxes;
    for (auto &r_bbox : results.bboxes) {
      bboxes.push_back(ofRectangle(
        ofPoint(r_bbox.x1 * scaleFactor, r_bbox.y1 * scaleFactor),
        ofPoint(r_bbox.x2 * scaleFactor, r_bbox.y2 * scaleFactor)
      ));
    }
    ofNotifyEvent(onDetectionResults, bboxes);

  }
}
