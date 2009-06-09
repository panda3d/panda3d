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

#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <assert.h>
#include <Python.h>
#include <tinyxml.h>
#include "p3d_lock.h"
#include "handleStream.h"
#include "p3dCInstance.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

////////////////////////////////////////////////////////////////////
//       Class : P3DPythonRun
// Description : This class is used to run, and communicate with,
//               embedded Python in a sub-process.  It is compiled and
//               launched as a separate executable from the p3d_plugin
//               dll, because that's the only way Windows can launch a
//               sub-process, and also because it makes it possible to
//               compile-time link with the Python dll, instead of
//               having to go through the clumsy dynamic-loading
//               interface.
//
//               Communication is via XML files exchanged via
//               anonymous pipes from the parent process.  This
//               executable is not designed to stand alone.
////////////////////////////////////////////////////////////////////
class P3DPythonRun {
public:
  P3DPythonRun(int argc, char *argv[]);
  ~P3DPythonRun();

  bool run_python();

private:
  void handle_command(TiXmlDocument *doc);
  void check_comm();
  static PyObject *py_check_comm(PyObject *, PyObject *args);

  void spawn_read_thread();
  void join_read_thread();

  void start_instance(P3DCInstance *inst);

private:
  // This method runs only within the read thread.
  void rt_thread_run();
#ifdef _WIN32
  static DWORD WINAPI win_rt_thread_run(LPVOID data);
#endif

private:
  typedef vector<P3DCInstance *> Instances;
  Instances _instances;

  string _program_name;
  int _py_argc;
  char **_py_argv;

  PyObject *_runPackedApp;
  PyObject *_setupWindow;

  // The remaining members are manipulated by the read thread.
  typedef deque<TiXmlDocument *> Commands;
  Commands _commands;
  LOCK _commands_lock;

  HandleStream _pipe_read;
  HandleStream _pipe_write;

  bool _read_thread_alive;
#ifdef _WIN32
  HANDLE _read_thread;
#endif
};

#include "p3dPythonRun.I"

#endif

