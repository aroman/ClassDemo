#include "ofApp.h"

#define USE_CREEPYFACES false

using namespace std;

    void bufferFrame::draw() {
        if (!hasData) return;

        ofTexture tRGB;
        tRGB.loadData(pRGB);
        tRGB.draw(0,0);

        ofTexture tDepth;
        tDepth.loadData(pDepth);
        tDepth.draw(0, 360, 640, 360);

        ofTexture tBigDepth;
        tBigDepth.loadData(pBigDepth);
        tBigDepth.draw(640, 0, 640, 360);

        if (USE_CREEPYFACES) {
          for (int i = 0; i < faces.size(); i++) {
              rect r = faces[i].r;

              ofPath p;
              p.setFillColor(ofColor::blue);
              p.rectangle(r.x,
                          r.y,
                          r.width,
                          r.height);

              p.rectangle(r.x + (r.width*0.1),
                          r.y + (r.height*0.1),
                          (r.width*0.8),
                          (r.height*0.8));

              p.draw();

              ofPath p1;
              p1.setFillColor(ofColor(0, 255, 255, 200));
              p1.circle(faces[i].points[0], faces[i].points[5], r.width*0.05);
              p1.close();
              p1.circle(faces[i].points[1], faces[i].points[6], r.width*0.05);
              p1.close();
              p1.circle(faces[i].points[2], faces[i].points[7], r.width*0.05);
              p1.close();
              p1.circle(faces[i].points[3], faces[i].points[8], r.width*0.05);
              p1.close();
              p1.circle(faces[i].points[4], faces[i].points[9], r.width*0.05);
              p1.close();
              p1.draw();

              int eye_size = 14;

              tRGB.drawSubsection(r.x, r.y, 50, 50, faces[i].points[0] - (eye_size / 2), faces[i].points[5] - (eye_size / 2), eye_size, eye_size);
              tRGB.drawSubsection(r.x + (r.width/2) + (r.width/2), r.y, 50, 50, faces[i].points[1] - (eye_size / 2), faces[i].points[6] - (eye_size / 2), eye_size, eye_size);
              tRGB.drawSubsection(
                r.x + (r.width/4),
                r.y + (r.height/2),
                100,
                50,
                faces[i].points[3], // x
                faces[i].points[8] - (r.height *0.1), // y
                faces[i].points[4] - faces[i].points[3],
                r.height * 0.20);
        }
        }
    }

    void bufferFrame::findFaces(FaceDetector *faceDetector) {
        if (!hasData) return;
        faces.clear();

        pRGB.resize(640, 360, OF_INTERPOLATE_NEAREST_NEIGHBOR);
        cv::Mat matAdjust = ofxCv::toCv(pRGB);
        // XXX: When I rewrote the kinect integration, this conversion became necessary...
        cv::cvtColor(ofxCv::toCv(pRGB), matAdjust, CV_BGRA2BGR);
        faceDetector->updateImage(matAdjust);
        vector<mtcnn_face_bbox> mxFaces = faceDetector->detectedFaces.bboxes;
        int numFacesFound = mxFaces.size();

        faces.reserve(numFacesFound); // increases the capacity

        for (uint i = 0; i < numFacesFound; i++) {
          faceData face;

          face.r.x = mxFaces[i].x1;
          face.r.y = mxFaces[i].y1;
          face.r.width = abs(mxFaces[i].x2-mxFaces[i].x1);
          face.r.height = abs(mxFaces[i].y2-mxFaces[i].y1);

          face.points = faceDetector->detectedFaces.pointGroups[i];

          faces.push_back(face);
        }
    }


    void bufferFrame::updateOpenFace(OpenFace *openFace) {
      if (faces.size() == 0) return;

      if (openFace->isSetup) {
        openFace->updateFaces(faces);
        openFace->updateImage(pRGB);
        cv::Mat mat;
        cv::cvtColor(ofxCv::toCv(pRGB), mat, CV_BGRA2RGB);
        // openFace->drawTo(mat);
        ofxCv::toOf(mat, pRGB);
      }
    }

// figKinect

    void figKinect::setup() {
        // cout << "figKinect::setup()" << endl;
        if (openFace != NULL) delete openFace;
        openFace = new OpenFace();
        openFace->doSetup();
        openFace->startThread(true);

        if (kinect != NULL) delete kinect;
        kinect = new KinectHelper();

        bool didConnectSuccessfully = kinect->connect();
        if (!didConnectSuccessfully) {
            std::exit(1);
        }

        if (frame == NULL) frame = new bufferFrame();
        frame->hasData = false;

        faceDetector = new FaceDetector();
        faceDetector->startThread(true);
    }

    void figKinect::update() {
        if (!kinect->isConnected) {
          return;
        }
        frame->pRGB = kinect->getRgbPixels();
        frame->pDepth = kinect->getDepthPixels();
        frame->pBigDepth = kinect->getBigDepthPixels();
        frame->hasData = (frame->pRGB.size() > 0);
        frame->findFaces(faceDetector);
        frame->updateOpenFace(openFace);
    }

    void figKinect::draw() {
        if (frame != NULL) {
            frame->draw();
        }
      }

    figKinect::~figKinect() {
        cout << "figKinect::~figKinect" << endl;

        frame->faces.clear();
        delete frame;
        kinect->disconnect();
        delete kinect;
    }

// ofApp

    void ofApp::setup() {
        if(kinect != NULL) delete kinect;
        kinect = new figKinect();
        kinect->setup(); // if this returns, we connected
    }

    void ofApp::update() {
        kinect->update();
    }

    void ofApp::draw() {
        //cout << "ofApp::draw()" << endl;
        ofClear(0);
        kinect->draw();
    }

    ofApp::~ofApp() {
        if(kinect != NULL) delete kinect;
    }

    // ofApp I/O
    void ofApp::keyPressed(int key){}
    void ofApp::keyReleased(int key){}
    void ofApp::mouseMoved(int x, int y){}
    void ofApp::mouseDragged(int x, int y, int button){}
    void ofApp::mousePressed(int x, int y, int button){}
    void ofApp::mouseReleased(int x, int y, int button){}
    void ofApp::mouseEntered(int x, int y){}
    void ofApp::mouseExited(int x, int y){}
    void ofApp::windowResized(int w, int h){}
    void ofApp::gotMessage(ofMessage msg){}
    void ofApp::dragEvent(ofDragInfo dragInfo){}
