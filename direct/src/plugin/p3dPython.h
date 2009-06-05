// Filename: p3dPython.h
// Created by:  drose (04Jun09)
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

#ifndef P3DPYTHON_H
#define P3DPYTHON_H

#include "p3d_plugin_common.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DPython
// Description : Corresponds to a single instance of the Python
//               interpreter.  Since Python is single-threaded and
//               global-namespace, there can only be one Python
//               instance in a given address space.
//
//               Note that, due to Python's "NewInterpreter"
//               mechanism, it *might* be possible to have multiple
//               virtual interpreters within a single Python instance.
//               This will require some work to integrate successfully
//               with Panda, though, so it is not currently
//               implemented.
////////////////////////////////////////////////////////////////////
class P3DPython {
public:
  P3DPython(const string &python_version);
  ~P3DPython();

  INLINE const string &get_python_version() const;

  void start_session(P3DSession *session, P3DInstance *inst);
  void terminate_session(P3DSession *session);

  INLINE int get_num_sessions() const;
  INLINE bool is_valid() const;

private:
  void spawn_thread();
  void join_thread();

private:
  // Methods that run only within the sub-thread.
  bool tr_init_python();
  void tr_thread_run();

#ifdef _WIN32
  static DWORD WINAPI win_thread_func(LPVOID data);
#endif

private:
  string _python_version;
  bool _is_valid;

  string _root_dir;
  string _program_name;

  int _py_argc;
  char **_py_argv;

  typedef set<P3DSession *> Sessions;
  Sessions _sessions;

  // Temporary.
  P3DInstance *_inst;

#ifdef _WIN32
  HMODULE _python_module;
  HANDLE _thread;
#endif

  typedef struct _object PyObject;

  PyObject *_runPackedApp;
  PyObject *_setupWindow;
  PyObject *_run;

  // Pointers to dynamically-loaded Python functions.
  typedef void Py_SetProgramName_func(char *name);
  typedef void PySys_SetArgv_func(int argc, char **argv);
  typedef void Py_SetPythonHome_func(char *name);
  typedef void Py_Initialize_func(void);
  typedef void Py_Finalize_func(void);
  typedef void PyEval_InitThreads_func(void);
  typedef void PyEval_AcquireLock_func(void);
  typedef void PyEval_ReleaseLock_func(void);
  typedef int PyRun_SimpleString_func(const char *command);
  typedef void PyErr_Print_func(void);
  typedef void Py_XDECREF_func(PyObject *o);
  typedef PyObject *PyImport_ImportModule_func(const char *name);
  typedef int PyObject_SetAttrString_func(PyObject *o, const char *attr_name, PyObject *v);
  typedef PyObject *PyObject_GetAttrString_func(PyObject *o, const char *attr_name);
  typedef PyObject *Py_BuildValue_func(const char *format, ...);
  typedef PyObject *PyObject_CallFunction_func(PyObject *callable, char *format, ...);

  Py_SetProgramName_func *Py_SetProgramName;
  PySys_SetArgv_func *PySys_SetArgv;
  Py_SetPythonHome_func *Py_SetPythonHome;
  Py_Initialize_func *Py_Initialize;
  Py_Finalize_func *Py_Finalize;
  PyEval_InitThreads_func *PyEval_InitThreads;
  PyEval_AcquireLock_func *PyEval_AcquireLock;
  PyEval_ReleaseLock_func *PyEval_ReleaseLock;
  PyRun_SimpleString_func *PyRun_SimpleString;
  PyErr_Print_func *PyErr_Print;
  Py_XDECREF_func *Py_XDECREF;
  PyImport_ImportModule_func *PyImport_ImportModule;
  PyObject_SetAttrString_func *PyObject_SetAttrString;
  PyObject_GetAttrString_func *PyObject_GetAttrString;
  Py_BuildValue_func *Py_BuildValue;
  PyObject_CallFunction_func *PyObject_CallFunction;
};

#include "p3dPython.I"

#endif
