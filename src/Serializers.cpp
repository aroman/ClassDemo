#include "Serializers.h"
#include "ofUtils.h"
#include "gzstream/gzstream.h"

void serializeDepth(std::string path, ofFloatPixels depthPixels) {
  ofBuffer depthBuf((char*)depthPixels.getData(), depthPixels.getTotalBytes());
  ogzstream depthFileOf(path.c_str());
  depthFileOf << depthBuf;
  depthFileOf.close();
}

void serializeColor(std::string path, ofPixels colorPixels) {
  ofImage colorImage;
  colorImage.setFromPixels(
    colorPixels.getData(),
    colorPixels.getWidth(),
    colorPixels.getHeight(),
    OF_IMAGE_COLOR_ALPHA,
    false // isRGBOrder -> false, because the data is encoded as BGR
  );
  colorImage.save(path, OF_IMAGE_QUALITY_MEDIUM);
}
