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
#include "p3dPackage.h"
#include "p3dConditionVar.h"
#include "p3dReferenceCount.h"
#include "get_tinyxml.h"

#include <map>
#include <vector>

class P3DInstance;
class P3DProgressWindow;

////////////////////////////////////////////////////////////////////
//       Class : P3DSession
// Description : Corresponds to a single session: a subprocess with a
//               unique instance of Python running within it, which
//               might include one or more P3DInstance objects running
//               in the same memory space with each other.
////////////////////////////////////////////////////////////////////
class P3DSession : public P3DReferenceCount {
public:
  P3DSession(P3DInstance *inst);
  ~P3DSession();

  void shutdown();

  inline const string &get_session_key() const;
  inline const string &get_log_pathname() const;
  inline bool get_matches_script_origin() const;

  void start_instance(P3DInstance *inst);
  void terminate_instance(P3DInstance *inst);

  inline int get_num_instances() const;

  void send_command(TiXmlDocument *command);
  TiXmlDocument *command_and_response(TiXmlDocument *command);
  P3D_object *xml_to_p3dobj(const TiXmlElement *xvalue);
  TiXmlElement *p3dobj_to_xml(P3D_object *obj);
  void send_windows_message(P3DInstance *inst, unsigned int msg, 
                            int wparam, int lparam);

  void signal_request_ready(P3DInstance *inst);

  void drop_pyobj(int object_id);
  void drop_p3dobj(int object_id);

private:
  void start_p3dpython(P3DInstance *inst);
  void set_failed();

  void spawn_read_thread();
  void join_read_thread();

  static void replace_slashes(string &str);

private:
  // These methods run only within the read thread.
  THREAD_CALLBACK_DECLARATION(P3DSession, rt_thread_run);
  void rt_thread_run();
  void rt_terminate();
  void rt_handle_request(TiXmlDocument *doc);

#ifdef _WIN32
  HANDLE win_create_process();
#else
  int posix_create_process();
#endif

  // In case we can't get a separate process, we'll run p3dpython in a
  // sub-thread.
  THREAD_CALLBACK_DECLARATION(P3DSession, p3dpython_thread_run);
  void p3dpython_thread_run();

  static bool get_env(string &value, const string &varname);
  void write_env() const;

private:
  int _session_id;
  string _session_key;
  string _log_pathname;
  string _python_root_dir;
  string _start_dir;
  bool _matches_script_origin;
  bool _keep_user_env;
  bool _failed;

  // This information is passed to create_process(), or to
  // p3dpython_thread_run().
  string _p3dpython_exe;
  string _p3dpython_dll;
  string _mf_filename;
  string _env;
  FHandle _input_handle, _output_handle;
  bool _interactive_console;

  typedef map<int, P3DInstance *> Instances;
  Instances _instances;
  LOCK _instances_lock;

  // Commands that are queued up to send down the pipe.  Normally
  // these only accumulate before the python process has been started;
  // after that, commands are written to the pipe directly.
  typedef vector<TiXmlDocument *> Commands;
  Commands _commands;

  // This map keeps track of the P3D_object pointers we have delivered
  // to the child process.  We have to keep each of these until the
  // child process tells us it's safe to delete them.
  typedef map<int, P3D_object *> SentObjects;
  SentObjects _sent_objects;

  P3DPackage *_panda3d;

  // If this is true, then CreateProcess() or fork() failed (or we had
  // one_process set true in the tokens), and we're forced to run
  // p3dpython in a sub-thread within the same process, rather than in
  // a separate process.  This means we can't have multiple sessions
  // running simultaneously, because Python don't play that way.
  bool _p3dpython_one_process;

  // Members for communicating with the p3dpython child process (or
  // thread, as the case may be).
#ifdef _WIN32
  HANDLE _p3dpython_handle;
#else
  int _p3dpython_pid;
#endif
  THREAD _p3dpython_thread;
  bool _p3dpython_started;
  bool _p3dpython_running;

  // The _response_ready mutex protects this structure.
  typedef map<int, TiXmlDocument *> Responses;
  Responses _responses;
  P3DConditionVar _response_ready;

  // The remaining members are manipulated by or for the read thread.
  bool _started_read_thread;
  HandleStream _pipe_read;
  HandleStream _pipe_write;

  bool _read_thread_continue;
  THREAD _read_thread;

  friend class P3DInstance;
};

#include "p3dSession.I"

#endif
