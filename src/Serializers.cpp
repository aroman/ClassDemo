#include "Serializers.h"
#include "ofUtils.h"
#include "gzstream/gzstream.h"

#include "drawUtils.h"

static const int IM_HEIGHT = 800;
static const int IM_WIDTH = 550;

void serializeDepthRaw(std::string path, ofFloatPixels depthPixels) {
  ofBuffer depthBuf((char*)depthPixels.getData(), depthPixels.getTotalBytes());
  ogzstream depthFileOf(path.c_str());
  depthFileOf << depthBuf;
  depthFileOf.close();
}

ofFloatPixels parseDepth(std::string path) {
  igzstream compressedDepth;
  compressedDepth.open(path.c_str());
  if (!compressedDepth.good()) {
    ofLogError("Serializer") << "Couldn't decompress depth file at '" << path << "'";
    exit(1);
  }

  ofBuffer depthBufRead(compressedDepth);

  ofFloatPixels depthPixelsRead;
  depthPixelsRead.setFromPixels(reinterpret_cast<float *>(depthBufRead.getData()), 800, 550, OF_PIXELS_GRAY);
  return depthPixelsRead;
}

void serializeColor(std::string path, ofPixels colorPixels) {
  ofImage colorImage;
  colorImage.setFromPixels(
    colorPixels.getData(),
    colorPixels.getWidth(),
    colorPixels.getHeight(),
    OF_IMAGE_COLOR_ALPHA,
    true // isRGBOrder -> false, because the data is encoded as BGR
  );
  colorImage.setImageType(OF_IMAGE_COLOR);
  colorImage.resize(IM_HEIGHT, IM_WIDTH);
  colorImage.save(path, OF_IMAGE_QUALITY_HIGH);
}

void serializeGrayscaleDepth(std::string path, ofPixels grayscalePixels) {
  ofImage depthImage;
  depthImage.setFromPixels(grayscalePixels);
  depthImage.setImageType(OF_IMAGE_COLOR_ALPHA);
  depthImage.resize(IM_HEIGHT, IM_WIDTH);
  depthImage.save(path);
}
