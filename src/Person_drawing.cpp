#include "Person.h"

#include <math.h>
#include "ofImage.h"
#include "ofColor.h"
#include "drawUtils.h"

static const float dotRadius = 1;

void Person::drawFrontDepth() const{
  // ofFloatImage image;
  // image.setFromPixels(hnn.depthPixels);
  // scaleDepthPixelsForDrawing(&image.getPixels());
  // image.draw(hnn.r.x, hnn.r.y);
}

void Person::drawFrontDepthPoints(ofColor c) const{
  ofSetColor(c);
  for (uint i = 0; i < depthLandmarks.size(); i++) {
    ofPoint p = depthLandmarks[i];
    ofDrawCircle(p.x,p.y,dotRadius);
  }
  ofSetColor(ofColor::white);
}

void Person::drawFrontHandbox(ofColor c) const{
  if (openFaceModel == nullptr) return;

  if (isRaisingHand) {
    drawBoundBox(h.r, c);
  } else {
    drawBoundBox(h.r, ofColor(30,30,30,200));
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
}
