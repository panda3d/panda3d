// Filename: pStatCollector.h
// Created by:  drose (10Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PSTATCOLLECTOR_H
#define PSTATCOLLECTOR_H

#include "pandabase.h"

#include "pStatThread.h"
#include "pStatClient.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : PStatCollector
// Description : A lightweight class that represents a single element
//               that may be timed and/or counted via stats.
//
//               Collectors can be used to measure two different kinds
//               of values: elapsed time, and "other".
//
//               To measure elapsed time, call start() and stop() as
//               appropriate to bracket the section of code you want
//               to time (or use a PStatTimer to do this
//               automatically).
//
//               To measure anything else, call set_level() and/or
//               add_level() to set the "level" value associated with
//               this collector.  The meaning of the value set for the
//               "level" is entirely up to the user; it may represent
//               the number of triangles rendered or the kilobytes of
//               texture memory consumed, for instance.  The level set
//               will remain fixed across multiple frames until it is
//               reset via another set_level() or adjusted via a call
//               to add_level().  It may also be completely removed
//               via clear_level().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatCollector {
#ifdef DO_PSTATS

private:
  INLINE PStatCollector();
  INLINE PStatCollector(PStatClient *client, int index);

PUBLISHED:
  INLINE PStatCollector(const string &name,
                        PStatClient *client = NULL);
  INLINE PStatCollector(const PStatCollector &parent,
                        const string &name);

  INLINE PStatCollector(const PStatCollector &copy);
  INLINE void operator = (const PStatCollector &copy);

  INLINE bool is_active();
  INLINE void start();
  INLINE void stop();

  INLINE void clear_level();
  INLINE void set_level(float level);
  INLINE void add_level(float increment);
  INLINE void sub_level(float decrement);
  INLINE float get_level();

public:
  INLINE bool is_active(const PStatThread &thread);
  INLINE void start(const PStatThread &thread);
  INLINE void start(const PStatThread &thread, float as_of);
  INLINE void stop(const PStatThread &thread);
  INLINE void stop(const PStatThread &thread, float as_of);

  INLINE void clear_level(const PStatThread &thread);
  INLINE void set_level(const PStatThread &thread, float level);
  INLINE void add_level(const PStatThread &thread, float increment);
  INLINE void sub_level(const PStatThread &thread, float decrement);
  INLINE float get_level(const PStatThread &thread);

private:
  PStatClient *_client;
  int _index;

friend class PStatClient;

#else  // DO_PSTATS
PUBLISHED:
  INLINE PStatCollector(const string &name,
                        PStatClient *client = NULL);
  INLINE PStatCollector(const PStatCollector &parent,
                        const string &name);

  INLINE bool is_active() { return false; }
  INLINE void start() { }
  INLINE void stop() { }

  INLINE void clear_level() { }
  INLINE void set_level(float) { }
  INLINE void add_level(float) { }
  INLINE void sub_level(float) { }
  INLINE float get_level() { return 0.0; }

public:
  INLINE bool is_active(const PStatThread &) { return false; }
  INLINE void start(const PStatThread &) { }
  INLINE void start(const PStatThread &, float) { }
  INLINE void stop(const PStatThread &) { }
  INLINE void stop(const PStatThread &, float) { }

  INLINE void clear_level(const PStatThread &) { }
  INLINE void set_level(const PStatThread &, float) { }
  INLINE void add_level(const PStatThread &, float) { }
  INLINE void sub_level(const PStatThread &, float) { }
  INLINE float get_level(const PStatThread &) { return 0.0; }

#endif  // DO_PSTATS
};

#include "pStatCollector.I"

#endif

