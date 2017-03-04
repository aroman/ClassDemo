#pragma once

#include <stdio.h>
#include "ofThread.h"
#include "ofxCv.h"
#include "MtcnnDetector.h"

static const double DOWNSCALE_FACTOR = 3.0;

class FaceDetector : public ofThread {

public:
    FaceDetector();
    ~FaceDetector();
    void threadedFunction();
    void updateImage(ofPixels newImage);

    mtcnn_detect_results detectedFaces;

private:
    MtcnnDetector *detector;
    bool isImageDirty;

    ofPixels image;
    // image scaled down by DOWNSCALE_FACTOR
    ofPixels imageScaled;

};
