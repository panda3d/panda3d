/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVPointer.cxx
 * @author rdb
 * @date 2026-07-15
 */

#include "spirVPointer.h"
#include "spirVBuilder.h"

/**
 * Outputs the id's name if it has one, or %id.
 */
static void
output_name_or_id(std::ostream &out, SpirVModule &module, SpirVId id) {
  const std::string &name = module.get_name(id);
  if (!name.empty()) {
    out << name;
  } else {
    out << '%' << id;
  }
}

/**
 * Outputs the literal value of an integer scalar constant, returning false
 * (with nothing output) for any other id.
 */
static bool
output_constant_index(std::ostream &out, SpirVModule &module, SpirVId id) {
  if (module.get_definition_type(id) != SpirVModule::DT_constant) {
    return false;
  }
  const SpirVModule::Instruction *decl = module.find_declaration(id);
  if (decl == nullptr || decl->opcode != spv::OpConstant ||
      decl->args.size() < 3) {
    return false;
  }
  const ShaderType *type = module.resolve_type(id);
  const ShaderType::Scalar *scalar =
    (type != nullptr) ? type->as_scalar() : nullptr;
  if (scalar == nullptr) {
    return false;
  }
  switch (scalar->get_scalar_type()) {
  case ShaderType::ST_int:
    out << (int32_t)decl->args[2];
    return true;
  case ShaderType::ST_uint:
    out << decl->args[2];
    return true;
  default:
    return false;
  }
}

/**
 * Returns a pointer with the given dynamic index appended.  Emits nothing.
 */
SpirVPointer SpirVPointer::
operator [](glsl::Value index) const {
  nassertr(_builder != nullptr, SpirVPointer());
  nassertr(index.get_builder() == _builder, SpirVPointer());

  SpirVPointer result(*this);
  result._chain.push_back(index);
  return result;
}

/**
 * Returns a pointer with the given constant index appended.  Emits nothing;
 * the index becomes a module-scope int constant.
 */
SpirVPointer SpirVPointer::
operator [](uint32_t index) const {
  nassertr(_builder != nullptr, SpirVPointer());

  SpirVPointer result(*this);
  result._chain.push_back(
    _builder->get_module().define_int_constant((int32_t)index));
  return result;
}

/**
 * Emits the accumulated OpAccessChain (if this pointer was indexed) followed
 * by an OpLoad, returning the loaded value as an expression.
 */
glsl::Value SpirVPointer::
load(spv::MemoryAccessMask access) const {
  nassertr(_builder != nullptr, glsl::Value());

  return glsl::Value(*_builder, _builder->op_load(emit_pointer(), access));
}

/**
 * Emits the accumulated OpAccessChain (if this pointer was indexed) followed
 * by an OpStore of the given value.  A literal materializes against the
 * pointed-to type.
 */
void SpirVPointer::
store(glsl::Argument value_arg, spv::MemoryAccessMask access) const {
  nassertv(_builder != nullptr);

  Id pointer = emit_pointer();
  glsl::Value value = value_arg.resolve(*_builder, get_pointed_type(pointer));
  nassertv(value.get_builder() == _builder);

  _builder->op_store(pointer, value, access);
}

/**
 * Outputs an ampersand, the base variable's name (or %id), and the
 * accumulated indices, such as "&mat_modelproj[0][1]".  A constant index is
 * shown as its literal value, a dynamic one by its own name or %id.
 */
void SpirVPointer::
output(std::ostream &out) const {
  out << '&';
  if (_builder == nullptr) {
    out << '%' << _base;
    return;
  }
  SpirVModule &module = _builder->get_module();

  output_name_or_id(out, module, _base);
  for (Id index : _chain) {
    out << '[';
    if (!output_constant_index(out, module, index)) {
      output_name_or_id(out, module, index);
    }
    out << ']';
  }
}

/**
 * Emits the pointer to operate on: the base variable directly, or an
 * OpAccessChain collecting the accumulated indices.
 */
SpirVId SpirVPointer::
emit_pointer() const {
  if (_chain.empty()) {
    return _base;
  }
  return _builder->op_access_chain(_base, _chain);
}

/**
 * Returns the type an emitted pointer points to, for use as the anchor of a
 * literal argument.
 */
const ShaderType *SpirVPointer::
get_pointed_type(Id pointer) const {
  return _builder->get_module().resolve_type(pointer);
}

/**
 * Emits the given atomic opcode on the emitted pointer, at device scope with
 * relaxed memory ordering.  A null value means the opcode takes no value
 * operand; a literal materializes against the pointed-to type.
 */
glsl::Value SpirVPointer::
emit_atomic(spv::Op opcode, const glsl::Argument *value_arg) const {
  SpirVModule &module = _builder->get_module();

  Id pointer = emit_pointer();
  Id type_id = module.unwrap_pointer_type(module.get_type_id(pointer));
  Id scope = module.define_constant(ShaderType::UINT, spv::ScopeDevice);
  Id semantics = module.define_constant(ShaderType::UINT, spv::MemorySemanticsMaskNone);

  Id value;
  if (value_arg != nullptr) {
    glsl::Value resolved = value_arg->resolve(*_builder, get_pointed_type(pointer));
    nassertr(resolved.get_builder() == _builder, glsl::Value());
    value = resolved;
  }

  Id id = _builder->allocate_id();
  if (value) {
    _builder->insert(opcode, {type_id, id, pointer, scope, semantics, value});
  } else {
    _builder->insert(opcode, {type_id, id, pointer, scope, semantics});
  }
  return glsl::Value(*_builder, id);
}
