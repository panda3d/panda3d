/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file completionCounter.h
 * @author rdb
 * @date 2025-01-22
 */

#ifndef COMPLETIONCOUNTER_H
#define COMPLETIONCOUNTER_H

#include "pandabase.h"
#include "completionToken.h"

#include <cstddef>

/**
 * Shared counter that generates "completion tokens" incrementing a counter,
 * which will decrement the counter once they are finished.  After the tokens
 * are handed out, a callback may be registered using then(), which will be
 * called as soon as the last token is done.
 */
class EXPCL_PANDA_PUTIL CompletionCounter {
public:
  constexpr CompletionCounter() = default;
  CompletionCounter(const CompletionCounter &copy) = delete;

  INLINE ~CompletionCounter();

  INLINE CompletionToken make_token();

  template<class Callable>
  INLINE void then(Callable callable) &&;

private:
  static void initial_callback(Completable::Data *data, bool success);
  static void abandon_callback(Completable::Data *data, bool success);

protected:
  struct CounterData : public Completable::Data {
    // Least significant half is counter, most significant half is error count
    patomic_signed_lock_free _counter { 0 };

    // Just raise this if the static_assert fires (or limit the size of your
    // lambda captures).
    alignas(std::max_align_t) unsigned char _storage[64];
  };
  CounterData *_data = nullptr;
};

#include "completionCounter.I"

#endif
