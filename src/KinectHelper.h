#pragma once

#include "ofThread.h"
#include "ofxCv.h"
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>

class KinectHelper : public ofThread {

public:
    KinectHelper();
    void threadedFunction();
    bool connect();
    void disconnect();

    bool isConnected;

    ofPixels getColorPixels();
    ofFloatPixels getDepthPixels();
    ofFloatPixels getBigDepthPixels();

    // Color camera parameters. Used for OpenFace.
    float fx, fy, cx, cy;

private:
    // Double-buffer to avoid tearing and artifacts
    ofFloatPixels depthPixelsBack;
    ofPixels colorPixelsBack;
    ofFloatPixels bigDepthPixelsBack;

    ofFloatPixels bigDepthPixelsFront;
    ofPixels colorPixelsFront;
    ofFloatPixels depthPixelsFront;

    libfreenect2::FrameMap frames;
    libfreenect2::Freenect2 freenect2;
    libfreenect2::Freenect2Device *device = NULL;
    libfreenect2::SyncMultiFrameListener *listener = NULL;

    libfreenect2::Registration *registration = NULL;
    libfreenect2::Frame *undistorted = NULL;
    libfreenect2::Frame *registered = NULL;
    libfreenect2::Frame *bigDepth = NULL;
};
