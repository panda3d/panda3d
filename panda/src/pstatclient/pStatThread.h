/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatThread.h
 * @author drose
 * @date 2000-07-11
 */

#ifndef PSTATTHREAD_H
#define PSTATTHREAD_H

#include "pandabase.h"

#include "pStatClient.h"
#include "pStatFrameData.h"

class Thread;

/**
 * A lightweight class that represents a single thread of execution to PStats.
 * It corresponds one-to-one with Panda's Thread instance.
 */
class EXPCL_PANDA_PSTATCLIENT PStatThread {
public:
  INLINE PStatThread();

PUBLISHED:
  INLINE PStatThread(PStatClient *client, int index);
  INLINE PStatThread(Thread *thread, PStatClient *client = nullptr);

  INLINE PStatThread(const PStatThread &copy);
  INLINE void operator = (const PStatThread &copy);

  void new_frame();
  void add_frame(const PStatFrameData &frame_data);

  Thread *get_thread() const;
  INLINE int get_index() const;

  MAKE_PROPERTY(thread, get_thread);
  MAKE_PROPERTY(index, get_index);

private:
  PStatClient *_client;
  int _index;

friend class PStatClient;
friend class PStatCollector;
};

#include "pStatThread.I"

#endif
