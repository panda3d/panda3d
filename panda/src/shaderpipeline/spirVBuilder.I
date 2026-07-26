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

/**
 * Makes sure that an input variable with the given builtin exists, returning
 * it wrapped for use in the body.
 */
INLINE SpirVPointer SpirVBuilder::
ensure_builtin_input(spv::BuiltIn builtin) {
  return Pointer(*this, _module.ensure_builtin_input(builtin));
}

/**
 * Makes sure that an output variable with the given builtin exists, returning
 * it wrapped for use in the body.
 */
INLINE SpirVPointer SpirVBuilder::
ensure_builtin_output(spv::BuiltIn builtin) {
  return Pointer(*this, _module.ensure_builtin_output(builtin));
}

/**
 * Returns the given value as a module-scope bool constant, wrapped for use
 * in the body.
 */
INLINE SpirVValue SpirVBuilder::
const_bool(bool value) {
  return Value(*this, _module.define_constant(ShaderType::BOOL, value));
}

/**
 * Returns the given value as a module-scope int constant, wrapped for use in
 * the body.
 */
INLINE SpirVValue SpirVBuilder::
const_int(int32_t value) {
  return Value(*this, _module.define_int_constant(value));
}

/**
 * Returns the given value as a module-scope uint constant, wrapped for use in
 * the body.
 */
INLINE SpirVValue SpirVBuilder::
const_uint(uint32_t value) {
  return Value(*this, _module.define_constant(ShaderType::UINT, value));
}

/**
 * Returns the given value as a module-scope float constant, wrapped for use
 * in the body.
 */
INLINE SpirVValue SpirVBuilder::
const_float(float value) {
  return Value(*this, _module.define_float_constant(value));
}

/**
 * Builds a composite of the given type from the given constituents, of which
 * a number materializes as a constant of the type's own scalar type.
 */
INLINE SpirVValue SpirVBuilder::
construct(const ShaderType *type,
          std::initializer_list<glsl::Argument> constituents) {
  return construct(type, constituents.begin(), constituents.size());
}

/**
 * Inserts an OpAccessChain with the given base id (which must be a pointer)
 * and the ids of the member/array indices (constants or values).
 */
INLINE SpirVId SpirVBuilder::
op_access_chain(Id var_id, std::initializer_list<Id> chain) {
  return op_access_chain(var_id, chain.begin(), chain.size());
}

/**
 * Inserts an OpAccessChain with the given base id (which must be a pointer)
 * and the ids of the member/array indices (constants or values).
 */
INLINE SpirVId SpirVBuilder::
op_access_chain(Id var_id, const pvector<Id> &chain) {
  return op_access_chain(var_id, chain.data(), chain.size());
}
