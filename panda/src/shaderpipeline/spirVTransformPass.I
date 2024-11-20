/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVTransformPass.I
 * @author rdb
 * @date 2024-10-08
 */

/**
 * Returns the type ID of the given ID.
 */
INLINE uint32_t SpirVTransformPass::
get_type_id(uint32_t id) const {
  const Definition &def = _db.get_definition(id);
  nassertr(def._type_id != 0, 0);
  nassertr(is_defined(id) || def.is_function(), def._type_id);
  return def._type_id;
}

/**
 * Unwraps the given type pointer, checking that it has been defined already in
 * this pass.
 */
INLINE uint32_t SpirVTransformPass::
unwrap_pointer_type(uint32_t id) const {
  const Definition &def = _db.get_definition(id);
  if (!def.is_pointer_type()) {
    return error_expected(id, "a pointer type");
  }
  uint32_t type_id = def._type_id;
  nassertr(is_defined(id), type_id);
  return type_id;
}

/**
 * Looks up the given constant, checking that it has been defined already in
 * this pass.
 */
INLINE uint32_t SpirVTransformPass::
resolve_constant(uint32_t id) const {
  const Definition &def = _db.get_definition(id);
  if (!def.is_constant()) {
    return error_expected(id, "a constant");
  }
  uint32_t constant = def._constant;
  nassertr(is_defined(id), constant);
  return constant;
}

/**
 * Looks up the given type, checking that it has been defined already in this
 * pass.
 */
INLINE const ShaderType *SpirVTransformPass::
resolve_type(uint32_t id) const {
  const Definition &def = _db.get_definition(id);
  if (!def.is_type() || def.is_pointer_type()) {
    error_expected(id, "a non-pointer type");
    return nullptr;
  }
  nassertr(def.is_type() && !def.is_pointer_type(), nullptr);
  nassertr(is_defined(id), def._type);
  return def._type;
}

/**
 * Looks up the type underneath the given type pointer, checking that it has
 * been defined already in this pass.
 */
INLINE const ShaderType *SpirVTransformPass::
resolve_pointer_type(uint32_t id) const {
  const Definition &def = _db.get_definition(id);
  if (!def.is_pointer_type()) {
    error_expected(id, "a pointer type");
    return nullptr;
  }
  nassertr(is_defined(id), def._type);
  return def._type;
}

/**
 *
 */
INLINE uint32_t SpirVTransformPass::
get_id_bound() const {
  return _new_preamble[3];
}

/**
 * Allocates a fresh identifier and returns it.
 */
INLINE uint32_t SpirVTransformPass::
allocate_id() {
  uint32_t next_id = _next_id;
  if (next_id != 0) {
    _next_id = 0;
    return next_id;
  }
  return _new_preamble[3]++;
}

/**
 * Sets the next id that should be returned by allocate_id().
 */
INLINE void SpirVTransformPass::
push_id(uint32_t id) {
  _next_id = id;
}

/**
 * Returns true if the given id was deleted during this pass.
 */
INLINE bool SpirVTransformPass::
is_deleted(uint32_t id) const {
  return _deleted_ids.count(id);
}

/**
 * Returns true if the given member of the given struct type id was deleted
 * during this pass.
 */
INLINE bool SpirVTransformPass::
is_member_deleted(uint32_t type_id, uint32_t member) const {
  auto it = _deleted_members.find(type_id);
  if (it != _deleted_members.end()) {
    return it->second.count(member);
  }
  return false;
}

/**
 * Inserts a decoration instruction taking no arguments.
 */
INLINE void SpirVTransformPass::
decorate(uint32_t id, spv::Decoration decoration) {
  nassertv(decoration != spv::DecorationLocation);
  _new_annotations.insert(_new_annotations.end(),
    {spv::OpDecorate | (3 << spv::WordCountShift), id, (uint32_t)decoration});
}

/**
 * Inserts a decoration instruction taking one argument.
 */
INLINE void SpirVTransformPass::
decorate(uint32_t id, spv::Decoration decoration, uint32_t value) {
  _new_annotations.insert(_new_annotations.end(),
    {spv::OpDecorate | (4 << spv::WordCountShift), id, (uint32_t)decoration, value});

  if (decoration == spv::DecorationLocation) {
    _db.modify_definition(id)._location = (int)value;
  }
}

/**
 * Returns true if the given id has already been defined during this pass.
 */
INLINE bool SpirVTransformPass::
is_defined(uint32_t id) const {
  return _defined.get_bit(id);
}

/**
 * Marks the given definition as having been defined.
 */
INLINE void SpirVTransformPass::
mark_defined(uint32_t id) {
  _defined.set_bit(id);
}

/**
 * Marks the given definition as having been used.
 */
INLINE void SpirVTransformPass::
mark_used(uint32_t id) {
  _db.mark_used(id);
}

/**
 * Adds an instruction to the end of the new debug section.
 */
INLINE void SpirVTransformPass::
add_debug(spv::Op opcode, std::initializer_list<uint32_t> args) {
  return add_debug(opcode, args.begin(), args.size());
}

/**
 * Adds an instruction to the end of the new debug section.
 */
INLINE void SpirVTransformPass::
add_debug(spv::Op opcode, const uint32_t *args, uint16_t nargs) {
  _new_preamble.push_back(((nargs + 1) << spv::WordCountShift) | opcode);
  _new_preamble.insert(_new_preamble.end(), args, args + nargs);
}

/**
 * Adds an instruction to the end of the new annotations section.
 */
INLINE void SpirVTransformPass::
add_annotation(spv::Op opcode, std::initializer_list<uint32_t> args) {
  return add_annotation(opcode, args.begin(), args.size());
}

/**
 * Adds an instruction to the end of the new annotations section.
 */
INLINE void SpirVTransformPass::
add_annotation(spv::Op opcode, const uint32_t *args, uint16_t nargs) {
  _new_annotations.push_back(((nargs + 1) << spv::WordCountShift) | opcode);
  _new_annotations.insert(_new_annotations.end(), args, args + nargs);
}

/**
 * Adds an instruction to the end of the new definitions section.
 */
INLINE void SpirVTransformPass::
add_definition(spv::Op opcode, std::initializer_list<uint32_t> args) {
  return add_definition(opcode, args.begin(), args.size());
}

/**
 * Adds an instruction to the current function.
 */
INLINE void SpirVTransformPass::
add_instruction(spv::Op opcode, std::initializer_list<uint32_t> args) {
  return add_instruction(opcode, args.begin(), args.size());
}

/**
 *
 */
INLINE void SpirVTransformPass::AccessChain::
prepend(uint32_t id) {
  _chain.insert(_chain.begin(), id);
}

/**
 *
 */
INLINE void SpirVTransformPass::AccessChain::
append(uint32_t id) {
  _chain.push_back(id);
}

/**
 *
 */
INLINE void SpirVTransformPass::AccessChain::
extend(const AccessChain &other) {
  _chain.insert(_chain.end(), other._chain.begin(), other._chain.end());
}

/**
 *
 */
INLINE bool SpirVTransformPass::AccessChain::
startswith(const AccessChain &other) const {
  if (_var_id != other._var_id || other._chain.size() > _chain.size()) {
    return false;
  }
  for (size_t i = 0; i < other.size(); ++i) {
    if (_chain[i] != other._chain[i]) {
      return false;
    }
  }
  return true;
}

/**
 *
 */
INLINE bool SpirVTransformPass::AccessChain::
operator < (const AccessChain &other) const {
  if (_var_id != other._var_id) {
    return _var_id < other._var_id;
  }
  return _chain < other._chain;
}

/**
 *
 */
INLINE void SpirVTransformPass::AccessChain::
output(std::ostream &out) const {
  out << _var_id;
  for (size_t i = 0; i < _chain.size(); ++i) {
    out << '[' << _chain[i] << ']';
  }
}

/**
 *
 */
INLINE std::ostream &
operator << (std::ostream &out, const SpirVTransformPass::AccessChain &obj) {
  obj.output(out);
  return out;
}
