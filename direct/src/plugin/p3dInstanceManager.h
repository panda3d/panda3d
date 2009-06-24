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

#include <set>
#include <map>

class P3DInstance;
class P3DSession;
class P3DPackage;

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
  bool initialize();

  inline bool is_initialized() const;

  inline const string &get_root_dir() const;
  inline const string &get_download_url() const;
  inline const string &get_platform() const;

  P3DInstance *
  create_instance(P3D_request_ready_func *func,
                  const string &p3d_filename, 
                  const P3D_token tokens[], size_t num_tokens);

  void
  finish_instance(P3DInstance *inst);

  P3DInstance *check_request();
  void wait_request();

  P3DPackage *get_package(const string &package_name, 
                          const string &package_version,
                          const string &package_display_name);

  inline int get_num_instances() const;

  int get_unique_session_index();
  void signal_request_ready();

  static bool mkdir_public(const string &dirname);
  static bool mkfile_public(const string &dirname);

  static P3DInstanceManager *get_global_ptr();

private:
  string find_root_dir() const;
  static inline bool is_pathsep(char ch);
  static string get_dirname(const string &filename);

private:
  bool _is_initialized;
  string _root_dir;
  string _download_url;
  string _platform;

  typedef set<P3DInstance *> Instances;
  Instances _instances;

  typedef map<string, P3DSession *> Sessions;
  Sessions _sessions;

  typedef map<string, P3DPackage *> Packages;
  Packages _packages;

  int _unique_session_index;

  // Implements a condition-var like behavior.
  volatile int _request_seq;
#ifdef _WIN32
  HANDLE _request_ready;
#else
  LOCK _request_ready_lock;
  pthread_cond_t _request_ready_cvar;
#endif
  static P3DInstanceManager *_global_ptr;
};

#include "p3dInstanceManager.I"

#endif
