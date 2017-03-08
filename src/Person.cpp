#include "ofxCv.h"
#include "Person.h"

static const double FACE_SCALE_RATIO = 1.5;
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

float* scaleDepthPixelsForDrawing(ofFloatPixels *depthPixels) {
  // Scale from meters to 0-1 (float)
  float *pixelData = depthPixels->getData();
  for (int i = 0; i < depthPixels->size(); i++) {
      pixelData[i] = ofMap(pixelData[i], 500, 4500, 1, 0, true);
  }
}

void Space::updateDepthPixels(const ofFloatPixels &newDepthPixels) {
  if (r.x < 0 || r.y < 0) return;
  // TODO since we now store depthPixels as meters, we need to update
  // the code to deal work in meters. For now, we just transform them as before.
  newDepthPixels.cropTo(depthPixels, r.x, r.y, r.width, r.height);

  //converts 500,4500 -> 1,0
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



DepthStat Space::doDepthMath(ofRectangle b){
  cout << "doDepthMath" << endl;
  cout << "  r.x: " << b.x << endl;
  cout << "  r.y: " << b.y << endl;
  cout << "  r.w: " << b.width << endl;
  cout << "  r.h: " << b.height << endl;

  DepthStat d;
  d.min = 1.0; //0.0 is the farthest
  d.max = 0.0; //1.0 is the closest
  d.avg = 0.0;
  d.mode = 0.0;
  d.valid = false;

  //return invalid stat if request is bad
    if(depthMap == NULL) return d; //no data
    if(depthPixels.size() <= 0) return d; //no data
    if(((int)b.width) <= 0) return d; //not asking for anything
    if(((int)b.height) <= 0) return d; //not asking for anything

    if(((int)b.x) >= ((int)depthPixels.getWidth())) return d; //out of bounds, no overlap
    if(((int)b.y) >= ((int)depthPixels.getHeight())) return d; //out of bounds, no overlap
    if(((int)(b.x + b.width)) <= 0) return d; //out of bounds, no overlap
    if(((int)(b.y + b.height)) <= 0) return d; //out of bounds, no overlap

  cout << "-- is valid invalid (if this prints)" << endl;

  ofSetColor(ofColor::yellow);
  ofFill();
  ofDrawRectangle(r.x + b.x,r.y + b.y,b.width,b.height);

  //now we know that:
  //  there's data in the depth maps
  //  that there's a nonzero area to compute over
  //  the overlap between request and data is nonzero

  int x0 = (int)min(max(0,(int)b.x),((int)depthPixels.getWidth())-1);
  int y0 = (int)min(max(0,(int)b.y),((int)depthPixels.getHeight())-1);
  int x1 = (int)min(max(1,(x0 + ((int)b.width))),(int)depthPixels.getWidth());
  int y1 = (int)min(max(1,(y0 + ((int)b.height))),(int)depthPixels.getHeight());

  assert(x0 < x1);
  assert(y0 < y1);


  //initialize mode bucket
    uint numBuckets = 16;
    uint buckets[16];
    for(int i = 0; i < numBuckets; i++){
      buckets[i] = 0;
    }

  cout << "-- buckets initialized" << endl;

  //initialize temp variables
    float sum = 0.0;
    float minVal = 1.0;
    float maxVal = 0.0;
    int pixelCounter = 0;
    uint index = 0;
    uint bindex = 0;

  for (int x = x0; x < x1; x++) {
    for (int y = y0; y < y1; y++) {
      index = (y * (uint)depthPixels.getWidth()) + x;
      assert(index > 0);
      assert(index < (((uint)depthPixels.getWidth())*((uint)depthPixels.getHeight())));
      float val = (float)max(min((double)depthMap[index],0.999),0.0);
      cout << "-" << index << ", "  << val << endl;

      //do computation
        //do max
          if(val > maxVal){
            maxVal = val;
          }
        //do min
          if(val < minVal){
            minVal = val;
          }
        //accumulate sum
          sum = sum + val;
        //add to bin
          bindex = (int)(val*numBuckets);
          assert(bindex < numBuckets);
          assert(bindex >= 0);
          buckets[bindex] = buckets[bindex] + 1;

      pixelCounter++;
    }
  }

  int bestIndex = 0;
  int maxCount = 0;
  for(int i = 0; i < numBuckets; i++){
    if(buckets[i] > maxCount){
      maxCount = buckets[i];
      bestIndex = i;
    }
  }

  assert(bestIndex < numBuckets);


  d.min = maxVal; //0.0 is the farthest
  d.max= minVal; //1.0 is the closest
  d.mode = buckets[bestIndex]; 

  assert(pixelCounter > 0);
  if(pixelCounter != 0){
     d.avg = sum / pixelCounter;  
  }
  else{
    d.avg = sum;
  }

  d.valid = true;
  cout << "avg: " << d.avg << ", sum: " << sum << endl;
  cout << "min: " << d.min << endl;
  cout << "max:"  << d.max << endl;
  return d;
}

DepthStat Space::doDepthMathAt(float x, float y, float radius){
  ofRectangle r;
  r.x = x - radius;
  r.y = y - radius;
  r.width = 2 * radius;
  r.height = 2 * radius;
  return doDepthMath(r);
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

//updates both the hand and face boxes
void Person::recalculateBoundingBox() {
  f.r = currentBoundingBox();
  f.r.scaleFromCenter(FACE_SCALE_RATIO);

  auto h_w = HANDBOX_X_RATIO * f.r.width;
  auto h_h = (HANDBOX_Y_RATIO * f.r.height);
  h.r = ofRectangle(
    f.r.x + (f.r.width/2.0) - (h_w/2.0),
    f.r.y - h_h*2,
    h_w,
    h_h
  );
}

ofRectangle Person::currentBoundingBox() const {
    if (openFaceModel != nullptr && openFaceModel->isActive()) {
      return openFaceModel->get2DBoundingBox();
    }
    return mtcnnBoundingBox;
}

/*
void Person::drawFrontalView() const {

  ofColor bboxColor;
  if (isRaisingHand) {
    bboxColor = ofColor::blue;
  } else {
    bboxColor = ofColor::white;
  }

  drawBoundBox(h.r, ofColor::black);

  if (openFaceModel == nullptr) {
    drawBoundBox(f.r, bboxColor);
  } else {
    openFaceModel->getLandmarksPolyline().draw();
    for (auto pointPair : openFaceModel->get3DBoundingBox()) {
      ofPath path;
      path.setStrokeColor(bboxColor);
      path.setStrokeWidth(5);
      path.lineTo(std::get<0>(pointPair));
      path.lineTo(std::get<1>(pointPair));
      path.draw();
    }
  }


  ofDrawBitmapStringHighlight("avg: " + ofToString(f.avgDepth), f.r.x, f.r.y + f.r.height - 40);
}
*/

/*
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
*/

void Person::update(const ofPixels &newColorPixels, const ofFloatPixels &newDepthPixels) {
  //if we find the depth data to be sufficient,
  //we'll update this!
    hasGoodDepth = false;
    isRaisingHand = false;

  //basically update f.r and h.r
  //using both mtcnn and OpenFace
    recalculateBoundingBox();

  //update imagery using new data and new r's
    //update face region
      f.updateColorPixels(newColorPixels);
      f.updateDepthPixels(newDepthPixels);
    //update hand region
      h.updateColorPixels(newColorPixels);
      h.updateDepthPixels(newDepthPixels);

  //get depth for head:
    bool usedOpenFace = false;
    //try to use openface
      if (openFaceModel == nullptr) {
        //get nose 2D coordinates
        uint id = 28;
        if(id < openFaceModel->getLandmarks().size()){
          ofPoint nose = openFaceModel->getLandmarks()[id];
          DepthStat d = f.doDepthMathAt(nose.x-f.r.x,nose.y-f.r.y,2);

          if(d.valid){
            depth = d.max;
            hasGoodDepth = true;
            usedOpenFace = true;      
          }
        }
      }
    //if we failed to, be clever instead
      if(!usedOpenFace){
        DepthStat d = f.doDepthMathAt((f.r.width/2),(f.r.height/2),2);
          if(d.valid){
            depth = d.max;
            hasGoodDepth = true;    
          }
      }

  //if there's data worth checking
    if(hasGoodDepth){
      //check depth for hands!
      
      DepthStat d = h.doDepthMath(ofRectangle(0,0,h.r.width,h.r.height));
      if(d.valid){
        if(d.max > depth){
          isRaisingHand = true;
        }
      }
    }
}
