// Filename: p3dPython.cxx
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

#include "p3dPython.h"
#include "p3dSession.h"
#include "p3dInstanceManager.h"

#include <malloc.h>

////////////////////////////////////////////////////////////////////
//     Function: P3DPython::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPython::
P3DPython(const string &python_version) {
  _python_version = python_version;
  _is_valid = false;

  _runPackedApp = NULL;
  _setupWindow = NULL;
  _run = NULL;


  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  _program_name = inst_mgr->get_dll_filename();

  size_t slash = _program_name.rfind('/');

#ifdef _WIN32
  // Windows tolerates slashes or backslashes.  Look for the rightmost
  // of either.
  size_t backslash = _program_name.rfind('\\');
  if (backslash != string::npos && (slash == string::npos || backslash > slash)) {
    slash = backslash;
  }
#endif

  //  _root_dir = _program_name.substr(0, slash);
  _root_dir = "C:/p3drun";

  _py_argc = 1;
  _py_argv = (char **)malloc(2 * sizeof(char *));
  _py_argv[0] = (char *)_program_name.c_str();
  _py_argv[1] = NULL;

  // Guess everything's OK.
  _is_valid = true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPython::Destructor
//       Access: Public
//  Description: Terminates the python by shutting down Python and
//               stopping the subprocess.
////////////////////////////////////////////////////////////////////
P3DPython::
~P3DPython() {
  if (_python_module != NULL) {
    if (_is_valid) {
      Py_XDECREF(_runPackedApp);
      Py_XDECREF(_setupWindow);
      Py_XDECREF(_run);
    }

    Py_Finalize();
    FreeLibrary(_python_module);
    _python_module = NULL;
  }    
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPython::start_session
//       Access: Public
//  Description: Starts the indicated session running within the
//               Python interpreter.  For the moment, each session
//               must contain only a single P3DInstance, which is also
//               started.
////////////////////////////////////////////////////////////////////
void P3DPython::
start_session(P3DSession *session, P3DInstance *inst) {
  assert(_is_valid);

  assert(session->_python == NULL);
  assert(session->get_python_version() == _python_version);

  // For now, only one session at a time is allowed.
  assert(_sessions.empty());

  session->_python = this;
  bool inserted = _sessions.insert(session).second;
  assert(inserted);

  _inst = inst;
  spawn_thread();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPython::terminate_session
//       Access: Public
//  Description: Removes the indicated session from the python, and
//               stops it.  It is an error if the session is not
//               already running on this python.
////////////////////////////////////////////////////////////////////
void P3DPython::
terminate_session(P3DSession *session) {
  join_thread();

  if (session->_python == this) {
    session->_python = NULL;
    _sessions.erase(session);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPython::spawn_thread
//       Access: Private
//  Description: Starts the sub-thread.  All calls to Python are made
//               within the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DPython::
spawn_thread() {
#ifdef _WIN32
  _thread = CreateThread(NULL, 0, &win_thread_func, this, 0, NULL);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPython::join_thread
//       Access: Private
//  Description: Waits for the sub-thread to stop.
////////////////////////////////////////////////////////////////////
void P3DPython::
join_thread() {
  cerr << "waiting for thread\n";
  
#ifdef _WIN32
  assert(_thread != NULL);
  WaitForSingleObject(_thread, INFINITE);
  CloseHandle(_thread);
  _thread = NULL;
#endif
  cerr << "done waiting for thread\n";
}


////////////////////////////////////////////////////////////////////
//     Function: P3DPython::tr_init_python
//       Access: Private
//  Description: Initializes the Python interpreter.  This method, and
//               all methods that interact with Python, is called only
//               within the sub-thread.  Returns true if successful,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool P3DPython::
tr_init_python() {
#ifdef _WIN32
  string python_dll_filename = _root_dir + string("/python24.dll");
  _python_module = LoadLibrary(python_dll_filename.c_str());
  if (_python_module == NULL) {
    // Couldn't load Python.
    cerr << "Unable to load " << python_dll_filename << "\n";
    return false;
  }

  Py_SetProgramName = (Py_SetProgramName_func *)GetProcAddress(_python_module, "Py_SetProgramName");  
  PySys_SetArgv = (PySys_SetArgv_func *)GetProcAddress(_python_module, "PySys_SetArgv");  
  Py_SetPythonHome = (Py_SetPythonHome_func *)GetProcAddress(_python_module, "Py_SetPythonHome");  
  Py_Initialize = (Py_Initialize_func *)GetProcAddress(_python_module, "Py_Initialize");  
  Py_Finalize = (Py_Finalize_func *)GetProcAddress(_python_module, "Py_Finalize");  
  PyEval_InitThreads = (PyEval_InitThreads_func *)GetProcAddress(_python_module, "PyEval_InitThreads");  
  PyEval_AcquireLock = (PyEval_AcquireLock_func *)GetProcAddress(_python_module, "PyEval_AcquireLock");  
  PyEval_ReleaseLock = (PyEval_ReleaseLock_func *)GetProcAddress(_python_module, "PyEval_ReleaseLock");  
  PyRun_SimpleString = (PyRun_SimpleString_func *)GetProcAddress(_python_module, "PyRun_SimpleString");  
  PyErr_Print = (PyErr_Print_func *)GetProcAddress(_python_module, "PyErr_Print");  
  Py_XDECREF = (Py_XDECREF_func *)GetProcAddress(_python_module, "Py_DecRef");  
  PyImport_ImportModule = (PyImport_ImportModule_func *)GetProcAddress(_python_module, "PyImport_ImportModule");  
  PyObject_SetAttrString = (PyObject_SetAttrString_func *)GetProcAddress(_python_module, "PyObject_SetAttrString");  
  PyObject_GetAttrString = (PyObject_GetAttrString_func *)GetProcAddress(_python_module, "PyObject_GetAttrString");  
  Py_BuildValue = (Py_BuildValue_func *)GetProcAddress(_python_module, "Py_BuildValue");  
  PyObject_CallFunction = (PyObject_CallFunction_func *)GetProcAddress(_python_module, "PyObject_CallFunction");  
  
#endif  // _WIN32

  if (Py_SetProgramName == NULL ||
      PySys_SetArgv == NULL ||
      Py_SetPythonHome == NULL ||
      Py_Initialize == NULL ||
      Py_Finalize == NULL ||
      PyEval_InitThreads == NULL ||
      PyEval_AcquireLock == NULL ||
      PyEval_ReleaseLock == NULL ||
      PyRun_SimpleString == NULL ||
      PyErr_Print == NULL ||
      Py_XDECREF == NULL ||
      PyImport_ImportModule == NULL || 
      PyObject_SetAttrString == NULL || 
      PyObject_GetAttrString == NULL || 
      Py_BuildValue == NULL || 
      PyObject_CallFunction == NULL) {
    // Couldn't get all of the needed Python functions for some reason.
    cerr << "Py_SetProgramName = " << Py_SetProgramName << "\n"
         << "PySys_SetArgv = " << PySys_SetArgv << "\n"
         << "Py_SetPythonHome = " << Py_SetPythonHome << "\n"
         << "Py_Initialize = " << Py_Initialize << "\n"
         << "Py_Finalize = " << Py_Finalize << "\n"
         << "PyEval_InitThreads = " << PyEval_InitThreads << "\n"
         << "PyEval_AcquireLock = " << PyEval_AcquireLock << "\n"
         << "PyEval_ReleaseLock = " << PyEval_ReleaseLock << "\n"
         << "PyRun_SimpleString = " << PyRun_SimpleString << "\n"
         << "PyErr_Print = " << PyErr_Print << "\n"
         << "Py_XDECREF = " << Py_XDECREF << "\n"
         << "PyImport_ImportModule = " << PyImport_ImportModule << "\n"
         << "PyObject_SetAttrString = " << PyObject_SetAttrString << "\n"
         << "PyObject_GetAttrString = " << PyObject_GetAttrString << "\n"
         << "Py_BuildValue = " << Py_BuildValue << "\n"
         << "PyObject_CallFunction = " << PyObject_CallFunction << "\n"
         << "\n";
    FreeLibrary(_python_module);
    _python_module = NULL;
    return false;
  }

  // All right, initialize Python.
  Py_SetProgramName((char *)_program_name.c_str());
  Py_Initialize();
  Py_SetPythonHome((char *)_root_dir.c_str());
  PySys_SetArgv(_py_argc, _py_argv);

  // Set sys.path appropriately.
  PyObject *sys = PyImport_ImportModule("sys");
  if (sys == NULL) {
    PyErr_Print();
    return false;
  }

  PyObject *path = Py_BuildValue("[s]", _root_dir.c_str());
  PyObject_SetAttrString(sys, "path", path);
  Py_XDECREF(path);
  Py_XDECREF(sys);

  // Now load runappmf.pyd.
  PyObject *runappmf = PyImport_ImportModule("runappmf");
  if (runappmf == NULL) {
    PyErr_Print();
    return false;
  }
  Py_XDECREF(runappmf);

  // And get the pointers to the functions needed within the module.
  PyObject *appmf = PyImport_ImportModule("direct.showbase.RunAppMF");
  if (appmf == NULL) {
    PyErr_Print();
    return false;
  }
  _runPackedApp = PyObject_GetAttrString(appmf, "runPackedApp");
  if (_runPackedApp == NULL) {
    PyErr_Print();
    return false;
  }
  _setupWindow = PyObject_GetAttrString(appmf, "setupWindow");
  if (_setupWindow == NULL) {
    PyErr_Print();
    return false;
  }
  _run = PyObject_GetAttrString(appmf, "run");
  if (_run == NULL) {
    PyErr_Print();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPython::tr_thread_run
//       Access: Private
//  Description: The main function for the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DPython::
tr_thread_run() {
  bool valid = true;
  if (!tr_init_python()) {
    // Couldn't get Python going.
    valid = false;
  }

  if (valid) {
    string window_type;
    switch (_inst->_window_type) {
    case P3D_WT_embedded:
      window_type = "embedded";
      break;
    case P3D_WT_toplevel:
      window_type = "toplevel";
      break;
    case P3D_WT_fullscreen:
      window_type = "fullscreen";
      break;
    case P3D_WT_hidden:
      window_type = "hidden";
      break;
    }

    PyObject *result = PyObject_CallFunction
      (_setupWindow, "siiiii", window_type.c_str(), 
       _inst->_win_x, _inst->_win_y,
       _inst->_win_width, _inst->_win_height,
#ifdef _WIN32
       (int)(_inst->_parent_window._hwnd)
#endif
       );
    if (result == NULL) {
      PyErr_Print();
      valid = false;
    }
    Py_XDECREF(result);
  }

  if (valid) {
    PyObject *result = PyObject_CallFunction(_runPackedApp, "[s]", _inst->get_p3d_filename().c_str());
    if (result == NULL) {
      PyErr_Print();
      valid = false;
    }
    Py_XDECREF(result);
  }

  _is_valid = valid;
  // Maybe signal the parent that we're ready?

  if (valid) {
    // Call run().  This function won't return until the p3d app
    // exits.
    PyObject *result = PyObject_CallFunction(_run, "");
    if (result == NULL) {
      PyErr_Print();
    }
    Py_XDECREF(result);
  }

  // The instance has finished.
  P3D_request *request = new P3D_request;
  request->_instance = _inst;
  request->_request_type = P3D_RT_stop;
  _inst->add_request(request);
}

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DPython::win_thread_func
//       Access: Private, Static
//  Description: The Windows flavor of the thread callback function.
////////////////////////////////////////////////////////////////////
DWORD P3DPython::
win_thread_func(LPVOID data) {
  ((P3DPython *)data)->tr_thread_run();
  return 0;
}
#endif
