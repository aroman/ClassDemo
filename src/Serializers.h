#include "ofImage.h"

ofFloatPixels parseDepth(std::string path);

void serializeDepth(std::string path, ofFloatPixels deptPixels);

void serializeColor(std::string path, ofPixels colorPixels);
