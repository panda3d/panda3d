/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVInstructionCursor.cxx
 * @author rdb
 * @date 2026-07-07
 */

#include "spirVInstructionCursor.h"

/**
 * Removes the current instruction.  If it has a result id, the id is deleted
 * through the module (clearing its names, annotations and index entry) and
 * the instruction is tombstoned in place; otherwise, it is tombstoned
 * directly.  The cursor remains valid; next() skips the tombstone.
 */
void SpirVInstructionCursor::
remove() {
  nassertv(_index < _function.instructions.size());
  Instruction &op = _function.instructions[_index];
  Id result_id = op.get_result();
  if (result_id != 0) {
    _module.delete_id(result_id);
  } else {
    op = Instruction(spv::OpNop);
  }
}

/**
 * Resynchronizes the cursor after the given builder inserted code before the
 * current instruction.
 */
void SpirVInstructionCursor::
after_insert(const SpirVBuilder &builder) {
  nassertv(builder.get_current_function_id() == _function.id);
  _index = builder.get_insertion_index();
}

/**
 * Implements the second half of replace(): renumbers the final inserted
 * instruction, which must produce result_id, to take over the replaced
 * instruction's result id, and removes the replaced instruction.
 */
void SpirVInstructionCursor::
finish_replace(const SpirVBuilder &builder, size_t first_inserted, Id result_id) {
  nassertv(builder.get_current_function_id() == _function.id);
  pvector<Instruction> &instructions = _function.instructions;

  // The builder's cursor now points at the instruction being replaced, with
  // the inserted code in [first_inserted, pos) before it.
  size_t pos = builder.get_insertion_index();
  nassertv(pos < instructions.size());

  Id orig_id = instructions[pos].get_result();
  if (orig_id != 0) {
    // The final inserted instruction's value takes over the original result
    // id, so that downstream uses of it keep working without a rewrite.
    nassertv(result_id != 0);
    nassertv(pos > first_inserted);

    Instruction &replacement = instructions[pos - 1];
    nassertv(replacement.get_result() == result_id);
    replacement.args[replacement.has_result_type() ? 1 : 0] = orig_id;
    _module.record_result(replacement, _function.id);

    // The id allocated for the renumbered instruction no longer names
    // anything; clear its entry so the id index does not go stale.
    _module.clear_id_state(result_id);
  }
  else {
    // Replacing a resultless instruction; there is nothing to renumber.
    nassertv(result_id == 0);
  }

  // Erase (rather than tombstone) the replaced instruction: its result id
  // lives on in the replacement, so no state needs to be cleared, and
  // leaving a tombstone would serve no purpose.
  instructions.erase(instructions.begin() + pos);

  // Leave the cursor on the last inserted instruction (or just before where
  // the replaced instruction was), so that next() continues with the
  // instruction that followed it.
  _index = pos - 1;
}
