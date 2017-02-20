//
//  FaceDetector.hpp
//  ClassDemo
//
//  Created by Avi Romanoff on 11/12/16.
//
//

#include <stdio.h>
#include "ofThread.h"
#include "ofxCv.h"
#include "MtcnnDetector.h"

class FaceDetector : public ofThread {

public:
    FaceDetector();
    void threadedFunction();
    void updateImage(cv::Mat newImage);

    mtcnn_detect_results detectedFaces;

private:
    MtcnnDetector *detector;
    bool isImageDirty;
    cv::Mat image;

};
