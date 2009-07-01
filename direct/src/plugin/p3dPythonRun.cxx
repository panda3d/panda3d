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
#include "asyncTaskManager.h"

// There is only one P3DPythonRun object in any given process space.
// Makes the statics easier to deal with, and we don't need multiple
// instances of this think.
P3DPythonRun *P3DPythonRun::_global_ptr = NULL;

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
    nout << "unable to reset input handle\n";
  }
  if (!SetStdHandle(STD_OUTPUT_HANDLE, INVALID_HANDLE_VALUE)) {
    nout << "unable to reset input handle\n";
  }

  _pipe_read.open_read(read);
  _pipe_write.open_write(write);
#else
  _pipe_read.open_read(STDIN_FILENO);
  _pipe_write.open_write(STDOUT_FILENO);
#endif  // _WIN32

  if (!_pipe_read) {
    nout << "unable to open read pipe\n";
  }
  if (!_pipe_write) {
    nout << "unable to open write pipe\n";
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
  // First, load runp3d_frozen.pyd.  Since this is a magic frozen pyd,
  // importing it automatically makes all of its frozen contents
  // available to import as well.
  PyObject *runp3d_frozen = PyImport_ImportModule("runp3d_frozen");
  if (runp3d_frozen == NULL) {
    PyErr_Print();
    return false;
  }
  Py_DECREF(runp3d_frozen);

  // So now we can import the module itself.
  PyObject *runp3d = PyImport_ImportModule("direct.showutil.runp3d");
  if (runp3d == NULL) {
    PyErr_Print();
    return false;
  }

  // Get the pointers to the objects needed within the module.
  PyObject *AppRunner = PyObject_GetAttrString(runp3d, "AppRunner");
  if (AppRunner == NULL) {
    PyErr_Print();
    return false;
  }

  // Construct an instance of AppRunner.
  _runner = PyObject_CallFunction(AppRunner, (char*) "");
  if (_runner == NULL) {
    PyErr_Print();
    return false;
  }
  Py_DECREF(AppRunner);

  // Get the global TaskManager.
  _taskMgr = PyObject_GetAttrString(runp3d, "taskMgr");
  if (_taskMgr == NULL) {
    PyErr_Print();
    return false;
  }

  Py_DECREF(runp3d);


  // Construct a Python wrapper around our request_func() method.
  static PyMethodDef p3dpython_methods[] = {
    {"request_func", P3DPythonRun::st_request_func, METH_VARARGS,
     "Check for communications to and from the plugin host."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
  };
  PyObject *p3dpython = Py_InitModule("p3dpython", p3dpython_methods);
  if (p3dpython == NULL) {
    PyErr_Print();
    return false;
  }
  PyObject *request_func = PyObject_GetAttrString(p3dpython, "request_func");
  if (request_func == NULL) {
    PyErr_Print();
    return false;
  }

  // Now pass that func pointer back to our AppRunner instance, so it
  // can call up to us.
  PyObject *result = PyObject_CallMethod(_runner, (char*) "setRequestFunc", (char*) "O", request_func);
  if (result == NULL) {
    PyErr_Print();
    return false;
  }
  Py_DECREF(result);
  Py_DECREF(request_func);
 

  // Now add check_comm() as a task.
  _check_comm_task = new GenericAsyncTask("check_comm", st_check_comm, this);
  AsyncTaskManager *task_mgr = AsyncTaskManager::get_global_ptr();
  task_mgr->add(_check_comm_task);

  // Finally, get lost in taskMgr.run().
  nout << "calling run()\n";
  PyObject *done = PyObject_CallMethod(_taskMgr, (char*) "run", (char*) "");
  if (done == NULL) {
    PyErr_Print();
    return false;
  }
  Py_DECREF(done);
  nout << "done calling run()\n";

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
  nout << "got command: " << *doc << "\n";
  TiXmlElement *xcommand = doc->FirstChildElement("command");
  if (xcommand != NULL) {
    const char *cmd = xcommand->Attribute("cmd");
    if (cmd != NULL) {
      if (strcmp(cmd, "start_instance") == 0) {
        TiXmlElement *xinstance = xcommand->FirstChildElement("instance");
        if (xinstance != (TiXmlElement *)NULL) {
          P3DCInstance *inst = new P3DCInstance(xinstance);
          start_instance(inst, xinstance);
        }
      } else if (strcmp(cmd, "terminate_instance") == 0) {
        int id;
        if (xcommand->Attribute("id", &id)) {
          terminate_instance(id);
        }
      } else if (strcmp(cmd, "setup_window") == 0) {
        int id;
        TiXmlElement *xwparams = xcommand->FirstChildElement("wparams");
        if (xwparams != (TiXmlElement *)NULL && 
            xcommand->Attribute("id", &id)) {
          setup_window(id, xwparams);
        }
      } else if (strcmp(cmd, "exit") == 0) {
        terminate_session();
        
      } else {
        nout << "Unhandled command " << cmd << "\n";
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
//               from the plugin host in the parent process.
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus P3DPythonRun::
check_comm(GenericAsyncTask *task) {
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

  return AsyncTask::DS_cont;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::st_check_comm
//       Access: Private, Static
//  Description: This static function wrapper around check_comm is
//               necessary to add the method function to the
//               GenericAsyncTask object.
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus P3DPythonRun::
st_check_comm(GenericAsyncTask *task, void *user_data) {
  P3DPythonRun *self = (P3DPythonRun *)user_data;
  return self->check_comm(task);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::py_request_func
//       Access: Private
//  Description: This method is a special Python function that is
//               added as a callback to the AppRunner class, to allow
//               Python to upcall into this object.
////////////////////////////////////////////////////////////////////
PyObject *P3DPythonRun::
py_request_func(PyObject *args) {
  int instance_id;
  const char *request_type;
  PyObject *extra_args;
  if (!PyArg_ParseTuple(args, "isO", &instance_id, &request_type, &extra_args)) {
    return NULL;
  }

  TiXmlDocument doc;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
  TiXmlElement *xrequest = new TiXmlElement("request");
  xrequest->SetAttribute("id", instance_id);
  xrequest->SetAttribute("rtype", request_type);

  if (strcmp(request_type, "notify") == 0) {
    // A general notification to be sent directly to the instance.
    const char *message;
    if (!PyArg_ParseTuple(extra_args, "s", &message)) {
      return NULL;
    }

    xrequest->SetAttribute("message", message);
    doc.LinkEndChild(decl);
    doc.LinkEndChild(xrequest);
    nout << "sending " << doc << "\n" << flush;
    _pipe_write << doc << flush;

  } else {
    string message = string("Unsupported request type: ") + string(request_type);
    PyErr_SetString(PyExc_ValueError, message.c_str());
    return NULL;
  }

  return Py_BuildValue("");
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::st_request_func
//       Access: Private, Static
//  Description: This is the static wrapper around py_request_func.
////////////////////////////////////////////////////////////////////
PyObject *P3DPythonRun::
st_request_func(PyObject *, PyObject *args) {
  return P3DPythonRun::_global_ptr->py_request_func(args);
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

  // We have to use direct OS calls to create the thread instead of
  // Panda constructs, because it has to be an actual thread, not
  // necessarily a Panda thread (we can't use Panda's simple threads
  // implementation, because we can't get overlapped I/O on an
  // anonymous pipe in Windows).

  _read_thread_continue = true;
#ifdef _WIN32
  _read_thread = CreateThread(NULL, 0, &win_rt_thread_run, this, 0, NULL);
#else
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_create(&_read_thread, &attr, &posix_rt_thread_run, (void *)this);
  pthread_attr_destroy(&attr);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::join_read_thread
//       Access: Private
//  Description: Waits for the read thread to stop.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
join_read_thread() {
  nout << "waiting for thread\n";
  _read_thread_continue = false;
  _pipe_read.close();
  
#ifdef _WIN32
  assert(_read_thread != NULL);
  WaitForSingleObject(_read_thread, INFINITE);
  CloseHandle(_read_thread);
  _read_thread = NULL;
#else
  void *return_val;
  pthread_join(_read_thread, &return_val);
#endif
  nout << "done waiting for thread\n";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::start_instance
//       Access: Private
//  Description: Starts the indicated instance running within the
//               Python process.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
start_instance(P3DCInstance *inst, TiXmlElement *xinstance) {
  nout << "starting instance " << inst << "\n";
  _instances[inst->get_instance_id()] = inst;

  TiXmlElement *xfparams = xinstance->FirstChildElement("fparams");
  if (xfparams != (TiXmlElement *)NULL) {
    set_p3d_filename(inst, xfparams);
  }

  TiXmlElement *xwparams = xinstance->FirstChildElement("wparams");
  if (xwparams != (TiXmlElement *)NULL) {
    setup_window(inst, xwparams);
  }
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
    nout << "Can't stop instance " << id << ": not started.\n";
    return;
  }

  P3DCInstance *inst = (*ii).second;
  _instances.erase(ii);
  delete inst;

  // TODO: we don't currently have any way to stop just one instance
  // of a multi-instance session.  This will require a different
  // Python interface than ShowBase.
  terminate_session();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::set_p3d_filename
//       Access: Private
//  Description: Sets the startup filename and tokens for the
//               indicated instance.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
set_p3d_filename(P3DCInstance *inst, TiXmlElement *xfparams) {
  string p3d_filename;
  const char *p3d_filename_c = xfparams->Attribute("p3d_filename");
  if (p3d_filename_c != NULL) {
    p3d_filename = p3d_filename_c;
  }

  PyObject *token_list = PyList_New(0);

  TiXmlElement *xtoken = xfparams->FirstChildElement("token");
  while (xtoken != NULL) {
    string keyword, value;
    const char *keyword_c = xtoken->Attribute("keyword");
    if (keyword_c != NULL) {
      keyword = keyword_c;
    }

    const char *value_c = xtoken->Attribute("value");
    if (value_c != NULL) {
      value = value_c;
    }

    PyObject *tuple = Py_BuildValue("(ss)", keyword.c_str(), 
                                    value.c_str());
    PyList_Append(token_list, tuple);
    Py_DECREF(tuple);

    xtoken = xtoken->NextSiblingElement("token");
  }
  
  PyObject *result = PyObject_CallMethod
    (_runner, (char*) "setP3DFilename", (char*) "sOi", p3d_filename.c_str(),
     token_list, inst->get_instance_id());
  Py_DECREF(token_list);

  if (result == NULL) {
    PyErr_Print();
  }
  Py_XDECREF(result);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::setup_window
//       Access: Private
//  Description: Sets the window parameters for the indicated instance.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
setup_window(int id, TiXmlElement *xwparams) {
  Instances::iterator ii = _instances.find(id);
  if (ii == _instances.end()) {
    nout << "Can't setup window for " << id << ": not started.\n";
    return;
  }

  P3DCInstance *inst = (*ii).second;
  setup_window(inst, xwparams);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::setup_window
//       Access: Private
//  Description: Sets the window parameters for the indicated instance.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
setup_window(P3DCInstance *inst, TiXmlElement *xwparams) {
  string window_type;
  const char *window_type_c = xwparams->Attribute("window_type");
  if (window_type_c != NULL) {
    window_type = window_type_c;
  }

  int win_x, win_y, win_width, win_height;
  
  xwparams->Attribute("win_x", &win_x);
  xwparams->Attribute("win_y", &win_y);
  xwparams->Attribute("win_width", &win_width);
  xwparams->Attribute("win_height", &win_height);

  long parent_window_handle = 0;

#ifdef _WIN32
  int hwnd;
  if (xwparams->Attribute("parent_hwnd", &hwnd)) {
    parent_window_handle = (long)hwnd;
  }
#endif

  // TODO: direct this into the particular instance.  This will
  // require a specialized ShowBase replacement.
  PyObject *result = PyObject_CallMethod
    (_runner, (char*)"setupWindow", (char*)"siiiii", window_type.c_str(),
     win_x, win_y, win_width, win_height,
     parent_window_handle);
  if (result == NULL) {
    PyErr_Print();
  }
  Py_XDECREF(result);
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

  nout << "calling stop()\n";
  PyObject *result = PyObject_CallMethod(_taskMgr, (char*) "stop", (char*) "");
  if (result == NULL) {
    PyErr_Print();
    return;
  }
  Py_DECREF(result);
  nout << "done calling stop()\n";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::rt_thread_run
//       Access: Private
//  Description: The main function for the read thread.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
rt_thread_run() {
  nout << "thread reading.\n";
  while (_read_thread_continue) {
    TiXmlDocument *doc = new TiXmlDocument;

    _pipe_read >> *doc;
    if (!_pipe_read || _pipe_read.eof()) {
      // Some error on reading.  Abort.
      nout << "Error on reading.\n";
      _program_continue = false;
      return;
    }

    // Successfully read an XML document.
    
    // Check for one special case: the "exit" command means we shut
    // down the read thread along with everything else.
    TiXmlElement *xcommand = doc->FirstChildElement("command");
    if (xcommand != NULL) {
      const char *cmd = xcommand->Attribute("cmd");
      if (cmd != NULL) {
        if (strcmp(cmd, "exit") == 0) {
          _read_thread_continue = false;
        }
      }
    }

    // Feed the command up to the parent.
    ACQUIRE_LOCK(_commands_lock);
    _commands.push_back(doc);
    RELEASE_LOCK(_commands_lock);
  }
}

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::win_rt_thread_run
//       Access: Private, Static
//  Description: The Windows flavor of the thread callback function.
////////////////////////////////////////////////////////////////////
DWORD P3DPythonRun::
win_rt_thread_run(LPVOID data) {
  ((P3DPythonRun *)data)->rt_thread_run();
  return 0;
}
#endif

#ifndef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::posix_rt_thread_run
//       Access: Private, Static
//  Description: The Posix flavor of the thread callback function.
////////////////////////////////////////////////////////////////////
void *P3DPythonRun::
posix_rt_thread_run(void *data) {
  ((P3DPythonRun *)data)->rt_thread_run();
  return NULL;
}
#endif


////////////////////////////////////////////////////////////////////
//     Function: main
//  Description: Starts the program running.
////////////////////////////////////////////////////////////////////
int
main(int argc, char *argv[]) {
  P3DPythonRun::_global_ptr = new P3DPythonRun(argc, argv);
  
  if (!P3DPythonRun::_global_ptr->run_python()) {
    nout << "Couldn't initialize Python.\n";
    return 1;
  }
  return 0;
}
