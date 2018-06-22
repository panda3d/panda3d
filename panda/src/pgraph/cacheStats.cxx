/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cacheStats.cxx
 * @author drose
 * @date 2007-07-24
 */

#include "cacheStats.h"

/**
 * Initializes the CacheStats for the first time.  We don't use the
 * constructor for this, since we can't guarantee ordering of static
 * constructors.
 */
void CacheStats::
init() {
#ifndef NDEBUG
  // Let's not use the clock at static init time.
  //reset(ClockObject::get_global_clock()->get_real_time());

  _cache_report = ConfigVariableBool("cache-report", false);
  _cache_report_interval = ConfigVariableDouble("cache-report-interval", 5.0);
#endif  // NDEBUG
}

/**
 * Reinitializes just those parts of the CacheStats that should be reset
 * between each reporting interval.
 */
void CacheStats::
reset(double now) {
#ifndef NDEBUG
  _cache_hits = 0;
  _cache_misses = 0;
  _cache_adds = 0;
  _cache_new_adds = 0;
  _cache_dels = 0;
  _last_reset = now;
#endif  // NDEBUG
}

/**
 *
 */
void CacheStats::
write(std::ostream &out, const char *name) const {
#ifndef NDEBUG
  out << name << " cache: " << _cache_hits << " hits, "
      << _cache_misses << " misses\n"
      << _cache_adds + _cache_new_adds << "(" << _cache_new_adds << ") adds(new), "
      << _cache_dels << " dels, "
      << _total_cache_size << " / " << _num_states << " = "
      << (double)_total_cache_size / (double)_num_states
      << " average cache size\n";
#endif  // NDEBUG
}
