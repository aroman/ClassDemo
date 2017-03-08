#include "OpenFace.h"

static const int NUM_FACES_MAX = 1; // 10
static const int MAX_MODEL_FAILURES_IN_A_ROW = 1;
static const double MIN_CERTAINTY_FOR_VISUALIZATION = 0.35;

double FaceTracker::getX() const {
  return model.params_global[4];
}

double FaceTracker::getY() const {
  return model.params_global[5];
}

OpenFace::OpenFace(float fx, float fy, float cx, float cy) {
  isSetup = false;
  this->fx = fx;
  this->fy = fy;
  this->cx = cx;
  this->cy = cy;
}

OpenFace::~OpenFace() {
  stopThread();
}

void OpenFace::threadedFunction() {
  // while (isThreadRunning()) {
  //   if (!isSetup) continue;
  //   if (isMatDirty) {
  //     mutex.lock();
  //     updateTrackers();
  //     mutex.unlock();
  //     yield();
  //   }
  // }
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
  cv::cvtColor(ofxCv::toCv(rgb), matGrayscaleBack, CV_BGRA2GRAY);
  matGrayscaleFront = matGrayscaleBack;
  isMatDirty = true;
}

void OpenFace::updateFaces(vector<ofRectangle> newFaces) {
  mutex.lock();
  faces.clear();
  for (int i = 0; i < newFaces.size(); i++) {
    auto r = newFaces[i];
    ofLogNotice("OpenFace") << "Check tracking";
    alreadyTrackingFaceWithin(r);
    faces.push_back(cv::Rect(
      (r.width * -0.0075) + r.x,
      (r.height * 0.2459) + r.y,
      r.width * 1.0323,
      r.height * 0.7751
    ));
  }
  mutex.unlock();
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
    double detectionCertainty = -tracker.model.detection_certainty;
    ofLogNotice("OpenFace") << "certainty: " << detectionCertainty;
    if (detectionCertainty < MIN_CERTAINTY_FOR_VISUALIZATION) {
      ofLogNotice("OpenFace") << "Skipping because tracking below min certainty: " << detectionCertainty;
      continue;
    }

    cv::Vec6d pose_estimate = LandmarkDetector::GetCorrectedPoseWorld(tracker.model, fx, fy, cx, cy);
    LandmarkDetector::Draw(mat, tracker.model);
    LandmarkDetector::DrawBox(mat, pose_estimate, cv::Scalar(255, 0, 0), 10, fx, fy, cx, cy);
  }
}

void OpenFace::alreadyTrackingFaceWithin(ofRectangle bbox) {
  std::cout << bbox << '\n';
  for (auto const &tracker : trackers) {
    if (!tracker.isActive) continue;
    printf("Tracker at %f, %f\n", tracker.getX(), tracker.getY());
  }
}

void OpenFace::updateTrackers() {
  ofLogNotice("OpenFace") << "updateTrackers";

  if (matGrayscaleFront.empty()) {
    ofLogNotice("OpenFace") << "matGrayscaleFront is empty!";
    return;
  }

  mutex.lock();
  std::vector<bool> skipFace;
  skipFace.reserve(faces.size());
  for (auto const & face : faces) {
    skipFace.push_back(false);
  }
  mutex.unlock();

  for (auto &tracker : trackers) {
    // Try to update active models, or reset them if they've stopped tracking.
    if (tracker.isActive == true) {
      bool didContinueTrackingSuccessfully = LandmarkDetector::DetectLandmarksInVideo(
        matGrayscaleFront,
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

    mutex.lock();
    for (int face_ind = 0; face_ind < faces.size(); face_ind++) {
      // Another tracker tried and failed on this face, so let's not
      // try again with this tracker.
      if (skipFace[face_ind]) {
        continue;
      }
      if (isFaceAtIndexAlreadyBeingTracked(face_ind)) {
        continue;
      }

      // Note: We ARE passing a bounding box in
      bool didBeginTrackingSuccessfully = LandmarkDetector::DetectLandmarksInVideo(
        matGrayscaleFront,
        faces[face_ind],
        tracker.model,
        tracker.parameters
      );

      ofLogNotice("OpenFace") << "didBeginTrackingSuccessfully = " << didBeginTrackingSuccessfully;
      if (!didBeginTrackingSuccessfully) {
        // We don't want other trackers to bother attempting with this one
        skipFace[face_ind] = true;
      }
      tracker.isActive = true;
      tracker.faceIndex = face_ind;
    }
    mutex.unlock();
  }
}
