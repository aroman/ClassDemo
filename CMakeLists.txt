cmake_minimum_required(VERSION 3.0)
set(APP sensei)


# XXX: This isn't working for some reason (conditional always skipped)
# if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#     set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE )
# endif()

project(${APP} CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")
set(OF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/of/" CACHE PATH "The root directory of ofnode/of project.")
include(${OF_ROOT}/openFrameworks.cmake)

option(COTIRE "Use cotire" ON)

### source files for out project
set(sources
  src/main.cpp
  src/ofApp.cpp
  src/OpenFaceModel.cpp
  src/OpenFaceModelPool.cpp
  src/ClassVisualizer.cpp
  src/KinectHelper.cpp
  src/FaceDetector.cpp
  src/MtcnnDetector.cpp
  src/Person.cpp
)
# set_source_files_properties(3rdParty/of/src/poco/Foundation/include/Poco/AtomicCounter.h PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)

### dependencies
find_package(OpenCV 3.2 REQUIRED)
find_package(Boost 1.36.0 COMPONENTS filesystem system REQUIRED)
find_package(Dlib REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)
find_package(PythonInterp 2.7 REQUIRED)
find_package(TurboJPEG REQUIRED)
find_package(PkgConfig)
find_package(LibUSB REQUIRED)
find_package(TBB REQUIRED)
find_package(OpenFace REQUIRED)
find_package(freenect2 REQUIRED)
find_package(OpenCL REQUIRED)
find_package(Numpy REQUIRED)

include_directories(BEFORE ${OpenCV_INCLUDE_DIRS})
include_directories(${TurboJPEG_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIR}/boost)
include_directories(${OpenFace_INCLUDE_DIRS})
include_directories(${freenect2_INCLUDE_DIRS})
include_directories(${TBB_INCLUDE_DIRS})
include_directories(${NUMPY_INCLUDES})
include_directories(${PYTHON_INCLUDE_DIRS})

### addons
ofxaddon(ofxGui)
ofxaddon(3rdParty/addons/ofxCv)
ofxaddon(3rdParty/addons/ofxTimeMeasurements)

set(ofx_addon_includes 3rdParty/addons/ofxCv/libs/CLD/include/CLD 3rdParty/addons/ofxCv/libs/ofxCv/include/ofxCv)
include_directories(${ofx_addon_includes})

### target definitions
add_executable(${APP} ${sources} ${OFXADDONS_SOURCES})
target_compile_options(${APP} PUBLIC -std=c++1y)
# this lets me include files relative to the root src dir with a <> pair
target_include_directories(${APP} PUBLIC src/main)

# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
# file(GLOB_RECURSE models models)
# file(GLOB_RECURSE assets assets)
# file(COPY ${models} DESTINATION models)
# file(COPY ${assets} DESTINATION assets)

target_link_libraries(${APP} PUBLIC
  ${PYTHON_LIBRARIES}
  ${NUMPY_LIBRARIES}
  ${OpenCV_LIBS}
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

install(TARGETS ${APP} DESTINATION .)
# this is basically a repeat of the file copy instruction that copies the
# resources in the build directory, but here we tell cmake that we want it
# in the package
install(DIRECTORY assets/models DESTINATION .)
install(DIRECTORY assets/images DESTINATION data)
install(DIRECTORY assets/fonts DESTINATION data)
install(FILES src/mtcnn_runner.py DESTINATION .)

if (COTIRE)
    cotire(${APP})
endif()
