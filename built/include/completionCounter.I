/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file completionCounter.I
 * @author rdb
 * @date 2025-01-22
 */

/**
 *
 */
INLINE CompletionCounter::
~CompletionCounter() {
  CounterData *data = _data;
  if (data != nullptr) {
    // then() is not called; we still need something that destructs the data
    // when done.
    auto prev_function = data->_function.exchange(&abandon_callback, std::memory_order_relaxed);
    if (prev_function == nullptr) {
      // Was already done.
      delete data;
    }
  }
}

/**
 * Returns a new token.  May not be called after then().
 */
INLINE CompletionToken CompletionCounter::
make_token() {
  CompletionToken token;
  if (_data == nullptr) {
    _data = new CounterData;
    _data->_function = &initial_callback;
  }
  auto old_value = _data->_counter.fetch_add(1);
  nassertr(old_value >= 0, token);
  token._callback._data = _data;
  return token;
}

/**
 * Runs the given callback immediately upon completion.  If the counter is
 * already done, runs it immediately.  This requires an rvalue because it
 * consumes the counter, use std::move() if you don't have an rvalue.
 *
 * The callback will either be called immediately or directly when the last
 * token calls complete(), however, it may also be called if a token is
 * destroyed.  This may happen at unexpected times, such as when the lambda
 * holding the token is destroyed prematurely.  In this case, however, the
 * passed success argument will always be false.
 */
template<class Callable>
INLINE void CompletionCounter::
then(Callable callable) && {
  // Replace the callback pointer with something that calls the given callable
  // once the count reaches 0.
  CounterData *data = _data;
  nassertv(data != nullptr);
  _data = nullptr;
  if (data->_function.load(std::memory_order_acquire) == nullptr) {
    // Already done.
    callable((data->_counter.load(std::memory_order_relaxed) & ~0xffff) == 0);
    delete data;
    return;
  }

  static_assert(sizeof(Callable) <= sizeof(data->_storage),
    "raise storage size in completionCounter.h or reduce lambda captures");

  new (data->_storage) Callable(std::move(callable));

  Completable::CallbackFunction *new_function =
    [] (Completable::Data *data_ptr, bool success) {
      CounterData *data = (CounterData *)data_ptr;
      auto prev_count = data->_counter.fetch_add((success ? 0 : 0x10000) - 1, std::memory_order_release);
      if ((short)(prev_count & 0xffff) > 1) {
        return;
      }

      Callable *callable = (Callable *)data->_storage;
      std::move(*callable)(success && (prev_count & ~0xffff) == 0);
      callable->~Callable();
      delete data;
    };

  auto prev_function = data->_function.exchange(new_function, std::memory_order_acq_rel);
  if (UNLIKELY(prev_function == nullptr)) {
    // Last token finished in the meantime.
    new_function(data, (data->_counter.load(std::memory_order_relaxed) & ~0xffff) == 0);
  }
}
