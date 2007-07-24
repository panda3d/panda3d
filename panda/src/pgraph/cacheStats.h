// Filename: cacheStats.h
// Created by:  drose (24Jul07)
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

#ifndef CACHESTATS_H
#define CACHESTATS_H

#include "pandabase.h"
#include "clockObject.h"
#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//       Class : CacheStats
// Description : This is used to track the utilization of the
//               TransformState and RenderState caches, for low-level
//               performance tuning information.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH CacheStats {
public:
  void init();
  void reset(double now);
  void write(ostream &out, const char *name) const;
  INLINE void maybe_report(const char *name);

  INLINE void inc_hits();
  INLINE void inc_misses();
  INLINE void inc_adds(bool is_new);
  INLINE void inc_dels();
  INLINE void add_total_size(int count);
  INLINE void add_num_states(int count);

private:
#ifndef NDEBUG
  int _cache_hits;
  int _cache_misses;
  int _cache_adds;
  int _cache_new_adds;
  int _cache_dels;
  int _total_cache_size;
  int _num_states;
  double _last_reset;

  bool _cache_report;
  double _cache_report_interval;
#endif  // NDEBUG
};

#include "cacheStats.I"

#endif
