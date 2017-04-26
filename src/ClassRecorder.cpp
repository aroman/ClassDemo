#include "ClassRecorder.h"

ClassRecorder::ClassRecorder() {

}

void ClassRecorder::update() {

}

void ClassRecorder::draw() {

}

ClassRecorder::~ClassRecorder() {
  this->kinect->waitForThread(true);
  this->kinect->disconnect();
}
