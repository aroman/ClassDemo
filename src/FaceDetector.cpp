#include "FaceDetector.h"
#include "ofxTimeMeasurements.h"

FaceDetector::FaceDetector() {
  detector = new MtcnnDetector();
  isImageDirty = false;
}

FaceDetector::~FaceDetector() {
  stopThread();
  if (detector != nullptr) delete detector;
}

void FaceDetector::updateImage(ofPixels newColorPixels) {
  mutex.lock();
  colorPixels = newColorPixels;
  mutex.unlock();
  isImageDirty = true;
}

void FaceDetector::threadedFunction() {
  while (isThreadRunning()) {
    if (!isImageDirty) {
      yield();
      continue;
    }

    ofPixels scaledColorPixels;
    mutex.lock();
    ofPixels copyColorPixels = colorPixels;
    mutex.unlock();
    yield();

    // resize the copyColorPixels to the size of scaledColorPixels
    scaledColorPixels.allocate(
      copyColorPixels.getWidth() / scaleFactor,
      copyColorPixels.getHeight() / scaleFactor,
      copyColorPixels.getPixelFormat()
    );
    yield();
    copyColorPixels.resizeTo(scaledColorPixels);

    cv::Mat scaledColorMat = ofxCv::toCv(scaledColorPixels);

    // Drop the (unused) alpha channel
    cv::cvtColor(scaledColorMat, scaledColorMat, CV_BGRA2BGR);

    yield();
    mtcnn_detect_results results;
    TS_START("[MTCNN] detect faces");
    results = detector->detectFaces(scaledColorMat);
    TS_STOP("[MTCNN] detect faces");
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
