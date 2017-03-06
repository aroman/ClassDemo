#include "KinectHelper.h"
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>

#define KINECT_FRAME_TIMEOUT 10*1000

KinectHelper::KinectHelper() {
  #ifdef __linux
    libfreenect2::setGlobalLogger(libfreenect2::createConsoleLogger(libfreenect2::Logger::Nada));
  #else
    libfreenect2::setGlobalLogger(libfreenect2::createConsoleLogger(libfreenect2::Logger::None));
  #endif
  isConnected = false;
}

// returns true if successful
bool KinectHelper::connect() {
  if (freenect2.enumerateDevices() == 0) {
    std::cerr << "No Kinect device detected" << std::endl;
    return false;
  }

  libfreenect2::PacketPipeline *pipeline = new libfreenect2::OpenCLPacketPipeline();
  device = freenect2.openDevice(freenect2.getDefaultDeviceSerialNumber(), pipeline);
  if (device == NULL) {
    std::cerr << "Failure opening Kinect device" << std::endl;
    return false;
  }

  std::cout << "device serial: " << device->getSerialNumber() << std::endl;

  int types = libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth;
  listener = new libfreenect2::SyncMultiFrameListener(types);
  libfreenect2::FrameMap frames;

  device->setColorFrameListener(listener);
  device->setIrAndDepthFrameListener(listener);

  if (!device->start()) {
    std::cerr << "Failure starting Kinect device" << std::endl;
    return false;
  }

  registration = new libfreenect2::Registration(
    device->getIrCameraParams(),
    device->getColorCameraParams()
  );

  auto colorParams = device->getColorCameraParams();
  fx = colorParams.fx;
  fy = colorParams.fy;
  cx = colorParams.cx;
  cy = colorParams.cy;

  undistorted = new libfreenect2::Frame(512, 424, 4);
  registered = new libfreenect2::Frame(512, 424, 4);
  bigDepth = new libfreenect2::Frame(1920, 1080 + 2, 4);

  startThread(true);
  isConnected = true;
  return true;
}

void KinectHelper::disconnect() {
  listener->release(frames);
  device->stop();
  device->close();
  delete registration;
  delete listener;
  listener = NULL;
  isConnected = false;
}

ofPixels KinectHelper::getColorPixels() {
  return colorPixelsFront;
}

ofFloatPixels KinectHelper::getDepthPixels() {
  return depthPixelsFront;
}

ofFloatPixels KinectHelper::getBigDepthPixels() {
  return bigDepthPixelsFront;
}

void KinectHelper::threadedFunction() {
    while (isThreadRunning()) {
      if (!listener->waitForNewFrame(frames, KINECT_FRAME_TIMEOUT)) {
        std::cerr << "Timed out waiting for new Kinect frame" << std::endl;
        break;
      }
      libfreenect2::Frame *color = frames[libfreenect2::Frame::Color];
      libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
      libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];

      registration->apply(color, depth, undistorted, registered, true, bigDepth);

      mutex.lock();

      colorPixelsBack.setFromPixels(color->data, color->width, color->height, OF_PIXELS_BGRA);
      depthPixelsBack.setFromPixels(reinterpret_cast<float *>(depth->data), ir->width, ir->height, OF_PIXELS_GRAY);
      bigDepthPixelsBack.setFromPixels(reinterpret_cast<float *>(bigDepth->data), bigDepth->width, bigDepth->height, OF_PIXELS_GRAY);

      float *pixels = bigDepthPixelsBack.getData();
      for (int i = 0; i < bigDepthPixelsBack.size(); i++) {
          pixels[i] = ofMap(bigDepthPixelsBack[i], 500, 4500, 1, 0, true);
      }

      colorPixelsFront.swap(colorPixelsBack);
      depthPixelsFront.swap(depthPixelsBack);
      bigDepthPixelsFront.swap(bigDepthPixelsBack);

      mutex.unlock();

      listener->release(frames);
    }
}
