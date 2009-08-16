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

class P3DInstance;
class P3DSession;
class P3DPackage;
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
                  const string &platform);

  inline bool is_initialized() const;

  inline const string &get_root_dir() const;
  inline const string &get_download_url() const;
  inline const string &get_platform() const;

  inline bool has_contents_file() const;
  bool read_contents_file(const string &contents_filename);

  P3DInstance *
  create_instance(P3D_request_ready_func *func, 
                  const P3D_token tokens[], size_t num_tokens, 
                  void *user_data);

  bool set_p3d_filename(P3DInstance *inst, bool is_local,
                        const string &p3d_filename);
  bool start_instance(P3DInstance *inst);
  void finish_instance(P3DInstance *inst);

  P3DInstance *validate_instance(P3D_instance *instance);

  P3DInstance *check_request();
  void wait_request();

  P3DPackage *get_package(const string &package_name, 
                          const string &package_version);
  bool get_package_desc_file(FileSpec &desc_file, 
                             const string &package_name,
                             const string &package_version);

  inline int get_num_instances() const;

  int get_unique_id();
  void signal_request_ready(P3DInstance *inst);

  P3D_class_definition *make_class_definition() const;

  inline P3D_object *new_undefined_object();
  inline P3D_object *new_none_object();
  inline P3D_object *new_bool_object(bool value);

  static P3DInstanceManager *get_global_ptr();
  static void delete_global_ptr();

private:
  bool copy_file(const string &from_filename, const string &to_filename);

private:
  // The notify thread.  This thread runs only for the purpose of
  // generating asynchronous notifications of requests, to callers who
  // ask for it.
  THREAD_CALLBACK_DECLARATION(P3DInstanceManager, nt_thread_run);
  void nt_thread_run();

private:
  bool _is_initialized;
  string _root_dir;
  string _download_url;
  string _platform;

  TiXmlElement *_xcontents;

  P3D_object *_undefined_object;
  P3D_object *_none_object;
  P3D_object *_true_object;
  P3D_object *_false_object;

  typedef set<P3DInstance *> Instances;
  Instances _instances;

  typedef map<string, P3DSession *> Sessions;
  Sessions _sessions;

  typedef map<string, P3DPackage *> Packages;
  Packages _packages;

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

  static P3DInstanceManager *_global_ptr;
};

#include "p3dInstanceManager.I"

#endif
