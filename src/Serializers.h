#include "ofImage.h"

ofFloatPixels parseDepth(std::string path);

void serializeDepthRaw(std::string path, ofFloatPixels deptPixels);

void serializeColor(std::string path, ofPixels colorPixels);

void serializeGrayscaleDepth(std::string path, ofPixels grayscalePixels);
