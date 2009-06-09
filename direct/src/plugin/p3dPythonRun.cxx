// Filename: p3dPythonRun.cxx
// Created by:  drose (05Jun09)
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

#include "p3dPythonRun.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPythonRun::
P3DPythonRun(int argc, char *argv[]) {
  _read_thread_continue = false;
  _program_continue = true;
  INIT_LOCK(_commands_lock);

  _program_name = argv[0];
  _py_argc = 1;
  _py_argv = (char **)malloc(2 * sizeof(char *));
  _py_argv[0] = argv[0];
  _py_argv[1] = NULL;

  // Initialize Python.  It appears to be important to do this before
  // we open the pipe streams and spawn the thread, below.
  Py_SetProgramName((char *)_program_name.c_str());
  Py_Initialize();
  PySys_SetArgv(_py_argc, _py_argv);

  // Open the pipe streams with the input and output handles from the
  // parent.
#ifdef _WIN32
  HANDLE read = GetStdHandle(STD_INPUT_HANDLE);
  HANDLE write = GetStdHandle(STD_OUTPUT_HANDLE);
  if (!SetStdHandle(STD_INPUT_HANDLE, INVALID_HANDLE_VALUE)) {
    cerr << "unable to reset input handle\n";
  }
  if (!SetStdHandle(STD_OUTPUT_HANDLE, INVALID_HANDLE_VALUE)) {
    cerr << "unable to reset input handle\n";
  }

  _pipe_read.open_read(read);
  _pipe_write.open_write(write);
#endif  // _WIN32
  if (!_pipe_read) {
    cerr << "unable to open read pipe\n";
  }
  if (!_pipe_write) {
    cerr << "unable to open write pipe\n";
  }

  spawn_read_thread();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPythonRun::
~P3DPythonRun() {
  Py_Finalize();

  join_read_thread();
  DESTROY_LOCK(_commands_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::run_python
//       Access: Public
//  Description: Runs the embedded Python process.  This method does
//               not return until the plugin is ready to exit.
////////////////////////////////////////////////////////////////////
bool P3DPythonRun::
run_python() {
  // Now load runappmf.pyd.
  PyObject *runappmf = PyImport_ImportModule("runappmf");
  if (runappmf == NULL) {
    PyErr_Print();
    return false;
  }
  Py_DECREF(runappmf);

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
  _exit = PyObject_GetAttrString(appmf, "exit");
  if (_exit == NULL) {
    PyErr_Print();
    return false;
  }
  _setupWindow = PyObject_GetAttrString(appmf, "setupWindow");
  if (_setupWindow == NULL) {
    PyErr_Print();
    return false;
  }
  Py_DECREF(appmf);

  // Construct a Python wrapper around our check_comm() method.

  static PyMethodDef p3dpython_methods[] = {
    {"check_comm", P3DPythonRun::py_check_comm, METH_VARARGS,
     "Check for communications to and from the plugin host."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
  };
  PyObject *p3dpython = Py_InitModule("p3dpython", p3dpython_methods);
  if (p3dpython == NULL) {
    PyErr_Print();
    return false;
  }

  PyObject *check_comm = PyObject_GetAttrString(p3dpython, "check_comm");
  if (check_comm == NULL) {
    PyErr_Print();
    return false;
  }

  // Now add check_comm() as a Python task.
  PyObject *add_check_comm = PyObject_GetAttrString(appmf, "add_check_comm");
  if (add_check_comm == NULL) {
    PyErr_Print();
    return false;
  }

  PyObject *task = PyObject_CallFunction
    (add_check_comm, "Ol", check_comm, (long)this);
  if (task == NULL) {
    PyErr_Print();
    return false;
  }
  Py_DECREF(task);
  Py_DECREF(add_check_comm);
  Py_DECREF(check_comm);

  // Finally, get lost in taskMgr.run().

  PyObject *taskMgr = PyObject_GetAttrString(appmf, "taskMgr");
  if (taskMgr == NULL) {
    PyErr_Print();
    return false;
  }
  cerr << "calling run()\n";
  PyObject *done = PyObject_CallMethod(taskMgr, "run", "");
  if (done == NULL) {
    PyErr_Print();
    return false;
  }
  Py_DECREF(done);
  cerr << "done calling run()\n";

  Py_DECREF(taskMgr);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::handle_command
//       Access: Private
//  Description: Handles a command received from the plugin host, via
//               an XML syntax on the wire.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
handle_command(TiXmlDocument *doc) {
  cerr << "got command: " << *doc << "\n";
  TiXmlElement *xcommand = doc->FirstChildElement("command");
  if (xcommand != NULL) {
    const char *cmd = xcommand->Attribute("cmd");
    if (cmd != NULL) {
      if (strcmp(cmd, "start_instance") == 0) {
        TiXmlElement *xinstance = xcommand->FirstChildElement("instance");
        if (xinstance != (TiXmlElement *)NULL) {
          P3DCInstance *inst = new P3DCInstance(xinstance);
          start_instance(inst);
        }
      } else if (strcmp(cmd, "terminate_instance") == 0) {
        int id;
        if (xcommand->Attribute("id", &id)) {
          terminate_instance(id);
        }
        
      } else {
        cerr << "Unhandled command " << cmd << "\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::check_comm
//       Access: Private
//  Description: This method is added to the Python task manager (via
//               py_check_comm, below) so that it gets a call every
//               frame.  Its job is to check for commands received
//               from, and requests to be delivered to, the plugin
//               host in the parent process.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
check_comm() {
  ACQUIRE_LOCK(_commands_lock);
  while (!_commands.empty()) {
    TiXmlDocument *doc = _commands.front();
    _commands.pop_front();
    assert(_commands.size() < 10);
    RELEASE_LOCK(_commands_lock);
    handle_command(doc);
    delete doc;
    ACQUIRE_LOCK(_commands_lock);
  }

  if (!_program_continue) {
    // The low-level thread detected an error, for instance pipe
    // closed.  We should exit gracefully.
    terminate_session();
  }

  RELEASE_LOCK(_commands_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::py_check_comm
//       Access: Private, Static
//  Description: This method is added as a Python task function on the
//               task manager.  It is the Python wrapper around the
//               check_comm method.
////////////////////////////////////////////////////////////////////
PyObject *P3DPythonRun::
py_check_comm(PyObject *, PyObject *args) {
  long this_int;
  PyObject *task; 
  if (!PyArg_ParseTuple(args, "lO:check_comm", &this_int, &task)) {
    return NULL;
  }

  P3DPythonRun *self = (P3DPythonRun *)(void *)this_int;
  self->check_comm();

  return PyObject_GetAttrString(task, "cont");
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::spawn_read_thread
//       Access: Private
//  Description: Starts the read thread.  This thread is responsible
//               for reading the standard input socket for XML
//               commands and storing them in the _commands queue.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
spawn_read_thread() {
  assert(!_read_thread_continue);

  _read_thread_continue = true;
#ifdef _WIN32
  _read_thread = CreateThread(NULL, 0, &win_rt_thread_run, this, 0, NULL);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::join_read_thread
//       Access: Private
//  Description: Waits for the read thread to stop.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
join_read_thread() {
  cerr << "waiting for thread\n";
  _read_thread_continue = false;
  
#ifdef _WIN32
  assert(_read_thread != NULL);
  WaitForSingleObject(_read_thread, INFINITE);
  CloseHandle(_read_thread);
  _read_thread = NULL;
#endif
  cerr << "done waiting for thread\n";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::start_instance
//       Access: Private
//  Description: Starts the indicated instance running within the
//               Python process.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
start_instance(P3DCInstance *inst) {
  cerr << "starting instance " << inst->get_p3d_filename() << "\n";
  _instances[inst->get_instance_id()] = inst;

  string window_type;
  switch (inst->_window_type) {
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
     inst->_win_x, inst->_win_y,
     inst->_win_width, inst->_win_height,
#ifdef _WIN32
     (int)(inst->_parent_window._hwnd)
#endif
     );
  if (result == NULL) {
    PyErr_Print();
  }
  Py_XDECREF(result);
  
  result = PyObject_CallFunction(_runPackedApp, "[s]", inst->get_p3d_filename().c_str());
  if (result == NULL) {
    PyErr_Print();
  }
  Py_XDECREF(result);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::terminate_instance
//       Access: Private
//  Description: Stops the instance with the indicated id.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
terminate_instance(int id) {
  Instances::iterator ii = _instances.find(id);
  if (ii == _instances.end()) {
    cerr << "Can't stop instance " << id << ": not started.\n";
    return;
  }

  P3DCInstance *inst = (*ii).second;
  _instances.erase(ii);
  delete inst;

  // TODO: we don't currently have any way to stop just one instance
  // of a multi-instance session.  We could maybe close its window or
  // something.
  terminate_session();
}


////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::terminate_session
//       Access: Private
//  Description: Stops all currently-running instances.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
terminate_session() {
  Instances::iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    P3DCInstance *inst = (*ii).second;
    delete inst;
  }
  _instances.clear();

  PyObject *result = PyObject_CallFunction(_exit, "");
  if (result == NULL) {
    PyErr_Print();
  }
  Py_XDECREF(result);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::rt_thread_run
//       Access: Private
//  Description: The main function for the read thread.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
rt_thread_run() {
  cerr << "thread reading.\n";
  while (_read_thread_continue) {
    TiXmlDocument *doc = new TiXmlDocument;

    _pipe_read >> *doc;
    if (!_pipe_read || _pipe_read.eof()) {
      // Some error on reading.  Abort.
      cerr << "Error on reading.\n";
      _program_continue = false;
      return;
    }

    // Successfully read an XML document.  Feed it to the parent.
    ACQUIRE_LOCK(_commands_lock);
    _commands.push_back(doc);
    RELEASE_LOCK(_commands_lock);
  }
}

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DPython::win_rt_thread_run
//       Access: Private, Static
//  Description: The Windows flavor of the thread callback function.
////////////////////////////////////////////////////////////////////
DWORD P3DPythonRun::
win_rt_thread_run(LPVOID data) {
  ((P3DPythonRun *)data)->rt_thread_run();
  return 0;
}
#endif


////////////////////////////////////////////////////////////////////
//     Function: main
//  Description: Starts the program running.
////////////////////////////////////////////////////////////////////
int
main(int argc, char *argv[]) {
  P3DPythonRun run(argc, argv);
  
  if (!run.run_python()) {
    cerr << "Couldn't initialize Python.\n";
    return 1;
  }
  return 0;
}
