#include "ofxCv.h"
#include "Person.h"

static const double HANDBOX_X_RATIO = 2.0;
static const double HANDBOX_Y_RATIO = 0.25;

// helper
void drawBoundBox(ofRectangle r, ofColor color) {
  ofPath p;
  p.setFillColor(color);
  p.rectangle(r);
  ofRectangle outer(r);
  outer.scaleFromCenter(1.1);
  p.rectangle(outer);
  p.draw();
}

void scaleDepthPixelsForDrawing(ofFloatPixels *depthPixels) {
  // Scale from meters to 0-1 (float)
  float *pixelData = depthPixels->getData();
  for (int i = 0; i < depthPixels->size(); i++) {
      pixelData[i] = ofMap(pixelData[i], 500, 4500, 1, 0, true);
  }
}

void Space::doDepthAverage(ofRectangle b) {
  if (depthMap == NULL) return;
  float tempAvgDepth = 0.0;
  int counter = 0;

  for (int x = b.x; x < (b.x + b.width); x++) {
    for (int y = b.y; y < (b.y + b.height); y++) {
      uint index = (y * depthPixels.getWidth()) + x;
      if (index < depthPixels.getWidth() * depthPixels.getHeight()) {
        float val = depthMap[index];
        if (val < 0.0) val = 0.0;
        if (val > 1.0) val = 1.0;
        tempAvgDepth += val;
        counter++;
      }
    }
  }

  if (counter != 0) {
    tempAvgDepth = tempAvgDepth / counter;
  }

  avgDepth = tempAvgDepth;
}

void Space::doDepthMinMax(ofRectangle b) {
  if (depthMap == NULL) return;

  float tempMinDepth = 1.0; // actually closest to screen
  float tempMaxDepth = 0.0; // actually farthest from screen

  for (int x = b.x; x < (b.x + b.width); x++) {
    for (int y = b.y; y < (b.y + b.height); y++) {
      uint index = (y * depthPixels.getWidth()) + x;
      if (index < depthPixels.getWidth() * depthPixels.getHeight()) {
        float val = depthMap[index];
        if (val < 0.0) val = 0.0;
        if (val > 1.0) val = 1.0;

        if (val < tempMinDepth) {
          tempMinDepth = val;
        }

        if (val > tempMaxDepth) {
          tempMaxDepth = val;
        }
      }
    }
  }

  minDepth = tempMinDepth;
  maxDepth = tempMaxDepth;
}

void Space::updateDepthPixels(const ofFloatPixels &newDepthPixels) {
  if (r.x < 0 || r.y < 0) return;
  // TODO since we now store depthPixels as meters, we need to update
  // the code to deal work in meters. For now, we just transform them as before.
  newDepthPixels.cropTo(depthPixels, r.x, r.y, r.width, r.height);
  scaleDepthPixelsForDrawing(&depthPixels);
  if (r.y > 0) {
    ofxCv::blur(depthPixels, 20);
  } else {
    ofLogNotice("Space::updateDepth r.y out of screen: ") << r.y;
  }
  depthMap = depthPixels.getData();
}

void Space::updateColorPixels(const ofPixels &newColorPixels) {
  if (r.x < 0 || r.y < 0) return;
  newColorPixels.cropTo(colorPixels, r.x, r.y, r.width, r.height);
}

// Construct a person from a bounding box
Person::Person(ofRectangle bbox) {
  updateMtcnnBoundingBox(bbox);
}

std::ostream& operator<<(std::ostream &strm, const Person &person) {
  ofRectangle bb = person.currentBoundingBox();
  if (person.openFaceModel != nullptr) {
    return strm << "Person(x=" << bb.x << ", " << "y=" << bb.y << ", " << *person.openFaceModel << ")";
  } else {
    return strm << "Person(x=" << bb.x << ", " << "y=" << bb.y << ")";
  }
}

void Person::updateMtcnnBoundingBox(ofRectangle bboxFromMtcnn) {
  mtcnnBoundingBox = bboxFromMtcnn;
  recalculateBoundingBox();
  isConfirmed = true;
}

void Person::recalculateBoundingBox() {
  f.r = currentBoundingBox();
  f.r.scaleFromCenter(1.5);
}

ofRectangle Person::currentBoundingBox() const {
    if (openFaceModel != nullptr && openFaceModel->isActive()) {
      return openFaceModel->get2DBoundingBox();
    }
    return mtcnnBoundingBox;
}

void Person::drawFrontalView() const {
  drawBoundBox(h.r, ofColor::purple);

  if (isRaisingHand) {
    drawBoundBox(f.r, ofColor::red);
  } else {
    drawBoundBox(f.r, ofColor::blue);
  }

  ofDrawBitmapStringHighlight("avg: " + ofToString(f.avgDepth), f.r.x, f.r.y + f.r.height - 40);
}

void Person::drawBirdseyeView() const {
  ofRectangle r(f.r);
  r.y = (f.avgDepth * f.colorPixels.getHeight()) - (r.height / 2);

  // f.colorPixels.draw(r.x, r.y);

  if (isRaisingHand) {
    drawBoundBox(r, ofColor::red);
  } else {
    drawBoundBox(r, ofColor::blue);
  }

  ofDrawBitmapStringHighlight("avg: " + ofToString(f.avgDepth), r.x, r.y - 15);
}

void Person::update(const ofPixels &newColorPixels, const ofFloatPixels &newDepthPixels) {
  recalculateBoundingBox();
  // Create handbox
  auto h_w = HANDBOX_X_RATIO * f.r.width;
  auto h_h = (HANDBOX_Y_RATIO * f.r.height);
  h.r = ofRectangle(
    f.r.x + (f.r.width/2.0) - (h_w/2.0),
    f.r.y - h_h*2,
    h_w,
    h_h
  );
  f.updateColorPixels(newColorPixels);
  f.updateDepthPixels(newDepthPixels);

  ofRectangle r1;
  r1.width = 50;
  r1.height = 50;
  r1.x = (f.r.width - r1.width) / 2.0;
  r1.y = (f.r.height - r1.height) / 2.0;

  f.doDepthAverage(r1);

  r1.x = 0;
  r1.y = 0;
  r1.width = f.r.width;
  r1.height = f.r.height;

  f.doDepthMinMax(r1);

  ofRectangle r2;
  r2.x = 0;
  r2.y = 0;
  r2.width = h.r.width;
  r2.height = h.r.height;

  h.updateColorPixels(newColorPixels);
  h.updateDepthPixels(newDepthPixels);
  h.doDepthAverage(r2);
  h.doDepthMinMax(r2);

  float val1 = h.maxDepth;

  float val2 = f.maxDepth;
  float val3 = 0.3;

  float upper = val2 + val3;
  float lower = val2 - val3;

  if (lower < 0.0) lower = 0.0;
  if (upper > 1.0) upper = 1.0;

  if ((val1 <= (upper)) && (val1 >= (lower))) {
    isRaisingHand = true;
  }
  else {
    isRaisingHand = false;
  }

  // cout << "face: " << endl;
  // cout << "   avgDepth:     " << f.avgDepth << endl;
  // // cout << "   avgDepthDiff: " << f.avgDepthDiff << endl;
  // cout << "   minDepth:     " << f.minDepth << endl;
  // cout << "   maxDepth:     " << f.maxDepth << endl;
  //
  // cout << "hand: " << endl;
  // cout << "   avgDepth:     " << h.avgDepth << endl;
  // // cout << "   avgDepthDiff: " << h.avgDepthDiff << endl;
  // cout << "   minDepth:     " << h.minDepth  << endl;
  // cout << "   maxDepth:     " << h.maxDepth  << endl;
}
