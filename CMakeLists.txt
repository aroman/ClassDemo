cmake_minimum_required(VERSION 3.0)

project(sensei CXX)
set(APP sensei)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")
set(OF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../of/" CACHE PATH "The root directory of ofnode/of project.")
include(${OF_ROOT}/openFrameworks.cmake)

option(COTIRE "Use cotire" ON)

# ### file globbing
# # these instructions search the directory tree when cmake is
# # invoked and put all files that match the pattern in the variables
# # `sources` and `data`
# file(GLOB_RECURSE sources *.cpp *.h)

set(sources
  src/main.cpp
  src/FaceDetector.cpp
  src/FaceDetector.hpp
  src/ofApp.cpp
  src/ofApp.h
  src/mtcnn.cpp
)

###############################################################################
## dependencies ###############################################################
###############################################################################

# this defines the variables Boost_LIBRARIES that contain all library names
# that we need to link to
find_package(OpenCV 3.2 REQUIRED)
find_package(Boost 1.36.0 COMPONENTS filesystem system REQUIRED)
find_package(Dlib REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)
find_package(TurboJPEG REQUIRED)
find_package(PkgConfig)
find_package(LibUSB REQUIRED)
find_package(TBB REQUIRED)
find_package(OpenFace REQUIRED)
find_package(freenect2 REQUIRED)
find_package(OpenCL REQUIRED)
include_directories(BEFORE ${OpenCV_INCLUDE_DIRS})
include_directories(${TurboJPEG_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIR}/boost)
include_directories(${OpenFace_INCLUDE_DIRS})
include_directories(${freenect2_INCLUDE_DIRS})
include_directories(${TBB_INCLUDE_DIRS})

### addons
ofxaddon(ofxGui)
ofxaddon(addons/ofxCv)
ofxaddon(addons/ofxKinectV2)

file(GLOB_RECURSE ofx_addon_includes LIST_DIRECTORIES true addons/ofxCv/libs/*.h addons/ofxKinectV2/libs/*.h)
include_directories(${ofx_addon_includes})

file(GLOB_RECURSE ofx_kinect_v2_sources addons/ofxKinectV2/src/*.cpp)

### target definitions

# add the data to the target, so it becomes visible in some IDE
add_executable(${APP} ${sources} ${data} ${OFXADDONS_SOURCES} ${ofx_kinect_v2_sources})

# just for example add some compiler flags
target_compile_options(${APP} PUBLIC -std=c++1y)

# this lets me include files relative to the root src dir with a <> pair
target_include_directories(${APP} PUBLIC src/main)

# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
file(COPY ${data} DESTINATION bin/data)

target_link_libraries(${APP} PUBLIC
  ${OpenCV_LIBS}
  python2.7
  ${Boost_LIBRARIES}
  ${OPENFRAMEWORKS_LIBRARIES}
  ${TurboJPEG_LIBRARIES}
  ${LibUSB_LIBRARIES}
  ${dlib_LIBRARIES}
  ${OpenFace_LIBRARIES}
  ${OpenCL_LIBRARIES}
  ${freenect2_LIBRARIES}
  ${TBB_LIBRARIES}
  ${DLIB_LIBRARIES}
)

if (COTIRE)
    cotire(${APP})
endif()
