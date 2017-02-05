#include "ofApp.h"

#include <GazeEstimation.h>

#include "tbb/tbb.h"

static const int NUM_FACES_MAX = 5;
static const int MODEL_MAX_FAILURES_IN_A_ROW = 4;

/**
 * @brief   Creates an ofPath representing a bounding box with the given parameters
 *
 * @param 		  	x          The x position of the top-left corner of the bounding box
 * @param 		  	y          The y position of the top-left corner of the bounding box
 * @param 		  	width      The width of the bounding box
 * @param 		  	height     The height of the bounding box
 * @param 		  	thickness  The thickness of the bounding box
 *
 * @retval the ofPath instance created
 */
ofPath createBoundingBoxPath(double x, double y, double width, double height, double thickness) {
    ofPath path;
    path.setFillColor(ofColor::blue);
    path.rectangle(x, y, width, height);
    path.rectangle(
                   x + (thickness / 2),
                   y + (thickness / 2),
                   width - thickness,
                   height - thickness);
    return path;
}

/**
 * @brief   Non overlaping detections
 *
 * Go over the model and eliminate detections that are not informative (there already is a tracker there)
 *
 * @param 		  	models          The clnf models.
 * @param [in,out]	face_detections	The face detections.
 */
void NonOverlapingDetections(const vector<LandmarkDetector::CLNF>& models, vector<cv::Rect_<double> >& face_detections) {
    for (size_t model = 0; model < models.size(); ++model) {
        // See if the detections intersect
        cv::Rect_<double> model_rect = models[model].GetBoundingBox();
        for (int detection = face_detections.size() - 1; detection >= 0; --detection) {
            double intersection_area = (model_rect & face_detections[detection]).area();
            double union_area = model_rect.area() + face_detections[detection].area() - 2 * intersection_area;
            // If the model is already tracking what we're detecting ignore the detection, this is determined by amount of overlap
            if (intersection_area / union_area > 0.5) {
                face_detections.erase(face_detections.begin() + detection);
            }
        }
    }
}


/**
 * @brief   Initial openFrameworks setup
 *
 * General set-up, initializations, etc. Called once, before update() or draw() get called.
 */
void ofApp::setup() {
    kinectFrameCounter = 0;

    string CWD = ofFilePath::getCurrentWorkingDirectory();

    // ofxKinect2 guarantees that device ID 0 will always refer to the same kinect
    // if there's only ever one plugged in
    // see: https://github.com/ofTheo/ofxKinectV2/blob/a536824/src/ofxKinectV2.h#L20
    int kinectId = 0;

    textFont.load(OF_TTF_MONO, 28, true);

    kinect = new ofxKinectV2();

    // connect to the kinect. updates occur on a separate thread
    bool didOpenSuccessfully = kinect->open(kinectId);

    if (!didOpenSuccessfully) {
        detectMxnet();
        std::exit(1);
    }

    LandmarkDetector::FaceModelParameters default_parameters;

    default_parameters.use_face_template = true;
    // Model should not try to re-initialising itself
    // TODO @avi more accurate comment
    default_parameters.reinit_video_every = -1;
    default_parameters.model_location = CWD + "/models/model/main_clnf_general.txt";
    default_parameters.track_gaze = true;

    LandmarkDetector::CLNF default_model(default_parameters.model_location);

    models.reserve(NUM_FACES_MAX);
    for (int i = 0; i < NUM_FACES_MAX; ++i) {
        models.push_back(default_model);
        model_parameters.push_back(default_parameters);
        active_models.push_back(false);
    }

    // Used for image masking
    const string tri_loc = CWD + "/models/model/tris_68_full.txt";

    string au_loc;

    bool dynamic = true;
    // Indicates if a dynamic AU model should be used (dynamic is useful if the video is long enough to include neutral expressions)
    if (dynamic) {
        au_loc = CWD + "/models/model/AU_all_best.txt";
    } else {
        au_loc = CWD + "/models/model/AU_all_static.txt";
    }

    // If optical centers are not defined just use center of image
    cx = 1920 / 2.0f;
    cy = 1080 / 2.0f;

    // Use a rough guess-timate of focal length
    fx = 500 * (1920 / 640.0);
    fy = 500 * (1080 / 480.0);
    fx = (fx + fy) / 2.0;
    fy = fx;

//    tracker.setFaceDetectorImageSize(1920*1200);
//    tracker.setup();

    faceDetector.startThread();

//    // Creating a face analyser that will be used for AU extraction
//     face_analyser = FaceAnalysis::FaceAnalyser(vector<cv::Vec3d>(), 0.7, 112, 112, au_loc, tri_loc);
}


// Visualising the results
void visualise_tracking(cv::Mat& captured_image, const LandmarkDetector::CLNF& face_model, const LandmarkDetector::FaceModelParameters& det_parameters, cv::Point3f gazeDirection0, cv::Point3f gazeDirection1, double fx, double fy, double cx, double cy) {

    // Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
    double detection_certainty = face_model.detection_certainty;
    bool detection_success = face_model.detection_success;
    double visualisation_boundary = 0.2;

    // Only draw if the reliability is reasonable, the value is slightly ad-hoc
    if (detection_certainty < visualisation_boundary) {
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
            FaceAnalysis::DrawGaze(captured_image, face_model, gazeDirection0, gazeDirection1, fx, fy, cx, cy);
        }
        else {
            fx;
        }
    }
}


/**
 * @brief   Update Kinect frames
 *
 * General  Requests new depth depth and RGB frames from the kinect, and converts to OpenCV (Mat) format
 */
void ofApp::updateKinect() {

    kinect->update();

    if (kinect->isFrameNew()) {
        // Update Kinect FPS tracking
        if (++kinectFrameCounter == 15) {
            double tsNow = (double)ofGetElapsedTimeMillis();
            kinectFPS = (double)kinectFrameCounter / ((tsNow/1000) - (tsKinectFPS/1000));
            tsKinectFPS = tsNow;
            kinectFrameCounter = 0;
        }

        ofPixels pixelsRGB = kinect->getRgbPixels();
        ofPixels pixelsDepth = kinect->getDepthPixels();
        ofFloatPixels pixelsDepthRaw = kinect->getRawDepthPixels();

        texRGB.loadData(pixelsRGB);
        texDepth.loadData(pixelsDepth);

        // Convert RGB frame grayscale
        cv::cvtColor(ofxCv::toCv(pixelsRGB), matGrayscale, CV_RGB2GRAY);

        // Convert depth frame to mat
        matDepth = cv::Mat(pixelsDepthRaw.getHeight(), pixelsDepthRaw.getWidth(), ofxCv::getCvImageType(pixelsDepthRaw), pixelsDepthRaw.getData(), 0);
    }
}



void ofApp::detectMxnet() {
  ofImage testImg;
  testImg.load(ofFilePath::getCurrentWorkingDirectory() + "/test.jpg");
  cv::Mat imgMat = ofxCv::toCv(testImg);
  cv::Mat imgConv;
  cv::cvtColor(imgMat, imgConv, CV_RGB2BGR);
  // ofLog() << "dims " << imgMat.dims << "flags " << imgMat.flags;
	// namedWindow("Display window", cv::WINDOW_AUTOSIZE);
  // imshow("Display window", imgMat);
  mxnet_detect(imgConv);
}

/**
 * @brief        Detects landmarks and facial features in the current frame based on detected faces
 *
 * General       TODO @avi
 */
void ofApp::detectLandmarks() {

    bool detection_success = false;

    // This is useful for a second pass run (if want AU predictions)
    vector<cv::Vec6d> params_global_video;
    vector<bool> successes_video;
    vector<cv::Mat_<double>> params_local_video;
    vector<cv::Mat_<double>> detected_landmarks_video;

    // also convert to a concurrent vector
    vector<tbb::atomic<bool>> faces_used(faceDetector.faces_detected.size());

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
            for (size_t detection_ind = 0; detection_ind < faceDetector.faces_detected.size(); ++detection_ind) {
                // if it was not taken by another tracker take it (if it is false swap it to true and enter detection, this makes it parallel safe)
                if (faces_used[detection_ind].compare_and_swap(true, false) == false) {
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

/**
 * @brief        Updates kinect and face tracking/analysis
 *
 * General       Called up to n times per second, where n is the max FPS setting
 */
void ofApp::update() {
    updateKinect();
    detectMxnet();

//    tracker.update(matGrayscale);
    faceDetector.updateImage(matGrayscale);
    NonOverlapingDetections(models, faceDetector.faces_detected);

    bool all_models_active = true;
    for (unsigned int i = 0; i < models.size(); ++i) {
        if (!active_models[i]) {
            all_models_active = false;
        }
    }

    if ((ofGetFrameNum() % 2 == 0) && !all_models_active) {
        detectLandmarks();
    }
}

/**
 * @brief        Draws to screen
 *
 * General       Called up to n times per second, where n is the max FPS setting
 */
void ofApp::draw() {
    // Go through every model and visualise the results
    for (size_t model_ind = 0; model_ind < models.size(); ++model_ind) {
        if (!models[model_ind].detection_success) {
            continue;
        }

        // Gaze tracking, absolute gaze direction
        cv::Point3f gazeDirection0(0, 0, -1);
        cv::Point3f gazeDirection1(0, 0, -1);
        FaceAnalysis::EstimateGaze(models[model_ind], gazeDirection0, fx, fy, cx, cy, true);
        FaceAnalysis::EstimateGaze(models[model_ind], gazeDirection1, fx, fy, cx, cy, false);

        // Do face alignment
        cv::Mat sim_warped_img;
        cv::Mat_<double> hog_descriptor;

        //        // But only if needed in output
        //        if (!output_similarity_align.empty()) {
        //            face_analyser.AddNextFrame(captured_image, clnf_models[model], time_stamp, false, !det_parameters[model].quiet_mode);
        //            face_analyser.GetLatestAlignedFace(sim_warped_img);
        //            if (hog_output_file.is_open()) {
        //                FaceAnalysis::Extract_FHOG_descriptor(hog_descriptor, sim_warped_img, num_hog_rows, num_hog_cols);
        //                if (visualise_hog && !det_parameters[model].quiet_mode) {
        //                    cv::Mat_<double> hog_descriptor_vis;
        //                    FaceAnalysis::Visualise_FHOG(hog_descriptor, num_hog_rows, num_hog_cols, hog_descriptor_vis);
        //                }
        //            }
        //        }

        // Work out the pose of the head from the tracked model
        cv::Vec6d pose_estimate;
        // XXX: @avi do we want world coordinates?
        pose_estimate = LandmarkDetector::GetCorrectedPoseWorld(models[model_ind], fx, fy, cx, cy);

        ofPixels texPixels;
        texRGB.readToPixels(texPixels);
        cv::Mat matForDrawing = ofxCv::toCv(texPixels);
        visualise_tracking(matForDrawing, models[model_ind], model_parameters[model_ind], gazeDirection0, gazeDirection1, fx, fy, cx, cy);
        texRGB.loadData(texPixels);
    }

    texRGB.draw(0, 0);
    texDepth.draw(0, 0); //512 x 424

    ofSetColor(ofColor::black);
    ofDrawRectangle(0, ofGetHeight() - 140, 410, 6 * textFont.getSize());
    ofSetColor(ofColor::red);
    textFont.drawString("Kinect FPS: " + ofToString(kinectFPS, 2), 10, ofGetHeight() - textFont.getSize() + 10);
    textFont.drawString("Draw FPS: " + ofToString(ofGetFrameRate(), 2), 10, ofGetHeight() - (3 * textFont.getSize()));
//    textFont.drawString("Tracker FPS: " + ofToString(tracker.getThreadFps(), 2), 10, ofGetHeight() - (6 * textFont.getSize()));
    ofSetColor(ofColor::white);

  //  tracker.drawDebug();
  //  tracker.drawDebugPose();

   for (int i = 0; i < faceDetector.faces_detected.size(); i++) {
       cv::Rect d = faceDetector.faces_detected[i];
       ofPath boundingBox = createBoundingBoxPath(d.x, d.y, d.width, d.height, 5);
       boundingBox.draw();

   }

}

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
