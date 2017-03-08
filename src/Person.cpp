#include "ofxCv.h"
#include "Person.h"
#include <math.h>

static const double FACE_SCALE_RATIO = 1.5;
static const double HANDBOX_X_RATIO = 1.75;
static const double HANDBOX_Y_RATIO = 0.375;


#define MAX_MILLIMETERS ((float)4500.0)
#define MIN_MILLIMETERS ((float)500.0)


// inter-class helpers
void drawStringCentered(ofTrueTypeFont font, string s, int xc, int yc, ofColor boxColor, ofColor textColor){
  //do some voodoo
  int sw = (font.stringWidth(s));
  int sh = (font.stringHeight(s));

  int sw1 = sw * 1.05;
  int sh1 = sh * 1.8;

  int x1 = xc - (sw1/2);
  int y1 = yc - (sh1/2);

  ofSetColor(boxColor);
  ofFill();
  ofDrawRectangle(x1,y1,sw1,sh1);

  int sw2 = sw;
  int sh2 = sh;

  int x2 = xc - (sw2/2);
  int y2 = yc - ((sh2*1.3)/2);

  ofSetColor(textColor);
  font.drawString(s, x2, y2 + (sh2));}

void drawStringTopLeft(ofTrueTypeFont font, string s, int xl, int yl, ofColor boxColor, ofColor textColor){
  //do some voodoo
  int sw = (font.stringWidth(s));
  int sh = (font.stringHeight(s));

  int sw1 = sw * 1.05;
  int sh1 = sh * 1.8;

  int x1 = xl;
  int y1 = yl;

  ofSetColor(boxColor);
  ofFill();
  ofDrawRectangle(x1,y1,sw1,sh1);

  int sw2 = sw;
  int sh2 = sh;

  int xc = x1 + (sw1/2);
  int yc = y1 + (sh1/2);

  int x2 = xc - (sw2/2);
  int y2 = yc - ((sh2*1.3)/2);

  ofSetColor(textColor);
  font.drawString(s, x2, y2 + (sh2));}

void drawStringTopRight(ofTrueTypeFont font, string s, int xr, int yr, ofColor boxColor, ofColor textColor){

  //do some voodoo
  int sw = (font.stringWidth(s));
  int sh = (font.stringHeight(s));

  int sw1 = sw * 1.05;
  int sh1 = sh * 1.8;

  xr -= sw1;

  int x1 = xr;
  int y1 = yr;

  ofSetColor(boxColor);
  ofFill();
  ofDrawRectangle(x1,y1,sw1,sh1);

  int sw2 = sw;
  int sh2 = sh;

  int xc = x1 + (sw1/2);
  int yc = y1 + (sh1/2);

  int x2 = xc - (sw2/2);
  int y2 = yc - ((sh2*1.3)/2);

  ofSetColor(textColor);
  font.drawString(s, x2, y2 + (sh2));}

void drawStringBottomLeft(ofTrueTypeFont font, string s, int xl, int yl, ofColor boxColor, ofColor textColor){

  //do some voodoo
  int sw = (font.stringWidth(s));
  int sh = (font.stringHeight(s));

  int sw1 = sw * 1.05;
  int sh1 = sh * 1.8;

  yl -= sh1;

  int x1 = xl;
  int y1 = yl;

  ofSetColor(boxColor);
  ofFill();
  ofDrawRectangle(x1,y1,sw1,sh1);

  int sw2 = sw;
  int sh2 = sh;

  int xc = x1 + (sw1/2);
  int yc = y1 + (sh1/2);

  int x2 = xc - (sw2/2);
  int y2 = yc - ((sh2*1.3)/2);

  ofSetColor(textColor);
  font.drawString(s, x2, y2 + (sh2));}

void drawStringBottomRight(ofTrueTypeFont font, string s, int xr, int yr, ofColor boxColor, ofColor textColor){

  //do some voodoo
  int sw = (font.stringWidth(s));
  int sh = (font.stringHeight(s));

  int sw1 = sw * 1.05;
  int sh1 = sh * 1.8;

  xr -= sw1;
  yr -= sh1;

  int x1 = xr;
  int y1 = yr;

  ofSetColor(boxColor);
  ofFill();
  ofDrawRectangle(x1,y1,sw1,sh1);

  int sw2 = sw;
  int sh2 = sh;

  int xc = x1 + (sw1/2);
  int yc = y1 + (sh1/2);

  int x2 = xc - (sw2/2);
  int y2 = yc - ((sh2*1.3)/2);

  ofSetColor(textColor);
  font.drawString(s, x2, y2 + (sh2));}

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
      pixelData[i] = ofMap(pixelData[i], MIN_MILLIMETERS, MAX_MILLIMETERS, 1, 0, true);
  }
}





void Person::drawFrontDepth() const{
  //DEBUG

  ofFloatPixels copy = f.depthPixels;

  ofTexture scaledDepthTexture;
  ofFloatPixels scaledDepth;
  scaledDepth.setFromPixels(reinterpret_cast<float *>(copy.getData()),f.r.width,
    f.r.height, OF_PIXELS_GRAY);

  scaleDepthPixelsForDrawing(&scaledDepth);
  scaledDepthTexture.loadData(scaledDepth);
  scaledDepthTexture.draw(f.r.x,f.r.y);
}






void Space::updateDepthPixels(const ofFloatPixels &newDepthPixels) {
  if (r.x < 0 || r.y < 0) return;
  // TODO since we now store depthPixels as meters, we need to update
  // the code to deal work in meters. For now, we just transform them as before.
  newDepthPixels.cropTo(depthPixels, r.x, r.y, r.width, r.height);

  //converts 500,4500 -> 1,0

  //scaleDepthPixelsForDrawing(&depthPixels);

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

  /*

  cout << "doDepthMath" << endl;
  cout << "  r.x: " << b.x << endl;
  cout << "  r.y: " << b.y << endl;
  cout << "  r.w: " << b.width << endl;
  cout << "  r.h: " << b.height << endl;

  */

  DepthStat d;
  d.min = MAX_MILLIMETERS; //0.0 is the farthest
  d.max = MIN_MILLIMETERS; //1.0 is the closest
  d.avg = 0.0;
  //d.mode = 0.0;

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
    float minVal = MAX_MILLIMETERS;
    float maxVal = MIN_MILLIMETERS;
    int pixelCounter = 0;
    uint index = 0;


  for (int x = x0; x < x1; x++) {
    for (int y = y0; y < y1; y++) {
      index = (y * (uint)depthPixels.getWidth()) + x;
      assert(index > 0);
      assert(index < (((uint)depthPixels.getWidth())*((uint)depthPixels.getHeight())));

      
      //returns val from 0 to 1
      //float val = (float)max(min((double)depthMap[index],0.999),0.001);
      float temp = depthMap[index];


      if(temp < 0.0){
        temp = MAX_MILLIMETERS;
      }
      if(!isfinite(temp)){
        temp = MAX_MILLIMETERS;
      }
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


      float val = (float)max(min((double)temp,(double)MAX_MILLIMETERS),(double)MIN_MILLIMETERS);


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


  cout << "avg: " << d.avg << ", sum: " << sum << endl;
  cout << "counter:"  << pixelCounter << endl;
  cout << "min: " << d.min << endl;
  cout << "max:"  << d.max << endl;
  //cout << "mode:"  << d.mode << endl;

  //if it's all bad data
  if(sum == MAX_MILLIMETERS * pixelCounter){
    return d;
  }else{
    d.valid = true; 
  }

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
  auto h_h = HANDBOX_Y_RATIO * f.r.height;
  h.r = ofRectangle(
    f.r.x + (f.r.width/2.0) - (h_w/2.0),
    f.r.y - (f.r.height) - h_h,
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



void Person::drawFrontDepthPoints(ofColor c) const{
  ofSetColor(c);
  for(uint i = 0; i < depthLandmarks.size(); i++){
    ofPoint p = depthLandmarks[i];

    ofDrawCircle(p.x,p.y,dotRadius);

  }
  ofSetColor(ofColor::white);
}


void Person::drawFrontHandbox(ofColor c) const{
  if(isRaisingHand){
    drawBoundBox(h.r, c);
  }
  else{
    drawBoundBox(h.r, ofColor::black);
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

    ofSetColor(c);
    //draw polyline version
      //ofPolyline line = openFaceModel->getLandmarksPolyline();
      //line.draw();

    //draw points for landmarks
      for(uint i = 0; i < openFaceModel->getLandmarks().size(); i++){
        ofPoint p = openFaceModel->getLandmarks()[i];
        ofDrawCircle(p.x,p.y,1.0);
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
    temp.draw(r.x,r.y);  

  }
}
void Person::drawTopHandbox(ofColor c) const{
  
}
void Person::drawTopLandmarks(ofColor c) const{
  
}

void Person::drawFrontPersonInfo(ofTrueTypeFont font) const{
  int x = f.r.x;
  int y = f.r.y + (1.5*f.r.height);
  drawPersonInfo(font, x, y);

}
void Person::drawTopHandbox(ofColor c) const{


void Person::drawTopPersonInfo(ofTrueTypeFont font) const{
  int x = f.r.x;
  int y = y_depth;
  drawPersonInfo(font, x, y);

}





void Person::drawPersonInfo(ofTrueTypeFont font, int x,int y) const{
  ofColor box = ofColor(0,0,0,50);
  ofColor text = ofColor(255,255,255,100);
  string s;
  s = "ID: " + name;
  drawStringTopLeft(font, s, x, y, box, text);
  s = "Depth: " + ofToString(depth);
  y += font.getLineHeight();
  drawStringTopLeft(font, s, x, y, box, text);
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

    //accumulate depths at each of those locations
      for(uint i = 0; i < depthLandmarks.size(); i++){
        DepthStat d = f.doDepthMathAt(depthLandmarks[i].x-f.r.x,
          depthLandmarks[i].y-f.r.y,1+(dotRadius*2));
        if(d.valid){
          depths.push_back(d.max);
          depths.push_back(d.avg);
          //depths.push_back(d.mode);
        }
      }
    
    //it's possible none of our queries were valid
    if(depths.size() > 0){
      hasGoodDepth = true;  
      
      //find max depth from any of the locations
        float minDepth = MAX_MILLIMETERS;
        for(uint i = 0; i < depths.size(); i++){
          if(depths[i] < minDepth){
            minDepth = depths[i];
          }
        } 

        depth = minDepth;
        y_depth = (int)((1920 * (depth - MIN_MILLIMETERS))/(MAX_MILLIMETERS - MIN_MILLIMETERS));
        cout << "face Depth: " << depth << endl;
        cout << "y_depth: " << y_depth << endl;
    }

  //if there's data worth checking
    if(hasGoodDepth){
      //check depth for hands!
      DepthStat d = h.doDepthMath(ofRectangle(0,0,h.r.width,h.r.height));
      if(d.valid){

        
        cout << "hand Depth: " << d.min << endl;
      
        if((d.min < (depth + 400))||(d.min > (depth - 400))){

          isRaisingHand = true;
        }
      }
    }
}


