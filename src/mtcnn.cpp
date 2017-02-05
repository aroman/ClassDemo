#include <python2.7/Python.h>
#include <stdio.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
#include "mtcnn.h"

void mxnet_detect(const cv::Mat& mat) {
    PyObject *pModule, *pDetectFunc, *pResults, *pArray;

    Py_SetProgramName("mtcnn-bridge");
    Py_Initialize();
    import_array();

    PyObject *pModuleName = PyString_FromString((char*)"mtcnn_runner");
    pModule = PyImport_Import(pModuleName);
    if (pModule == NULL) {
      PyErr_SetString(PyExc_TypeError, "could not import module");
      PyErr_Print();
      return;
    }
    pDetectFunc = PyObject_GetAttrString(pModule,(char*)"detect");
    npy_intp dims[] = {1080, 1920, 3};
    pArray = PyArray_SimpleNewFromData(3, dims, NPY_UINT8, mat.data);

    if (!PyCallable_Check(pDetectFunc)) {
        PyErr_SetString(PyExc_TypeError, "function is not callable?!");
        PyErr_Print();
        return;
    }
    PyObject *pArgList = Py_BuildValue("(O)", pArray);
    // Py_BEGIN_ALLOW_THREADS
    pResults = PyObject_CallObject(pDetectFunc, pArgList);

    // long result = PyInt_AsLong(pResults);
    // cout << "Faces detected: " << result << endl;

    // Py_END_ALLOW_THREADS
    Py_Finalize();
}
