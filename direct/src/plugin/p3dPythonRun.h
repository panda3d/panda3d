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

#include <Python.h>
#include <tinyxml.h>

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
  AsyncTask::DoneStatus check_comm(GenericAsyncTask *task);
  static AsyncTask::DoneStatus st_check_comm(GenericAsyncTask *task, void *user_data);

  void spawn_read_thread();
  void join_read_thread();

  void start_instance(P3DCInstance *inst);
  void terminate_instance(int id);
  void setup_window(int id, TiXmlElement *xwparams);

  void terminate_session();

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

  PyObject *_runPackedApp;
  PyObject *_setupWindow;
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
};

#include "p3dPythonRun.I"

#endif

