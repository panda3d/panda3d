/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVPointer.I
 * @author rdb
 * @date 2026-07-15
 */

/**
 * Wraps the given id, which must name a pointer (a variable, or the result
 * of an existing access chain).
 */
INLINE SpirVPointer::
SpirVPointer(SpirVBuilder &builder, Id var_id) :
  _builder(&builder),
  _base(var_id) {
}

/**
 * Returns true if this pointer was bound to a variable.
 */
INLINE SpirVPointer::
operator bool() const {
  return (bool)_base;
}

/**
 * Returns the builder this pointer refers into.
 */
INLINE SpirVBuilder *SpirVPointer::
get_builder() const {
  return _builder;
}

/**
 * Emits the accumulated OpAccessChain (if this pointer was indexed) followed
 * by an OpAtomicIAdd of the given value, returning the value the memory held
 * before the addition.  A literal materializes against the pointed-to type.
 */
INLINE glsl::Value SpirVPointer::
atomic_add(glsl::Argument value) const {
  nassertr(_builder != nullptr, glsl::Value());

  return emit_atomic(spv::OpAtomicIAdd, &value);
}

/**
 * Emits the accumulated OpAccessChain (if this pointer was indexed) followed
 * by an OpAtomicIIncrement, returning the value the memory held before the
 * increment.
 */
INLINE glsl::Value SpirVPointer::
atomic_increment() const {
  nassertr(_builder != nullptr, glsl::Value());

  return emit_atomic(spv::OpAtomicIIncrement, nullptr);
}

/**
 *
 */
INLINE std::ostream &
operator << (std::ostream &out, const SpirVPointer &pointer) {
  pointer.output(out);
  return out;
}
