#pragma once

#include "ofTrueTypeFont.h"
#include "ofColor.h"
#include "ofRectangle.h"

static const float MIN_DEPTH_MILLIMETERS = 250;
static const float MAX_DEPTH_MILLIMETERS = 10000;

void drawBoundBox(ofRectangle r, ofColor color);

void drawStringCentered(ofTrueTypeFont font, string s, int xc, int yc, ofColor boxColor, ofColor textColor);

void drawStringTopLeft(ofTrueTypeFont font, string s, int xl, int yl, ofColor boxColor, ofColor textColor);

void drawStringTopRight(ofTrueTypeFont font, string s, int xr, int yr, ofColor boxColor, ofColor textColor);

void drawStringBottomLeft(ofTrueTypeFont font, string s, int xl, int yl, ofColor boxColor, ofColor textColor);

void drawStringBottomRight(ofTrueTypeFont font, string s, int xr, int yr, ofColor boxColor, ofColor textColor);

void scaleDepthPixelsForDrawing(ofFloatPixels *depthPixels);
