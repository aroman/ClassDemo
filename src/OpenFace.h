#include "ofThread.h"
#include "ofxCv.h"
#include "LandmarkCoreIncludes.h"
#include <FaceAnalyser.h>
#include "MtcnnDetector.h"
#include "tbb/tbb.h"

struct rect {
    double x;
    double y;
    double width;
    double height;
};

struct faceData {
    rect r;
    vector<double> points;
};

struct FaceTracker {
  int faceIndex;
  LandmarkDetector::CLNF model;
  LandmarkDetector::FaceModelParameters parameters;
  bool isActive;
};

class OpenFace : public ofThread  {

public:
  OpenFace();
  void updateFaces(vector<faceData> newFaces);
  void updateImage(ofPixels rgb);
  void drawTo(cv::Mat mat);
  bool isSetup;
  void doSetup();

private:
  void threadedFunction();
  void updateTrackers();

  bool isFaceAtIndexAlreadyBeingTracked(int face_ind);

  float fx, fy, cx, cy;

  bool isMatDirty;
  cv::Mat matGrayscale;

  vector<cv::Rect> faces;
  vector<FaceTracker> trackers;

};
