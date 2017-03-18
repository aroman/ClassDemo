#include "Person.h"

#include <math.h>
#include "drawUtils.h"

static const double FACE_SCALE_RATIO = 1.5;
static const double HANDBOX_X_RATIO = 2.4;
static const double HANDBOX_Y_RATIO = 2.0;

#define SCREEN_HEIGHT (1080)
#define SCREEN_WIDTH (1920)




//updates both the hand and face boxes
void Person::recalculateBoundingBox() {
  f.r = currentBoundingBox();
  f.r.scaleFromCenter(FACE_SCALE_RATIO);



  auto h_w = HANDBOX_X_RATIO * f.r.width;
  auto h_h = HANDBOX_Y_RATIO * f.r.height;
  h.r = ofRectangle(
    f.r.x + (f.r.width/2.0) - (h_w/2.0),
    f.r.y - h_h - ((1.0*f.r.height)/2.0),
    h_w,
    h_h
  );



  ofRectangle screen = ofRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);

  //cout << endl << ">>>>>>>>>>>>>>>>>>>>>>>>"<< endl;
  //cout << "ORIGINAL: " << h.r << endl;
  if (!screen.inside(h.r)) {
    //cout << "screen is not inside" << endl;
    // crop or it may not be here
    if (!screen.intersects(h.r)) {
      //cout << "screen does not intersect" << endl;
      h.r = ofRectangle(0,0,0,0);
    } else {
      //cout << "screen DOES intersect" << endl;

      auto x0 = h.r.x;
      auto y0 = h.r.y;

      auto x1 = h.r.x + h.r.width;
      auto y1 = h.r.y + h.r.height;

      x0 = MAX(0.0, x0);
      y0 = MAX(0.0, y0);

      x1 = MIN(x1, SCREEN_WIDTH);
      y1 = MIN(y1, SCREEN_HEIGHT);

      h.r.x = x0;
      h.r.y = y0;
      h.r.width = x1 - x0;
      h.r.height = y1 - y0;

    }
   } else{
      //cout << "screen is COMPLETELY inside" << endl;
   }
   //cout << "EDITED: " << h.r << endl;
   //cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<< endl << endl;
}




int Person::thresholdPixels(ofFloatPixels* depthPixels, float low, float high){
  // Scale from meters to 0-1 (float)
  float *pixelData = depthPixels->getData();

  int counter = 0;

  for (int i = 0; i < depthPixels->size(); i++) {
    if((pixelData[i] > low) && ( pixelData[i] < high)){
      pixelData[i] = 1.0;
      counter++;
    } else{
      pixelData[i] = 0.0;
    }
  }
  return counter;
}

int Person::callThresholdPixels(float low, float high){
  ofFloatPixels copy = h.depthPixels;

  ofFloatPixels scaledDepth;
  scaledDepth.setFromPixels(reinterpret_cast<float *>(copy.getData()),copy.getWidth(),copy.getHeight(), OF_PIXELS_GRAY);

  return thresholdPixels(&scaledDepth, low, high);
}


void Person::drawFrontHandThresholded() const{
  if((thresholdDepthPixels.getWidth()*thresholdDepthPixels.getHeight())>0){
    ofTexture scaledDepthTexture;
    scaledDepthTexture.loadData(thresholdDepthPixels);
    scaledDepthTexture.draw(h.r.x,h.r.y);
  }
}






void Person::drawFrontDepth() const{
  //DEBUG
  /*
  ofFloatPixels copy = f.depthPixels;

  ofTexture scaledDepthTexture;
  ofFloatPixels scaledDepth;
  scaledDepth.setFromPixels(reinterpret_cast<float *>(copy.getData()),f.r.width,
    f.r.height, OF_PIXELS_GRAY);

  scaleDepthPixelsForDrawing(&scaledDepth);
  scaledDepthTexture.loadData(scaledDepth);
  scaledDepthTexture.draw(f.r.x,f.r.y);
  */
}



void Space::updateDepthPixels(const ofFloatPixels &newDepthPixels) {
  if (r.x < 0 || r.y < 0) return;
  // TODO since we now store depthPixels as meters, we need to update
  // the code to deal work in meters. For now, we just transform them as before.
  newDepthPixels.cropTo(depthPixels, r.x, r.y, r.width, r.height);

  //converts 500,4500 -> 1,0
  scaleDepthPixelsForDrawing(&depthPixels);
  /*
  if (r.y > 0) {
    ofxCv::blur(depthPixels, 20);
  } else {
    ofLogNotice("Space:") << ":updateDepth r.y out of screen: ") << r.y;
  }
  */
  depthMap = depthPixels.getData();
}

void Space::updateColorPixels(const ofPixels &newColorPixels) {
  if (r.x < 0 || r.y < 0) return;
  newColorPixels.cropTo(colorPixels, r.x, r.y, r.width, r.height);
}



DepthStat Space::doDepthMath(ofRectangle b){


  /*
  cout << "doDepthMath" << endl;
  cout << "  r.x: " << b.x << endl;
  cout << "  r.y: " << b.y << endl;
  cout << "  r.w: " << b.width << endl;
  cout << "  r.h: " << b.height << endl;

  */


  DepthStat d;
  d.min = 0.0; //0.0 is the farthest
  d.max = 0.0; //1.0 is the closest
  d.avg = 0.0;
  //d.mode = 0.0;

  d.valid = false;

  //return invalid stat if request is bad
    if(depthMap == NULL){
      cout << "BAD: depthMap is NULL" << endl;
      return d; //no data
    }
    if(depthPixels.size() <= 0){
      cout << "BAD: depthPixels is SIZE 0" << endl;
      return d; //no data
    }
    if(((int)b.width) <= 0){
      cout << "BAD: depthMap width <= 0" << endl;
      return d; //not asking for anything
    }
    if(((int)b.height) <= 0){
      cout << "BAD: depthMap height <= 0" << endl;
      return d; //not asking for anything
    }

    if(((int)b.x) >= ((int)depthPixels.getWidth())){
      cout << "BAD: x >= width" << endl;
      return d; //out of bounds, no overlap
    }
    if(((int)b.y) >= ((int)depthPixels.getHeight())){
      cout << "BAD: y >= height" << endl;
      return d; //out of bounds, no overlap
    }
    if(((int)(b.x + b.width)) <= 0){
      cout << "BAD: x + width <= 0" << endl;
      return d; //out of bounds, no overlap
    }
    if(((int)(b.y + b.height)) <= 0){
      cout << "BAD: y + height <= 0" << endl;
      return d; //out of bounds, no overlap
    }

  //cout << "initially VALID request" << endl;

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


  /*
  //initialize mode bucket
    uint bindex = 0;
    uint numBuckets = 16;
    uint buckets[numBuckets];
    for(int i = 0; i < numBuckets; i++){
      buckets[i] = 0;
    }
  */

  //initialize temp variables
    float sum = 0.0;
    float minVal = 1.0;
    float maxVal = 0.0;
    int pixelCounter = 0;
    uint index = 0;


  for (int x = x0; x < x1; x++) {
    for (int y = y0; y < y1; y++) {
      index = (y * (uint)depthPixels.getWidth()) + x;
      assert(index > 0);
      assert(index < (((uint)depthPixels.getWidth())*((uint)depthPixels.getHeight())));


      //returns val from 0 to 1
      float val = (float)max(min((double)depthMap[index],0.999),0.001);

      /*
      float temp = depthMap[index];
      if(temp < 0.0){
        temp = MAX_MILLIMETERS;
      }
      if(!isfinite(temp)){
        temp = MAX_MILLIMETERS;
      }
      */
      /*
      if(temp == std::numeric_limits<float>::signaling_NaN()){
        temp = MAX_MILLIMETERS;
      }
      if(temp == std::numeric_limits<float>::quiet_NaN()){
        temp = MAX_MILLIMETERS;
      }
      if(temp == std::numeric_limits<float>::infinity()){
        temp = MAX_MILLIMETERS;
      }
      if()
      */


      //float val = (float)max(min((double)temp,(double)MAX_MILLIMETERS),(double)MIN_MILLIMETERS);


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

        /*

        //add to bin
          bindex = (int)(val*numBuckets);
          assert(bindex < numBuckets);
          assert(bindex >= 0);
          buckets[bindex] = buckets[bindex] + 1;

        */

      pixelCounter++;
    }
  }


  /*

  int bestIndex = 0;
  int maxCount = 0;
  for(int i = 0; i < numBuckets; i++){
    if(buckets[i] > maxCount){
      maxCount = buckets[i];
      bestIndex = i;
    }
  }

  assert(bestIndex < numBuckets);

  */

  d.min = minVal; //0.0 is the farthest
  d.max = maxVal; //1.0 is the closest
  //d.mode = (1.0 * bestIndex) / (1.0 * numBuckets);

  assert(pixelCounter > 0);
  if(pixelCounter != 0){
     d.avg = sum / pixelCounter;

  }
  else{
    d.avg = sum;
  }

  //cout << "avg: " << d.avg << ", sum: " << sum << endl;
  //cout << "counter:"  << pixelCounter << endl;
  //cout << "min: " << d.min << endl;
  //cout << "max:"  << d.max << endl;

  //cout << "mode:"  << d.mode << endl;

  d.valid = true;

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

ofRectangle Person::currentBoundingBox() const {
    if (openFaceModel != nullptr && openFaceModel->isActive()) {
      return openFaceModel->get2DBoundingBox();
    }
    return mtcnnBoundingBox;
}


string Person::getName() const {
  if (openFaceModel == nullptr) {
    return "unknown";
  } else {
    int numPossibleNames = sizeof(possibleNames) / sizeof(char*);
    return possibleNames[openFaceModel->getId() % numPossibleNames];
  }
}


void Person::drawFrontDepthPoints(ofColor c) const{
  ofSetColor(c);
  for(uint i = 0; i < depthLandmarks.size(); i++){
    ofPoint p = depthLandmarks[i];

    ofDrawCircle(p.x,p.y,dotRadius);

  }
  ofSetColor(ofColor::white);
}


void Person::drawFrontHandbox(ofColor c) const{
  drawFrontHandThresholded();
  if(openFaceModel != nullptr){
    if(isRaisingHand){
      drawBoundBox(h.r, c);
    }
    else{
      drawBoundBox(h.r, ofColor(30,30,30,200));
    }
  }
}
void Person::drawFrontPose(ofColor c) const{
  if (openFaceModel != nullptr) {
    for (auto pointPair : openFaceModel->get3DBoundingBox()) {
      ofPath path;
      path.setStrokeColor(c);
      path.setStrokeWidth(5);
      path.lineTo(std::get<0>(pointPair));
      path.lineTo(std::get<1>(pointPair));
      path.draw();
    }
  } else {
      drawBoundBox(f.r, c);
  }
}
void Person::drawFrontBBox(ofColor c) const {
  drawBoundBox(f.r, c);
}
void Person::drawFrontLandmarks(ofColor c) const{
  if (openFaceModel != nullptr) {


    ofFill();
    ofSetColor(c);
    //draw polyline version
      //ofPolyline line = openFaceModel->getLandmarksPolyline();
      //line.draw();

    //draw points for landmarks
      for(uint i = 0; i < openFaceModel->getLandmarks().size(); i++){
        ofPoint p = openFaceModel->getLandmarks()[i];
        ofDrawCircle(p.x,p.y,(f.r.width/50.0f));
      }

    ofSetColor(ofColor::white);

  }
}



void Person::drawTopColor() const{
  if(hasGoodDepth){
    ofRectangle r(f.r);

    r.y = y_depth;

    ofTexture temp;
    temp.loadData(f.colorPixels);

    int width = topBoxWidth;
    int height = ((width * r.height)/r.width);

    temp.draw(r.x,r.y,width,height);
  }
}
void Person::drawTopHandbox(ofColor c) const{
  if(hasGoodDepth){
    if(isRaisingHand){
      ofRectangle r(f.r);

      r.y = y_depth;

      int width = topBoxWidth;
      int height = ((width * r.height)/r.width);

      ofNoFill();
      ofSetColor(ofColor::red);
      ofDrawRectangle(r.x,r.y,width,height);
    }
  }
}
void Person::drawTopLandmarks(ofColor c) const{

}

void Person::drawFrontPersonInfo(ofTrueTypeFont font) const{
  int x = f.r.x + f.r.width;
  int y = f.r.y;
  drawPersonInfo(font, x, y);

}

void Person::drawTopPersonInfo(ofTrueTypeFont font) const{
  int x = f.r.x + topBoxWidth;
  int y = y_depth;
  drawPersonInfo(font, x, y);
}

void Person::drawPersonInfo(ofTrueTypeFont font, int x, int y) const{
  x += 17;

  ofColor text = ofColor(0,0,0,255);

  ofSetColor(ofColor(205,205,205,140));
  ofFill();

  ofDrawRectRounded(x, y, 260, 125, 30.0);

  x += 10;
  y += 10;

  string s;
  s = "ID: " + getName();
  drawStringTopLeft(font, s, x, y, ofColor(0,0,0,0), text);

  s = "Distance: " + ofToString((int)((400*(1.0-depth))+50))  + "cm";
  y += font.getLineHeight();
  drawStringTopLeft(font, s, x, y, ofColor(0,0,0,0), text);

  if (timestampHandRaised != 0) {
    s = "Hand airtime: " + ofToString(ofGetUnixTime() - timestampHandRaised) + "s";
    y += font.getLineHeight();
    drawStringTopLeft(font, s, x, y, ofColor(0,0,0,0), text);
  }
  else{
    s = "Hand raised: No";
    y += font.getLineHeight();
    drawStringTopLeft(font, s, x, y, ofColor(0,0,0,0), text);
  }

  //DELETE
  /*
  s = "count: " + ofToString((int)(test1));
  y += font.getLineHeight();
  drawStringTopLeft(font, s, x, y, ofColor::red, text);

  s = "total: " + ofToString((int)(test3));
  y += font.getLineHeight();
  drawStringTopLeft(font, s, x, y, ofColor::red, text);

  s = "ratio: " + ofToString((test2)) + " / " + ofToString((threshold));
  y += font.getLineHeight();
  drawStringTopLeft(font, s, x, y, ofColor::red, text);

  s = "avg handbox depth: " + ofToString((test4));
  y += font.getLineHeight();
  drawStringTopLeft(font, s, x, y, ofColor::red, text);

  s = "is okay: " + ofToString((test5));
  y += font.getLineHeight();
  drawStringTopLeft(font, s, x, y, ofColor::red, text);
  */
}

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
    //zero out previous test locations
    depthLandmarks.clear();

    //retrieve depths from those locations
    vector<float> depths;

    //try to use openface's locations
      if (openFaceModel != nullptr) {

        vector<uint> indices;
        indices.push_back(28); //nose
        indices.push_back(36); //left eye
        indices.push_back(45); //right eye

        for(uint i = 0; i < indices.size(); i++){
          uint id = indices[i];
          if(id < openFaceModel->getLandmarks().size()){
            //add the locations of these landmarks
            depthLandmarks.push_back(openFaceModel->getLandmarks()[id]);
          }
        }
      }


    //add our backup locations
      ofPoint center = ofPoint(f.r.x + (f.r.width/2),f.r.y + (f.r.height/2));
      depthLandmarks.push_back(center);

    assert(depthLandmarks.size() > 0);

      ofRectangle tr;
      tr.x = 0;
      tr.y = 0;
      tr.width = h.r.width;
      tr.height = h.r.height;

      DepthStat test = h.doDepthMath(tr);
    //accumulate depths at each of those locations
      for(uint i = 0; i < depthLandmarks.size(); i++){
        //cout << "===========> FACE >=============== " << endl;
        DepthStat d = f.doDepthMathAt(depthLandmarks[i].x-f.r.x,
          depthLandmarks[i].y-f.r.y,1+(dotRadius*2));
        //cout << FACE <=============== " << endl;
        if(d.valid){
          depths.push_back(d.max);
          depths.push_back(d.avg);
          //depths.push_back(d.mode);
        }
      }

    //it's possible none of our queries were valid
    if(depths.size() > 0){
      //find max depth from any of the locations
        //float minDepth = MAX_MILLIMETERS;
        float maxDepth = 0.0;
        for(uint i = 0; i < depths.size(); i++){
          if(depths[i] > maxDepth){
            maxDepth = depths[i];
          }
        }

        depth = maxDepth;
        //y_depth = (int)((SCREEN_HEIGHT * (depth - MIN_MILLIMETERS))/(MAX_MILLIMETERS - MIN_MILLIMETERS));
        y_depth = (int)((SCREEN_HEIGHT) * (1.0 - (0.1 + (0.6*(depth)))));

        //cout << "face Depth: " << depth << endl;
        //cout << "y_depth: " << y_depth << endl;

        if(depth > 0.025){
          hasGoodDepth = true;
        }
    }
    test1 = -3.0;
    test2 = -3.0;
    test3 = -3.0;
    test4 = -3.0;
    test5 = 1.0;
  //if there's data worth checking
    if(hasGoodDepth){
      tempBack = offsetBack;
      if((depth - test.avg)  < (2.25 * offsetBack)){
        tempBack = 0.0175; //depth - (test.avg + 0.005);
        test5 = 0.0;
      }

      int count = callThresholdPixels(depth - tempBack, depth + offsetFront);
      test1 = (float)count;
      test2 = -1.0;
      test3 = (float)(h.r.width * h.r.height);
      test4 = test.avg;

      if((h.r.width * h.r.height) > 0){
        float ratio = ((float)count)/((float)(h.r.width * h.r.height));

        test2 = ratio;
        //cout << "ratio: " << ratio << " / " << threshold << endl;

        if(ratio > threshold){
          if(openFaceModel != nullptr){
            isRaisingHand = true;
          }
        }
      }

    }

    if (timestampHandRaised == 0 && isRaisingHand) {
      timestampHandRaised = ofGetUnixTime();
    }
    if (!isRaisingHand) {
      timestampHandRaised = 0;
    }
}
