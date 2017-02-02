#include <python2.7/Python.h>
#include <stdio.h>
#include <boost/filesystem.hpp>

#include "mtcnn.h"



static PyObject* pyopencv_from(const cv::Mat_<uchar>& m)
{
    if( !m.data )
        Py_RETURN_NONE;
    cv::Mat_<uchar> temp, *p = (cv::Mat_<uchar>*)&m;
    if(!p->refcount || p->allocator != &g_numpyAllocator)
    {
        temp.allocator = &g_numpyAllocator;
        m.copyTo(temp);
        p = &temp;
    }
    p->addref();
    return pyObjectFromRefcount(p->refcount);
}

void mxnet_detect(cv::Mat_<uchar> mat) {

    PyObject *pDetector, *pModuleMX, *pModuleMtcnnDetector;

    Py_SetProgramName("mtcnn-bridge");  /* optional but recommended */
    Py_Initialize();

    // // pModuleMX = PyImport_Import()
    // boost::filesystem::path full_path( boost::filesystem::current_path());
    //
    // PySys_SetPath(const_cast<char*> (full_path.string().c_str()));
    //

    // PyObject* myModuleString = PyString_FromString((char*)"mtcnn_detector");
    // PyObject* myModule = PyImport_Import(myModuleString);


    PyObject *pMat = pyopencv_from(mat);

    PyRun_SimpleString(
      "from pprint import pprint as pp\n"
      "import mxnet as mx\n"
      "import cv2\n"
      "from mtcnn_detector import MtcnnDetector\n"
      "detector = MtcnnDetector(model_folder='model', ctx=mx.cpu(0), num_worker = 4 , accurate_landmark = False)\n"
      // "img = cv2.imread('test.jpg')\n"
      "results = detector.detect_face(img)\n"
      "print detector\n"
      "pp(results)"
    );

    // // PyRun_SimpleString("print 'hello, world!'\n"
    // //                    "import mxnet as mx\n"
    // //                    "from mtcnn_detector import MtcnnDetector\n"
    // //                    "print mx\n");
    // PyRun_SimpleString("print 'hello, world!'\n"
    //                    "import mxnet as mx\n"
    //                    "print mx\n");
    Py_Finalize();
}

// int main(int argc, char *argv[]) {
//   Py_SetProgramName("mtcnn-bridge");  /* optional but recommended */
//   Py_Initialize();
//   PyRun_SimpleString("print 'hello, world!'");
//   Py_Finalize();
//   return 0;
// }
