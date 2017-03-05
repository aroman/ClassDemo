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

void region::doDepthStats(ofRectangle b) {
  doDepthAverage(b);
  // doDepthVariance(b);
  doDepthMinMax(b);
}

void region::doDepthAverage(ofRectangle b) {
  if (depthMap == NULL) return;
  float tempAvgDepth = 0.0;
  int counter = 0;

  for (int x = b.x; x < (b.x + b.width); x++) {
    for (int y = b.y; y < (b.y + b.height); y++) {
      uint index = (y * imDepth.getWidth()) + x;
      if (index < imDepth.getWidth() * imDepth.getHeight()) {
        float val = depthMap[index];
        if (val < 0.0) val = 0.0;
        if (val > 1.0) val = 1.0;
        tempAvgDepth += val;
        counter++;
      }
    }
  }

  cout << "size: " << counter << endl;

  if (counter != 0) {
    tempAvgDepth = tempAvgDepth / counter;
  }

  avgDepth = tempAvgDepth;
}

void region::doDepthVariance(ofRectangle b) {
  if (depthMap == NULL) return;
  float tempAvgDepthDiff = 0.0;
  int counter = 0;

  for (int x = b.x; x < (b.x + b.width); x++) {
    for (int y = b.y; y < (b.y + b.height); y++) {
      uint index = (y * imDepth.getWidth()) + x;
      if (index < imDepth.getWidth() * imDepth.getHeight()) {
        float val = depthMap[index];
        if (val < 0.0) val = 0.0;
        if (val > 1.0) val = 1.0;
        tempAvgDepthDiff += abs(val - avgDepth);
        counter++;
      }
    }
  }

  if (counter != 0) {
    tempAvgDepthDiff  = tempAvgDepthDiff  / counter;
  }

  avgDepthDiff = tempAvgDepthDiff;
}

void region::doDepthMinMax(ofRectangle b) {
  if (depthMap == NULL) return;

  float tempMinDepth = 1.0; // actually closest to screen
  float tempMaxDepth = 0.0; // actually farthest from screen

  for (int x = b.x; x < (b.x + b.width); x++) {
    for (int y = b.y; y < (b.y + b.height); y++) {
      uint index = (y * imDepth.getWidth()) + x;
      if (index < imDepth.getWidth() * imDepth.getHeight()) {
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

void region::updateDepth(const ofFloatPixels &pDepth) {
  if (r.x < 0 || r.y < 0) return;
  pDepth.cropTo(imDepth.getPixels(), r.x, r.y, r.width, r.height);
  if (r.y > 0) {
    ofxCv::blur(imDepth, 20);
  } else {
    ofLogNotice("region::updateDepth r.y out of screen: ") << r.y;
  }
  imDepth.update();
  depthMap = imDepth.getPixels().getData();
}

void region::updateRGB(const ofPixels &pRGB) {
  if (r.x < 0 || r.y < 0) return;
  pRGB.cropTo(imRGB.getPixels(), r.x, r.y, r.width, r.height);
  imRGB.update();
}

// Construct a person from a bounding box
Person::Person(ofRectangle bbox) {
  f.r = bbox;
  f.r.scaleFromCenter(1.5);
}

void Person::drawTopView() const {
  int imageHeight = 1080;
  //draw faceDepth image
  //f.imDepth.draw(f.r.x,f.r.y);

  ofRectangle r(f.r);
  r.y = (f.avgDepth * imageHeight) - (r.height / 2);

  // f.imRGB.drawSubsection(
  //   r.x, r.y, f.r.width, f.r.height,
  //   f.r.x, f.r.y
  // );

  f.imRGB.draw(r.x, r.y);

  if (raisedHand) {
    drawBoundBox(r, ofColor::red);
  }

  //ofSetColor(ofColor::red);
  ofDrawBitmapString("avg: " + ofToString(f.avgDepth), r.x, r.y + r.height - 15);
  //ofDrawBitmapString("min: " + ofToString(f.minDepth), r.x, r.y + r.height - 30);
  //ofDrawBitmapString("max: " + ofToString(f.maxDepth), r.x, r.y + r.height - 45);

  //ofSetColor(ofColor::black);
}

void Person::drawFrontView() const {
  //f.imDepth.draw(f.r.x, f.r.y);

  //h.imDepth.draw(h.r.x,h.r.y);

  // f.imRGB.draw(f.r.x, f.r.y);

  //set color from average distance
  //ofColor avgColor;
  //avgColor.r = 255 * f.avgDepth;
  //avgColor.g = 255 * f.avgDepth;
  //avgColor.b = 255 * f.avgDepth;

  //draw constant color blob over face
  //iFace.setColor(avgColor);
  //iFace.update();

  //draw bounding box to screen
  drawBoundBox(h.r, ofColor::black);

  drawBoundBox(f.r, ofColor::blue);

  if (raisedHand) {
    drawBoundBox(f.r, ofColor::red);\
  }

  //ofSetColor(ofColor::red);
  ofDrawBitmapStringHighlight("avg: " + ofToString(f.avgDepth), f.r.x, f.r.y - 15);
  ofDrawBitmapStringHighlight("min: " + ofToString(f.minDepth), f.r.x, f.r.y - 30);
  //ofDrawBitmapStringHighlight("max: " + ofToString(maxDepth), f.r.x, f.r.y - 45);

  //ofSetColor(ofColor::black);
}

void Person::update(const ofPixels &pRGB, const ofFloatPixels &pBigDepth) {
  // Create handbox
  auto h_w = HANDBOX_X_RATIO * f.r.width;
  auto h_h = (HANDBOX_Y_RATIO * f.r.height);
  h.r = ofRectangle(
    f.r.x + (f.r.width/2.0) - (h_h/2.0),
    f.r.y - h_h*2,
    h_w,
    h_h
  );

  ofLogNotice("Person::update h.r =") << h.r;

  f.updateRGB(pRGB);
  f.updateDepth(pBigDepth);

  ofRectangle r1;
  r1.width = 50;
  r1.height = 50;
  r1.x = (f.r.width - r1.width) / 2.0;
  r1.y = (f.r.height - r1.height) / 2.0;

  f.doDepthAverage(r1);
  // f.doDepthVariance(r1);

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

  h.updateRGB(pRGB);
  h.updateDepth(pBigDepth);
  h.doDepthStats(r2);

  float val1 = h.maxDepth;

  float val2 = f.maxDepth;
  float val3 = 0.3;

  float upper = val2 + val3;
  float lower = val2 - val3;

  if (lower < 0.0) lower = 0.0;
  if (upper > 1.0) upper = 1.0;

  if ((val1 <= (upper)) && (val1 >= (lower))) {
    raisedHand = true;
  }
  else {
    raisedHand = false;
  }

  cout << "face: " << endl;
  cout << "   avgDepth:     " << f.avgDepth << endl;
  // cout << "   avgDepthDiff: " << f.avgDepthDiff << endl;
  cout << "   minDepth:     " << f.minDepth << endl;
  cout << "   maxDepth:     " << f.maxDepth << endl;

  cout << "hand: " << endl;
  cout << "   avgDepth:     " << h.avgDepth << endl;
  // cout << "   avgDepthDiff: " << h.avgDepthDiff << endl;
  cout << "   minDepth:     " << h.minDepth  << endl;
  cout << "   maxDepth:     " << h.maxDepth  << endl;
}
