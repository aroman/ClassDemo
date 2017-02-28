#include "OpenFace.h"
#include "tbb/tbb.h"

#define NUM_FACES_MAX 5
static const int MODEL_MAX_FAILURES_IN_A_ROW = 4;

OpenFace::OpenFace() {
  isSetup = false;
}

void OpenFace::threadedFunction() {
    while(isThreadRunning()) {
      if (!isSetup) {
        doSetup();
        isSetup = true;
      }
    }
}

void OpenFace::doSetup() {
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

void OpenFace::detectSolo(ofPixels rgb, rect box) {
  // cv::Rect box_cv(box.x - box.width*0.1, box.y - box.height*0.1, box.width*1.2, box.height*1.2);
  cv::Rect box_cv(0, 0, 640, 360);


  cv::Mat matGrayscale;
  cv::cvtColor(ofxCv::toCv(rgb), matGrayscale, CV_BGR2GRAY);
  LandmarkDetector::CLNF localModel = models[0];
  LandmarkDetector::FaceModelParameters localParams = model_parameters[0];

  LandmarkDetector::CLNF globalModel = models[1];
  LandmarkDetector::FaceModelParameters globalParams = model_parameters[1];

  // Reinitialise the model
  localModel.Reset();

  // This ensures that a wider window is used for the initial landmark localisation
  localModel.detection_success = false;
  bool localSuccess = LandmarkDetector::DetectLandmarksInVideo(matGrayscale, localModel, localParams);
  std::cout << "detection success = " << localSuccess << '\n';

  globalModel.Reset();

  // This ensures that a wider window is used for the initial landmark globalisation
  globalModel.detection_success = false;
  bool globalSuccess = LandmarkDetector::DetectLandmarksInVideo(matGrayscale, box_cv, globalModel, globalParams);
  std::cout << "detection success = " << globalSuccess << '\n';
}

// void OpenFace::detectLandmarks(ofPixels rgb, vector<mtcnn_face_bbox> bboxes) {
//     cout << "detectLandmarks" << endl;
//     cv::Mat matGrayscale;
//     cv::cvtColor(ofxCv::toCv(rgb), matGrayscale, CV_BGR2GRAY);
//
//     bool detection_success = false;
//
//     // This is useful for a second pass run (if want AU predictions)
//     vector<cv::Vec6d> params_global_video;
//     vector<bool> successes_video;
//     vector<cv::Mat_<double>> params_local_video;
//     vector<cv::Mat_<double>> detected_landmarks_video;
//
//     // also convert to a concurrent vector
//     //vector<tbb::atomic<bool>> faces_used(faceDetector.faces_detected.size());
//
//     cout << "going parallel" << endl;
//     // Go through every model and update the tracking
//     tbb::parallel_for(0, (int)models.size(), [&](int model_ind) {
//         // If the current model has failed more than MODEL_MAX_FAILURES_IN_A_ROW, remove it
//         if (models[model_ind].failures_in_a_row > MODEL_MAX_FAILURES_IN_A_ROW) {
//             active_models[model_ind] = false;
//             ofLog() << "Resetting model #" << model_ind;
//             models[model_ind].Reset();
//         }
//
//         // If the model is inactive reactivate it with new detections
//         if (!active_models[model_ind]) {
//             cout << "reactivated model " << model_ind << endl;
//             //for (size_t detection_ind = 0; detection_ind < faceDetector.faces_detected.size(); ++detection_ind) {
//             for (size_t detection_ind = 0; detection_ind < NUM_FACES_MAX; ++detection_ind) {
//                 cout << "working on detection " << detection_ind << endl;
//                 // if it was not taken by another tracker take it (if it is false swap it to true and enter detection, this makes it parallel safe)
//                 //if (faces_used[detection_ind].compare_and_swap(true, false) == false) {
//                 if(true){
//                     // Reinitialise the modelm
//                     models[model_ind].Reset();
//                     //
//                     // // This ensures that a wider window is used for the initial landmark localisation
//                     // ofLog() << "Reinitializing model #" << model_ind;
//                     // models[model_ind].detection_success = false;
//
//                     detection_success = LandmarkDetector::DetectLandmarksInVideo(matGrayscale, models[model_ind], model_parameters[model_ind]);
//
//                     // This activates the model
//                     active_models[model_ind] = true;
//
//                     // break out of the loop as the tracker has been reinitialised
//                     break;
//                 }
//             }
//         } else {
//             detection_success = LandmarkDetector::DetectLandmarksInVideo(matGrayscale, models[model_ind], model_parameters[model_ind]);
//         }
//     });
// }
