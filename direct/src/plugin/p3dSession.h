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
class P3DSession {
public:
  P3DSession(P3DInstance *inst);
  ~P3DSession();

  inline const string &get_session_key() const;
  inline const string &get_python_version() const;

  void start_instance(P3DInstance *inst);
  void terminate_instance(P3DInstance *inst);

  inline int get_num_instances() const;

  void send_command(TiXmlDocument *command);

private:
  void install_progress(P3DPackage *package, double progress);
  void start_p3dpython();

  void spawn_read_thread();
  void join_read_thread();

private:
  // These methods run only within the read thread.
  void rt_thread_run();
  void rt_terminate();
  void rt_handle_request(TiXmlDocument *doc);
  P3D_request *rt_make_p3d_request(TiXmlElement *xrequest);
  P3DValue *rt_from_xml_value(TiXmlElement *xvalue);

#ifdef _WIN32
  static DWORD WINAPI win_rt_thread_run(LPVOID data);
#else
  static void *posix_rt_thread_run(void *data);
#endif

#ifdef _WIN32
  static HANDLE 
  win_create_process(const string &program, const string &start_dir,
                     const string &env, const string &output_filename,
                     HandleStream &pipe_read, HandleStream &pipe_write);
#else
  static int 
  posix_create_process(const string &program, const string &start_dir,
                       const string &env, const string &output_filename,
                       HandleStream &pipe_read, HandleStream &pipe_write);
#endif

  class PackageCallback : public P3DPackage::Callback {
  public:
    PackageCallback(P3DSession *session);
    
    virtual void install_progress(P3DPackage *package, double progress);
    virtual void package_ready(P3DPackage *package, bool success);
    
  protected:
    P3DSession *_session;
  };

private:
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

  P3DPackage *_panda3d;
  PackageCallback *_panda3d_callback;

  // Members for communicating with the p3dpython child process.
#ifdef _WIN32
  HANDLE _p3dpython_handle;
#else
  int _p3dpython_pid;
#endif
  bool _p3dpython_running;

  // The remaining members are manipulated by or for the read thread.
  bool _started_read_thread;
  HandleStream _pipe_read;
  HandleStream _pipe_write;

  bool _read_thread_continue;
#ifdef _WIN32
  HANDLE _read_thread;
#else
  pthread_t _read_thread;
#endif

  friend class PackageCallback;
};

#include "p3dSession.I"

#endif
