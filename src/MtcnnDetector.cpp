#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
#include "MtcnnDetector.h"

MtcnnDetector::MtcnnDetector() {
  Py_SetProgramName((char*)"mtcnn-bridge");
  Py_Initialize();
  PyEval_InitThreads();
  import_array();

  PyObject *pModule, *pModuleName;

  pModuleName = PyString_FromString((char*)"mtcnn_runner");
  pModule = PyImport_Import(pModuleName);
  Py_DECREF(pModuleName);
  if (pModule == NULL) {
    PyErr_SetString(PyExc_TypeError, "could not import mtcnn_runner module");
    PyErr_Print();
  }
  pDetectFunc = PyObject_GetAttrString(pModule,(char*)"detect");
  if (pDetectFunc == NULL || !PyCallable_Check(pDetectFunc)) {
      PyErr_SetString(PyExc_TypeError, "detect function is not callable");
      PyErr_Print();
  }
  Py_DECREF(pModule);
}

MtcnnDetector::~MtcnnDetector() {
  Py_XDECREF(pDetectFunc);
  Py_Finalize();
}

mtcnn_detect_results MtcnnDetector::detectFaces(const cv::Mat& mat) {
    PyThreadState* state = PyEval_SaveThread();
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    mtcnn_detect_results results;
    PyObject *pValue, *pArray, *pArgs;

    npy_intp dims[] = {mat.rows, mat.cols, 3};
    pArray = PyArray_SimpleNewFromData(3, dims, NPY_UINT8, mat.data);
    pArgs = Py_BuildValue("(O)", pArray);
    pValue = PyObject_CallObject(pDetectFunc, pArgs);
    if (PyErr_Occurred()) {
      PyErr_Print();
    }
    Py_DECREF(pArgs);
    Py_DECREF(pArray);

    if (pValue == NULL) {
      Py_XDECREF(pValue);
      PyErr_SetString(PyExc_TypeError, "pValue was NULL");
      PyErr_Print();
      return results;
    }

    if (pValue == Py_None) {
      return results;
    } else if (!PyTuple_Check(pValue)) {
      PyErr_SetString(PyExc_TypeError, "expected pValue to be a tuple");
      PyErr_Print();
      return results;
    }

    PyObject *pBboxes = PyTuple_GetItem(pValue, 0);
    PyObject *pPointGroups = PyTuple_GetItem(pValue, 1);

    for (int i = 0; i < PyList_Size(pBboxes); i++) {
      PyObject *pCurBbox = PyList_GetItem(pBboxes, i);
      mtcnn_face_bbox bbox;
      bbox.x1 = PyFloat_AsDouble(PyList_GetItem(pCurBbox, 0));
      bbox.y1 = PyFloat_AsDouble(PyList_GetItem(pCurBbox, 1));
      bbox.x2 = PyFloat_AsDouble(PyList_GetItem(pCurBbox, 2));
      bbox.y2 = PyFloat_AsDouble(PyList_GetItem(pCurBbox, 3));
      bbox.score = PyFloat_AsDouble(PyList_GetItem(pCurBbox, 4));
      results.bboxes.push_back(bbox);
    }

    for (int i = 0; i < PyList_Size(pPointGroups); i++) {
      PyObject *pCurPointGroup = PyList_GetItem(pPointGroups, i);
      vector<double> points;
      for (int j = 0; j < PyList_Size(pCurPointGroup); j++) {
        points.push_back(PyFloat_AsDouble(PyList_GetItem(pCurPointGroup, j)));
      }
      results.pointGroups.push_back(points);
    }
    Py_XDECREF(pValue);
    PyGILState_Release(gstate);
    PyEval_RestoreThread(state);
    return results;
}
