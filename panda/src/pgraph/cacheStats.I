/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cacheStats.I
 * @author drose
 * @date 2007-07-24
 */

/**
 * Outputs a report if enough time has elapsed.
 */
INLINE void CacheStats::
maybe_report(const char *name) {
#ifndef NDEBUG
  if (UNLIKELY(_cache_report)) {
    double now = ClockObject::get_global_clock()->get_real_time();
    if (now - _last_reset < _cache_report_interval) {
      return;
    }
    write(Notify::out(), name);
    reset(now);
  }
#endif  // NDEBUG
}

/**
 * Increments by 1 the count of cache hits.
 */
INLINE void CacheStats::
inc_hits() {
#ifndef NDEBUG
  ++_cache_hits;
#endif // NDEBUG
}

/**
 * Increments by 1 the count of cache misses.
 */
INLINE void CacheStats::
inc_misses() {
#ifndef NDEBUG
  ++_cache_misses;
#endif // NDEBUG
}

/**
 * Increments by 1 the count of elements added to the cache.  If is_new is
 * true, the element was added to a previously empty hashtable.
 */
INLINE void CacheStats::
inc_adds(bool is_new) {
#ifndef NDEBUG
  if (is_new) {
    ++_cache_new_adds;
  }
  ++_cache_adds;
#endif // NDEBUG
}

/**
 * Increments by 1 the count of elements removed from the cache.
 */
INLINE void CacheStats::
inc_dels() {
#ifndef NDEBUG
  ++_cache_dels;
#endif // NDEBUG
}

/**
 * Adds the indicated count (positive or negative) to the total number of
 * entries for the cache (net occupied size of all the hashtables).
 */
INLINE void CacheStats::
add_total_size(int count) {
#ifndef NDEBUG
  _total_cache_size += count;
#endif  // NDEBUG
}

/**
 * Adds the indicated count (positive or negative) to the total count of
 * individual RenderState or TransformState objects.
 */
INLINE void CacheStats::
add_num_states(int count) {
#ifndef NDEBUG
  _num_states += count;
#endif  // NDEBUG
}
