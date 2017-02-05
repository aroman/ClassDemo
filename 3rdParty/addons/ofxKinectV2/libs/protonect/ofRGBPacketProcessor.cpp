//
//  ofRGBPacketProcessor.cpp
//
//  Created by Theodore Watson on 6/23/14.
//
//

#include "ofRGBPacketProcessor.h"

namespace libfreenect2
{

ofRGBPacketProcessor::ofRGBPacketProcessor(){

    handleDecompress = tjInitDecompress();
    if (handleDecompress == NULL) {
        printf("Error in tjInitDeCompress():\n%s\n", tjGetErrorStr());
    }

    newFrame();
}

ofRGBPacketProcessor::~ofRGBPacketProcessor(){

}

void ofRGBPacketProcessor::newFrame(){
    frame = new Frame(1920, 1080, 3);
}

//void ofRGBPacketProcessor::process(const libfreenect2::RgbPacket &packet){
//    ofBuffer tmp;
//    tmp.set( ( char * )packet.jpeg_buffer, (unsigned int)packet.jpeg_buffer_length);
//
//    if( ofLoadImage(pix, tmp) ){
//
//        memcpy(frame->data, pix.getPixels(), pix.size());
//
//        if(listener_->onNewFrame(Frame::Color, frame)){
//            newFrame();
//        }
//    }
//
//}
//
void ofRGBPacketProcessor::process(const libfreenect2::RgbPacket &packet) {
    int ret = tjDecompress(handleDecompress, packet.jpeg_buffer, packet.jpeg_buffer_length, frame->data, 1920, 0, 1080, TJPF_BGRX, 0);

    if (ret != 0) {
        printf("Error in tjDecompress(): \n%s\n", tjGetErrorStr());
       return;
    }

    if (listener_->onNewFrame(Frame::Color, frame)){
        newFrame();
    }
}

}
