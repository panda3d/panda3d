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
class P3DPython;

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
  bool initialize(const string &config_xml, const string &dll_filename);

  P3DInstance *
  create_instance(P3D_request_ready_func *func,
                  const string &p3d_filename, 
                  P3D_window_type window_type,
                  int win_x, int win_y,
                  int win_width, int win_height,
                  P3D_window_handle parent_window,
                  const P3D_token *tokens[], size_t tokens_size);

  void
  finish_instance(P3DInstance *inst);

  P3DInstance *check_request();
  void wait_request();

  P3DPython *start_python(const string &python_version);

  INLINE int get_num_instances() const;

  INLINE const string &get_dll_filename() const;

  int get_unique_session_index();
  void signal_request_ready();

  static P3DInstanceManager *get_global_ptr();

private:
  string _dll_filename;
  string _p3d_root_directory;

  typedef set<P3DInstance *> Instances;
  Instances _instances;

  typedef map<string, P3DSession *> Sessions;
  Sessions _sessions;

  P3DPython *_current_python;
  int _unique_session_index;

  // Implements a condition-var like behavior.
  volatile int _request_seq;
#ifdef _WIN32
  HANDLE _request_ready;
#endif

  static P3DInstanceManager *_global_ptr;
};

#include "p3dInstanceManager.I"

#endif
