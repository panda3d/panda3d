// Filename: pStatThread.h
// Created by:  drose (11Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATTHREAD_H
#define PSTATTHREAD_H

#include <pandabase.h>

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

