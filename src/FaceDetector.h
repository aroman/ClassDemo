#pragma once

#include <stdio.h>
#include "ofThread.h"
#include "ofxCv.h"
#include "MtcnnDetector.h"

class FaceDetector : public ofThread {

public:
    FaceDetector();
    ~FaceDetector();
    void threadedFunction();
    void updateImage(ofPixels *newImage);

    const double DOWNSCALE_FACTOR = 3.0;
    ofEvent<mtcnn_detect_results> onDetectionResults;

private:
    MtcnnDetector *detector;
    bool isImageDirty;

    ofPixels *image;
    // image scaled down by DOWNSCALE_FACTOR
    ofPixels imageScaled;

};
