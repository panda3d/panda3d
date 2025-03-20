/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file completionToken.I
 * @author rdb
 * @date 2025-01-22
 */

#ifndef CPPPARSER
/**
 * Creates a token that calls the given callback when it's done, passing it
 * true on success and false on failure or abandonment.
 */
template<class Callable>
INLINE CompletionToken::
CompletionToken(Callable callback) {
  // Main difference over a Completable is that this will always call the
  // callback, even on failure, so that cleanup can be done.
  _callback._data = new Completable::LambdaData<Callable>(std::move(callback), [](Completable::Data *data, bool success) {
    Completable::LambdaData<Callable> *self = (Completable::LambdaData<Callable> *)data;
    std::move(self->_lambda)(success);
    delete self;
  });
}
#endif

/**
 *
 */
INLINE void CompletionToken::
complete(bool success) {
  Completable::Data *data = _callback._data;
  if (data != nullptr) {
    _callback._data = nullptr;
    data->_function.load(std::memory_order_relaxed)(data, success);
  }
}
