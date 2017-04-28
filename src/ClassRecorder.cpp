#include "ClassRecorder.h"

#include "ofUtils.h"
#include "gzstream/gzstream.h"
#include "ofxTimeMeasurements.h"
#include "drawUtils.h"


// path to save recordings relative to the 'data' directory
static const std::string RECORDINGS_DIR_PATH = "../recordings";

ClassRecorder::ClassRecorder() {
  auto recordingPath = ofFilePath::join(RECORDINGS_DIR_PATH, ofGetTimestampString());
  colorDirectory = new ofDirectory(ofFilePath::join(recordingPath, "color"));
  depthDirectory = new ofDirectory(ofFilePath::join(recordingPath, "depth"));

  colorDirectory->create(true);
  depthDirectory->create(true);

  // readDepthFile("/home/sensei/Developer/sensei/install/recordings/2017-04-28-01-51-48-360/depth/81.dat");
}

void ClassRecorder::readDepthFile(std::string path) {
  igzstream compressedDepth;
  compressedDepth.open(path.c_str());
  if (!compressedDepth.good()) {
    ofLogError("ClassRecorder") << "Couldn't open depth file at '" << path << "'";
    exit(1);
  }

  ofBuffer depthBufRead(compressedDepth);
  // ofLogNotice("ClassRecorder") << "file size = " << depthBufRead.size();

  ofFloatPixels depthPixelsRead;
  depthPixelsRead.setFromPixels(reinterpret_cast<float *>(depthBufRead.getData()), 1920, 1082, OF_PIXELS_GRAY);
  scaleDepthPixelsForDrawing(&depthPixelsRead);
  depthImage.setFromPixels(depthPixelsRead);
}

void ClassRecorder::update() {
  ofLogNotice("ClassRecorder") << "writing frame #" << ofGetFrameNum();
  if (!this->kinect->isConnected) return;

  ofPixels colorPixels = this->kinect->getColorPixels();
  ofFloatPixels depthPixels = this->kinect->getAlignedDepthPixels();

  bool hasData = (colorPixels.size() > 0);

  if (!hasData) return;

  auto colorImagePath = ofFilePath::join(colorDirectory->getAbsolutePath(), ofToString(ofGetFrameNum()) + ".jpg");
  auto depthImagePath = ofFilePath::join(depthDirectory->getAbsolutePath(), ofToString(ofGetFrameNum()) + ".dat");

  TS_START("save color");
  colorImage.setFromPixels(colorPixels.getData(), 1920, 1080, OF_IMAGE_COLOR_ALPHA, false);
  colorImage.setImageType(OF_IMAGE_COLOR);
  colorImage.save(colorImagePath, OF_IMAGE_QUALITY_MEDIUM);
  TS_STOP("save color");

  TS_START("save depth");
  ofBuffer depthBuf((char*)depthPixels.getData(), depthPixels.getTotalBytes());
  ogzstream depthFileOf(depthImagePath.c_str());
  depthFileOf << depthBuf;
  depthFileOf.close();
  TS_STOP("save depth");
}

void ClassRecorder::draw() {
  if (depthImage.isAllocated()) {
    depthImage.draw(0,0);
  }
}

ClassRecorder::~ClassRecorder() {
  delete colorDirectory;
  delete depthDirectory;
  this->kinect->waitForThread(true);
  this->kinect->disconnect();
}
