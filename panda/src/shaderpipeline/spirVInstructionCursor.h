/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVInstructionCursor.h
 * @author rdb
 * @date 2026-07-07
 */

#ifndef SPIRVINSTRUCTIONCURSOR_H
#define SPIRVINSTRUCTIONCURSOR_H

#include "spirVModule.h"
#include "spirVBuilder.h"

#include <type_traits>

/**
 * Iterates over the instructions of a function in a SPIR-V module, while
 * allowing rewriting them.  This serves to assist transformation passes to get
 * the fiddly details of in-place-rewriting right.
 *
 * The three rewriting methods (insert_before, insert_after, replace) all take
 * a callable which is to perform the rewriting operation, whereas remove() and
 * detach() get rid of the current instruction.  The cursor remains valid
 * throughout all this (though the Instruction reference may not be); next()
 * is used to advance the cursor.
 *
 * Please note that existing cursors are invalidated if the function list
 * on the SpirVModule changes.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVInstructionCursor {
public:
  using Instruction = SpirVModule::Instruction;
  using Function = SpirVModule::Function;
  using Id = SpirVId;

  INLINE SpirVInstructionCursor(SpirVModule &module, Function &function);

  INLINE bool next();

  // Access to the current instruction.  The reference is invalidated by
  // insert_before() and replace(); take it again afterwards.
  INLINE Instruction &operator *();
  INLINE Instruction *operator ->();

  INLINE size_t get_index() const;
  INLINE SpirVModule &get_module();
  INLINE Function &get_function();

  template<class Callable>
  INLINE void insert_before(Callable &&fn);

  template<class Callable>
  INLINE void insert_after(Callable &&fn);

  template<class Callable>
  INLINE void replace(Callable &&fn);

  void remove();
  INLINE void detach();

private:
  void after_insert(const SpirVBuilder &builder);
  void finish_replace(const SpirVBuilder &builder, size_t first_inserted, Id result_id);

  SpirVModule &_module;
  Function &_function;
  size_t _index = (size_t)-1;
};

#include "spirVInstructionCursor.I"

#endif
