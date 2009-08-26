// Filename: p3dInstanceManager.h
// Created by:  drose (29May09)
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

#ifndef P3DINSTANCEMANAGER_H
#define P3DINSTANCEMANAGER_H

#include "p3d_plugin_common.h"
#include "p3dConditionVar.h"

#include <set>
#include <map>
#include <vector>

#ifndef _WIN32
#include <signal.h>
#endif

class P3DInstance;
class P3DSession;
class P3DHost;
class FileSpec;
class TiXmlElement;

////////////////////////////////////////////////////////////////////
//       Class : P3DInstanceManager
// Description : This global class manages the set of instances in the
//               universe.
////////////////////////////////////////////////////////////////////
class P3DInstanceManager {
private:
  P3DInstanceManager();
  ~P3DInstanceManager();

public:
  bool initialize(const string &contents_filename,
                  const string &download_url,
                  bool verify_contents,
                  const string &platform,
                  const string &log_directory,
                  const string &log_basename,
                  bool keep_cwd);

  inline bool is_initialized() const;
  inline bool get_verify_contents() const;
  inline void reset_verify_contents();

  inline const string &get_root_dir() const;
  inline const string &get_platform() const;
  inline const string &get_log_directory() const;
  inline bool get_keep_cwd() const;

  P3DInstance *
  create_instance(P3D_request_ready_func *func, 
                  const P3D_token tokens[], size_t num_tokens, 
                  int argc, const char *argv[], void *user_data);

  bool set_p3d_filename(P3DInstance *inst, bool is_local,
                        const string &p3d_filename);
  bool start_instance(P3DInstance *inst);
  void finish_instance(P3DInstance *inst);

  P3DInstance *validate_instance(P3D_instance *instance);

  P3DInstance *check_request();
  void wait_request(double timeout);

  P3DHost *get_host(const string &host_url);

  inline int get_num_instances() const;

  int get_unique_id();
  void signal_request_ready(P3DInstance *inst);

  P3D_class_definition *make_class_definition() const;

  inline P3D_object *new_undefined_object();
  inline P3D_object *new_none_object();
  inline P3D_object *new_bool_object(bool value);

  string make_temp_filename(const string &extension);
  void release_temp_filename(const string &filename);

  static P3DInstanceManager *get_global_ptr();
  static void delete_global_ptr();

private:
  // The notify thread.  This thread runs only for the purpose of
  // generating asynchronous notifications of requests, to callers who
  // ask for it.
  THREAD_CALLBACK_DECLARATION(P3DInstanceManager, nt_thread_run);
  void nt_thread_run();

private:
  bool _is_initialized;
  string _root_dir;
  bool _verify_contents;
  string _platform;
  string _log_directory;
  string _log_basename;
  string _log_pathname;
  string _temp_directory;
  bool _keep_cwd;

  P3D_object *_undefined_object;
  P3D_object *_none_object;
  P3D_object *_true_object;
  P3D_object *_false_object;

  typedef set<P3DInstance *> Instances;
  Instances _instances;

  typedef map<string, P3DSession *> Sessions;
  Sessions _sessions;

  typedef map<string, P3DHost *> Hosts;
  Hosts _hosts;

  typedef set<string> TempFilenames;
  TempFilenames _temp_filenames;
  int _next_temp_filename_counter;

  int _unique_id;

  // This condition var is waited on the main thread and signaled in a
  // sub-thread when new request notices arrive.
  P3DConditionVar _request_ready;

  // We may need a thread to send async request notices to callers.
  bool _notify_thread_continue;
  bool _started_notify_thread;
  THREAD _notify_thread;
  // This queue of instances that need to send notifications is
  // protected by _notify_ready's mutex.
  typedef vector<P3DInstance *> NotifyInstances;
  NotifyInstances _notify_instances;
  P3DConditionVar _notify_ready;

#ifndef _WIN32
  struct sigaction _old_sigpipe;
#endif

  static P3DInstanceManager *_global_ptr;
};

#include "p3dInstanceManager.I"

#endif
