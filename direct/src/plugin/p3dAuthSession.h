// Filename: P3DAuthSession.h
// Created by:  drose (17Sep09)
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

#ifndef P3DAUTHSESSION_H
#define P3DAUTHSESSION_H

#include "p3d_plugin_common.h"
#include "p3dPackage.h"
#include "get_tinyxml.h"
#include "p3dTemporaryFile.h"
#include "p3dReferenceCount.h"

class P3DInstance;

////////////////////////////////////////////////////////////////////
//       Class : P3DAuthSession
// Description : This is an instance of a p3dcert program running in a
//               subprocess.  There's no communication with the
//               process, or none of that complicated stuff the
//               P3DSession has to do; all we do here is fire off the
//               process, then wait for it to exit.
////////////////////////////////////////////////////////////////////
class P3DAuthSession : public P3DReferenceCount {
public:
  P3DAuthSession(P3DInstance *inst);
  ~P3DAuthSession();

  void shutdown(bool send_message);

private:
  void start_p3dcert();

  void spawn_wait_thread();
  void join_wait_thread();

  void write_env() const;

private:
  // These methods run only within the read thread.
  THREAD_CALLBACK_DECLARATION(P3DAuthSession, wt_thread_run);
  void wt_thread_run();

#ifdef _WIN32
  HANDLE win_create_process();
#else
  int posix_create_process();
#endif

private:
  P3DInstance *_inst;
  string _start_dir;

  // This information is passed to create_process().
  P3DTemporaryFile *_cert_filename;
  string _cert_dir;
  string _p3dcert_exe;
  string _env;

#ifdef _WIN32
  HANDLE _p3dcert_handle;
#else
  int _p3dcert_pid;
#endif
  bool _p3dcert_started;
  bool _p3dcert_running;

  // The remaining members are manipulated by or for the read thread.
  bool _started_wait_thread;
  THREAD _wait_thread;
};

#include "p3dAuthSession.I"

#endif
