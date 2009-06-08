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

#include <set>

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
  string _session_key;
  string _python_version;

  string _python_root_dir;

  typedef set<P3DInstance *> Instances;
  Instances _instances;

  // Members for communicating with the p3dpython child process.
  bool _started_p3dpython;
#ifdef _WIN32
  PROCESS_INFORMATION _p3dpython;
#endif
  HandleStream _pipe_read;
  HandleStream _pipe_write;
};

#include "p3dSession.I"

#endif
