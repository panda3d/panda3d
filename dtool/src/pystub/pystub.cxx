// Filename: pystub.cxx
// Created by:  drose (09Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pystub.h"

extern "C" {
  EXPCL_PYSTUB int PyArg_Parse(...);
  EXPCL_PYSTUB int PyArg_ParseTuple(...);
  EXPCL_PYSTUB int PyArg_ParseTupleAndKeywords(...);
  EXPCL_PYSTUB int PyArg_UnpackTuple(...);
  EXPCL_PYSTUB int PyBool_FromLong(...);
  EXPCL_PYSTUB int PyBuffer_Release(...);
  EXPCL_PYSTUB int PyBytes_AsString(...);
  EXPCL_PYSTUB int PyBytes_AsStringAndSize(...);
  EXPCL_PYSTUB int PyBytes_FromStringAndSize(...);
  EXPCL_PYSTUB int PyBytes_Size(...);
  EXPCL_PYSTUB int PyCFunction_New(...);
  EXPCL_PYSTUB int PyCFunction_NewEx(...);
  EXPCL_PYSTUB int PyCallable_Check(...);
  EXPCL_PYSTUB int PyDict_DelItem(...);
  EXPCL_PYSTUB int PyDict_DelItemString(...);
  EXPCL_PYSTUB int PyDict_GetItem(...);
  EXPCL_PYSTUB int PyDict_GetItemString(...);
  EXPCL_PYSTUB int PyDict_New(...);
  EXPCL_PYSTUB int PyDict_SetItem(...);
  EXPCL_PYSTUB int PyDict_SetItemString(...);
  EXPCL_PYSTUB int PyDict_Size(...);
  EXPCL_PYSTUB int PyDict_Type(...);
  EXPCL_PYSTUB int PyErr_Clear(...);
  EXPCL_PYSTUB int PyErr_ExceptionMatches(...);
  EXPCL_PYSTUB int PyErr_Fetch(...);
  EXPCL_PYSTUB int PyErr_Format(...);
  EXPCL_PYSTUB int PyErr_NoMemory(...);
  EXPCL_PYSTUB int PyErr_Occurred(...);
  EXPCL_PYSTUB int PyErr_Print(...);
  EXPCL_PYSTUB int PyErr_Restore(...);
  EXPCL_PYSTUB int PyErr_SetString(...);
  EXPCL_PYSTUB int PyErr_Warn(...);
  EXPCL_PYSTUB int PyErr_WarnEx(...);
  EXPCL_PYSTUB int PyEval_CallFunction(...);
  EXPCL_PYSTUB int PyEval_CallObjectWithKeywords(...);
  EXPCL_PYSTUB int PyEval_InitThreads(...);
  EXPCL_PYSTUB int PyEval_RestoreThread(...);
  EXPCL_PYSTUB int PyEval_SaveThread(...);
  EXPCL_PYSTUB int PyFloat_AsDouble(...);
  EXPCL_PYSTUB int PyFloat_FromDouble(...);
  EXPCL_PYSTUB int PyFloat_Type(...);
  EXPCL_PYSTUB int PyGen_Check(...);
  EXPCL_PYSTUB int PyGen_Type(...);
  EXPCL_PYSTUB int PyGILState_Ensure(...);
  EXPCL_PYSTUB int PyGILState_Release(...);
  EXPCL_PYSTUB int PyImport_GetModuleDict(...);
  EXPCL_PYSTUB int PyInt_AsLong(...);
  EXPCL_PYSTUB int PyInt_AsSsize_t(...);
  EXPCL_PYSTUB int PyInt_FromLong(...);
  EXPCL_PYSTUB int PyInt_FromSize_t(...);
  EXPCL_PYSTUB int PyInt_Type(...);
  EXPCL_PYSTUB int PyList_Append(...);
  EXPCL_PYSTUB int PyList_AsTuple(...);
  EXPCL_PYSTUB int PyList_GetItem(...);
  EXPCL_PYSTUB int PyList_New(...);
  EXPCL_PYSTUB int PyList_SetItem(...);
  EXPCL_PYSTUB int PyList_Type(...);
  EXPCL_PYSTUB int PyLong_AsLong(...);
  EXPCL_PYSTUB int PyLong_AsLongLong(...);
  EXPCL_PYSTUB int PyLong_AsSsize_t(...);
  EXPCL_PYSTUB int PyLong_AsUnsignedLong(...);
  EXPCL_PYSTUB int PyLong_AsUnsignedLongLong(...);
  EXPCL_PYSTUB int PyLong_FromLong(...);
  EXPCL_PYSTUB int PyLong_FromLongLong(...);
  EXPCL_PYSTUB int PyLong_FromSize_t(...);
  EXPCL_PYSTUB int PyLong_FromUnsignedLong(...);
  EXPCL_PYSTUB int PyLong_FromUnsignedLongLong(...);
  EXPCL_PYSTUB int PyLong_Type(...);
  EXPCL_PYSTUB int PyMapping_GetItemString(...);
  EXPCL_PYSTUB int PyMem_Free(...);
  EXPCL_PYSTUB int PyMemoryView_FromObject(...);
  EXPCL_PYSTUB int PyModule_AddIntConstant(...);
  EXPCL_PYSTUB int PyModule_AddObject(...);
  EXPCL_PYSTUB int PyModule_AddStringConstant(...);
  EXPCL_PYSTUB int PyModule_Create2(...);
  EXPCL_PYSTUB int PyNumber_Check(...);
  EXPCL_PYSTUB int PyNumber_Float(...);
  EXPCL_PYSTUB int PyNumber_Int(...);
  EXPCL_PYSTUB int PyNumber_Long(...);
  EXPCL_PYSTUB int PyObject_ASCII(...);
  EXPCL_PYSTUB int PyObject_Call(...);
  EXPCL_PYSTUB int PyObject_CallFunction(...);
  EXPCL_PYSTUB int PyObject_CallFunctionObjArgs(...);
  EXPCL_PYSTUB int PyObject_CallMethod(...);
  EXPCL_PYSTUB int PyObject_CallMethodObjArgs(...);
  EXPCL_PYSTUB int PyObject_CallObject(...);
  EXPCL_PYSTUB int PyObject_Cmp(...);
  EXPCL_PYSTUB int PyObject_Compare(...);
  EXPCL_PYSTUB int PyObject_Free(...);
  EXPCL_PYSTUB int PyObject_GenericGetAttr(...);
  EXPCL_PYSTUB int PyObject_GenericSetAttr(...);
  EXPCL_PYSTUB int PyObject_GetAttrString(...);
  EXPCL_PYSTUB int PyObject_GetBuffer(...);
  EXPCL_PYSTUB int PyObject_HasAttrString(...);
  EXPCL_PYSTUB int PyObject_IsInstance(...);
  EXPCL_PYSTUB int PyObject_IsTrue(...);
  EXPCL_PYSTUB int PyObject_Repr(...);
  EXPCL_PYSTUB int PyObject_RichCompareBool(...);
  EXPCL_PYSTUB int PyObject_SetAttrString(...);
  EXPCL_PYSTUB int PyObject_Str(...);
  EXPCL_PYSTUB int PyObject_Type(...);
  EXPCL_PYSTUB int PySequence_Check(...);
  EXPCL_PYSTUB int PySequence_Fast(...);
  EXPCL_PYSTUB int PySequence_GetItem(...);
  EXPCL_PYSTUB int PySequence_Size(...);
  EXPCL_PYSTUB int PySequence_Tuple(...);
  EXPCL_PYSTUB int PyString_AsString(...);
  EXPCL_PYSTUB int PyString_AsStringAndSize(...);
  EXPCL_PYSTUB int PyString_FromFormat(...);
  EXPCL_PYSTUB int PyString_FromString(...);
  EXPCL_PYSTUB int PyString_FromStringAndSize(...);
  EXPCL_PYSTUB int PyString_InternFromString(...);
  EXPCL_PYSTUB int PyString_InternInPlace(...);
  EXPCL_PYSTUB int PyString_Size(...);
  EXPCL_PYSTUB int PyString_Type(...);
  EXPCL_PYSTUB int PySys_GetObject(...);
  EXPCL_PYSTUB int PyThreadState_Clear(...);
  EXPCL_PYSTUB int PyThreadState_Delete(...);
  EXPCL_PYSTUB int PyThreadState_Get(...);
  EXPCL_PYSTUB int PyThreadState_New(...);
  EXPCL_PYSTUB int PyThreadState_Swap(...);
  EXPCL_PYSTUB int PyTuple_GetItem(...);
  EXPCL_PYSTUB int PyTuple_New(...);
  EXPCL_PYSTUB int PyTuple_Pack(...);
  EXPCL_PYSTUB int PyTuple_Size(...);
  EXPCL_PYSTUB int PyTuple_Type(...);
  EXPCL_PYSTUB int PyType_GenericAlloc(...);
  EXPCL_PYSTUB int PyType_IsSubtype(...);
  EXPCL_PYSTUB int PyType_Ready(...);
  EXPCL_PYSTUB int PyUnicodeUCS2_FromStringAndSize(...);
  EXPCL_PYSTUB int PyUnicodeUCS2_FromWideChar(...);
  EXPCL_PYSTUB int PyUnicodeUCS2_AsWideChar(...);
  EXPCL_PYSTUB int PyUnicodeUCS2_GetSize(...);
  EXPCL_PYSTUB int PyUnicodeUCS4_FromStringAndSize(...);
  EXPCL_PYSTUB int PyUnicodeUCS4_FromWideChar(...);
  EXPCL_PYSTUB int PyUnicodeUCS4_AsWideChar(...);
  EXPCL_PYSTUB int PyUnicodeUCS4_GetSize(...);
  EXPCL_PYSTUB int PyUnicode_AsUTF8(...);
  EXPCL_PYSTUB int PyUnicode_AsUTF8AndSize(...);
  EXPCL_PYSTUB int PyUnicode_AsWideChar(...);
  EXPCL_PYSTUB int PyUnicode_AsWideCharString(...);
  EXPCL_PYSTUB int PyUnicode_FromFormat(...);
  EXPCL_PYSTUB int PyUnicode_FromString(...);
  EXPCL_PYSTUB int PyUnicode_FromStringAndSize(...);
  EXPCL_PYSTUB int PyUnicode_FromWideChar(...);
  EXPCL_PYSTUB int PyUnicode_GetSize(...);
  EXPCL_PYSTUB int PyUnicode_InternFromString(...);
  EXPCL_PYSTUB int PyUnicode_InternInPlace(...);
  EXPCL_PYSTUB int PyUnicode_Type(...);
  EXPCL_PYSTUB int Py_BuildValue(...);
  EXPCL_PYSTUB int Py_InitModule4(...);
  EXPCL_PYSTUB int Py_InitModule4_64(...);
  EXPCL_PYSTUB int Py_InitModule4TraceRefs(...);
  EXPCL_PYSTUB int _PyArg_ParseTuple_SizeT(...);
  EXPCL_PYSTUB int _PyArg_ParseTupleAndKeywords_SizeT(...);
  EXPCL_PYSTUB int _PyArg_Parse_SizeT(...);
  EXPCL_PYSTUB int _PyObject_CallFunction_SizeT(...);
  EXPCL_PYSTUB int _PyObject_CallMethod_SizeT(...);
  EXPCL_PYSTUB int _PyObject_DebugFree(...);
  EXPCL_PYSTUB int _PyObject_Del(...);
  EXPCL_PYSTUB int _Py_BuildValue_SizeT(...);
  EXPCL_PYSTUB int _Py_Dealloc(...);
  EXPCL_PYSTUB int _Py_NegativeRefcount(...);
  EXPCL_PYSTUB int _Py_RefTotal(...);

  EXPCL_PYSTUB void Py_Initialize();
  EXPCL_PYSTUB int Py_IsInitialized();

  EXPCL_PYSTUB extern void *PyExc_AssertionError;
  EXPCL_PYSTUB extern void *PyExc_AttributeError;
  EXPCL_PYSTUB extern void *PyExc_BufferError;
  EXPCL_PYSTUB extern void *PyExc_ConnectionError;
  EXPCL_PYSTUB extern void *PyExc_Exception;
  EXPCL_PYSTUB extern void *PyExc_FutureWarning;
  EXPCL_PYSTUB extern void *PyExc_IndexError;
  EXPCL_PYSTUB extern void *PyExc_RuntimeError;
  EXPCL_PYSTUB extern void *PyExc_StandardError;
  EXPCL_PYSTUB extern void *PyExc_StopIteration;
  EXPCL_PYSTUB extern void *PyExc_SystemExit;
  EXPCL_PYSTUB extern void *PyExc_TypeError;
  EXPCL_PYSTUB extern void *PyExc_ValueError;
  EXPCL_PYSTUB extern void *_PyThreadState_Current;
  EXPCL_PYSTUB extern void *_Py_FalseStruct;
  EXPCL_PYSTUB extern void *_Py_NoneStruct;
  EXPCL_PYSTUB extern void *_Py_NotImplementedStruct;
  EXPCL_PYSTUB extern void *_Py_TrueStruct;
  EXPCL_PYSTUB extern void *_Py_ZeroStruct;
};


int PyArg_Parse(...) { return 0; };
int PyArg_ParseTuple(...) { return 0; }
int PyArg_ParseTupleAndKeywords(...) { return 0; }
int PyArg_UnpackTuple(...) { return 0; };
int PyBool_FromLong(...) { return 0; }
int PyBuffer_Release(...) { return 0; }
int PyBytes_AsString(...) { return 0; }
int PyBytes_AsStringAndSize(...) { return 0; }
int PyBytes_FromStringAndSize(...) { return 0; }
int PyBytes_Size(...) { return 0; }
int PyCFunction_New(...) { return 0; };
int PyCFunction_NewEx(...) { return 0; };
int PyCallable_Check(...) { return 0; }
int PyDict_DelItem(...) { return 0; }
int PyDict_DelItemString(...) { return 0; }
int PyDict_GetItem(...) { return 0; }
int PyDict_GetItemString(...) { return 0; }
int PyDict_New(...) { return 0; };
int PyDict_SetItem(...) { return 0; };
int PyDict_SetItemString(...) { return 0; };
int PyDict_Size(...){ return 0; }
int PyDict_Type(...) { return 0; };
int PyErr_Clear(...) { return 0; };
int PyErr_ExceptionMatches(...) { return 0; };
int PyErr_Fetch(...) { return 0; }
int PyErr_Format(...) { return 0; };
int PyErr_NoMemory(...) { return 0; }
int PyErr_Occurred(...) { return 0; }
int PyErr_Print(...) { return 0; }
int PyErr_Restore(...) { return 0; }
int PyErr_SetString(...) { return 0; }
int PyErr_Warn(...) { return 0; }
int PyErr_WarnEx(...) { return 0; }
int PyEval_CallFunction(...) { return 0; }
int PyEval_CallObjectWithKeywords(...) { return 0; }
int PyEval_InitThreads(...) { return 0; }
int PyEval_RestoreThread(...) { return 0; }
int PyEval_SaveThread(...) { return 0; }
int PyFloat_AsDouble(...) { return 0; }
int PyFloat_FromDouble(...) { return 0; }
int PyFloat_Type(...) { return 0; }
int PyGen_Check(...) { return 0; }
int PyGen_Type(...) { return 0; }
int PyGILState_Ensure(...) { return 0; }
int PyGILState_Release(...) { return 0; }
int PyImport_GetModuleDict(...) { return 0; }
int PyInt_AsLong(...) { return 0; }
int PyInt_AsSsize_t(...) { return 0; }
int PyInt_FromLong(...) { return 0; }
int PyInt_FromSize_t(...) { return 0; }
int PyInt_Type(...) { return 0; }
int PyList_Append(...) { return 0; }
int PyList_AsTuple(...) { return 0; }
int PyList_GetItem(...) { return 0; }
int PyList_New(...) { return 0; }
int PyList_SetItem(...) { return 0; }
int PyList_Type(...) { return 0; }
int PyLong_AsLong(...) { return 0; }
int PyLong_AsLongLong(...) { return 0; }
int PyLong_AsSsize_t(...) { return 0; }
int PyLong_AsUnsignedLong(...) { return 0; }
int PyLong_AsUnsignedLongLong(...) { return 0; }
int PyLong_FromLong(...) { return 0; }
int PyLong_FromLongLong(...) { return 0; }
int PyLong_FromSize_t(...) { return 0; }
int PyLong_FromUnsignedLong(...) { return 0; }
int PyLong_FromUnsignedLongLong(...) { return 0; }
int PyLong_Type(...) { return 0; }
int PyMapping_GetItemString(...) { return 0; }
int PyMem_Free(...) { return 0; }
int PyMemoryView_FromObject(...) { return 0; }
int PyModule_AddIntConstant(...) { return 0; };
int PyModule_AddObject(...) { return 0; };
int PyModule_AddStringConstant(...) { return 0; };
int PyModule_Create2(...) { return 0; };
int PyNumber_Check(...) { return 0; }
int PyNumber_Float(...) { return 0; }
int PyNumber_Int(...) { return 0; }
int PyNumber_Long(...) { return 0; }
int PyObject_ASCII(...) { return 0; }
int PyObject_Call(...) { return 0; }
int PyObject_CallFunction(...) { return 0; }
int PyObject_CallFunctionObjArgs(...) { return 0; }
int PyObject_CallMethod(...) { return 0; }
int PyObject_CallMethodObjArgs(...) { return 0; }
int PyObject_CallObject(...) { return 0; }
int PyObject_Cmp(...) { return 0; }
int PyObject_Compare(...) { return 0; }
int PyObject_Free(...) { return 0; }
int PyObject_GenericGetAttr(...) { return 0; };
int PyObject_GenericSetAttr(...) { return 0; };
int PyObject_GetAttrString(...) { return 0; }
int PyObject_GetBuffer(...) { return 0; }
int PyObject_HasAttrString(...) { return 0; }
int PyObject_IsInstance(...) { return 0; }
int PyObject_IsTrue(...) { return 0; }
int PyObject_Repr(...) { return 0; }
int PyObject_RichCompareBool(...) { return 0; }
int PyObject_SetAttrString(...) { return 0; }
int PyObject_Str(...) { return 0; }
int PyObject_Type(...) { return 0; }
int PySequence_Check(...) { return 0; }
int PySequence_Fast(...) { return 0; }
int PySequence_GetItem(...) { return 0; }
int PySequence_Size(...) { return 0; }
int PySequence_Tuple(...) { return 0; }
int PyString_AsString(...) { return 0; }
int PyString_AsStringAndSize(...) { return 0; }
int PyString_FromFormat(...) { return 0; }
int PyString_FromString(...) { return 0; }
int PyString_FromStringAndSize(...) { return 0; }
int PyString_InternFromString(...) { return 0; }
int PyString_InternInPlace(...) { return 0; }
int PyString_Size(...) { return 0; }
int PyString_Type(...) { return 0; }
int PySys_GetObject(...) { return 0; }
int PyThreadState_Clear(...) { return 0; }
int PyThreadState_Delete(...) { return 0; }
int PyThreadState_Get(...) { return 0; }
int PyThreadState_New(...) { return 0; }
int PyThreadState_Swap(...) { return 0; }
int PyTuple_GetItem(...) { return 0; }
int PyTuple_New(...) { return 0; }
int PyTuple_Pack(...) { return 0; }
int PyTuple_Size(...) { return 0; };
int PyTuple_Type(...) { return 0; };
int PyType_GenericAlloc(...) { return 0; };
int PyType_IsSubtype(...) { return 0; }
int PyType_Ready(...) { return 0; };
int PyUnicodeUCS2_FromStringAndSize(...) { return 0; }
int PyUnicodeUCS2_FromWideChar(...) { return 0; }
int PyUnicodeUCS2_AsWideChar(...) { return 0; }
int PyUnicodeUCS2_GetSize(...) { return 0; }
int PyUnicodeUCS4_FromStringAndSize(...) { return 0; }
int PyUnicodeUCS4_FromWideChar(...) { return 0; }
int PyUnicodeUCS4_AsWideChar(...) { return 0; }
int PyUnicodeUCS4_GetSize(...) { return 0; }
int PyUnicode_AsUTF8(...) { return 0; }
int PyUnicode_AsUTF8AndSize(...) { return 0; }
int PyUnicode_AsWideChar(...) { return 0; }
int PyUnicode_AsWideCharString(...) { return 0; }
int PyUnicode_FromFormat(...) { return 0; }
int PyUnicode_FromString(...) { return 0; }
int PyUnicode_FromStringAndSize(...) { return 0; }
int PyUnicode_FromWideChar(...) { return 0; }
int PyUnicode_GetSize(...) { return 0; }
int PyUnicode_InternFromString(...) { return 0; }
int PyUnicode_InternInPlace(...) { return 0; }
int PyUnicode_Type(...) { return 0; }
int Py_BuildValue(...) { return 0; }
int Py_InitModule4(...) { return 0; }
int Py_InitModule4_64(...) { return 0; }
int Py_InitModule4TraceRefs(...) { return 0; };
int _PyArg_ParseTuple_SizeT(...) { return 0; };
int _PyArg_ParseTupleAndKeywords_SizeT(...) { return 0; };
int _PyArg_Parse_SizeT(...) { return 0; };
int _PyObject_CallFunction_SizeT(...) { return 0; };
int _PyObject_CallMethod_SizeT(...) { return 0; };
int _PyObject_DebugFree(...) { return 0; };
int _PyObject_Del(...) { return 0; };
int _Py_BuildValue_SizeT(...) { return 0; };
int _Py_Dealloc(...) { return 0; };
int _Py_NegativeRefcount(...) { return 0; };
int _Py_RefTotal(...) { return 0; };

// We actually might call this one.
void Py_Initialize() {
}
int Py_IsInitialized() {
  return 0;
}


void *PyExc_AssertionError = (void *)NULL;
void *PyExc_AttributeError = (void *)NULL;
void *PyExc_BufferError = (void *)NULL;
void *PyExc_ConnectionError = (void *)NULL;
void *PyExc_Exception = (void *)NULL;
void *PyExc_FutureWarning = (void *)NULL;
void *PyExc_IndexError = (void *)NULL;
void *PyExc_RuntimeError = (void *)NULL;
void *PyExc_StandardError = (void *)NULL;
void *PyExc_StopIteration = (void *)NULL;
void *PyExc_SystemExit = (void *)NULL;
void *PyExc_TypeError = (void *)NULL;
void *PyExc_ValueError = (void *)NULL;
void *_PyThreadState_Current = (void *)NULL;
void *_Py_FalseStruct = (void *)NULL;
void *_Py_NoneStruct = (void *)NULL;
void *_Py_NotImplementedStruct = (void *)NULL;
void *_Py_TrueStruct = (void *)NULL;
void *_Py_ZeroStruct = (void *)NULL;


void
pystub() {
}
