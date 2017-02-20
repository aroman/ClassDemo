#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxKinectV2.h"
#include "LandmarkCoreIncludes.h"
#include <FaceAnalyser.h>
#include "FaceDetector.hpp"

typedef long long frameTime;

//typedef cv::Rect_<double> rect;

struct rect{
    double x;
    double y;
    double width;
    double height;
};

struct faceData{
    rect r;
};

class bufferFrame{
public:

    const libfreenect2::Frame *rgbFrame;
    const libfreenect2::Frame *depthFrame;

    libfreenect2::Frame *undistorted;
    libfreenect2::Frame *registered;
    libfreenect2::Frame *bigdepth;

    int* color_depth_map;

    ofPixels pRGB;
    ofPixels pDepth;

    ofPixels test1;
    ofPixels test2;
    ofPixels test3;

    bool hasData = false;

    vector<faceData> faces;

    void draw();

    void findFaces(FaceDetector *faceDetector);

    void doRGBD(libfreenect2::Registration* registration);

    vector<rect> randNRects(uint nRects);
};

class figKinect{
public:

    libfreenect2::Freenect2 freenect2;
    libfreenect2::Registration* registration;

    //bool isOpen();
    //bool hasNewFrame();
    //bufferFrame *getNewFrame();


    ~figKinect();
    void setup();
    void update();
    void draw();

private:
    ofxKinectV2 *kinect = NULL;
    bufferFrame *frame = NULL;
    FaceDetector *faceDetector = NULL;
};


class ofApp : public ofBaseApp{
private:
    figKinect *kinect;
    ofTrueTypeFont font;

public:
    void setup();
    void update();
    void draw();
    ~ofApp();

    void updateKinect();
    void detectLandmarks();
    void detectMxnet();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

};
