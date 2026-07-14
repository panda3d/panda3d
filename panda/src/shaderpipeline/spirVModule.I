/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVModule.I
 * @author rdb
 * @date 2026-07-07
 */

/**
 *
 */
INLINE SpirVModule::Instruction::
Instruction(spv::Op opcode) : opcode(opcode) {
}

/**
 *
 */
INLINE SpirVModule::Instruction::
Instruction(spv::Op opcode, std::initializer_list<uint32_t> args) :
  opcode(opcode), args(args) {
}

/**
 *
 */
INLINE SpirVModule::Instruction::
Instruction(spv::Op opcode, const uint32_t *args, size_t nargs) :
  opcode(opcode), args(args, args + nargs) {
}

/**
 *
 */
INLINE size_t SpirVModule::Instruction::
size() const {
  return args.size();
}

/**
 *
 */
INLINE uint32_t SpirVModule::Instruction::
operator [] (size_t i) const {
  return args[i];
}

/**
 *
 */
INLINE uint32_t &SpirVModule::Instruction::
operator [] (size_t i) {
  return args[i];
}

/**
 * Returns true if this is an OpNop, which is used as a tombstone for deleted
 * declarations (emit() skips them).
 */
INLINE bool SpirVModule::Instruction::
is_nop() const {
  return opcode == spv::OpNop;
}

/**
 * Returns true if this annotation applies to a struct member (in which case
 * args[0] is the member index).
 */
INLINE bool SpirVModule::Annotation::
is_member_annotation() const {
  return opcode == spv::OpMemberDecorate ||
         opcode == spv::OpMemberDecorateString;
}

/**
 * Returns the decoration of this annotation.
 */
INLINE spv::Decoration SpirVModule::Annotation::
get_decoration() const {
  size_t i = is_member_annotation() ? 1 : 0;
  return (i < args.size()) ? (spv::Decoration)args[i] : spv::DecorationMax;
}

/**
 * Returns the parameter result ids in signature order.
 */
INLINE const pvector<SpirVId> &SpirVModule::Function::
get_parameters() const {
  return parameters;
}

/**
 *
 */
INLINE size_t SpirVModule::
get_num_ext_inst_imports() const {
  return _ext_inst_imports.size();
}

/**
 *
 */
INLINE const SpirVModule::Instruction &SpirVModule::
get_ext_inst_import(size_t i) const {
  return _ext_inst_imports[i];
}

/**
 *
 */
INLINE size_t SpirVModule::
get_num_entry_points() const {
  return _entry_points.size();
}

/**
 *
 */
INLINE const SpirVModule::EntryPoint &SpirVModule::
get_entry_point(size_t i) const {
  return _entry_points[i];
}

/**
 *
 */
INLINE SpirVModule::EntryPoints &SpirVModule::
modify_entry_points() {
  return _entry_points;
}

/**
 *
 */
INLINE size_t SpirVModule::
get_num_execution_modes() const {
  return _execution_modes.size();
}

/**
 *
 */
INLINE const SpirVModule::Instruction &SpirVModule::
get_execution_mode(size_t i) const {
  return _execution_modes[i];
}

/**
 * Looks up the given string.
 */
INLINE std::string SpirVModule::
resolve_string(Id id) const {
  if (get_definition(id)._dtype != DT_string) {
    error_expected(id, "a string");
    return std::string();
  }
  for (const Instruction &op : _debug) {
    if (op.opcode == spv::OpString && !op.args.empty() && op.args[0] == id) {
      return op.get_string(1);
    }
  }
  return std::string();
}

/**
 * Returns the annotations attached to the given id, or null if it has none.
 */
INLINE const SpirVModule::Annotations *SpirVModule::
get_annotations(Id id) const {
  auto it = _annotations.find(id);
  return it != _annotations.end() ? &it->second : nullptr;
}

/**
 * Returns the location decoration of the given id, or -1 if it has none.
 */
INLINE int SpirVModule::
get_location(Id id) const {
  return has_decoration(id, spv::DecorationLocation)
    ? (int)get_decoration(id, spv::DecorationLocation)
    : -1;
}

/**
 * Returns the builtin decoration of the given id, or spv::BuiltInMax if it
 * has none.
 */
INLINE spv::BuiltIn SpirVModule::
get_builtin(Id id) const {
  return (spv::BuiltIn)get_decoration(id, spv::DecorationBuiltIn,
                                      (uint32_t)spv::BuiltInMax);
}

/**
 * Returns the SpecId decoration of the given id, or 0 if it has none.
 */
INLINE uint32_t SpirVModule::
get_spec_id(Id id) const {
  return get_decoration(id, spv::DecorationSpecId, 0);
}

/**
 * Returns the ArrayStride decoration of the given type id, or 0 if it has
 * none.
 */
INLINE uint32_t SpirVModule::
get_array_stride(Id type_id) const {
  return get_decoration(type_id, spv::DecorationArrayStride, 0);
}

/**
 * Returns the Offset decoration of the given struct member, or -1 if it has
 * none.
 */
INLINE int SpirVModule::
get_member_offset(Id type_id, uint32_t member_index) const {
  return has_member_decoration(type_id, member_index, spv::DecorationOffset)
    ? (int)get_member_decoration(type_id, member_index, spv::DecorationOffset)
    : -1;
}

/**
 * Returns the builtin decoration of the given struct member, or
 * spv::BuiltInMax if it has none.
 */
INLINE spv::BuiltIn SpirVModule::
get_member_builtin(Id type_id, uint32_t member_index) const {
  return (spv::BuiltIn)get_member_decoration(type_id, member_index,
    spv::DecorationBuiltIn, (uint32_t)spv::BuiltInMax);
}

/**
 * Adds a decoration without extra operands to the given id.
 */
INLINE void SpirVModule::
decorate(Id id, spv::Decoration decoration) {
  decorate(id, Annotation {spv::OpDecorate, {(uint32_t)decoration}});
}

/**
 * Adds a decoration with one extra operand to the given id.
 */
INLINE void SpirVModule::
decorate(Id id, spv::Decoration decoration, uint32_t value) {
  decorate(id, Annotation {spv::OpDecorate, {(uint32_t)decoration, value}});
}

/**
 * Returns the number of module-level declaration instructions.  This includes
 * OpNop from tombstoned declarations so this number only increases, not
 * decreases.
 */
INLINE size_t SpirVModule::
get_num_declarations() const {
  return _declarations.size();
}

/**
 * Returns the module-level declaration with the given index.  This may be
 * OpNop for a tombstoned declaration, which should be ignored.  Returns a copy
 * to make in-place modification safe.
 *
 * Note that declarations aren't guaranteed to be in topological order; see
 * sort_declarations() if you wish to traverse this in topological order.
 */
INLINE SpirVModule::Instruction SpirVModule::
get_declaration(size_t i) const {
  return _declarations[i];
}

/**
 *
 */
INLINE size_t SpirVModule::
get_num_functions() const {
  return _functions.size();
}

/**
 *
 */
INLINE const SpirVModule::Function &SpirVModule::
get_function(size_t i) const {
  return _functions[i];
}

/**
 *
 */
INLINE SpirVModule::Functions &SpirVModule::
modify_functions() {
  return _functions;
}

/**
 * Returns the current id bound; all ids in the module are lower than this.
 */
INLINE uint32_t SpirVModule::
get_id_bound() const {
  return _id_bound;
}

/**
 * Allocates a fresh identifier and returns it.  Ids are never reused.
 */
INLINE SpirVId SpirVModule::
allocate_id() {
  return Id(_id_bound++);
}

/**
 * Returns what kind of thing the given id refers to.
 */
INLINE SpirVModule::DefinitionType SpirVModule::
get_definition_type(Id id) const {
  if (id < _defs.size()) {
    return _defs[id]._dtype;
  } else {
    return DT_none;
  }
}

/**
 * Returns true if the given id is a type (including pointer and function
 * types).
 */
INLINE bool SpirVModule::
is_type(Id id) const {
  DefinitionType dtype = get_definition_type(id);
  return dtype == DT_type || dtype == DT_pointer_type ||
         dtype == DT_function_type;
}

/**
 * Returns the id of the function the given id is declared in, or 0 if it is
 * declared at module scope.  For a function, returns the function's own id.
 */
INLINE SpirVId SpirVModule::
get_function_id(Id id) const {
  return get_definition(id)._function_id;
}

/**
 * Returns true if the given id is a type and the canonical declaration of
 * that type, ie. declared the same way define_type(...) would declare it.
 */
INLINE bool SpirVModule::
is_canonical_type(Id type_id) const {
  ensure_type_map();
  const ShaderType *type = get_definition(type_id)._type;
  if (type == nullptr) {
    return false;
  }
  TypeMap::const_iterator tit = _type_map.find(type);
  return tit != _type_map.end() && tit->second == type_id;
}

/**
 * Defines a new constant of type float.
 */
INLINE SpirVId SpirVModule::
define_float_constant(float constant) {
  union {
    float f;
    uint32_t ui;
  } u;
  u.f = constant;
  return define_constant(ShaderType::FLOAT, u.ui);
}

/**
 * Returns the index entry for the given id, or a default entry if the id is
 * out of range.
 */
INLINE const SpirVModule::Definition &SpirVModule::
get_definition(Id id) const {
  if (id >= _defs.size()) {
    static const Definition default_def;
    return default_def;
  }
  return _defs[id];
}

/**
 * Returns a mutable index entry for the given id, growing the index as
 * needed.
 */
INLINE SpirVModule::Definition &SpirVModule::
modify_definition(Id id) {
  if (id >= _defs.size()) {
    _defs.resize(id + 1);
  }
  return _defs[id];
}
