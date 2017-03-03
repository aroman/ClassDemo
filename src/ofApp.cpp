#include <iostream>
#include "ofApp.h"
#include <GazeEstimation.h>
#include "tbb/tbb.h"

using namespace std;

//helper

    void drawBoundBox(rect r, ofColor color){
        double padding = (r.width*0.05);

        ofPath p;
        p.setFillColor(color);

        p.rectangle(r.x,
                    r.y,
                    r.width,
                    r.height);
        p.rectangle(r.x + padding,
                    r.y + padding,
                    r.width - (2*padding),
                    r.height - (2*padding));

        p.draw();
        }

//rect
    //scaleAboutCenter
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

//region
    void region::doDepthStats(rect b){
        doDepthAverage(b);
        doDepthVariance(b);
        doDepthMinMax(b);
    }
    void region::doDepthAverage(rect b){
        if(fDepth == NULL) return;
        float tempAvgDepth = 0.0;
        int counter = 0;

        for(int x = b.x; x < (b.x + b.width); x++){
            for(int y = b.y; y < (b.y + b.height); y++){
                uint index = (y * ofpDepth.getWidth()) + x;
                if(index < numPixels){
                    float val = fDepth[index];
                    if(val < 0.0) val = 0.0;
                    if(val > 1.0) val = 1.0;
                    tempAvgDepth += val;
                    counter++;
                }
            }
        }

        cout << "size: " << counter << endl;

        if(counter != 0){
            tempAvgDepth = tempAvgDepth / counter;   
        } 

        avgDepth = tempAvgDepth;
    }
    void region::doDepthVariance(rect b){
        if(fDepth == NULL) return;
        float tempAvgDepthDiff = 0.0;
        int counter = 0;

        for(int x = b.x; x < (b.x + b.width); x++){
            for(int y = b.y; y < (b.y + b.height); y++){
                uint index = (y * ofpDepth.getWidth()) + x;
                if(index < numPixels){
                    float val = fDepth[index];
                    if(val < 0.0) val = 0.0;
                    if(val > 1.0) val = 1.0;
                    tempAvgDepthDiff += abs(val - avgDepth);
                    counter++;
                }
            }
        }

        if(counter != 0){
            tempAvgDepthDiff  = tempAvgDepthDiff  / counter;   
        }

        avgDepthDiff = tempAvgDepthDiff; 
    }
    void region::doDepthMinMax(rect b){
        if(fDepth == NULL) return;

        float tempMinDepth = 1.0; //actually closest to screen
        float tempMaxDepth = 0.0; //actually farthest from screen 

        for(int x = b.x; x < (b.x + b.width); x++){
            for(int y = b.y; y < (b.y + b.height); y++){
                uint index = (y * ofpDepth.getWidth()) + x;
                if(index < numPixels){
                    float val = fDepth[index];
                    if(val < 0.0) val = 0.0;
                    if(val > 1.0) val = 1.0;

                    if(val < tempMinDepth){
                        tempMinDepth = val;
                    }
                    
                    if(val > tempMaxDepth){
                        tempMaxDepth = val;
                    }
                }
            }
        }

        minDepth = tempMinDepth;
        maxDepth = tempMaxDepth;
    }
    void region::updateDepth(ofFloatPixels pDepth){
        //copy ofFloatPixels pBigDepth to ofPixels faceDepth
            ofTexture t;
            ofPixels d;
            t.loadData(pDepth);
            t.readToPixels(d);

        //crop faceDepth
            d.crop(r.x,r.y,r.width,r.height);

        //run cv
            imDepth.setFromPixels(d);
            ofxCv::blur(imDepth,20);
            imDepth.update();

        //update statistics on depth
            ofImage t2;
            t2.setFromPixels(d);
            t2.update();

            ofFloatPixels p2 = t2.getPixels();
            fDepth = p2.getData();

            ofpDepth = imDepth.getPixels();

            numPixels = (ofpDepth.getWidth() * ofpDepth.getHeight());
    }

    void region::updateRGB(ofPixels pRGB){
        //copy ofFloatPixels pBigDepth to ofPixels faceDepth
            ofTexture t;
            ofPixels d;
            t.loadData(pRGB);
            t.readToPixels(d);

        //crop faceDepth
            d.crop(r.x,r.y,r.width,r.height);

        //run cv
            imRGB.setFromPixels(d);
            //ofxCv::blur(imRGB,5);
            imRGB.update(); 
    }

    void region::free(){
        imRGB.clear();
        imDepth.clear();
        ofpDepth.clear();
    }

//person
    void person::drawTopView(){
        int imageHeight = 1080;
        //draw faceDepth image
            //f.imDepth.draw(f.r.x,f.r.y);
        
        //draw bounding box to screen

            rect r;
            r.x = f.r.x;
            r.width = f.r.width;
            r.height = f.r.height;
            r.y = (f.avgDepth * imageHeight) - (r.height / 2);

            f.imRGB.draw(r.x,r.y);

            if(raisedHand)drawBoundBox(r,ofColor::red);


        //ofSetColor(ofColor::red);
        font.drawString("avg: " + ofToString(f.avgDepth), r.x, r.y + r.height - 15);
        //font.drawString("min: " + ofToString(f.minDepth), r.x, r.y + r.height - 30);
        //font.drawString("max: " + ofToString(f.maxDepth), r.x, r.y + r.height - 45);

        //ofSetColor(ofColor::black);
    }

    void person::drawFrontView(){
        //draw images
            f.imDepth.draw(f.r.x,f.r.y);
            //h.imDepth.draw(h.r.x,h.r.y);

        //set color from average distance
            //ofColor avgColor;
            //avgColor.r = 255 * f.avgDepth;
            //avgColor.g = 255 * f.avgDepth;
            //avgColor.b = 255 * f.avgDepth;
        
        //draw constant color blob over face
            //iFace.setColor(avgColor);
            //iFace.update();
        
        //draw bounding box to screen
            drawBoundBox(h.r,ofColor::black);

            if(raisedHand){
                drawBoundBox(f.r,ofColor::red);
                //ofImage icon;
                //icon.load("logo1.png");
                //icon.draw(f.r.x,f.r.y);
            }

        //ofSetColor(ofColor::red);
        font.load("/opt/sensei/Hack-Regular.ttf", 15, true);
        font.drawString("avg: " + ofToString(f.avgDepth), f.r.x, f.r.y - 15);
        font.drawString("min: " + ofToString(f.minDepth), f.r.x, f.r.y - 30);
        //font.drawString("max: " + ofToString(maxDepth), f.r.x, f.r.y - 45);

        //ofSetColor(ofColor::black);
    }

    void person::update(ofPixels pRGB, ofFloatPixels pBigDepth){

        rect r1;
        r1.width = 50;
        r1.height = 50;
        r1.x = (f.r.width - r1.width) / 2.0;
        r1.y = (f.r.height - r1.height) / 2.0;

        f.updateRGB(pRGB);
        f.updateDepth(pBigDepth);
        //f.doDepthStats(r1);

        f.doDepthAverage(r1);
        f.doDepthVariance(r1);

        r1.x = 0;
        r1.y = 0;
        r1.width = f.r.width;
        r1.height = f.r.height;

        f.doDepthMinMax(r1);

        rect r2;
        r2.x = 0;
        r2.y = 0;
        r2.width = h.r.width;
        r2.height = h.r.height;

        h.updateRGB(pRGB);
        h.updateDepth(pBigDepth);
        h.doDepthStats(r2);

        float val1 = h.maxDepth;

        float val2 = f.maxDepth;
        float val3 = 0.3;
        //float val3 = f.avgDepthDiff;

        float upper = val2 + val3;
        float lower = val2 - val3;

        //if(lower > (val2 - 0.3)) lower = (val2 - 0.3);

        if(lower < 0.0) lower = 0.0;
        if(upper > 1.0) upper = 1.0;

        //cout << i << ": average hand depth: "  << val1 << endl;
        //cout <<      "   average face depth: " << val2 << endl;
        //cout <<      "   average depth diff: " << val3 << endl;

        if((val1 <= (upper))&&(val1 >= (lower))){
            raisedHand = true;
        }
        else{
            raisedHand = false;
        }

        
        cout << "face: " << endl;
        cout << "   avgDepth:     " << f.avgDepth << endl;
        cout << "   avgDepthDiff: " << f.avgDepthDiff << endl;
        cout << "   minDepth:     " << f.minDepth << endl;
        cout << "   maxDepth:     " << f.maxDepth << endl;

        cout << "hand: " << endl;
        cout << "   avgDepth:     " << h.avgDepth << endl;
        cout << "   avgDepthDiff: " << h.avgDepthDiff << endl;
        cout << "   minDepth:     " << h.minDepth  << endl;
        cout << "   maxDepth:     " << h.maxDepth  << endl;
        
    }

    void person::init(ofPixels pRGB, ofFloatPixels pBigDepth){
        font.load("/opt/sensei/Hack-Regular.ttf", 15, true);
        f.r.scaleAboutCenter(1.5);

        //set hand position
            rect r = f.r;

            double yRatio = 0.25;
            double xRatio = 2.0;
            double new_h = (yRatio * r.height);
            double new_w = (xRatio * r.width);
            double new_y = r.y - (4 * new_h);
            double new_x = r.x + (r.width/2.0) - (new_w/2.0);

            r.x = new_x;
            r.y = new_y;
            r.width = new_w;
            r.height = new_h;

            h.r = r;       
        update(pRGB, pBigDepth);
    }

    void person::free(){
        f.free();
        h.free();
    }

//bufferFrame


    void bufferFrame::draw(){
        if (!hasData) return;

        if(toggleView) drawTopView();
        else drawFrontView();
        }

    void bufferFrame::drawFrontView(){
        /*
        if (!doRender){
            //if we're not ready to show our results, just draw RGB
                ofTexture tRGB;
                tRGB.loadData(pRGB);
                tRGB.draw(0,0);
            return;
        }
        */

        //allocate canvas if we haven't already
            if(render.size() == 0) {
                render.allocate(pRGB.getWidth(),pRGB.getHeight(),pRGB.getNumChannels());
            }

        //draw RGB for background
            pRGB.swapRgb(); //need to swizzle R and B
            pRGB.pasteInto(render,0,0);


            ofTexture tRender;
            tRender.loadData(render);
            tRender.draw(0,0);

        //exit if not ready to draw additional:
            if(!doRender) return;

        //draw people on top
            for(int i = 0; i < people.size(); i++){
                people[i].drawFrontView();
            }
        font.drawString("Front Facing View", 960, 15);
        }

    void bufferFrame::drawTopView(){
        //exit if not ready to draw additional:
            if(!doRender) return;

        //draw people
            for(int i = 0; i < people.size(); i++){
                people[i].drawTopView();
            }

        font.drawString("Bird's Eye View", 960, 15);
        font.drawString("back", 10, 15);
        font.drawString("front", 10, 1065);
        }

    void bufferFrame::initPeople(FaceDetector *faceDetector){
        findPeople(faceDetector);
        for(int i = 0; i < people.size(); i++){
            people[i].init(pRGB,pBigDepth);
        }}

    void bufferFrame::findPeople(FaceDetector *faceDetector) {
        if (!hasData) return;
        if(people.size() > 0){
            for(int i = 0; i < people.size(); i++){
                people[i].free();
            }
        }
        people.clear();
 
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

        people.reserve(numFacesFound); //increases the capacity

        for(uint i = 0; i < numFacesFound; i++){
            person p;

            p.f.r.x = (mxFaces[i].x1) * rgbShrink;
            p.f.r.y = (mxFaces[i].y1) * rgbShrink;
            p.f.r.width = abs(mxFaces[i].x2-mxFaces[i].x1) * rgbShrink;
            p.f.r.height = abs(mxFaces[i].y2-mxFaces[i].y1) * rgbShrink;

            people.push_back(p);
        }}

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

        //cout << "figKinect is connected and updated" << endl;
        frame->pRGB = kinect->getRgbPixels();
        //frame->pDepth = kinect->getDepthPixels();
        frame->pBigDepth = kinect->getBigDepthPixels();
        frame->hasData = (frame->pRGB.size() > 0);

        frame->doRender = false;
        frame->initPeople(faceDetector);
        frame->doRender = true;
    }

    void figKinect::draw() {
        //cout << "figKinect::draw()" << endl;

        if(frame != NULL){
            frame->toggleView = toggleView;
            frame->draw();
        }

    }

    figKinect::~figKinect() {
        cout << "figKinect::~figKinect" << endl;

        frame->people.clear();
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
    void ofApp::keyPressed(int key){
        if(kinect != NULL){
            if(key == 32){ //spacebar
                //cout << "key: " << key << endl;
                kinect->toggleView = true;
            }
        }
    }
    void ofApp::keyReleased(int key){
        if(kinect != NULL){
            if(key == 32){ //spacebar
                //cout << "key: " << key << endl;
                kinect->toggleView = false;
            }
        }
    }
    void ofApp::mouseMoved(int x, int y){}
    void ofApp::mouseDragged(int x, int y, int button){}
    void ofApp::mousePressed(int x, int y, int button){}
    void ofApp::mouseReleased(int x, int y, int button){}
    void ofApp::mouseEntered(int x, int y){}
    void ofApp::mouseExited(int x, int y){}
    void ofApp::windowResized(int w, int h){}
    void ofApp::gotMessage(ofMessage msg){}
    void ofApp::dragEvent(ofDragInfo dragInfo){}
