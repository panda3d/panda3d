/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatCollector.h
 * @author drose
 * @date 2000-07-10
 */

#ifndef PSTATCOLLECTOR_H
#define PSTATCOLLECTOR_H

#include "pandabase.h"

#include "pStatThread.h"
#include "pStatClient.h"

class Thread;

/**
 * A lightweight class that represents a single element that may be timed
 * and/or counted via stats.
 *
 * Collectors can be used to measure two different kinds of values: elapsed
 * time, and "other".
 *
 * To measure elapsed time, call start() and stop() as appropriate to bracket
 * the section of code you want to time (or use a PStatTimer to do this
 * automatically).
 *
 * To measure anything else, call set_level() and/or add_level() to set the
 * "level" value associated with this collector.  The meaning of the value set
 * for the "level" is entirely up to the user; it may represent the number of
 * triangles rendered or the kilobytes of texture memory consumed, for
 * instance.  The level set will remain fixed across multiple frames until it
 * is reset via another set_level() or adjusted via a call to add_level().  It
 * may also be completely removed via clear_level().
 */
class EXPCL_PANDA_PSTATCLIENT PStatCollector {
#ifdef DO_PSTATS

private:
  INLINE PStatCollector(PStatClient *client, int index);

public:
  PStatCollector() = default;

PUBLISHED:
  INLINE explicit PStatCollector(const std::string &name,
                                 PStatClient *client = nullptr);
  INLINE explicit PStatCollector(const PStatCollector &parent,
                                 const std::string &name);

  INLINE PStatCollector(const PStatCollector &copy);
  INLINE void operator = (const PStatCollector &copy);

  INLINE bool is_valid() const;
  INLINE std::string get_name() const;
  INLINE std::string get_fullname() const;
  INLINE void output(std::ostream &out) const;

  INLINE bool is_active();
  INLINE bool is_started();
  INLINE void start();
  INLINE void stop();

  INLINE void clear_level();
  INLINE void set_level(double level);
  INLINE void add_level(double increment);
  INLINE void sub_level(double decrement);
  INLINE void add_level_now(double increment);
  INLINE void sub_level_now(double decrement);
  INLINE void flush_level();
  INLINE double get_level();

  INLINE void clear_thread_level();
  INLINE void set_thread_level(double level);
  INLINE void add_thread_level(double increment);
  INLINE void sub_thread_level(double decrement);
  INLINE double get_thread_level();

  INLINE bool is_active(const PStatThread &thread);
  INLINE bool is_started(const PStatThread &thread);
  INLINE void start(const PStatThread &thread);
  INLINE void start(const PStatThread &thread, double as_of);
  INLINE void stop(const PStatThread &thread);
  INLINE void stop(const PStatThread &thread, double as_of);

  INLINE void clear_level(const PStatThread &thread);
  INLINE void set_level(const PStatThread &thread, double level);
  INLINE void add_level(const PStatThread &thread, double increment);
  INLINE void sub_level(const PStatThread &thread, double decrement);
  INLINE double get_level(const PStatThread &thread);

  INLINE int get_index() const;

private:
  PStatClient *_client = nullptr;
  int _index = 0;
  double _level = 0.0;

friend class PStatClient;

#else  // DO_PSTATS
public:
  INLINE PStatCollector();

PUBLISHED:
  INLINE PStatCollector(const std::string &name,
                        PStatClient *client = nullptr);
  INLINE PStatCollector(const PStatCollector &parent,
                        const std::string &name);

  INLINE bool is_active() { return false; }
  INLINE bool is_started() { return false; }
  INLINE void start() { }
  INLINE void stop() { }

  INLINE void clear_level() { }
  INLINE void set_level(double) { }
  INLINE void add_level(double) { }
  INLINE void sub_level(double) { }
  INLINE void add_level_now(double) { }
  INLINE void sub_level_now(double) { }
  INLINE void flush_level() { }
  INLINE double get_level() { return 0.0; }

  INLINE bool is_active(const PStatThread &) { return false; }
  INLINE void start(const PStatThread &) { }
  INLINE void start(const PStatThread &, double) { }
  INLINE void stop(const PStatThread &) { }
  INLINE void stop(const PStatThread &, double) { }

  INLINE void clear_level(const PStatThread &) { }
  INLINE void set_level(const PStatThread &, double) { }
  INLINE void add_level(const PStatThread &, double) { }
  INLINE void sub_level(const PStatThread &, double) { }
  INLINE double get_level(const PStatThread &) { return 0.0; }

  INLINE int get_index() const { return 0; }

#endif  // DO_PSTATS
};

#include "pStatCollector.I"

inline std::ostream &operator << (std::ostream &out, const PStatCollector &pcol) {
#ifdef DO_PSTATS
  pcol.output(out);
#endif  // DO_PSTATS
  return out;
}

#endif
