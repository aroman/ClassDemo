#include <python2.7/Python.h>
#include "ofxCv.h"

struct mtcnn_face_bbox {
  double x1;
  double y1;
  double x2;
  double y2;
  double score;
};

struct mtcnn_detect_results {
  vector<mtcnn_face_bbox> bboxes;
  vector<vector<double>> pointGroups;
};

class MtcnnDetector {
    int width, height;
    PyObject *pDetectFunc;
  public:
    MtcnnDetector();
   ~MtcnnDetector();
    mtcnn_detect_results detectFaces(const cv::Mat& mat);
};
