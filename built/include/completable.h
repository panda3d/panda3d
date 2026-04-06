/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file completable.h
 * @author rdb
 * @date 2025-01-22
 */

#ifndef COMPLETABLE_H
#define COMPLETABLE_H

#include "pandabase.h"
#include "patomic.h"

/**
 * Stores a type-erased callable that is move-only.  May only be called once.
 */
class EXPCL_PANDA_PUTIL Completable {
public:
  constexpr Completable() = default;

#ifndef CPPPARSER
  template<class Callable>
  INLINE Completable(Callable callback);
#endif

  INLINE Completable(const Completable &copy) = delete;
  INLINE Completable(Completable &&from) noexcept;

  INLINE Completable &operator =(const Completable &copy) = delete;
  INLINE Completable &operator =(Completable &&from);

  INLINE void operator ()();

  INLINE ~Completable();

protected:
  // There are several design approaches here:
  // 1. Optimize for no data block: do not require dynamic allocation of a data
  //    block in the simple case where the callback data is only the size of a
  //    single pointer.  Store two pointers, one function pointer and a data
  //    pointer(-sized storage), directly on the class here.
  // 2. Optimize for a data block: store the function pointer on the data block,
  //    always requiring dynamic allocation.
  //
  // Right now I have opted for 2 because it allows the function pointer to be
  // dynamically swapped (used in CompletionCounter), but this decision may
  // change in the future.

  struct Data;
  typedef void CallbackFunction(Data *, bool);

  struct Data {
    patomic<CallbackFunction *> _function { nullptr };
  };

  template<typename Lambda>
  struct LambdaData : public Data {
    // Must unfortunately be defined inline, since this struct is protected.
    LambdaData(Lambda lambda, CallbackFunction *function) :
      _lambda(std::move(lambda)) {
      _function = function;
    }

    Lambda _lambda;
  };

  Data *_data = nullptr;

  friend class AsyncFuture;
  friend class CompletionCounter;
  friend class CompletionToken;
};

#include "completable.I"

#endif
