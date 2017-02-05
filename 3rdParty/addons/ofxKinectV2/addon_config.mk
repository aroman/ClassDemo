meta:
	ADDON_NAME = ofxKinectV2
	ADDON_DESCRIPTION = An addon for the new Kinect For Windows V2 sensor
	ADDON_AUTHOR = Theo Watson
	ADDON_TAGS = "kinect" "kinectv2" "libfreenect" "libfreenect2"
	ADDON_URL = https://github.com/ofTheo/ofxKinectV2

common:
	ADDON_INCLUDES += libs/libusb/include/libusb
	ADDON_INCLUDES += libs/turbo-jpeg/include
	ADDON_INCLUDES += libs/libfreenect2/include
	ADDON_INCLUDES += libs/libfreenect2/include/internal
	ADDON_INCLUDES += libs/libfreenect2/include/internal/libfreenect2/usb
	ADDON_INCLUDES += libs/protonect
	ADDON_INCLUDES += libs/turbo-jpeg/include
	ADDON_INCLUDES += src

