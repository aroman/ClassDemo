#include "OpenFace.h"

static const int NUM_FACES_MAX = 1; // 8
static const int MAX_MODEL_FAILURES_IN_A_ROW = 3;
static const double MIN_CERTAINTY_FOR_VISUALIZATION = 0.2;

OpenFace::OpenFace() {
  isSetup = false;
}

OpenFace::~OpenFace() {
  stopThread();
}

void OpenFace::threadedFunction() {
    while (isThreadRunning()) {
      if (!isSetup) continue;
      if (isMatDirty) {
        updateTrackers();
      }
    }
}

void OpenFace::doSetup() {
    LandmarkDetector::FaceModelParameters default_parameters;

    // maybe try turning it off, doesn't make much difference
    default_parameters.use_face_template = true;
    // Model should not try to re-initialising itself
    default_parameters.reinit_video_every = -1;
    default_parameters.model_location = "/opt/sensei/model/main_clnf_general.txt";
    default_parameters.track_gaze = true;

    LandmarkDetector::CLNF default_model(default_parameters.model_location);

    //int NUM_FACES_MAX = faces.size(); //current number of faces
    trackers.clear();
    trackers.reserve(NUM_FACES_MAX);

    for (int i = 0; i < NUM_FACES_MAX; ++i) {
      ofLogNotice("OpenFace") << "Pushing back model #" << i;
      trackers.push_back(FaceTracker{
        -1,
        default_model,
        default_parameters,
        false
      });
    }
    isSetup = true;
    ofLogNotice("OpenFace") << "doSetup() finished";
}

void OpenFace::updateImage(ofPixels rgb) {
  cv::Mat newGrayscale;
  cv::cvtColor(ofxCv::toCv(rgb), newGrayscale, CV_BGR2GRAY);
  newGrayscale.copyTo(matGrayscale);
  isMatDirty = true;
}

void OpenFace::updateFaces(vector<ofRectangle> newFaces) {
  faces.clear();
  for (int i = 0; i < newFaces.size(); i++) {
    auto r = newFaces[i];
    faces.push_back(cv::Rect(r.x, r.y, r.width * 1.05, r.height * 1.05));
  }
}

bool OpenFace::isFaceAtIndexAlreadyBeingTracked(int face_ind) {
  for (auto const &t : trackers) {
    if (t.faceIndex == face_ind) {
      return true;
    }
  }
  return false;
}

void OpenFace::drawTo(cv::Mat mat) {
  for (auto const &tracker : trackers) {
    if (!tracker.isActive) continue;
    // double detectionCertainty = -tracker.model.detection_certainty;
    // if (detectionCertainty < MIN_CERTAINTY_FOR_VISUALIZATION) {
    //   ofLogNotice("OpenFace") << "Skipping because tracking below min certainty: " << detectionCertainty;
    //   continue;
    // }
    LandmarkDetector::Draw(mat, tracker.model);
  }
}

void OpenFace::updateTrackers() {
  ofLogNotice("OpenFace") << "OpenFace::updateTrackers";

  for (auto &tracker : trackers) {
    // Try to update active models, or reset them if they've stopped tracking.
    if (tracker.isActive == true) {
      bool didContinueTrackingSuccessfully = LandmarkDetector::DetectLandmarksInVideo(
        matGrayscale,
        tracker.model,
        tracker.parameters
      );
      if (didContinueTrackingSuccessfully) {
        continue;
      }
      if (tracker.model.failures_in_a_row > MAX_MODEL_FAILURES_IN_A_ROW) {
        ofLogNotice("OpenFace") << "Resetting tracker";
        tracker.isActive = false;
        tracker.faceIndex = -1;
        tracker.model.Reset();
      // This ensures that a wider window is used for the initial landmark localisation
        tracker.model.detection_success = false;
      }
    }

    for (int face_ind = 0; face_ind < faces.size(); face_ind++) {
      if (isFaceAtIndexAlreadyBeingTracked(face_ind)) {
        continue;
      }

      // Note: We ARE passing a bounding box in
      bool didBeginTrackingSuccessfully = LandmarkDetector::DetectLandmarksInVideo(
        matGrayscale,
        faces[face_ind],
        tracker.model,
        tracker.parameters
      );

      ofLogNotice("OpenFace") << "didBeginTrackingSuccessfully = " << didBeginTrackingSuccessfully;
      if (!didBeginTrackingSuccessfully) {
        // We don't want other trackers to bother attempting with this one
        return;
      }
      tracker.isActive = true;
      tracker.faceIndex = face_ind;
    }
  }
}
