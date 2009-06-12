// Filename: p3dSession.h
// Created by:  drose (03Jun09)
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

#ifndef P3DSESSION_H
#define P3DSESSION_H

#include "p3d_plugin_common.h"
#include "handleStream.h"

#include <map>
#include <vector>
#include <tinyxml.h>

class P3DInstance;

////////////////////////////////////////////////////////////////////
//       Class : P3DSession
// Description : Corresponds to a single session: a subprocess with a
//               unique instance of Python running within it, which
//               might include one or more P3DInstance objects running
//               in the same memory space with each other.
////////////////////////////////////////////////////////////////////
class P3DSession {
public:
  P3DSession(P3DInstance *inst);
  ~P3DSession();

  INLINE const string &get_session_key() const;
  INLINE const string &get_python_version() const;

  void start_instance(P3DInstance *inst);
  void terminate_instance(P3DInstance *inst);

  INLINE int get_num_instances() const;

private:
  void send_command(TiXmlDocument *command);
  void download_p3dpython(P3DInstance *inst);
  void start_p3dpython();

  void spawn_read_thread();
  void join_read_thread();

private:
  // These methods run only within the read thread.
  void rt_thread_run();
  void rt_terminate();

#ifdef _WIN32
  static DWORD WINAPI win_rt_thread_run(LPVOID data);
#endif

private:
  enum PythonState {
    PS_init,
    PS_downloading,
    PS_running,
    PS_done,
  };
  PythonState _python_state;

  string _session_key;
  string _python_version;
  string _output_filename;

  string _python_root_dir;

  typedef map<int, P3DInstance *> Instances;
  Instances _instances;
  LOCK _instances_lock;

  // Commands that are queued up to send down the pipe.  Normally
  // these only accumulate before the python process has been started;
  // after that, commands are written to the pipe directly.
  typedef vector<TiXmlDocument *> Commands;
  Commands _commands;

  // Members for communicating with the p3dpython child process.
#ifdef _WIN32
  PROCESS_INFORMATION _p3dpython;
#endif

  // The remaining members are manipulated by or for the read thread.
  bool _started_read_thread;
  HandleStream _pipe_read;
  HandleStream _pipe_write;

  bool _read_thread_continue;
#ifdef _WIN32
  HANDLE _read_thread;
#endif
};

#include "p3dSession.I"

#endif
