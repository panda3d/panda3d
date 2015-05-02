// Filename: p3dPythonRun.h
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

#ifndef P3DPYTHONRUN_H
#define P3DPYTHONRUN_H

#include "pandabase.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "run_p3dpython.h"
#include "p3d_lock.h"
#include "handleStream.h"
#include "p3dCInstance.h"
#include "pandaFileStreamBuf.h"
#include "pmap.h"
#include "pdeque.h"
#include "pmutex.h"
#include "get_tinyxml.h"
#include "filename.h"

#include <Python.h>

// Python 2.5 adds Py_ssize_t; earlier versions don't have it.
#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

using namespace std;

////////////////////////////////////////////////////////////////////
//       Class : P3DPythonRun
// Description : This class is used to run, and communicate with,
//               embedded Python in a sub-process.  It is compiled and
//               launched as a separate executable from the p3d_plugin
//               dll, because that's the only way Windows can launch a
//               sub-process, and also because it makes it possible to
//               compile-time link with Panda and Python, instead of
//               having to go through the clumsy dynamic-loading
//               interface.
//
//               Communication is via XML files exchanged via
//               anonymous pipes from the parent process.  This isn't
//               terribly eficient, of course, but it's easy; and it's
//               a fairly low-bandwidth channel so efficiency is not
//               paramount.
//
//               This executable is not designed to stand alone; it is
//               designed to be invoked only by p3d_plugin.
////////////////////////////////////////////////////////////////////
class P3DPythonRun {
public:
  P3DPythonRun(const char *program_name, const char *archive_file,
               FHandle input_handle, FHandle output_handle, 
               const char *log_pathname, bool interactive_console);
  ~P3DPythonRun();

  bool run_python();

  void set_window_open(P3DCInstance *inst, bool is_open);
  void request_keyboard_focus(P3DCInstance *inst);

private:
  void run_interactive_console();
  void handle_command(TiXmlDocument *doc);
  void handle_pyobj_command(TiXmlElement *xcommand, bool needs_response,
                            int want_response_id);
  void handle_script_response_command(TiXmlElement *xcommand);

  void check_comm();
  static PyObject *st_check_comm(PyObject *, PyObject *args);
  TiXmlDocument *wait_script_response(int response_id);

  PyObject *py_request_func(PyObject *args);
  static PyObject *st_request_func(PyObject *, PyObject *args);

  void spawn_read_thread();
  void join_read_thread();

  void start_instance(P3DCInstance *inst, TiXmlElement *xinstance);
  void terminate_instance(int id);
  void set_instance_info(P3DCInstance *inst, TiXmlElement *xinstance);
  void add_package_info(P3DCInstance *inst, TiXmlElement *xpackage);
  void set_p3d_filename(P3DCInstance *inst, TiXmlElement *xfparams);
  void setup_window(int id, TiXmlElement *xwparams);
  void setup_window(P3DCInstance *inst, TiXmlElement *xwparams);

  void send_windows_message(int id, unsigned int msg, int wparam, int lparam);
  
  void terminate_session();

private:
  TiXmlElement *pyobj_to_xml(PyObject *value);
  PyObject *xml_to_pyobj(TiXmlElement *xvalue);

private:
  // This subclass of WindowHandle is associated with the parent
  // window we are given by the parent process.  We use it to add
  // hooks for communicating with the parent window, for instance to
  // ask for the parent window to manage keyboard focus when
  // necessary.
  class P3DWindowHandle : public WindowHandle {
  public:
    P3DWindowHandle(P3DPythonRun *p3dpython, P3DCInstance *inst,
                    const WindowHandle &copy);

  public:
    virtual void attach_child(WindowHandle *child);
    virtual void detach_child(WindowHandle *child);
    virtual void request_keyboard_focus(WindowHandle *child);

  private:
    P3DPythonRun *_p3dpython;
    P3DCInstance *_inst;
    int _child_count;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      WindowHandle::init_type();
      register_type(_type_handle, "P3DWindowHandle",
                    WindowHandle::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
    
  private:
    static TypeHandle _type_handle;
  };

private:
  // This method runs only within the read thread.
  THREAD_CALLBACK_DECLARATION(P3DPythonRun, rt_thread_run);
  void rt_thread_run();

private:
  typedef pmap<int, P3DCInstance *> Instances;
  Instances _instances;

  int _session_id;

  string _program_name;
  Filename _archive_file;
  int _py_argc;
  char **_py_argv;
  bool _interactive_console;

  PyObject *_runner;
  PyObject *_undefined_object_class;
  PyObject *_undefined;
  PyObject *_concrete_struct_class;
  PyObject *_browser_object_class;
  PyObject *_taskMgr;

  // This map keeps track of the PyObject pointers we have delivered
  // to the parent process.  We have to hold the reference count on
  // each of these until the parent process tells us it's safe to
  // release them.
  typedef pmap<int, PyObject *> SentObjects;
  SentObjects _sent_objects;
  int _next_sent_id;

  typedef pdeque<TiXmlDocument *> Commands;

  // This is a special queue of responses extracted from the _commands
  // queue, below.  It's protected by the Panda mutex.
  Commands _responses;
  Mutex _responses_lock;

  // The remaining members are manipulated by the read thread.
  Commands _commands;
  
  // This has to be an actual OS LOCK instead of Panda's Mutex,
  // because we have to use a true thread here, not one of Panda's
  // simple threads.
  LOCK _commands_lock;

  HandleStream _pipe_read;
  HandleStream _pipe_write;
  pofstream _error_log;

  bool _read_thread_continue;
  bool _program_continue;
  bool _session_terminated;
  THREAD _read_thread;

public:
  static P3DPythonRun *_global_ptr;
};

#include "p3dPythonRun.I"

#endif

