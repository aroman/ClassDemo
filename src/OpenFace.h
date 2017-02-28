#include "ofThread.h"
#include "ofxCv.h"
#include "LandmarkCoreIncludes.h"
#include <FaceAnalyser.h>
#include "MtcnnDetector.h"

struct rect {
    double x;
    double y;
    double width;
    double height;
};

class OpenFace : public ofThread  {

public:
  OpenFace();
  void detectLandmarks(ofPixels rgb, vector<mtcnn_face_bbox> bboxes);
  void detectSolo(ofPixels rgb, rect box);
  bool isSetup;

private:
  void doSetup();

  void threadedFunction();

  float fx, fy, cx, cy;
  vector<LandmarkDetector::CLNF> models;
  vector<LandmarkDetector::FaceModelParameters> model_parameters;
  vector<bool> active_models;

};
