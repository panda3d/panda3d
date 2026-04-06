/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file completionToken.h
 * @author rdb
 * @date 2025-01-22
 */

#ifndef COMPLETIONTOKEN_H
#define COMPLETIONTOKEN_H

#include "pandabase.h"
#include "pnotify.h"
#include "completable.h"

/**
 * A completion token can be created from a callback, future or
 * CompletionCounter and can be passed into an asynchronous operation in order
 * to receive a signal when it is done.
 *
 * The asynchronous operation should call complete() on it when it is done,
 * with a boolean value indicating success or failure.  If the token is
 * destroyed prematurely, it is treated as if it called complete(false).
 *
 * This should be preferred over passing an AsyncFuture into a method since
 * a CompletionToken provides both more flexibility in use (due to accepting
 * an arbitrary callback) and more safety (since the RAII semantics guarantees
 * that the callback is never silently dropped).
 *
 * The token may only be moved, not copied.
 */
class EXPCL_PANDA_PUTIL CompletionToken {
public:
  constexpr CompletionToken() = default;

#ifndef CPPPARSER
  template<class Callable>
  INLINE CompletionToken(Callable callback);
#endif

  void complete(bool success);

protected:
  Completable _callback;

  friend class CompletionCounter;
};

#include "completionToken.I"

#endif
