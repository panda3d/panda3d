// Filename: p3dSession.cxx
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

#include "p3dSession.h"
#include "p3dInstance.h"
#include "p3dInstanceManager.h"
#include <tinyxml.h>

#include <malloc.h>

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::Constructor
//       Access: Public
//  Description: Creates a new session, corresponding to a new
//               subprocess with its own copy of Python.  The initial
//               parameters for the session are taken from the
//               indicated instance object (but the instance itself is
//               not automatically started within the session).
////////////////////////////////////////////////////////////////////
P3DSession::
P3DSession(P3DInstance *inst) {
  _session_key = inst->get_session_key();
  _python_version = inst->get_python_version();
  _python_root_dir = "C:/p3drun";

  string p3dpython = "c:/cygwin/home/drose/player/direct/built/bin/p3dpython.exe";

  // Populate the new process' environment.
  string env;

  // These are the enviroment variables we forward from the current
  // environment, if they are set.
  const char *keep[] = {
    "TEMP", "HOME", "USER", 
#ifdef _WIN32
    "SYSTEMROOT",
#endif
    NULL
  };
  for (int ki = 0; keep[ki] != NULL; ++ki) {
    char *value = getenv(keep[ki]);
    if (value != NULL) {
      env += keep[ki];
      env += "=";
      env += value;
      env += '\0';
    }
  }

  // Define some new environment variables.
  env += "PATH=";
  env += _python_root_dir;
  env += '\0';

  env += "PYTHONPATH=";
  env += _python_root_dir;
  env += '\0';

  // Create a bi-directional pipe to communicate with the sub-process.
#ifdef _WIN32
  HANDLE r_to, w_to, r_from, w_from;

  // Create the pipe to the process.
  if (!CreatePipe(&r_to, &w_to, NULL, 0)) {
    cerr << "failed to create pipe\n";
  } else {
    // Make sure the right end of the pipe is inheritable.
    SetHandleInformation(r_to, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(w_to, HANDLE_FLAG_INHERIT, 0);
  }

  // Create the pipe from the process.
  if (!CreatePipe(&r_from, &w_from, NULL, 0)) {
    cerr << "failed to create pipe\n";
  } else { 
    // Make sure the right end of the pipe is inheritable.
    SetHandleInformation(w_from, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(r_from, HANDLE_FLAG_INHERIT, 0);
  }

  // Make sure we see an error dialog if there is a missing DLL.
  SetErrorMode(0);

  // Pass the appropriate ends of the bi-directional pipe as the
  // standard input and standard output of the child process.
  STARTUPINFO startup_info;
  ZeroMemory(&startup_info, sizeof(STARTUPINFO));
  startup_info.cb = sizeof(startup_info); 
  startup_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  startup_info.hStdOutput = w_from;
  startup_info.hStdInput = r_to;
  startup_info.dwFlags |= STARTF_USESTDHANDLES;
 
  BOOL result = CreateProcess
    (p3dpython.c_str(), NULL, NULL, NULL, TRUE, 0,
     (void *)env.c_str(), _python_root_dir.c_str(),
     &startup_info, &_p3dpython);
  _started_p3dpython = (result != 0);

  if (!_started_p3dpython) {
    cerr << "Failed to create process.\n";
  } else {
    cerr << "Created process: " << _p3dpython.dwProcessId << "\n";
  }

  // Close the pipe handles that are now owned by the child.
  CloseHandle(w_from);
  CloseHandle(r_to);

  _pipe_read.open_read(r_from);
  _pipe_write.open_write(w_to);
#endif  // _WIN32
  if (!_pipe_read) {
    cerr << "unable to open read pipe\n";
  }
  if (!_pipe_write) {
    cerr << "unable to open write pipe\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::Destructor
//       Access: Public
//  Description: Terminates the session by shutting down Python and
//               stopping the subprocess.
////////////////////////////////////////////////////////////////////
P3DSession::
~P3DSession() {
  if (_started_p3dpython) {
    cerr << "Terminating process.\n";
    // Messy.  Shouldn't use TerminateProcess unless necessary.
    TerminateProcess(_p3dpython.hProcess, 2);

    CloseHandle(_p3dpython.hProcess);
    CloseHandle(_p3dpython.hThread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::start_instance
//       Access: Public
//  Description: Adds the indicated instance to the session, and
//               starts it running.  It is an error if the instance
//               has been started anywhere else.
//
//               The instance must have the same session_key as the
//               one that was passed to the P3DSession constructor.
////////////////////////////////////////////////////////////////////
void P3DSession::
start_instance(P3DInstance *inst) {
  assert(inst->_session == NULL);
  assert(inst->get_session_key() == _session_key);
  assert(inst->get_python_version() == _python_version);

  inst->_session = this;
  bool inserted = _instances.insert(inst).second;
  assert(inserted);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  TiXmlDocument doc;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
  TiXmlElement *xinstance = inst->make_xml();
  
  doc.LinkEndChild(decl);
  doc.LinkEndChild(xinstance);

  cerr << "sending: " << doc << "\n";
  _pipe_write << doc << flush;

  /*
  P3DPython *python = inst_mgr->start_python(_python_version);
  if (python != NULL) {
    python->start_session(this, inst);
  }
  */
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::terminate_instance
//       Access: Public
//  Description: Removes the indicated instance from the session, and
//               stops it.  It is an error if the instance is not
//               already running on this session.
////////////////////////////////////////////////////////////////////
void P3DSession::
terminate_instance(P3DInstance *inst) {
  /*
  if (_python != NULL) {
    _python->terminate_session(this);
    assert(_python == NULL);
  }
  */

  if (inst->_session == this) {
    inst->_session = NULL;
    _instances.erase(inst);
  }
}
