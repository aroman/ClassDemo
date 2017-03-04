#include "Person.h"

// helper
void drawBoundBox(rect r, ofColor color) {
  double padding = (r.width * 0.05);

  ofPath p;
  p.setFillColor(color);

  p.rectangle(r.x,
              r.y,
              r.width,
              r.height);
  p.rectangle(r.x + padding,
              r.y + padding,
              r.width - (2 * padding),
              r.height - (2 * padding));

  p.draw();
}

void rect::scaleAboutCenter(double scale) {
  double n_x = x;
  double n_y = y;
  double n_width = width;
  double n_height = height;

  width = n_width * scale;
  height = n_height * scale;

  x = n_x - ((width - n_width) / 2.0);
  y = n_y - ((height - n_height) / 2.0);
}

//region
void region::free() {
  imRGB.clear();
  imDepth.clear();
  ofpDepth.clear();
}

void region::doDepthStats(rect b) {
  doDepthAverage(b);
  doDepthVariance(b);
  doDepthMinMax(b);
}

void region::doDepthAverage(rect b) {
  if (fDepth == NULL) return;
  float tempAvgDepth = 0.0;
  int counter = 0;

  for (int x = b.x; x < (b.x + b.width); x++) {
      for (int y = b.y; y < (b.y + b.height); y++) {
          uint index = (y * ofpDepth.getWidth()) + x;
          if(index < numPixels) {
              float val = fDepth[index];
              if(val < 0.0) val = 0.0;
              if(val > 1.0) val = 1.0;
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

void region::doDepthVariance(rect b) {
  if (fDepth == NULL) return;
  float tempAvgDepthDiff = 0.0;
  int counter = 0;

  for (int x = b.x; x < (b.x + b.width); x++) {
    for (int y = b.y; y < (b.y + b.height); y++) {
      uint index = (y * ofpDepth.getWidth()) + x;
      if (index < numPixels) {
        float val = fDepth[index];
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

void region::doDepthMinMax(rect b) {
  if (fDepth == NULL) return;

  float tempMinDepth = 1.0; // actually closest to screen
  float tempMaxDepth = 0.0; // actually farthest from screen

  for (int x = b.x; x < (b.x + b.width); x++) {
    for (int y = b.y; y < (b.y + b.height); y++) {
      uint index = (y * ofpDepth.getWidth()) + x;
      if (index < numPixels) {
        float val = fDepth[index];
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

void region::updateDepth(ofFloatPixels pDepth) {
  // copy ofFloatPixels pBigDepth to ofPixels faceDepth
  ofTexture t;
  ofPixels d;
  t.loadData(pDepth);
  t.readToPixels(d);

  ofLogNotice("ofApp") <<  r.x << " " << r.y << " " << r.width << " " << r.height;

  // crop faceDepth
  d.crop(r.x, r.y, r.width, r.height);

  // run cv
  imDepth.setFromPixels(d);
  ofxCv::blur(imDepth,20);
  imDepth.update();

  // update statistics on depth
  ofImage t2;
  t2.setFromPixels(d);
  t2.update();

  ofFloatPixels p2 = t2.getPixels();
  fDepth = p2.getData();
  ofpDepth = imDepth.getPixels();
  numPixels = (ofpDepth.getWidth() * ofpDepth.getHeight());
}

void region::updateRGB(ofPixels pRGB) {
  //copy ofFloatPixels pBigDepth to ofPixels faceDepth
  ofTexture t;
  ofPixels d;
  t.loadData(pRGB);
  t.readToPixels(d);

  //crop faceDepth
  // d.crop(r.x, r.y, r.width, r.height);

  //run cv
  imRGB.setFromPixels(d);
  //ofxCv::blur(imRGB,5);
  imRGB.update();
}

//Person
Person::Person() {
  // ofLogNotice("Person", "Person constructor called");
}

void Person::free() {
    f.free();
    h.free();
}

void Person::drawTopView() const {
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

  if(raisedHand) {
      drawBoundBox(f.r,ofColor::red);
      //ofImage icon;
      //icon.load("logo1.png");
      //icon.draw(f.r.x,f.r.y);
  }

  //ofSetColor(ofColor::red);
  ofDrawBitmapStringHighlight("avg: " + ofToString(f.avgDepth), f.r.x, f.r.y - 15);
  ofDrawBitmapStringHighlight("min: " + ofToString(f.minDepth), f.r.x, f.r.y - 30);
  //ofDrawBitmapStringHighlight("max: " + ofToString(maxDepth), f.r.x, f.r.y - 45);

  //ofSetColor(ofColor::black);
}

void Person::update(ofPixels pRGB, ofFloatPixels pBigDepth) {
  rect r1;
  r1.width = 50;
  r1.height = 50;
  r1.x = (f.r.width - r1.width) / 2.0;
  r1.y = (f.r.height - r1.height) / 2.0;

  f.updateRGB(pRGB);
  f.updateDepth(pBigDepth);

  f.doDepthAverage(r1);
  f.doDepthVariance(r1);
  return;

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
  cout << "   avgDepthDiff: " << f.avgDepthDiff << endl;
  cout << "   minDepth:     " << f.minDepth << endl;
  cout << "   maxDepth:     " << f.maxDepth << endl;

  cout << "hand: " << endl;
  cout << "   avgDepth:     " << h.avgDepth << endl;
  cout << "   avgDepthDiff: " << h.avgDepthDiff << endl;
  cout << "   minDepth:     " << h.minDepth  << endl;
  cout << "   maxDepth:     " << h.maxDepth  << endl;
}

void Person::init(ofPixels pRGB, ofFloatPixels pBigDepth) {
  f.r.scaleAboutCenter(1.5);

  // set hand position
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
