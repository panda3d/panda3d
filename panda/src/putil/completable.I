/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file completable.I
 * @author rdb
 * @date 2025-01-22
 */

#ifndef CPPPARSER
/**
 *
 */
template<class Callable>
INLINE Completable::
Completable(Callable callback) :
  _data(new LambdaData<Callable>(std::move(callback), [](Data *data, bool do_run) {
    LambdaData<Callable> *self = (LambdaData<Callable> *)data;
    if (do_run) {
      std::move(self->_lambda)();
    }
    delete self;
  })) {
}
#endif

/**
 *
 */
INLINE Completable::
Completable(Completable &&from) noexcept :
  _data(from._data) {
  from._data = nullptr;
}

/**
 *
 */
INLINE Completable &Completable::
operator =(Completable &&from) {
  Data *data = _data;
  _data = from._data;
  from._data = nullptr;
  if (data != nullptr) {
    data->_function.load(std::memory_order_relaxed)(data, false);
  }
  return *this;
}

/**
 *
 */
INLINE Completable::
~Completable() {
  Data *data = _data;
  if (data != nullptr) {
    data->_function.load(std::memory_order_relaxed)(data, false);
  }
}

/**
 *
 */
INLINE void Completable::
operator ()() {
  Data *data = _data;
  _data = nullptr;
  if (data != nullptr) {
    data->_function.load(std::memory_order_relaxed)(data, true);
  }
}
