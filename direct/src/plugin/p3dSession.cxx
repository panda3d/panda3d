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
  _python = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::Destructor
//       Access: Public
//  Description: Terminates the session by shutting down Python and
//               stopping the subprocess.
////////////////////////////////////////////////////////////////////
P3DSession::
~P3DSession() {
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
  assert(_python == NULL);
  assert(inst->_session == NULL);
  assert(inst->get_session_key() == _session_key);
  assert(inst->get_python_version() == _python_version);

  inst->_session = this;
  bool inserted = _instances.insert(inst).second;
  assert(inserted);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DPython *python = inst_mgr->start_python(_python_version);
  if (python != NULL) {
    python->start_session(this, inst);
  }
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
  if (_python != NULL) {
    _python->terminate_session(this);
    assert(_python == NULL);
  }

  if (inst->_session == this) {
    inst->_session = NULL;
    _instances.erase(inst);
  }
}
