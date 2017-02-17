#include <iostream>
#include "ofApp.h"
#include <GazeEstimation.h>
#include "tbb/tbb.h"

using namespace std;



// bufferFrame

    void bufferFrame::doRGBD(libfreenect2::Registration* registration){
        const bool enable_filter = true; //filter out pixels not visible to both

        //registration->apply(rgbFrame,depthFrame,undistorted,registered,enable_filter,bigdepth);

        //test1.setFromPixels(undistorted->data, undistorted->width, undistorted->height, 3);
        //test2.setFromPixels(registered->data, registered->width, registered->height, 3);
        //test3.setFromPixels(bigdepth->data, bigdepth->width, bigdepth->height, 3);

    }



    void bufferFrame::draw() {

        //int width = (frame->rgb).getWidth();
        //int height = (frame->rgb).getHeight();
        if(hasData){
            ofTexture tRGB;
            tRGB.loadData(pRGB);
            tRGB.draw(0,0);

            ofTexture tDepth;
            tDepth.loadData(pDepth);
            //tDepth.draw(0,0);

            for(int i = 0; i < faces.size(); i++){
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

            }

        }
    }


    vector<rect> bufferFrame::randNRects(uint nRects){
        vector<rect> rectVec;
        rectVec.reserve(nRects);

        for(int i = 0; i < nRects; i++){
            rect r;

            r.x      = rand() % 500;
            r.y      = rand() % 500;
            r.width  = rand() % 100;
            r.height = rand() % 100;

            rectVec.push_back(r);
        }
        return rectVec;
    }

    void bufferFrame::findFaces(MtcnnDetector *mtcnnDetector) {
        faces.clear();

        //>>>>>>>> INSERT FACE DETECTOR >>>>>>>>>>>>>>>

        //uint numRandFaces = 10;

        //vector<rect> randR = randNRects(numRandFaces);


        cv::Mat matAdjust = ofxCv::toCv(pRGB);
        mtcnn_detect_results detectResults = mtcnnDetector->detectFaces(matAdjust);

        vector<mtcnn_face_bbox> mxFaces = detectResults.bboxes;
        int numFacesFound = mxFaces.size();

        //cv::cvtColor(ofxCv::toCv(pRGB), matAdjust, CV_RGB2GRAY);
        //cv::cvtColor(ofxCv::toCv(pRGB), matAdjust, CV_RGB2BGR);


        //matDepth = cv::Mat(pixelsDepthRaw.getHeight(), pixelsDepthRaw.getWidth(), ofxCv::getCvImageType(pixelsDepthRaw), pixelsDepthRaw.getData(), 0);

        // here's how you use the mtcnn stuff

        // 1. load test image (for you, will be a frame)
        // ofImage testImg;
        // testImg.load(ofFilePath::getCurrentWorkingDirectory() + "/test.jpg");
        // cv::Mat imgMat = ofxCv::toCv(testImg);
        // cv::Mat imgConv;

        // 2. convert to grayscale
        // cv::cvtColor(imgMat, imgConv, CV_RGB2BGR);

        // 3. profit !
        // vector<mtcnn_face_bbox> faces = mxnet_detect(imgConv);
        // printf("%d faces found\n", faces.size());
        // for (mtcnn_face_bbox face : faces) {
        //   printf("\t[(%f,%f), (%f,%f), score = %f\n", face.x1, face.y1, face.x2`, face.y2, face.score);
        // }








        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

        faces.reserve(numFacesFound); //increases the capacity

        for(uint i = 0; i < numFacesFound; i++){
            faceData face;

            cout << "test 5" << endl;

            face.r.x = mxFaces[i].x1;
            face.r.y = mxFaces[i].y1;
            face.r.width = abs(mxFaces[i].x2-mxFaces[i].x1);
            face.r.height = abs(mxFaces[i].y2-mxFaces[i].y1);

            cout << "test 6" << endl;

            faces.push_back(face);
        }

    }




// figKinect

    void figKinect::setup() {
        cout << "figKinect::setup()" << endl;

        if(kinect != NULL) delete kinect;
        kinect = new ofxKinectV2();

        uint port = 0;
        bool connected = kinect->open(port);
        if(!connected){
            cerr << "ERROR: could not kinect on port " << port << endl;
            std::exit(1);
        }

        mtcnnDetector = new MtcnnDetector();

        if(frame != NULL) frame = new bufferFrame();
        frame->hasData = false;

        cout << "connected to kinect on port " << port << endl;

        freenect2 = kinect->freenect2;
        registration = kinect->registration;
    }

    void figKinect::update() {
        kinect->update();

        if(kinect->isFrameNew()){
            frame->hasData = false;

            frame->pRGB = kinect->getRgbPixels();
            frame->pDepth = kinect->getDepthPixels();

            frame->rgbFrame = (libfreenect2::Frame *)kinect->getRgbFrame();
            frame->depthFrame = (libfreenect2::Frame *)kinect->getDepthFrame();

	        //cout << "1: " << ofGetElapsedTimeMillis() << endl;
            frame->findFaces(mtcnnDetector);
	        //cout << "2: " << ofGetElapsedTimeMillis() << endl;

            //frame->doRGBD(registration);

            //frame->getPoses();

            frame->hasData = true;
        }
    }

    void figKinect::draw() {
        //cout << "figKinect::draw()" << endl;

        if(frame != NULL){
            frame->draw();
        }

    }

    figKinect::~figKinect() {
        cout << "figKinect::~figKinect" << endl;

        frame->faces.clear();
        delete frame;
        kinect->close();
        delete kinect;
    }

// ofApp

    void ofApp::setup() {
        cout << "ofApp::setup()" << endl;

        if(kinect != NULL) delete kinect;
        kinect = new figKinect();
        kinect->setup(); //if this returns, we connected

        //font.load("/opt/sensei/Hack-Regular.ttf", 28, true);
    }

    void ofApp::update() {
        //cout << "ofApp::update()" << endl;
        kinect->update();
    }

    void ofApp::draw() {
        //cout << "ofApp::draw()" << endl;
        kinect->draw();
    }

    ofApp::~ofApp() {
        //cout << "ofApp::~ofApp" << endl;
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
