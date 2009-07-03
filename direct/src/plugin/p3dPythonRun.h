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

#include "p3d_lock.h"
#include "handleStream.h"
#include "p3dCInstance.h"
#include "genericAsyncTask.h"
#include "pmap.h"
#include "pdeque.h"
#include "get_tinyxml.h"

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
  P3DPythonRun(int argc, char *argv[]);
  ~P3DPythonRun();

  bool run_python();

private:
  void handle_command(TiXmlDocument *doc);
  void handle_pyobj_command(TiXmlElement *xcommand, int want_response_id);

  AsyncTask::DoneStatus check_comm(GenericAsyncTask *task);
  static AsyncTask::DoneStatus st_check_comm(GenericAsyncTask *task, void *user_data);

  PyObject *py_request_func(PyObject *args);
  static PyObject *st_request_func(PyObject *, PyObject *args);

  void spawn_read_thread();
  void join_read_thread();

  void start_instance(P3DCInstance *inst, TiXmlElement *xinstance);
  void terminate_instance(int id);
  void set_p3d_filename(P3DCInstance *inst, TiXmlElement *xfparams);
  void setup_window(int id, TiXmlElement *xwparams);
  void setup_window(P3DCInstance *inst, TiXmlElement *xwparams);
  
  void terminate_session();

private:
  TiXmlElement *pyobj_to_xml(PyObject *value);
  PyObject *xml_to_pyobj(TiXmlElement *xvalue);

private:
  // This method runs only within the read thread.

  void rt_thread_run();
#ifdef _WIN32
  static DWORD WINAPI win_rt_thread_run(LPVOID data);
#else
  static void *posix_rt_thread_run(void *data);
#endif

private:
  typedef pmap<int, P3DCInstance *> Instances;
  Instances _instances;

  string _program_name;
  int _py_argc;
  char **_py_argv;

  PyObject *_runner;
  PyObject *_taskMgr;

  PT(GenericAsyncTask) _check_comm_task;

  // The remaining members are manipulated by the read thread.
  typedef pdeque<TiXmlDocument *> Commands;
  Commands _commands;
  
  // This has to be an actual OS LOCK instead of Panda's Mutex,
  // because we have to use a true thread here, not one of Panda's
  // simple threads.
  LOCK _commands_lock;

  HandleStream _pipe_read;
  HandleStream _pipe_write;

  bool _read_thread_continue;
  bool _program_continue;
#ifdef _WIN32
  HANDLE _read_thread;
#else
  pthread_t _read_thread;
#endif

public:
  static P3DPythonRun *_global_ptr;
};

#include "p3dPythonRun.I"

#endif

