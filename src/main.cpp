#include "SenseiApp.h" // Include things which include Xlib.h after
#include "ofUtils.h"
#include "ofAppRunner.h"
#include "ofxTimeMeasurements.h"

int main() {
  ofSetupOpenGL(1920, 1080, OF_WINDOW);

  // Set the framework to something sane, otherwise the frame rate will be unbounded
  // and eat your CPU alive.
  ofSetFrameRate(30);
  ofSetVerticalSync(false);
  ofSetDataPathRoot("data");
  ofSetWindowTitle("sensei");
  // ofSetFullscreen(true);

  TIME_SAMPLE_SET_FRAMERATE(30.0f);
  TIME_SAMPLE_GET_INSTANCE()->setUIActivationKey('T');
  TIME_SAMPLE_GET_INSTANCE()->setUiScale(2.0);
  TIME_SAMPLE_DISABLE();

  ofRunApp(new SenseiApp());
}
