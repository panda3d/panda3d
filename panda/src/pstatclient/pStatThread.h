// Filename: pStatThread.h
// Created by:  drose (11Jul00)
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

#ifndef PSTATTHREAD_H
#define PSTATTHREAD_H

#include "pandabase.h"

#include "pStatClient.h"

class Thread;

////////////////////////////////////////////////////////////////////
//       Class : PStatThread
// Description : A lightweight class that represents a single thread
//               of execution to PStats.  It corresponds one-to-one
//               with Panda's Thread instance.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PSTATCLIENT PStatThread {
private:
  INLINE PStatThread();
  INLINE PStatThread(PStatClient *client, int index);

PUBLISHED:
  INLINE PStatThread(Thread *thread, PStatClient *client = NULL);

  INLINE PStatThread(const PStatThread &copy);
  INLINE void operator = (const PStatThread &copy);

  INLINE void new_frame();

  Thread *get_thread() const;
  INLINE int get_index() const;

private:
  PStatClient *_client;
  int _index;

friend class PStatClient;
friend class PStatCollector;
};

#include "pStatThread.I"

#endif

