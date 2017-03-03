#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "KinectHelper.h"
#include "LandmarkCoreIncludes.h"
#include <FaceAnalyser.h>
#include "FaceDetector.hpp"

typedef long long frameTime;

struct rect {
    double x;
    double y;
    double width;
    double height;

    //scales it about it's center
    void scaleAboutCenter(double scale);
};

struct region {
    rect r;

    ofImage imRGB;

    ofImage imDepth;
    ofFloatPixels ofpDepth;
    float* fDepth;

    int numPixels;

    float minDepth;
    float maxDepth;
    float avgDepth;
    float avgDepthDiff;

    void doDepthStats(rect r);
    void doDepthAverage(rect r);
    void doDepthVariance(rect r);
    void doDepthMinMax(rect r);
    void updateDepth(ofFloatPixels pDepth);
    void updateRGB(ofPixels pRGB);
    void free();
};

struct person {
    region f;
    region h;

    bool raisedHand = false;

    ofTrueTypeFont font;

    void drawFrontView();
    void drawTopView();
    void init(ofPixels pRGB, ofFloatPixels pBigDepth);
    void update(ofPixels pRGB, ofFloatPixels pBigDepth);
    void free();
};


class bufferFrame {
public:
    ofPixels pRGB;
    ofPixels pSmallRGB; //pRGB scaled down by rgbShrink
    //ofFloatPixels pDepth;
    ofFloatPixels pBigDepth;
    ofPixels render;


   

    bool doRender = false;

    double rgbShrink = 3.0;
    bool hasData = false;
    bool toggleView;


    ofTrueTypeFont font;

    vector<person> people;

    void draw();
    void drawFrontView();
    void drawTopView();
    void initPeople(FaceDetector *faceDetector);
    void findPeople(FaceDetector *faceDetector);
};

class figKinect {
public:

    //bool isOpen();
    //bool hasNewFrame();
    //bufferFrame *getNewFrame();

    ~figKinect();
    void setup();
    void update();
    void draw();

    bool toggleView;

private:
    KinectHelper *kinect = NULL;
    bufferFrame *frame = NULL;
    FaceDetector *faceDetector = NULL;
};

class ofApp : public ofBaseApp {
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
