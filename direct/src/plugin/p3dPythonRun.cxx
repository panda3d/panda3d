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
#include "binaryXml.h"
#include "multifile.h"
#include "virtualFileSystem.h"
#include "nativeWindowHandle.h"

#include "py_panda.h"

// There is only one P3DPythonRun object in any given process space.
// Makes the statics easier to deal with, and we don't need multiple
// instances of this thing.
P3DPythonRun *P3DPythonRun::_global_ptr = NULL;

TypeHandle P3DPythonRun::P3DWindowHandle::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
P3DPythonRun::
P3DPythonRun(const char *program_name, const char *archive_file,
             FHandle input_handle, FHandle output_handle,
             const char *log_pathname, bool interactive_console) {
  P3DWindowHandle::init_type();
  init_xml();

  _read_thread_continue = false;
  _program_continue = true;
  _session_terminated = false;
  _taskMgr = NULL;

  INIT_LOCK(_commands_lock);
  INIT_THREAD(_read_thread);

  _session_id = 0;
  _next_sent_id = 0;

  _interactive_console = interactive_console;

  if (program_name != NULL) {
    _program_name = program_name;
  }
  if (archive_file != NULL) {
    _archive_file = Filename::from_os_specific(archive_file);
  }

  _py_argc = 1;
  _py_argv = (char **)malloc(2 * sizeof(char *));

  _py_argv[0] = (char *)_program_name.c_str();
  _py_argv[1] = NULL;

#ifdef NDEBUG
  // In OPTIMIZE 4 compilation mode, run Python in optimized mode too.
  extern int Py_OptimizeFlag;
  Py_OptimizeFlag = 2;
#endif

  // Turn off the automatic load of site.py at startup.
  extern int Py_NoSiteFlag;
  Py_NoSiteFlag = 1;

  // Initialize Python.  It appears to be important to do this before
  // we open the pipe streams and spawn the thread, below.
  Py_SetProgramName((char *)_program_name.c_str());
  Py_Initialize();
  PyEval_InitThreads();
  PySys_SetArgv(_py_argc, _py_argv);

  // Open the error output before we do too much more.
  if (log_pathname != NULL && *log_pathname != '\0') {
    Filename f = Filename::from_os_specific(log_pathname);
    f.set_text();
    if (f.open_write(_error_log)) {
      // Set up the indicated error log as the Notify output.
      _error_log.setf(ios::unitbuf);
      Notify::ptr()->set_ostream_ptr(&_error_log, false);
    }
  }

  // Open the pipe streams with the input and output handles from the
  // parent.
  _pipe_read.open_read(input_handle);
  _pipe_write.open_write(output_handle);

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
  terminate_session();

  // Close the write pipe, so the parent process will terminate us.
  _pipe_write.close();

  // Shut down Python and Panda.
  Py_Finalize();

  join_read_thread();
  DESTROY_LOCK(_commands_lock);

  // Restore the notify stream in case it tries to write to anything
  // else after our shutdown.
  Notify::ptr()->set_ostream_ptr(&cerr, false);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::run_python
//       Access: Public
//  Description: Runs the embedded Python process.  This method does
//               not return until the plugin is ready to exit.
////////////////////////////////////////////////////////////////////
bool P3DPythonRun::
run_python() {
#if defined(_WIN32) && defined(USE_DEBUG_PYTHON)
  // On Windows, in a debug build, we have to preload sys.dll_suffix =
  // "_d", so that the Panda DLL preloader can import the correct
  // filenames.
  PyRun_SimpleString("import sys; sys.dll_suffix = '_d'");
#endif

  Filename dir = _archive_file.get_dirname();

  // We'll need to synthesize a 'panda3d' module before loading
  // VFSImporter.  We could simply freeze it, but Python has a bug
  // setting __path__ of frozen modules properly.
  PyObject *panda3d_module = PyImport_AddModule("panda3d");
  if (panda3d_module == NULL) {
    nout << "Failed to create panda3d module:\n";
    PyErr_Print();
    return false;
  }

  // Set the __path__ such that it can find panda3d/core.pyd, etc.
  Filename panda3d_dir(dir, "panda3d");
  string dir_str = panda3d_dir.to_os_specific();
  PyObject *panda3d_dict = PyModule_GetDict(panda3d_module);
  PyObject *panda3d_path = Py_BuildValue("[s#]", dir_str.data(), dir_str.length());
  PyDict_SetItemString(panda3d_dict, "__path__", panda3d_path);
  Py_DECREF(panda3d_path);

  // Now we can load _vfsimporter.pyd.  Since this is a magic frozen
  // pyd, importing it automatically makes all of its frozen contents
  // available to import as well.
  PyObject *vfsimporter = PyImport_ImportModule("_vfsimporter");
  if (vfsimporter == NULL) {
    nout << "Failed to import _vfsimporter:\n";
    PyErr_Print();
    return false;
  }
  Py_DECREF(vfsimporter);

  // And now we can import the VFSImporter module that was so defined.
  PyObject *vfsimporter_module = PyImport_ImportModule("VFSImporter");
  if (vfsimporter_module == NULL) {
    nout << "Failed to import VFSImporter:\n";
    PyErr_Print();
    return false;
  }

  // And register the VFSImporter.
  PyObject *result = PyObject_CallMethod(vfsimporter_module, (char *)"register", (char *)"");
  if (result == NULL) {
    nout << "Failed to call VFSImporter.register():\n";
    PyErr_Print();
    return false;
  }
  Py_DECREF(result);
  Py_DECREF(vfsimporter_module);

  // Now, the VFSImporter has been registered, which means we can
  // start importing the rest of the Python modules, which are all
  // defined in the multifile.  First, we need to mount the multifile
  // into the VFS.
  PT(Multifile) mf = new Multifile;
  if (!mf->open_read(_archive_file)) {
    nout << "Could not read " << _archive_file << "\n";
    return false;
  }
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->mount(mf, dir, VirtualFileSystem::MF_read_only)) {
    nout << "Could not mount " << _archive_file << "\n";
    return false;
  }

  // And finally, we can import the startup module.
  PyObject *app_runner_module = PyImport_ImportModule("direct.p3d.AppRunner");
  if (app_runner_module == NULL) {
    nout << "Failed to import direct.p3d.AppRunner\n";
    PyErr_Print();
    return false;
  }

  // Get the pointers to the objects needed within the module.
  PyObject *app_runner_class = PyObject_GetAttrString(app_runner_module, "AppRunner");
  if (app_runner_class == NULL) {
    nout << "Failed to get AppRunner class\n";
    PyErr_Print();
    return false;
  }

  // Construct an instance of AppRunner.
  _runner = PyObject_CallFunction(app_runner_class, (char *)"");
  if (_runner == NULL) {
    nout << "Failed to construct AppRunner instance\n";
    PyErr_Print();
    return false;
  }
  Py_DECREF(app_runner_class);

  // Get the UndefinedObject class.
  _undefined_object_class = PyObject_GetAttrString(app_runner_module, "UndefinedObject");
  if (_undefined_object_class == NULL) {
    PyErr_Print();
    return false;
  }

  // And the "Undefined" instance.
  _undefined = PyObject_GetAttrString(app_runner_module, "Undefined");
  if (_undefined == NULL) {
    PyErr_Print();
    return false;
  }

  // Get the ConcreteStruct class.
  _concrete_struct_class = PyObject_GetAttrString(app_runner_module, "ConcreteStruct");
  if (_concrete_struct_class == NULL) {
    PyErr_Print();
    return false;
  }

  // Get the BrowserObject class.
  _browser_object_class = PyObject_GetAttrString(app_runner_module, "BrowserObject");
  if (_browser_object_class == NULL) {
    PyErr_Print();
    return false;
  }

  // Get the global TaskManager.
  _taskMgr = PyObject_GetAttrString(app_runner_module, "taskMgr");
  if (_taskMgr == NULL) {
    PyErr_Print();
    return false;
  }

  Py_DECREF(app_runner_module);


  // Construct a Python wrapper around our methods we need to expose to Python.
  static PyMethodDef p3dpython_methods[] = {
    { "check_comm", P3DPythonRun::st_check_comm, METH_VARARGS,
      "Poll for communications from the parent process" },
    { "request_func", P3DPythonRun::st_request_func, METH_VARARGS,
      "Send an asynchronous request to the plugin host" },
    { NULL, NULL, 0, NULL }        /* Sentinel */
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
  result = PyObject_CallMethod(_runner, (char *)"setRequestFunc", (char *)"N", request_func);
  if (result == NULL) {
    PyErr_Print();
    return false;
  }
  Py_DECREF(result);

  // Now add check_comm() as a task.  It can be a threaded task, but
  // this does mean that application programmers will have to be alert
  // to asynchronous calls coming in from JavaScript.  We'll put it on
  // its own task chain so the application programmer can decide how
  // it should be.
  AsyncTaskManager *task_mgr = AsyncTaskManager::get_global_ptr();
  PT(AsyncTaskChain) chain = task_mgr->make_task_chain("JavaScript");

  // The default is not threaded (num_threads == 0), but if the app
  // programmer decides to enable threads, the default is TP_low
  // priority.
  chain->set_thread_priority(TP_low);

  PyObject *check_comm = PyObject_GetAttrString(p3dpython, "check_comm");
  if (check_comm == NULL) {
    PyErr_Print();
    return false;
  }

  // Add it to the task manager.  We do this instead of constructing a
  // PythonTask because linking p3dpython with core.pyd is problematic.
  result = PyObject_CallMethod(_taskMgr, (char *)"add", (char *)"Ns", check_comm, "check_comm");
  if (result == NULL) {
    PyErr_Print();
    return false;
  }
  Py_DECREF(result);

  // Finally, get lost in AppRunner.run() (which is really a call to
  // taskMgr.run()).
  PyObject *done = PyObject_CallMethod(_runner, (char *)"run", (char *)"");
  if (done == NULL) {
    // An uncaught application exception, and not handled by
    // appRunner.exceptionHandler.
    PyErr_Print();

    if (_interactive_console) {
      // Give an interactive user a chance to explore the exception.
      run_interactive_console();
      return true;
    }

    // We're done.
    return false;
  }

  // A normal exit from the taskManager.  We're presumably done.
  Py_DECREF(done);

  if (_interactive_console) {
    run_interactive_console();
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::set_window_open
//       Access: Public
//  Description: Called from low-level Panda (via the P3DWindowHandle
//               object) when a child window has been attached or
//               detached from the browser window.  This triggers the
//               "onwindowattach" and "onwindowdetach" JavaScript
//               notifications.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
set_window_open(P3DCInstance *inst, bool is_open) {
  TiXmlDocument doc;
  TiXmlElement *xrequest = new TiXmlElement("request");
  xrequest->SetAttribute("instance_id", inst->get_instance_id());
  xrequest->SetAttribute("rtype", "notify");
  if (is_open) {
    xrequest->SetAttribute("message", "onwindowattach");
  } else {
    xrequest->SetAttribute("message", "onwindowdetach");
  }
  doc.LinkEndChild(xrequest);

  write_xml(_pipe_write, &doc, nout);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::request_keyboard_focus
//       Access: Public
//  Description: Called from low-level Panda (via the P3DWindowHandle
//               object) when its main window requires keyboard focus,
//               but is unable to assign it directly.  This is
//               particularly necessary under Windows Vista, where a
//               child window of the browser is specifically
//               disallowed from being given keyboard focus.
//
//               This sends a notify request up to the parent process,
//               to ask the parent to manage keyboard events by proxy,
//               and send them back down to Panda, again via the
//               P3DWindowHandle.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
request_keyboard_focus(P3DCInstance *inst) {
  TiXmlDocument doc;
  TiXmlElement *xrequest = new TiXmlElement("request");
  xrequest->SetAttribute("instance_id", inst->get_instance_id());
  xrequest->SetAttribute("rtype", "notify");
  xrequest->SetAttribute("message", "keyboardfocus");
  doc.LinkEndChild(xrequest);

  write_xml(_pipe_write, &doc, nout);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::run_interactive_console
//       Access: Private
//  Description: Gives the user a chance to type interactive Python
//               commands, for easy development of a p3d application.
//               This method is only called if "interactive_console=1"
//               is set as a web token, and "allow_python_dev" is set
//               within the application itself.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
run_interactive_console() {
#ifdef _WIN32
  // Make sure that control-C support is enabled for the interpreter.
  SetConsoleCtrlHandler(NULL, false);
#endif

  // The "readline" module makes the Python prompt friendlier, with
  // command history and everything.  Simply importing it is
  // sufficient.
  PyObject *readline_module = PyImport_ImportModule("readline");
  if (readline_module == NULL) {
    // But, the module might not exist on certain platforms.  If not,
    // no sweat.
    PyErr_Clear();
  } else {
    Py_DECREF(readline_module);
  }

  PyRun_InteractiveLoop(stdin, "<stdin>");
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::handle_command
//       Access: Private
//  Description: Handles a command received from the plugin host, via
//               an XML syntax on the wire.  Ownership of the XML
//               document object is passed into this method.
//
//               It's important *not* to be holding _commands_lock
//               when calling this method.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
handle_command(TiXmlDocument *doc) {
  TiXmlElement *xcommand = doc->FirstChildElement("command");
  if (xcommand != NULL) {
    bool needs_response = false;
    int want_response_id;
    if (xcommand->QueryIntAttribute("want_response_id", &want_response_id) == TIXML_SUCCESS) {
      // This command will be waiting for a response.
      needs_response = true;
    }

    const char *cmd = xcommand->Attribute("cmd");
    if (cmd != NULL) {
      if (strcmp(cmd, "init") == 0) {
        assert(!needs_response);

        // The only purpose of the "init" command is to send us a
        // unique session ID, which in fact we don't do much with.
        xcommand->Attribute("session_id", &_session_id);

        // We do use it to initiate our object id sequence with a
        // number at least a little bit distinct from other sessions,
        // though.  No technical requirement that we do this, but it
        // does make debugging the logs a bit easier.
        _next_sent_id = _session_id * 1000;

        PyObject *obj = PyObject_CallMethod(_runner, (char*)"setSessionId", (char *)"i", _session_id);
        Py_XDECREF(obj);

      } else if (strcmp(cmd, "start_instance") == 0) {
        assert(!needs_response);
        TiXmlElement *xinstance = xcommand->FirstChildElement("instance");
        if (xinstance != (TiXmlElement *)NULL) {
          P3DCInstance *inst = new P3DCInstance(xinstance);
          start_instance(inst, xinstance);
        }

      } else if (strcmp(cmd, "terminate_instance") == 0) {
        assert(!needs_response);
        int instance_id;
        if (xcommand->QueryIntAttribute("instance_id", &instance_id) == TIXML_SUCCESS) {
          terminate_instance(instance_id);
        }

      } else if (strcmp(cmd, "setup_window") == 0) {
        assert(!needs_response);
        int instance_id;
        TiXmlElement *xwparams = xcommand->FirstChildElement("wparams");
        if (xwparams != (TiXmlElement *)NULL &&
            xcommand->QueryIntAttribute("instance_id", &instance_id) == TIXML_SUCCESS) {
          setup_window(instance_id, xwparams);
        }

      } else if (strcmp(cmd, "windows_message") == 0) {
        assert(!needs_response);
        // This is a special message that we use to proxy keyboard
        // events from the parent process down into Panda, a necessary
        // hack on Vista.
        int instance_id = 0, msg = 0, wparam = 0, lparam = 0;
        xcommand->Attribute("instance_id", &instance_id);
        xcommand->Attribute("msg", &msg);
        xcommand->Attribute("wparam", &wparam);
        xcommand->Attribute("lparam", &lparam);
        send_windows_message(instance_id, msg, wparam, lparam);

      } else if (strcmp(cmd, "exit") == 0) {
        assert(!needs_response);
        terminate_session();

      } else if (strcmp(cmd, "pyobj") == 0) {
        // Manipulate or query a python object.
        handle_pyobj_command(xcommand, needs_response, want_response_id);

      } else if (strcmp(cmd, "script_response") == 0) {
        // Response from a script request.  In this case, we just
        // store it away instead of processing it immediately.

        MutexHolder holder(_responses_lock);
        _responses.push_back(doc);

        // And now we must return out, instead of deleting the
        // document at the bottom of this method.
        return;

      } else if (strcmp(cmd, "drop_pyobj") == 0) {
        int object_id;
        if (xcommand->QueryIntAttribute("object_id", &object_id) == TIXML_SUCCESS) {
          SentObjects::iterator si = _sent_objects.find(object_id);
          if (si != _sent_objects.end()) {
            PyObject *obj = (*si).second;
            Py_DECREF(obj);
            _sent_objects.erase(si);
          }
        }

      } else {
        nout << "Unhandled command " << cmd << "\n";
        if (needs_response) {
          // Better send a response.
          TiXmlDocument doc;
          TiXmlElement *xresponse = new TiXmlElement("response");
          xresponse->SetAttribute("response_id", want_response_id);
          doc.LinkEndChild(xresponse);
          write_xml(_pipe_write, &doc, nout);
        }
      }
    }
  }

  delete doc;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::handle_pyobj_command
//       Access: Private
//  Description: Handles the pyobj command, which queries or modifies
//               a Python object from the browser scripts.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
handle_pyobj_command(TiXmlElement *xcommand, bool needs_response,
                     int want_response_id) {
  TiXmlDocument doc;
  TiXmlElement *xresponse = new TiXmlElement("response");
  xresponse->SetAttribute("response_id", want_response_id);
  doc.LinkEndChild(xresponse);

  const char *op = xcommand->Attribute("op");
  if (op != NULL && !PyErr_Occurred()) {
    if (strcmp(op, "get_panda_script_object") == 0) {
      // Get Panda's toplevel Python object.
      PyObject *obj = PyObject_CallMethod(_runner, (char*)"getPandaScriptObject", (char *)"");
      if (obj != NULL) {
        xresponse->LinkEndChild(pyobj_to_xml(obj));
        Py_DECREF(obj);
      }

    } else if (strcmp(op, "set_browser_script_object") == 0) {
      // Set the Browser's toplevel window object.
      PyObject *obj;
      TiXmlElement *xvalue = xcommand->FirstChildElement("value");
      if (xvalue != NULL) {
        obj = xml_to_pyobj(xvalue);
      } else {
        obj = Py_None;
        Py_INCREF(obj);
      }

      Py_XDECREF(PyObject_CallMethod
        (_runner, (char *)"setBrowserScriptObject", (char *)"N", obj));

    } else if (strcmp(op, "call") == 0) {
      // Call the named method on the indicated object, or the object
      // itself if method_name isn't given.
      TiXmlElement *xobject = xcommand->FirstChildElement("object");
      if (xobject != NULL) {
        PyObject *obj = xml_to_pyobj(xobject);

        const char *method_name = xcommand->Attribute("method_name");

        // Build up a list of params.
        PyObject *list = PyList_New(0);

        TiXmlElement *xchild = xcommand->FirstChildElement("value");
        while (xchild != NULL) {
          PyObject *child = xml_to_pyobj(xchild);
          PyList_Append(list, child);
          Py_DECREF(child);
          xchild = xchild->NextSiblingElement("value");
        }

        // Convert the list to a tuple for the call.
        PyObject *params = PyList_AsTuple(list);
        Py_DECREF(list);

        // Now call the method.
        PyObject *result = NULL;
        if (method_name == NULL) {
          // No method name; call the object directly.
          result = PyObject_CallObject(obj, params);

          // Several special-case "method" names.
        } else if (strcmp(method_name, "__bool__") == 0) {
          result = PyBool_FromLong(PyObject_IsTrue(obj));

        } else if (strcmp(method_name, "__int__") == 0) {
          result = PyNumber_Int(obj);

        } else if (strcmp(method_name, "__float__") == 0) {
          result = PyNumber_Float(obj);

        } else if (strcmp(method_name, "__repr__") == 0) {
          result = PyObject_Repr(obj);

        } else if (strcmp(method_name, "__str__") == 0 ||
                   strcmp(method_name, "toString") == 0) {
          result = PyObject_Str(obj);

        } else if (strcmp(method_name, "__set_property__") == 0) {
          // We call these methods __set_property__ et al instead of
          // __setattr__ et al, because they do not precisely
          // duplicate the Python semantics.
          char *property_name;
          PyObject *value;
          if (PyArg_ParseTuple(params, "sO", &property_name, &value)) {
            bool success = false;

            // If it already exists as an attribute, update it there.
            if (PyObject_HasAttrString(obj, property_name)) {
              if (PyObject_SetAttrString(obj, property_name, value) != -1) {
                success = true;
              } else {
                PyErr_Clear();
              }
            }

            // If the object supports the mapping protocol, store it
            // in the object's dictionary.
            if (!success && PyMapping_Check(obj)) {
              if (PyMapping_SetItemString(obj, property_name, value) != -1) {
                success = true;
              } else {
                PyErr_Clear();
              }
            }

            // Finally, try to store it on the object.
            if (!success) {
              if (PyObject_SetAttrString(obj, property_name, value) != -1) {
                success = true;
              } else {
                PyErr_Clear();
              }
            }

            if (success) {
              result = Py_True;
            } else {
              result = Py_False;
            }
            Py_INCREF(result);
          }

        } else if (strcmp(method_name, "__del_property__") == 0) {
          char *property_name;
          if (PyArg_ParseTuple(params, "s", &property_name)) {
            bool success = false;

            if (PyObject_HasAttrString(obj, property_name)) {
              if (PyObject_DelAttrString(obj, property_name) != -1) {
                success = true;
              } else {
                PyErr_Clear();
              }
            }

            if (!success) {
              if (PyObject_DelItemString(obj, property_name) != -1) {
                success = true;
              } else {
                PyErr_Clear();
              }
            }

            if (success) {
              result = Py_True;
            } else {
              result = Py_False;
            }
            Py_INCREF(result);
          }

        } else if (strcmp(method_name, "__get_property__") == 0) {
          char *property_name;
          if (PyArg_ParseTuple(params, "s", &property_name)) {
            bool success = false;

            if (PyObject_HasAttrString(obj, property_name)) {
              result = PyObject_GetAttrString(obj, property_name);
              if (result != NULL) {
                success = true;
              } else {
                PyErr_Clear();
              }
            }

            if (!success) {
              if (PyMapping_HasKeyString(obj, property_name)) {
                result = PyMapping_GetItemString(obj, property_name);
                if (result != NULL) {
                  success = true;
                } else {
                  PyErr_Clear();
                }
              }
            }

            if (!success) {
              result = NULL;
            }
          }

        } else if (strcmp(method_name, "__has_method__") == 0) {
          char *property_name;
          result = Py_False;
          if (PyArg_ParseTuple(params, "s", &property_name)) {
            if (*property_name) {
              // Check for a callable method
              if (PyObject_HasAttrString(obj, property_name)) {
                PyObject *prop = PyObject_GetAttrString(obj, property_name);
                if (PyCallable_Check(prop)) {
                  result = Py_True;
                }
                Py_DECREF(prop);
              }
            } else {
              // Check for the default method
              if (PyCallable_Check(obj)) {
                result = Py_True;
              }
            }
          }
          Py_INCREF(result);

        } else {
          // Not a special-case name.  Call the named method.
          PyObject *method = PyObject_GetAttrString(obj, (char *)method_name);
          if (method != NULL) {
            result = PyObject_CallObject(method, params);
            Py_DECREF(method);
          }
        }
        Py_DECREF(params);

        // Feed the return value back through the XML pipe to the
        // caller.
        if (result != NULL) {
          xresponse->LinkEndChild(pyobj_to_xml(result));
          Py_DECREF(result);
        } else {
          // Print the Python error message if there was one.
          PyErr_Print();
        }

        Py_DECREF(obj);
      }
    }
  }

  if (needs_response) {
    write_xml(_pipe_write, &doc, nout);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::check_comm
//       Access: Private
//  Description: This method is added to the task manager (via
//               st_check_comm, below) so that it gets a call every
//               frame.  Its job is to check for commands received
//               from the plugin host in the parent process.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
check_comm() {
  //  nout << ":";
  ACQUIRE_LOCK(_commands_lock);
  while (!_commands.empty()) {
    TiXmlDocument *doc = _commands.front();
    _commands.pop_front();
    RELEASE_LOCK(_commands_lock);
    handle_command(doc);
    if (_session_terminated) {
      return;
    }
    ACQUIRE_LOCK(_commands_lock);
  }
  RELEASE_LOCK(_commands_lock);

  if (!_program_continue) {
    // The low-level thread detected an error, for instance pipe
    // closed.  We should exit gracefully.
    terminate_session();
    return;
  }

  // Sleep to yield the timeslice, but only if we're not running in
  // the main thread.
  if (Thread::get_current_thread() != Thread::get_main_thread()) {
    Thread::sleep(0.001);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::st_check_comm
//       Access: Private, Static
//  Description: This is a static Python wrapper around py_check_comm,
//               needed to add the function to a PythonTask.
////////////////////////////////////////////////////////////////////
PyObject *P3DPythonRun::
st_check_comm(PyObject *, PyObject *args) {
  P3DPythonRun::_global_ptr->check_comm();
  return Py_BuildValue("i", AsyncTask::DS_cont);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::wait_script_response
//       Access: Private
//  Description: This method is similar to check_comm(), above, but
//               instead of handling all events, it waits for a
//               specific script_response ID to come back from the
//               browser, and leaves all other events in the queue.
////////////////////////////////////////////////////////////////////
TiXmlDocument *P3DPythonRun::
wait_script_response(int response_id) {
  //  nout << "waiting script_response " << response_id << "\n";
  while (true) {
    Commands::iterator ci;

    // First, walk through the _commands queue to see if there's
    // anything that needs immediate processing.
    ACQUIRE_LOCK(_commands_lock);
    for (ci = _commands.begin(); ci != _commands.end(); ++ci) {
      TiXmlDocument *doc = (*ci);

      TiXmlElement *xcommand = doc->FirstChildElement("command");
      if (xcommand != NULL) {
        const char *cmd = xcommand->Attribute("cmd");
        if ((cmd != NULL && strcmp(cmd, "script_response") == 0) ||
            xcommand->Attribute("want_response_id") != NULL) {

          // This is either a response, or it's a command that will
          // want a response itself.  In either case we should handle
          // it right away.  ("handling" a response means moving it to
          // the _responses queue.)
          _commands.erase(ci);
          RELEASE_LOCK(_commands_lock);
          handle_command(doc);
          if (_session_terminated) {
            return NULL;
          }
          ACQUIRE_LOCK(_commands_lock);
          break;
        }
      }
    }
    RELEASE_LOCK(_commands_lock);

    // Now, walk through the _responses queue to look for the
    // particular response we're waiting for.
    _responses_lock.acquire();
    for (ci = _responses.begin(); ci != _responses.end(); ++ci) {
      TiXmlDocument *doc = (*ci);

      TiXmlElement *xcommand = doc->FirstChildElement("command");
      assert(xcommand != NULL);
      const char *cmd = xcommand->Attribute("cmd");
      assert(cmd != NULL && strcmp(cmd, "script_response") == 0);

      int unique_id;
      if (xcommand->QueryIntAttribute("unique_id", &unique_id) == TIXML_SUCCESS) {
        if (unique_id == response_id) {
          // This is the response we were waiting for.
          _responses.erase(ci);
          _responses_lock.release();
          //          nout << "got script_response " << unique_id << "\n";
          return doc;
        }
      }
    }
    _responses_lock.release();

    if (!_program_continue) {
      terminate_session();
      return NULL;
    }

#ifdef _WIN32
    // Make sure we process the Windows event loop while we're
    // waiting, or everything that depends on Windows messages will
    // starve.

    // We appear to be best off with just a single PeekMessage() call
    // here; the full message pump seems to cause problems.
    MSG msg;
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE | PM_NOYIELD);
#endif  // _WIN32

    //    nout << ".";

    // It hasn't shown up yet.  Give the sub-thread a chance to
    // process the input and append it to the queue.
    Thread::force_yield();
  }
  assert(false);
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

  if (strcmp(request_type, "wait_script_response") == 0) {
    // This is a special case.  Instead of generating a new request,
    // this means to wait for a particular script_response to come in
    // on the wire.
    int response_id;
    if (!PyArg_ParseTuple(extra_args, "i", &response_id)) {
      return NULL;
    }

    TiXmlDocument *doc = wait_script_response(response_id);
    if (_session_terminated) {
      return Py_BuildValue("");
    }
    assert(doc != NULL);
    TiXmlElement *xcommand = doc->FirstChildElement("command");
    assert(xcommand != NULL);
    TiXmlElement *xvalue = xcommand->FirstChildElement("value");

    PyObject *value = NULL;
    if (xvalue != NULL) {
      value = xml_to_pyobj(xvalue);
    } else {
      // An absence of a <value> element is an exception.  We will
      // return NULL from this function, but first set the error
      // condition.
      PyErr_SetString(PyExc_EnvironmentError, "Error on script call");
    }

    delete doc;
    return value;
  }

  TiXmlDocument doc;
  TiXmlElement *xrequest = new TiXmlElement("request");
  xrequest->SetAttribute("instance_id", instance_id);
  xrequest->SetAttribute("rtype", request_type);
  doc.LinkEndChild(xrequest);

  if (strcmp(request_type, "notify") == 0) {
    // A general notification to be sent directly to the instance.
    const char *message;
    if (!PyArg_ParseTuple(extra_args, "s", &message)) {
      return NULL;
    }

    xrequest->SetAttribute("message", message);
    write_xml(_pipe_write, &doc, nout);

  } else if (strcmp(request_type, "script") == 0) {
    // Meddling with a scripting variable on the browser side.
    const char *operation;
    PyObject *object;
    const char *property_name;
    PyObject *value;
    int needs_response;
    int unique_id;
    if (!PyArg_ParseTuple(extra_args, "sOsOii",
                          &operation, &object, &property_name, &value,
                          &needs_response, &unique_id)) {
      return NULL;
    }

    xrequest->SetAttribute("operation", operation);
    xrequest->SetAttribute("property_name", property_name);
    xrequest->SetAttribute("needs_response", (int)(needs_response != 0));
    xrequest->SetAttribute("unique_id", unique_id);
    TiXmlElement *xobject = pyobj_to_xml(object);
    xobject->SetValue("object");
    xrequest->LinkEndChild(xobject);
    TiXmlElement *xvalue = pyobj_to_xml(value);
    xrequest->LinkEndChild(xvalue);

    write_xml(_pipe_write, &doc, nout);

  } else if (strcmp(request_type, "drop_p3dobj") == 0) {
    // Release a particular P3D_object that we were holding a
    // reference to.
    int object_id;
    if (!PyArg_ParseTuple(extra_args, "i", &object_id)) {
      return NULL;
    }

    xrequest->SetAttribute("object_id", object_id);
    write_xml(_pipe_write, &doc, nout);

  } else if (strcmp(request_type, "forget_package") == 0) {
    // A request to the instance to drop a particular package (or
    // host) from the cache.
    const char *host_url;
    const char *package_name;
    const char *package_version;
    if (!PyArg_ParseTuple(extra_args, "sss", &host_url, &package_name, &package_version)) {
      return NULL;
    }

    xrequest->SetAttribute("host_url", host_url);
    if (*package_name) {
      xrequest->SetAttribute("package_name", package_name);
      if (*package_version) {
        xrequest->SetAttribute("package_version", package_version);
      }
    }
    write_xml(_pipe_write, &doc, nout);

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
  SPAWN_THREAD(_read_thread, rt_thread_run, this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::join_read_thread
//       Access: Private
//  Description: Waits for the read thread to stop.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
join_read_thread() {
  _read_thread_continue = false;
  JOIN_THREAD(_read_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::start_instance
//       Access: Private
//  Description: Starts the indicated instance running within the
//               Python process.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
start_instance(P3DCInstance *inst, TiXmlElement *xinstance) {
  _instances[inst->get_instance_id()] = inst;

  set_instance_info(inst, xinstance);

  TiXmlElement *xpackage = xinstance->FirstChildElement("package");
  while (xpackage != (TiXmlElement *)NULL) {
    add_package_info(inst, xpackage);
    xpackage = xpackage->NextSiblingElement("package");
  }

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
//     Function: P3DPythonRun::set_instance_info
//       Access: Private
//  Description: Sets some global information about the instance.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
set_instance_info(P3DCInstance *inst, TiXmlElement *xinstance) {
  const char *root_dir = xinstance->Attribute("root_dir");
  if (root_dir == NULL) {
    root_dir = "";
  }

  const char *log_directory = xinstance->Attribute("log_directory");
  if (log_directory == NULL) {
    log_directory = "";
  }

  int verify_contents = 0;
  xinstance->Attribute("verify_contents", &verify_contents);

  const char *super_mirror = xinstance->Attribute("super_mirror");
  if (super_mirror == NULL) {
    super_mirror = "";
  }

  // Get the initial "main" object, if specified.
  PyObject *main;
  TiXmlElement *xmain = xinstance->FirstChildElement("main");
  if (xmain != NULL) {
    main = xml_to_pyobj(xmain);
  } else {
    main = Py_None;
    Py_INCREF(main);
  }

  int respect_per_platform = 0;
  xinstance->Attribute("respect_per_platform", &respect_per_platform);

  PyObject *result = PyObject_CallMethod
    (_runner, (char *)"setInstanceInfo", (char *)"sssiNi", root_dir,
     log_directory, super_mirror, verify_contents, main, respect_per_platform);

  if (result == NULL) {
    PyErr_Print();
    if (_interactive_console) {
      run_interactive_console();
    }
    exit(1);
  }
  Py_XDECREF(result);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::add_package_info
//       Access: Private
//  Description: Adds some information about a pre-loaded package.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
add_package_info(P3DCInstance *inst, TiXmlElement *xpackage) {
  const char *name = xpackage->Attribute("name");
  const char *platform = xpackage->Attribute("platform");
  const char *version = xpackage->Attribute("version");
  const char *host = xpackage->Attribute("host");
  const char *host_dir = xpackage->Attribute("host_dir");
  if (name == NULL || host == NULL) {
    return;
  }
  if (version == NULL) {
    version = "";
  }
  if (platform == NULL) {
    platform = "";
  }
  if (host_dir == NULL) {
    host_dir = "";
  }

  PyObject *result = PyObject_CallMethod
    (_runner, (char *)"addPackageInfo", (char *)"sssss",
     name, platform, version, host, host_dir);

  if (result == NULL) {
    PyErr_Print();
    if (_interactive_console) {
      run_interactive_console();
    }
    exit(1);
  }

  Py_XDECREF(result);
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

  int p3d_offset = 0;
  xfparams->Attribute("p3d_offset", &p3d_offset);

  string p3d_url;
  const char *p3d_url_c = xfparams->Attribute("p3d_url");
  if (p3d_url_c != NULL) {
    p3d_url = p3d_url_c;
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

  PyObject *arg_list = PyList_New(0);
  TiXmlElement *xarg = xfparams->FirstChildElement("arg");
  while (xarg != NULL) {
    string value;
    const char *value_c = xarg->Attribute("value");
    if (value_c != NULL) {
      value = value_c;
    }

    PyObject *str = Py_BuildValue("s", value.c_str());
    PyList_Append(arg_list, str);
    Py_DECREF(str);

    xarg = xarg->NextSiblingElement("arg");
  }

  PyObject *result = PyObject_CallMethod
    (_runner, (char *)"setP3DFilename", (char *)"sNNiiis", p3d_filename.c_str(),
     token_list, arg_list, inst->get_instance_id(), _interactive_console, p3d_offset,
     p3d_url.c_str());

  if (result == NULL) {
    PyErr_Print();
    if (_interactive_console) {
      run_interactive_console();
    }
    exit(1);
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

  PT(WindowHandle) parent_window_handle;

#ifdef _WIN32
  int hwnd;
  if (xwparams->Attribute("parent_hwnd", &hwnd)) {
    parent_window_handle = NativeWindowHandle::make_win((HWND)hwnd);
  }

#elif __APPLE__
  // On Mac, we don't parent windows directly to the browser; instead,
  // we have to go through this subprocess-window nonsense.

  const char *subprocess_window = xwparams->Attribute("subprocess_window");
  if (subprocess_window != NULL) {
    Filename filename = Filename::from_os_specific(subprocess_window);
    parent_window_handle = NativeWindowHandle::make_subprocess(filename);
  }

#elif defined(HAVE_X11)
  // Use stringstream to decode the "long" attribute.
  const char *parent_cstr = xwparams->Attribute("parent_xwindow");
  if (parent_cstr != NULL) {
    long window;
    istringstream strm(parent_cstr);
    strm >> window;
    parent_window_handle = NativeWindowHandle::make_x11((X11_Window)window);
  }
#endif

  PyObject *py_handle = Py_None;
  if (parent_window_handle != NULL) {

    // We have a valid parent WindowHandle, but replace it with a
    // P3DWindowHandle so we can get the callbacks.
    parent_window_handle = new P3DWindowHandle(this, inst, *parent_window_handle);
    inst->_parent_window_handle = parent_window_handle;

    // Also pass this P3DWindowHandle object down into Panda, via the
    // setupWindow() call.  For this, we need to create a Python
    // wrapper objcet.
    parent_window_handle->ref();
    py_handle = DTool_CreatePyInstanceTyped(parent_window_handle.p(), true);
  }
  Py_INCREF(py_handle);

  // TODO: direct this into the particular instance.  This will
  // require a specialized ShowBase replacement.
  PyObject *result = PyObject_CallMethod
    (_runner, (char *)"setupWindow", (char *)"siiiiN", window_type.c_str(),
     win_x, win_y, win_width, win_height, py_handle);

  if (result == NULL) {
    PyErr_Print();
    if (_interactive_console) {
      run_interactive_console();
    }
    exit(1);
  }
  Py_XDECREF(result);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::send_windows_message
//       Access: Public
//  Description: This is used to deliver a windows keyboard message to
//               the Panda process from the parent process, a
//               necessary hack on Vista.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
send_windows_message(int id, unsigned int msg, int wparam, int lparam) {
  Instances::iterator ii = _instances.find(id);
  if (ii == _instances.end()) {
    return;
  }

  P3DCInstance *inst = (*ii).second;
  if (inst->_parent_window_handle != (WindowHandle *)NULL) {
    inst->_parent_window_handle->send_windows_message(msg, wparam, lparam);
  }
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

  if (!_session_terminated) {
    if (_taskMgr != NULL) {
      PyObject *result = PyObject_CallMethod(_taskMgr, (char *)"stop", (char *)"");
      if (result == NULL) {
        PyErr_Print();
      } else {
        Py_DECREF(result);
      }
    }

    _session_terminated = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::pyobj_to_xml
//       Access: Private
//  Description: Converts the indicated PyObject to the appropriate
//               XML representation of a P3D_value type, and returns a
//               freshly-allocated TiXmlElement.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DPythonRun::
pyobj_to_xml(PyObject *value) {
  TiXmlElement *xvalue = new TiXmlElement("value");
  if (value == Py_None) {
    // None.
    xvalue->SetAttribute("type", "none");

  } else if (PyBool_Check(value)) {
    // A bool value.
    xvalue->SetAttribute("type", "bool");
    xvalue->SetAttribute("value", PyObject_IsTrue(value));

  } else if (PyInt_Check(value)) {
    // A plain integer value.
    xvalue->SetAttribute("type", "int");
    xvalue->SetAttribute("value", PyInt_AsLong(value));

  } else if (PyLong_Check(value)) {
    // A long integer value.  This gets converted either as an integer
    // or as a floating-point type, whichever fits.
    long lvalue = PyLong_AsLong(value);
    if (PyErr_Occurred()) {
      // It won't fit as an integer; make it a double.
      PyErr_Clear();
      xvalue->SetAttribute("type", "float");
      xvalue->SetDoubleAttribute("value", PyLong_AsDouble(value));
    } else {
      // It fits as an integer.
      xvalue->SetAttribute("type", "int");
      xvalue->SetAttribute("value", lvalue);
    }

  } else if (PyFloat_Check(value)) {
    // A floating-point value.
    xvalue->SetAttribute("type", "float");
    xvalue->SetDoubleAttribute("value", PyFloat_AsDouble(value));

  } else if (PyUnicode_Check(value)) {
    // A unicode value.  Convert to utf-8 for the XML encoding.
    xvalue->SetAttribute("type", "string");
    PyObject *as_str = PyUnicode_AsUTF8String(value);
    if (as_str != NULL) {
      char *buffer;
      Py_ssize_t length;
      if (PyString_AsStringAndSize(as_str, &buffer, &length) != -1) {
        string str(buffer, length);
        xvalue->SetAttribute("value", str);
      }
      Py_DECREF(as_str);
    }

  } else if (PyString_Check(value)) {
    // A string value.  Insist that it is utf-8 encoded, by decoding
    // it first using the standard encoding, then re-encoding it.
    xvalue->SetAttribute("type", "string");

    PyObject *ustr = PyUnicode_FromEncodedObject(value, NULL, NULL);
    if (ustr == NULL) {
      PyErr_Print();
      return xvalue;
    } else {
      PyObject *as_str = PyUnicode_AsUTF8String(ustr);
      if (as_str != NULL) {
        char *buffer;
        Py_ssize_t length;
        if (PyString_AsStringAndSize(as_str, &buffer, &length) != -1) {
          string str(buffer, length);
          xvalue->SetAttribute("value", str);
        }
        Py_DECREF(as_str);
      }
      Py_DECREF(ustr);
    }

  } else if (PyTuple_Check(value)) {
    // An immutable sequence.  Pass it as a concrete.
    xvalue->SetAttribute("type", "concrete_sequence");

    Py_ssize_t length = PySequence_Length(value);
    for (Py_ssize_t i = 0; i < length; ++i) {
      PyObject *item = PySequence_GetItem(value, i);
      if (item != NULL) {
        xvalue->LinkEndChild(pyobj_to_xml(item));
        Py_DECREF(item);
      }
    }

  } else if (PyObject_IsInstance(value, _concrete_struct_class)) {
    // This is a ConcreteStruct.
    xvalue->SetAttribute("type", "concrete_struct");

    PyObject *items = PyObject_CallMethod(value, (char *)"getConcreteProperties", (char *)"");
    if (items == NULL) {
      PyErr_Print();
      return xvalue;
    }

    Py_ssize_t length = PySequence_Length(items);
    for (Py_ssize_t i = 0; i < length; ++i) {
      PyObject *item = PySequence_GetItem(items, i);
      if (item != NULL) {
        PyObject *a = PySequence_GetItem(item, 0);
        if (a != NULL) {
          PyObject *b = PySequence_GetItem(item, 1);
          if (b != NULL) {
            TiXmlElement *xitem = pyobj_to_xml(b);
            Py_DECREF(b);

            PyObject *as_str;
            if (PyUnicode_Check(a)) {
              // The key is a unicode value.
              as_str = PyUnicode_AsUTF8String(a);
            } else {
              // The key is a string value or something else.  Make it
              // a string.
              as_str = PyObject_Str(a);
            }
            char *buffer;
            Py_ssize_t length;
            if (PyString_AsStringAndSize(as_str, &buffer, &length) != -1) {
              string str(buffer, length);
              xitem->SetAttribute("key", str);
            }
            Py_DECREF(as_str);

            xvalue->LinkEndChild(xitem);
          }
          Py_DECREF(a);
        }
        Py_DECREF(item);
      }
    }

    // We've already avoided errors in the above code; clear the error
    // flag.
    PyErr_Clear();

    Py_DECREF(items);

  } else if (PyObject_IsInstance(value, _undefined_object_class)) {
    // This is an UndefinedObject.
    xvalue->SetAttribute("type", "undefined");

  } else if (PyObject_IsInstance(value, _browser_object_class)) {
    // This is a BrowserObject, a reference to an object that actually
    // exists in the host namespace.  So, pass up the appropriate
    // object ID.
    PyObject *objectId = PyObject_GetAttrString(value, (char *)"_BrowserObject__objectId");
    if (objectId != NULL) {
      int object_id = PyInt_AsLong(objectId);
      xvalue->SetAttribute("type", "browser");
      xvalue->SetAttribute("object_id", object_id);
      Py_DECREF(objectId);
    }

  } else {
    // Some other kind of object.  Make it a generic Python object.
    // This is more expensive for the caller to deal with--it requires
    // a back-and-forth across the XML pipe--but it's much more
    // general.

    int object_id = _next_sent_id;
    ++_next_sent_id;
    bool inserted = _sent_objects.insert(SentObjects::value_type(object_id, value)).second;
    while (!inserted) {
      // Hmm, we must have cycled around the entire int space?  Either
      // that, or there's a logic bug somewhere.  Assume the former,
      // and keep looking for an empty slot.
      object_id = _next_sent_id;
      ++_next_sent_id;
      inserted = _sent_objects.insert(SentObjects::value_type(object_id, value)).second;
    }

    // Now that it's stored in the map, increment its reference count.
    Py_INCREF(value);

    xvalue->SetAttribute("type", "python");
    xvalue->SetAttribute("object_id", object_id);
  }

  return xvalue;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::xml_to_pyobj
//       Access: Private
//  Description: Converts the XML representation of a P3D_value type
//               into the equivalent Python object and returns it.
////////////////////////////////////////////////////////////////////
PyObject *P3DPythonRun::
xml_to_pyobj(TiXmlElement *xvalue) {
  const char *type = xvalue->Attribute("type");
  if (strcmp(type, "none") == 0) {
    return Py_BuildValue("");

  } else if (strcmp(type, "bool") == 0) {
    int value;
    if (xvalue->QueryIntAttribute("value", &value) == TIXML_SUCCESS) {
      return PyBool_FromLong(value);
    }

  } else if (strcmp(type, "int") == 0) {
    int value;
    if (xvalue->QueryIntAttribute("value", &value) == TIXML_SUCCESS) {
      return PyInt_FromLong(value);
    }

  } else if (strcmp(type, "float") == 0) {
    double value;
    if (xvalue->QueryDoubleAttribute("value", &value) == TIXML_SUCCESS) {
      return PyFloat_FromDouble(value);
    }

  } else if (strcmp(type, "string") == 0) {
    // Using the string form here instead of the char * form, so we
    // don't get tripped up on embedded null characters.
    const string *value = xvalue->Attribute(string("value"));
    if (value != NULL) {
      return PyUnicode_DecodeUTF8(value->data(), value->length(), NULL);
    }

  } else if (strcmp(type, "undefined") == 0) {
    Py_INCREF(_undefined);
    return _undefined;

  } else if (strcmp(type, "browser") == 0) {
    int object_id;
    if (xvalue->QueryIntAttribute("object_id", &object_id) == TIXML_SUCCESS) {
      // Construct a new BrowserObject wrapper around this object.
      return PyObject_CallFunction(_browser_object_class, (char *)"Oi",
                                   _runner, object_id);
    }

  } else if (strcmp(type, "concrete_sequence") == 0) {
    // Receive a concrete sequence as a tuple.
    PyObject *list = PyList_New(0);

    TiXmlElement *xitem = xvalue->FirstChildElement("value");
    while (xitem != NULL) {
      PyObject *item = xml_to_pyobj(xitem);
      if (item != NULL) {
        PyList_Append(list, item);
        Py_DECREF(item);
      }
      xitem = xitem->NextSiblingElement("value");
    }

    PyObject *result = PyList_AsTuple(list);
    Py_DECREF(list);
    return result;

  } else if (strcmp(type, "concrete_struct") == 0) {
    // Receive a concrete struct as a new ConcreteStruct instance.
    PyObject *obj = PyObject_CallFunction(_concrete_struct_class, (char *)"");

    if (obj != NULL) {
      TiXmlElement *xitem = xvalue->FirstChildElement("value");
      while (xitem != NULL) {
        const char *key = xitem->Attribute("key");
        if (key != NULL) {
          PyObject *item = xml_to_pyobj(xitem);
          if (item != NULL) {
            PyObject_SetAttrString(obj, (char *)key, item);
            Py_DECREF(item);
          }
        }
        xitem = xitem->NextSiblingElement("value");
      }
      return obj;
    }

  } else if (strcmp(type, "python") == 0) {
    int object_id;
    if (xvalue->QueryIntAttribute("object_id", &object_id) == TIXML_SUCCESS) {
      SentObjects::iterator si = _sent_objects.find(object_id);
      PyObject *result;
      if (si == _sent_objects.end()) {
        // Hmm, the parent process gave us a bogus object ID.
        result = _undefined;
      } else {
        result = (*si).second;
      }
      Py_INCREF(result);
      return result;
    }
  }

  // Something went wrong in decoding.
  PyObject *result = _undefined;
  Py_INCREF(result);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::rt_thread_run
//       Access: Private
//  Description: The main function for the read thread.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::
rt_thread_run() {
  while (_read_thread_continue) {
    TiXmlDocument *doc = read_xml(_pipe_read, nout);
    if (doc == NULL) {
      // Some error on reading.  Abort.
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

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::P3DWindowHandle::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
P3DPythonRun::P3DWindowHandle::
P3DWindowHandle(P3DPythonRun *p3dpython, P3DCInstance *inst,
                const WindowHandle &copy) :
  WindowHandle(copy),
  _p3dpython(p3dpython),
  _inst(inst),
  _child_count(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::P3DWindowHandle::attach_child
//       Access: Public, Virtual
//  Description: Called on a parent handle to indicate a child
//               window's intention to attach itself.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::P3DWindowHandle::
attach_child(WindowHandle *child) {
  WindowHandle::attach_child(child);
  ++_child_count;

  if (_child_count == 1) {
    // When the first child window is attached, we tell the plugin.
    _p3dpython->set_window_open(_inst, true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::P3DWindowHandle::detach_child
//       Access: Public, Virtual
//  Description: Called on a parent handle to indicate a child
//               window's intention to detach itself.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::P3DWindowHandle::
detach_child(WindowHandle *child) {
  WindowHandle::detach_child(child);
  --_child_count;

  if (_child_count == 0) {
    // When the last child window is detached, we tell the plugin.
    _p3dpython->set_window_open(_inst, false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonRun::P3DWindowHandle::request_keyboard_focus
//       Access: Public, Virtual
//  Description: Called on a parent handle to indicate a child
//               window's wish to receive keyboard button events.
////////////////////////////////////////////////////////////////////
void P3DPythonRun::P3DWindowHandle::
request_keyboard_focus(WindowHandle *child) {
  WindowHandle::request_keyboard_focus(child);
  _p3dpython->request_keyboard_focus(_inst);
}
