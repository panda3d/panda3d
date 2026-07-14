/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVBuilder.I
 * @author rdb
 * @date 2026-07-07
 */

/**
 *
 */
INLINE SpirVModule &SpirVBuilder::
get_module() {
  return _module;
}

/**
 * Returns the instruction index within the current function at which the
 * next instruction will be inserted.
 */
INLINE size_t SpirVBuilder::
get_insertion_index() const {
  return _index;
}

/**
 * Returns the id of the function the cursor is currently inside, or 0.
 */
INLINE SpirVId SpirVBuilder::
get_current_function_id() const {
  return _current_function_id;
}

/**
 * Allocates a fresh identifier.
 */
INLINE SpirVId SpirVBuilder::
allocate_id() {
  return _module.allocate_id();
}

/**
 * Inserts an arbitrary instruction at the cursor.
 */
INLINE void SpirVBuilder::
insert(spv::Op opcode, std::initializer_list<uint32_t> args) {
  insert(Instruction(opcode, args));
}
