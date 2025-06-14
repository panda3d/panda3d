/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file completionCounter.cxx
 * @author rdb
 * @date 2025-01-24
 */

#include "completionCounter.h"

/**
 * Called when a token is completed before then() is called.
 */
void CompletionCounter::
initial_callback(Completable::Data *data_ptr, bool success) {
  CounterData &data = *(CounterData *)data_ptr;
  auto prev_count = data._counter.fetch_add((success ? 0 : 0x10000) - 1, std::memory_order_release);
  if ((prev_count & 0xffff) == 1) {
    // We're done early.
    auto prev_callback = data._function.exchange(nullptr, std::memory_order_acq_rel);
    nassertv(prev_callback != nullptr);

    // Someone called then() in the meantime.  Call the new callback.  The
    // refcount will drop below 0 when that's called but they are designed to
    // handle that.
    if (prev_callback != &initial_callback) {
      prev_callback(data_ptr, success && (prev_count & ~0xffff) == 0);
    }
  }
}

/**
 * Called when a token is completed after this object is destroyed without
 * then() being called.
 */
void CompletionCounter::
abandon_callback(Completable::Data *data_ptr, bool success) {
  CounterData &data = *(CounterData *)data_ptr;
  auto prev_count = data._counter.fetch_sub(1, std::memory_order_relaxed);
  if ((prev_count & 0xffff) <= 1) {
    // Done.
    auto prev_callback = data._function.exchange(nullptr, std::memory_order_relaxed);
    nassertv(prev_callback != nullptr);
    nassertv(prev_callback == &abandon_callback);
    delete &data;
  }
}
