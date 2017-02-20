//
//  FaceDetector.cpp
//  ClassDemo
//
//  Created by Avi Romanoff on 11/12/16.
//
//

#include "FaceDetector.hpp"

FaceDetector::FaceDetector() {
  detector = new MtcnnDetector();
  isImageDirty = false;
}

void FaceDetector::updateImage(cv::Mat newImage) {
    ofLog() << "updateImage";
    newImage.copyTo(image);
    isImageDirty = true;
}

void FaceDetector::threadedFunction() {

    while(isThreadRunning()) {
        if (isImageDirty) {
            mutex.lock();
            detectedFaces = detector->detectFaces(image);
            mutex.unlock();
        }
    }

//    // Keep only non overlapping detections
//    NonOverlapingDetections(models, faces_detected);

}
