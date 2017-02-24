#include <iostream>
#include "ofApp.h"
#include <GazeEstimation.h>
#include "tbb/tbb.h"

using namespace std;

//faceData
    void rect::scaleAboutCenter(double scale){

        double n_x = x;
        double n_y = y;
        double n_width = width;
        double n_height = height;

        width = n_width * scale;
        height = n_height * scale;

        x = n_x - ((width - n_width)/2.0);
        y = n_y - ((height - n_height)/2.0);
    }

// bufferFrame

    void bufferFrame::draw() {
        if (!hasData) return;

        if (!doRender){
            //if we're not ready to show our results, just draw RGB
                ofTexture tRGB;
                tRGB.loadData(pRGB);
                tRGB.draw(0,0);
            return;
        }

        if(render.size() == 0) {
            //allocate a full image
                render.allocate(pRGB.getWidth(),pRGB.getHeight(),pRGB.getNumChannels());
        }

        pRGB.swapRgb(); //need to swizzle R and B
        pRGB.pasteInto(render,0,0);


        ofTexture tRender;
        tRender.loadData(render);
        tRender.draw(0,0);

        
        for (int i = 0; i < faces.size(); i++) {

            rect r = faces[i].r;
            r.scaleAboutCenter(1.5); //double size



            //draw to screen each face in depth values
                ofTexture temp1;
                temp1.loadData(pBigDepth);

                ofPixels faceDepth;
                temp1.readToPixels(faceDepth);


                faceDepth.crop(r.x,r.y,r.width,r.height);


                ofImage iFace;
                iFace.setFromPixels(faceDepth);
                //ofxCv::GaussianBlur(iFace,10);
                ofxCv::blur(iFace,5);
                iFace.update();
                iFace.draw(r.x,r.y);
                

            //get average depth of face

                ofFloatPixels pFace = iFace.getPixels();
                float *xFace = pFace.getData();

                float avgDist = 0;
                //float faceSize = pFace.getWidth() * pFace.getHeight();
                float numPixels = 0;
                for(int x = 0; x < pFace.getWidth(); x++){
                    for(int y = 0; y < pFace.getHeight(); y++){
                        uint index = (y * pFace.getWidth()) + x;
                        float val = xFace[index];
                        if(val != 0.0){
                            avgDist += val;
                            numPixels += 1.0;
                        }
                    }                
                }
                if(numPixels != 0.0){
                    avgDist = avgDist / numPixels;   
                }

            //set color from average distance
                ofColor avgColor;
                avgColor.r = 255 * avgDist;
                avgColor.g = 255 * avgDist;
                avgColor.b = 255 * avgDist;
            
            //draw constant color blob over face
                //iFace.setColor(avgColor);
                //iFace.update();
            
            //draw bounding box to screen
                double padding = (r.width*0.05);

                ofPath p;
                p.setFillColor(avgColor);

                p.rectangle(r.x,
                            r.y,
                            r.width,
                            r.height);
                p.rectangle(r.x + padding,
                            r.y + padding,
                            r.width - (2*padding),
                            r.height - (2*padding));

                p.draw();
      
            //ofSetColor(ofColor::red);
            font.drawString(ofToString(i) + ": " + ofToString(avgDist), r.x, r.y - 15);
            //ofSetColor(ofColor::black);

        }
    }

    void bufferFrame::findFaces(FaceDetector *faceDetector) {
        if (!hasData) return;
        faces.clear();
 
        //copy a smaller version of pRGB into pSmallRGB
            
            //copy full sized pRGB into pSmallRGB

                /*
                //V1
                    //clear out the previous image
                        pSmallRGB.clear();

                    //allocate a full image
                        pSmallRGB.allocate(pRGB.getWidth(),pRGB.getHeight(),pRGB.getNumChannels());
                    
                    //copy pRGB into pSmallRGB
                        //if you use pasteInto, the blue and red channels of pSmallRGB get swapped
                        //I tried swapping pSmallRGB's channels, but they didn't change
                        //Instead I swapped pRGB's before copying, then swap them back

                        //strangely enough, swapRgb() doesn't change how pRGB gets rendered?


                        pRGB.swapRgb(); 
                        pRGB.pasteInto(pSmallRGB,0,0);
                        pRGB.swapRgb();
                */
                
                //V2
                    //if we haven't allocated anything yet, do so
                        if(pSmallRGB.size() == 0) {
                            //allocate a full image
                                pSmallRGB.allocate(pRGB.getWidth(),pRGB.getHeight(),pRGB.getNumChannels());
                        }

                    //copy pRGB into pSmallRGB

                        ofTexture temp;
                        temp.loadData(pRGB);
                        temp.readToPixels(pSmallRGB);
                

            //resize the pSmallRGB
                pSmallRGB.resize((int)pSmallRGB.getWidth()/rgbShrink,(int)pSmallRGB.getHeight()/rgbShrink, OF_INTERPOLATE_NEAREST_NEIGHBOR);
        
        cv::Mat matAdjust = ofxCv::toCv(pSmallRGB);
        // XXX: When I rewrote the kinect integration, this conversion became necessary...
        cv::cvtColor(ofxCv::toCv(pSmallRGB), matAdjust, CV_BGRA2BGR);
        faceDetector->updateImage(matAdjust);
        vector<mtcnn_face_bbox> mxFaces = faceDetector->detectedFaces.bboxes;
        int numFacesFound = mxFaces.size();

        faces.reserve(numFacesFound); //increases the capacity

        for(uint i = 0; i < numFacesFound; i++){
            faceData face;

            face.r.x = (mxFaces[i].x1) * rgbShrink;
            face.r.y = (mxFaces[i].y1) * rgbShrink;
            face.r.width = abs(mxFaces[i].x2-mxFaces[i].x1) * rgbShrink;
            face.r.height = abs(mxFaces[i].y2-mxFaces[i].y1) * rgbShrink;

            faces.push_back(face);
        }
        
    }

    void bufferFrame::getFaceDepth() {
        
        /*
        for(uint i = 0; i < faces.size(); i++){
            faceData face = faces[i];

        }
        */
    }

// figKinect

    void figKinect::setup() {
        cout << "figKinect::setup()" << endl;

        if (kinect != NULL) delete kinect;
        kinect = new KinectHelper();

        bool didConnectSuccessfully = kinect->connect();
        if (!didConnectSuccessfully) {
            std::exit(1);
        }

        if (frame == NULL) frame = new bufferFrame();
        frame->font.load("/opt/sensei/Hack-Regular.ttf", 15, true);
        frame->hasData = false;

        faceDetector = new FaceDetector();
        faceDetector->startThread(true);
    }

    void figKinect::update() {
        if (!kinect->isConnected) return;

        frame->pRGB = kinect->getRgbPixels();
        //frame->pDepth = kinect->getDepthPixels();
        frame->pBigDepth = kinect->getBigDepthPixels();
        frame->hasData = (frame->pRGB.size() > 0);

        frame->doRender = false;
        frame->findFaces(faceDetector);
        //frame->padFaces();
        //frame->getFaceDepth();
        frame->doRender = true;
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
        kinect->disconnect();
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
        // ofClear(0);
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
