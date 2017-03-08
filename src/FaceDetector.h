#pragma once

#include <vector>
#include "ofThread.h"
#include "ofxCv.h"
#include "MtcnnDetector.h"

class FaceDetector : public ofThread {

public:
    FaceDetector();
    ~FaceDetector();
    void threadedFunction();
    void updateImage(ofPixels colorPixels);

    const double scaleFactor = 4.0;
    ofEvent<const vector<ofRectangle>> onDetectionResults;

private:
    MtcnnDetector *detector;
    bool isImageDirty;

    ofPixels colorPixels;

};
