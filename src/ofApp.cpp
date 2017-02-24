#include <iostream>
#include "ofApp.h"
#include <GazeEstimation.h>
#include "tbb/tbb.h"

#define NUM_FACES_MAX 5
static const int MODEL_MAX_FAILURES_IN_A_ROW = 4;

using namespace std;

// bufferFrame

    void visualizeTracking(cv::Mat& captured_image, const LandmarkDetector::CLNF& face_model, const LandmarkDetector::FaceModelParameters& det_parameters, cv::Point3f gazeDirection0, cv::Point3f gazeDirection1, double fx, double fy, double cx, double cy) {
        cout << "visualizeTracking" << endl;
        // Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
        double detection_certainty = face_model.detection_certainty;
        bool detection_success = face_model.detection_success;
        double visualisation_boundary = 0.2;

        // Only draw if the reliability is reasonable, the value is slightly ad-hoc
        //if (detection_certainty < visualisation_boundary) {
        if(true){
            cout << "overriding certainty if statement" << endl;
            LandmarkDetector::Draw(captured_image, face_model);
            double vis_certainty = detection_certainty;
            if (vis_certainty > 1)
                vis_certainty = 1;
            if (vis_certainty < -1)
                vis_certainty = -1;
            vis_certainty = (vis_certainty + 1) / (visualisation_boundary + 1);
            // A rough heuristic for box around the face width
            int thickness = (int)std::ceil(2.0* ((double)captured_image.cols) / 640.0);
            cv::Vec6d pose_estimate_to_draw = LandmarkDetector::GetCorrectedPoseWorld(face_model, fx, fy, cx, cy);
            // Draw it in reddish if uncertain, blueish if certain
            LandmarkDetector::DrawBox(captured_image, pose_estimate_to_draw, cv::Scalar((1 - vis_certainty)*255.0, 0, vis_certainty * 255), thickness, fx, fy, cx, cy);
            if (det_parameters.track_gaze && detection_success && face_model.eye_model) {
                cout << "drawGaze" << endl;
                FaceAnalysis::DrawGaze(captured_image, face_model, gazeDirection0, gazeDirection1, fx, fy, cx, cy);
            }
            else {
                cout << "complained but gonan drawGaze anyway" << endl;
                FaceAnalysis::DrawGaze(captured_image, face_model, gazeDirection0, gazeDirection1, fx, fy, cx, cy);
            }
        }
    }

    void bufferFrame::setupModel() {
        LandmarkDetector::FaceModelParameters default_parameters;

        default_parameters.use_face_template = true;
        // Model should not try to re-initialising itself
        // TODO @avi more accurate comment
        default_parameters.reinit_video_every = -1;
        default_parameters.model_location = "/opt/sensei/model/main_clnf_general.txt";
        default_parameters.track_gaze = true;

        LandmarkDetector::CLNF default_model(default_parameters.model_location);

        //int NUM_FACES_MAX = faces.size(); //current number of faces
        models.clear();
        models.reserve(NUM_FACES_MAX);

        cout << "making " << NUM_FACES_MAX << endl;
        for (int i = 0; i < NUM_FACES_MAX; ++i) {
            models.push_back(default_model);
            model_parameters.push_back(default_parameters);
            active_models.push_back(false);
        }
    }

    void bufferFrame::detectLandmarks() {
        cout << "detectLandmarks" << endl;
        cv::Mat_<uchar> matGrayscale;
        cv::cvtColor(ofxCv::toCv(pRGB), matGrayscale, CV_BGR2GRAY);

        bool detection_success = false;

        // This is useful for a second pass run (if want AU predictions)
        vector<cv::Vec6d> params_global_video;
        vector<bool> successes_video;
        vector<cv::Mat_<double>> params_local_video;
        vector<cv::Mat_<double>> detected_landmarks_video;

        // also convert to a concurrent vector
        //vector<tbb::atomic<bool>> faces_used(faceDetector.faces_detected.size());


        cout << "going parallel" << endl;
        // Go through every model and update the tracking
        tbb::parallel_for(0, (int)models.size(), [&](int model_ind) {
            // If the current model has failed more than MODEL_MAX_FAILURES_IN_A_ROW, remove it
            if (models[model_ind].failures_in_a_row > MODEL_MAX_FAILURES_IN_A_ROW) {
                active_models[model_ind] = false;
                ofLog() << "Resetting model #" << model_ind;
                models[model_ind].Reset();
            }

            // If the model is inactive reactivate it with new detections
            if (!active_models[model_ind]) {
                cout << "reactivated model " << model_ind << endl;
                //for (size_t detection_ind = 0; detection_ind < faceDetector.faces_detected.size(); ++detection_ind) {
                for (size_t detection_ind = 0; detection_ind < NUM_FACES_MAX; ++detection_ind) {
                    cout << "working on detection " << detection_ind << endl;
                    // if it was not taken by another tracker take it (if it is false swap it to true and enter detection, this makes it parallel safe)
                    //if (faces_used[detection_ind].compare_and_swap(true, false) == false) {
                    if(true){
                        // Reinitialise the modelm
                        models[model_ind].Reset();
                        // This ensures that a wider window is used for the initial landmark localisation

                        ofLog() << "Reinitializing model #" << model_ind;
                        models[model_ind].detection_success = false;

                        detection_success = LandmarkDetector::DetectLandmarksInVideo(matGrayscale, models[model_ind], model_parameters[model_ind]);

                        // This activates the model
                        active_models[model_ind] = true;

                        // break out of the loop as the tracker has been reinitialised
                        break;
                    }
                }
            } else {
                detection_success = LandmarkDetector::DetectLandmarksInVideo(matGrayscale, models[model_ind], model_parameters[model_ind]);
            }
        });
    }

    void bufferFrame::drawPoses() {
        cout << "draw " << NUM_FACES_MAX << " Poses" << endl;
        // Go through every model and visualise the results
        for (size_t model_ind = 0; model_ind < NUM_FACES_MAX; ++model_ind) {
            if (!models[model_ind].detection_success) {
                cout << "bad" << endl;
                continue;
            }
            cout << "good" << endl;
            // Gaze tracking, absolute gaze direction
            cv::Point3f gazeDirection0(0, 0, -1);
            cv::Point3f gazeDirection1(0, 0, -1);
            FaceAnalysis::EstimateGaze(models[model_ind], gazeDirection0, fx, fy, cx, cy, true);
            FaceAnalysis::EstimateGaze(models[model_ind], gazeDirection1, fx, fy, cx, cy, false);

            // Do face alignment
            cv::Mat sim_warped_img;
            cv::Mat_<double> hog_descriptor;

            // Work out the pose of the head from the tracked model
            cv::Vec6d pose_estimate;
            // XXX: @avi do we want world coordinates?
            pose_estimate = LandmarkDetector::GetCorrectedPoseWorld(models[model_ind], fx, fy, cx, cy);

            cout << "get RGB" << endl;
            //texRGB.loadData(pRGB);

            ofTexture temp;
            //temp.allocate(1920,1080,4);
            temp.loadData(pRGB);

            ofPixels texPixels;
            temp.readToPixels(texPixels);

            cv::Mat matForDrawing = ofxCv::toCv(texPixels);
            visualizeTracking(matForDrawing, models[model_ind], model_parameters[model_ind], gazeDirection0, gazeDirection1, fx, fy, cx, cy);


            temp.loadData(texPixels);
            temp.draw(0,0);
            //texRGB.loadData(texPixels);
        }
    }

    void bufferFrame::draw() {
        if (!hasData) return;

        ofTexture tRGB;
        tRGB.loadData(pRGB);
        tRGB.draw(0,0);

        // for(int i = 0; i < pDepth.size(); i++){
        //     // pixels[i] = ofMap(rawDepthPixels[i], minDistance, maxDistance, 255, 0, true);
        //     pDepth[i] /= 4500;
        // }
        ofTexture tDepth;
        tDepth.loadData(pDepth);
        tDepth.draw(0, 360, 640, 360);

        ofTexture tBigDepth;
        // tBigDepth.setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        tBigDepth.loadData(pBigDepth);
        tBigDepth.draw(640, 0, 640, 360);

        cout << "get some poses" << endl;
        detectLandmarks();

        cout << "draw some poses" << endl;
        drawPoses();

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

          // float *pixels = pBigDepth.getData();
          // int closePixel = static_cast<int>(r.x * r.y);
          // int closePixel = 1920*50 + 600;
          // ofDrawBitmapStringHighlight(std::to_string(pixels[closePixel]), r.x, r.y);
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
        frame->hasData = false;
        frame->setupModel();

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
    }

    void figKinect::draw() {
        if(frame != NULL){
            frame->draw();
        }}

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
        kinect->setup(); //if this returns, we connected}
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
