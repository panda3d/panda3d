/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cacheStats.h
 * @author drose
 * @date 2007-07-24
 */

#ifndef CACHESTATS_H
#define CACHESTATS_H

#include "pandabase.h"
#include "clockObject.h"
#include "pnotify.h"

/**
 * This is used to track the utilization of the TransformState and RenderState
 * caches, for low-level performance tuning information.
 */
class EXPCL_PANDA_PGRAPH CacheStats {
public:
  CacheStats() = default;
  void init();
  void reset(double now);
  void write(std::ostream &out, const char *name) const;
  INLINE void maybe_report(const char *name);

  INLINE void inc_hits();
  INLINE void inc_misses();
  INLINE void inc_adds(bool is_new);
  INLINE void inc_dels();
  INLINE void add_total_size(int count);
  INLINE void add_num_states(int count);

private:
#ifndef NDEBUG
  int _cache_hits = 0;
  int _cache_misses = 0;
  int _cache_adds = 0;
  int _cache_new_adds = 0;
  int _cache_dels = 0;
  int _total_cache_size = 0;
  int _num_states = 0;
  double _last_reset = 0.0;

  bool _cache_report = false;
  double _cache_report_interval = 0.0;
#endif  // NDEBUG
};

#include "cacheStats.I"

#endif
