// Filename: pStatThread.h
// Created by:  drose (11Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PSTATTHREAD_H
#define PSTATTHREAD_H

#include "pandabase.h"

#include "pStatClient.h"

////////////////////////////////////////////////////////////////////
//       Class : PStatThread
// Description : A lightweight class that represents a single thread
//               of execution to PStats.  It doesn't have any real
//               connection to any actual threads, but it's used to
//               differentiate tasks which run in different threads,
//               so we can measure them properly.  It is presently the
//               user's responsibility to correctly differentiate the
//               various threads.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatThread {
private:
  INLINE PStatThread();
  INLINE PStatThread(PStatClient *client, int index);

public:
  INLINE PStatThread(const string &name, PStatClient *client = NULL);

  INLINE PStatThread(const PStatThread &copy);
  INLINE void operator = (const PStatThread &copy);

  INLINE void new_frame();

private:
  PStatClient *_client;
  int _index;

friend class PStatClient;
friend class PStatCollector;
};

#include "pStatThread.I"

#endif

