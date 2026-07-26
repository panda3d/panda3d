/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVBuilder.cxx
 * @author rdb
 * @date 2026-07-07
 */

#include "spirVBuilder.h"

/**
 * Returns true if the given opcode terminates a block, so that no further
 * instruction may follow it before the next OpLabel.
 */
static bool
is_block_terminator(spv::Op opcode) {
  switch (opcode) {
  case spv::OpBranch:
  case spv::OpBranchConditional:
  case spv::OpSwitch:
  case spv::OpReturn:
  case spv::OpReturnValue:
  case spv::OpKill:
  case spv::OpUnreachable:
  case spv::OpTerminateInvocation:
  case spv::OpIgnoreIntersectionKHR:
  case spv::OpTerminateRayKHR:
  case spv::OpEmitMeshTasksEXT:
  case spv::OpAbortKHR:
    return true;
  default:
    return false;
  }
}

/**
 * Creates a builder without a cursor position.
 */
SpirVBuilder::
SpirVBuilder(SpirVModule &module) :
  _module(module) {
}

/**
 * Creates a builder with the cursor at the given instruction index of the
 * given function.
 */
SpirVBuilder::
SpirVBuilder(SpirVModule &module, Id function_id, size_t index) :
  _module(module) {
  set_insertion_point(function_id, index);
}

/**
 * Positions the cursor at the given instruction index of the given function.
 */
void SpirVBuilder::
set_insertion_point(Id function_id, size_t index) {
  for (size_t i = 0; i < _module.get_num_functions(); ++i) {
    const Function &function = _module.get_function(i);
    if (function.id == function_id) {
      _function_index = i;
      _index = index;
      _current_function_id = function_id;
      return;
    }
  }
  _function_index = (size_t)-1;
  _current_function_id = Id();
  nassert_raise("no function with this id");
}

/**
 * Positions the cursor at the top of the given function's body: after the
 * OpLabel opening the entry block and past the OpVariable declarations that
 * SPIR-V requires to lead it, so that inserted code may not accidentally
 * precede a variable declaration.
 */
void SpirVBuilder::
set_insertion_point_to_body_start(Id function_id) {
  set_insertion_point(function_id, 0);

  Function *function = get_current_function();
  nassertv(function != nullptr);
  const pvector<Instruction> &instructions = function->instructions;

  size_t index = 0;
  while (index < instructions.size() &&
         instructions[index].opcode != spv::OpLabel) {
    ++index;
  }
  nassertv(index < instructions.size());
  ++index;

  while (index < instructions.size()) {
    spv::Op opcode = instructions[index].opcode;
    if (opcode != spv::OpVariable && opcode != spv::OpLine &&
        opcode != spv::OpNoLine && opcode != spv::OpNop) {
      break;
    }
    ++index;
  }
  _index = index;
}

/**
 * Returns the function the cursor is currently inside, or null.  Note that
 * the pointer is invalidated when a function is added to the module.
 */
SpirVModule::Function *SpirVBuilder::
get_current_function() {
  if (_function_index >= _module.get_num_functions()) {
    return nullptr;
  }
  return &_module.modify_functions()[_function_index];
}

/**
 * Inserts a function-body instruction at the cursor, recording its result in
 * the module's id index.  Structural function parameter instructions are
 * represented by Function::parameters and may not be inserted here.
 */
void SpirVBuilder::
insert(Instruction op) {
  Function *function = get_current_function();
  nassertv(function != nullptr);
  nassertv(_index <= function->instructions.size());
  nassertv(op.opcode != spv::OpFunctionParameter);
  _module.record_result(op, _current_function_id);
  function->instructions.insert(function->instructions.begin() + _index,
                                std::move(op));
  ++_index;
}

/**
 * Inserts an arbitrary instruction at the cursor.
 */
void SpirVBuilder::
insert(spv::Op opcode, const uint32_t *args, size_t nargs) {
  insert(Instruction(opcode, args, nargs));
}

/**
 * Inserts an OpLoad from the given pointer id, returning the id of the
 * loaded value.  Generally, prefer SpirVPointer::load() instead.
 */
SpirVId SpirVBuilder::
op_load(Id var_id, spv::MemoryAccessMask access) {
  Id type_id = _module.unwrap_pointer_type(_module.get_type_id(var_id));

  Id id = allocate_id();
  if (access != spv::MemoryAccessMaskNone) {
    insert(spv::OpLoad, {type_id, id, var_id, (uint32_t)access});
  } else {
    insert(spv::OpLoad, {type_id, id, var_id});
  }
  return id;
}

/**
 * Inserts an OpStore to the given pointer id.  Generally, prefer the
 * higher-level SpirVPointer::store() instead.
 */
void SpirVBuilder::
op_store(Id var_id, Id value, spv::MemoryAccessMask access) {
  nassertv(_module.unwrap_pointer_type(_module.get_type_id(var_id)) ==
           _module.get_type_id(value));
  if (access != spv::MemoryAccessMaskNone) {
    insert(spv::OpStore, {var_id, value, (uint32_t)access});
  } else {
    insert(spv::OpStore, {var_id, value});
  }
}

/**
 * Inserts an OpAccessChain with the given base id (which must be a pointer)
 * and the ids of the member/array indices.  Struct member indices must be
 * constants; other indices may be any integer value id.
 */
SpirVId SpirVBuilder::
op_access_chain(Id var_id, const Id *chain, size_t count) {
  Id base_pointer_type_id = _module.get_type_id(var_id);
  nassertr(_module.get_definition_type(base_pointer_type_id) == SpirVModule::DT_pointer_type, Id());

  // The chain's pointer type must use the storage class as declared on the
  // base pointer type, not the canonicalized one that get_storage_class
  // reports: chaining into a legacy-style buffer block (Uniform storage
  // class with the BufferBlock decoration) must yield Uniform-class
  // pointers, since declaring a StorageBuffer-class pointer requires
  // SPV_KHR_storage_buffer_storage_class before SPIR-V 1.3.
  const SpirVModule::Instruction *base_decl = _module.find_declaration(base_pointer_type_id);
  nassertr(base_decl != nullptr && base_decl->args.size() >= 3, Id());
  spv::StorageClass storage_class = (spv::StorageClass)base_decl->args[1];

  Id type_id = _module.unwrap_pointer_type(base_pointer_type_id);
  for (size_t i = 0; i < count; ++i) {
    const Instruction *type_decl = _module.find_declaration(type_id);
    nassertr(type_decl != nullptr, Id());

    if (type_decl->opcode == spv::OpTypeStruct) {
      uint32_t member_index = _module.resolve_constant(chain[i]);
      type_id = _module.get_composite_member_type_id(type_id, member_index);
    } else {
      // Array, matrix, or vector
      type_id = _module.get_composite_member_type_id(type_id, 0);
    }
    nassertr(type_id != 0, Id());
  }

  Id id = allocate_id();
  Id pointer_type_id = _module.define_pointer_type(type_id, storage_class);

  Instruction op(spv::OpAccessChain, {pointer_type_id, id, var_id});
  op.args.insert(op.args.end(), chain, chain + count);
  insert(std::move(op));
  return id;
}

/**
 * Defines a new named variable in the UniformConstant storage class,
 * returning it wrapped for use in the body.
 */
SpirVPointer SpirVBuilder::
define_uniform(const ShaderType *type, std::string name) {
  Id variable_id = _module.define_variable(type, spv::StorageClassUniformConstant);
  _module.set_name(variable_id, std::move(name));
  return Pointer(*this, variable_id);
}

/**
 * Defines a new named input variable with the given location, returning it
 * wrapped for use in the body.
 */
SpirVPointer SpirVBuilder::
define_input(const ShaderType *type, std::string name, int location) {
  Id variable_id = _module.define_variable(type, spv::StorageClassInput);
  _module.set_name(variable_id, std::move(name));
  _module.set_location(variable_id, location);
  return Pointer(*this, variable_id);
}

/**
 * Defines a new named output variable with the given location, returning it
 * wrapped for use in the body.
 */
SpirVPointer SpirVBuilder::
define_output(const ShaderType *type, std::string name, int location) {
  Id variable_id = _module.define_variable(type, spv::StorageClassOutput);
  _module.set_name(variable_id, std::move(name));
  _module.set_location(variable_id, location);
  return Pointer(*this, variable_id);
}

/**
 * Defines a new private variable.
 */
SpirVPointer SpirVBuilder::
define_private(const ShaderType *type, std::string name) {
  Id variable_id = _module.define_variable(type, spv::StorageClassPrivate);
  if (!name.empty()) {
    _module.set_name(variable_id, std::move(name));
  }
  return Pointer(*this, variable_id);
}

/**
 * Returns the given values as a module-scope vec2 constant, wrapped for use
 * in the body.
 */
SpirVValue SpirVBuilder::
const_vec2(float x, float y) {
  static const ShaderType *type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 2));
  return Value(*this, _module.define_composite_constant(type,
    {_module.define_float_constant(x), _module.define_float_constant(y)}));
}

/**
 * Returns the given values as a module-scope vec3 constant, wrapped for use
 * in the body.
 */
SpirVValue SpirVBuilder::
const_vec3(float x, float y, float z) {
  static const ShaderType *type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  return Value(*this, _module.define_composite_constant(type,
    {_module.define_float_constant(x), _module.define_float_constant(y),
     _module.define_float_constant(z)}));
}

/**
 * Returns the given values as a module-scope vec4 constant, wrapped for use
 * in the body.
 */
SpirVValue SpirVBuilder::
const_vec4(float x, float y, float z, float w) {
  static const ShaderType *type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 4));
  return Value(*this, _module.define_composite_constant(type,
    {_module.define_float_constant(x), _module.define_float_constant(y),
     _module.define_float_constant(z), _module.define_float_constant(w)}));
}

/**
 * Returns the number of constituents an OpCompositeConstruct of the given
 * type takes, and the type each of them must have in element_type.  Returns 0
 * for a type that is not a composite, and leaves element_type null for a
 * struct, whose members are of differing types and are therefore only counted.
 */
static uint32_t
get_num_constituents(const ShaderType *type, const ShaderType *&element_type) {
  element_type = nullptr;

  if (const ShaderType::Vector *vector = type->as_vector()) {
    element_type = vector->get_scalar_type_obj();
    return vector->get_num_components();
  }
  if (const ShaderType::Matrix *matrix = type->as_matrix()) {
    // The constituents are rows, not scalars.
    element_type = ShaderType::register_type(ShaderType::Vector(
      matrix->get_scalar_type(), matrix->get_num_columns()));
    return matrix->get_num_rows();
  }
  if (const ShaderType::Array *array = type->as_array()) {
    element_type = array->get_element_type();
    return array->get_num_elements();
  }
  if (const ShaderType::Struct *struct_type = type->as_struct()) {
    return struct_type->get_num_members();
  }
  return 0;
}

/**
 * Builds a composite of the given type from the given constituents, of which
 * a number materializes as a constant of the constituent type (which must
 * therefore be a scalar for one to be given).
 */
SpirVValue SpirVBuilder::
construct(const ShaderType *type, const glsl::Argument *constituents,
          size_t num_args) {
  nassertr(type != nullptr, Value());

  const ShaderType *element_type;
  uint32_t num_constituents = get_num_constituents(type, element_type);
  nassertr(num_constituents > 0, Value());

  // A vector may be built from shorter vectors (vec4(rgb, 1.0) is one
  // instruction), so its constituents are counted in components.
  const ShaderType::Scalar *element_scalar =
    (element_type != nullptr) ? element_type->as_scalar() : nullptr;
  bool is_vector = type->as_vector() != nullptr;

  // A struct's members are of differing types, so element_type is null and
  // each constituent is instead checked against its own member's type.
  const ShaderType::Struct *struct_type = type->as_struct();

  pvector<Id> ids;
  ids.reserve(num_args);
  uint32_t total = 0;

  for (size_t i = 0; i < num_args; ++i) {
    const glsl::Argument &arg = constituents[i];
    if (arg.is_literal()) {
      nassertr(element_scalar != nullptr, Value());
      Id id = _module.define_scalar_constant(element_scalar->get_scalar_type(),
                                             arg.get_literal());
      nassertr(id != 0u, Value());
      ids.push_back(id);
      ++total;
    }
    else {
      Value value = arg.get_value();
      nassertr(value.get_builder() == this, Value());
      const ShaderType *arg_type = value.get_type();
      nassertr(arg_type != nullptr, Value());

      const ShaderType::Vector *arg_vector = is_vector ? arg_type->as_vector()
                                                       : nullptr;
      if (arg_vector != nullptr) {
        nassertr(element_scalar != nullptr, Value());
        nassertr(arg_vector->get_scalar_type() ==
                 element_scalar->get_scalar_type(), Value());
        total += arg_vector->get_num_components();
      } else {
        if (element_type != nullptr) {
          nassertr(arg_type == element_type, Value());
        } else if (struct_type != nullptr) {
          nassertr(total < struct_type->get_num_members(), Value());
          nassertr(arg_type == struct_type->get_member(total).type, Value());
        }
        ++total;
      }
      ids.push_back(value);
    }
  }
  nassertr(total == num_constituents, Value());

  Id id = allocate_id();
  Instruction op(spv::OpCompositeConstruct, {_module.define_type(type), id});
  op.args.insert(op.args.end(), ids.begin(), ids.end());
  insert(std::move(op));
  return Value(*this, id);
}

/**
 * Builds a composite of the given type from a list assembled at runtime.
 */
SpirVValue SpirVBuilder::
construct(const ShaderType *type, const pvector<Value> &constituents) {
  nassertr(!constituents.empty(), Value());

  pvector<glsl::Argument> args;
  args.reserve(constituents.size());
  for (Value constituent : constituents) {
    args.push_back(constituent);
  }
  return construct(type, args.data(), args.size());
}

/**
 * Builds a composite of the given type by broadcasting one value or number
 * into all of its constituents: the runtime-typed spelling of vecN(x), for
 * when the constituent count or scalar type is not statically known.
 */
SpirVValue SpirVBuilder::
construct(const ShaderType *type, glsl::Argument value) {
  nassertr(type != nullptr, Value());

  const ShaderType *element_type;
  uint32_t num_constituents = get_num_constituents(type, element_type);
  nassertr(num_constituents > 0, Value());

  // A struct's members are of differing types (element_type is null), so one
  // value cannot meaningfully fill all of them.
  nassertr(element_type != nullptr, Value());

  pvector<glsl::Argument> args(num_constituents, value);
  return construct(type, args.data(), args.size());
}

/**
 * Inserts an OpFunctionCall passing no arguments.
 */
SpirVId SpirVBuilder::
op_function_call(Id func_id) {
  Id id = allocate_id();

  nassertr(_module.get_definition_type(func_id) == SpirVModule::DT_function, Id());

  Id function_type_id = _module.get_type_id(func_id);
  nassertr(_module.get_definition_type(function_type_id) == SpirVModule::DT_function_type, Id());

  Id return_type_id = _module.get_type_id(function_type_id);

  insert(spv::OpFunctionCall, {return_type_id, id, func_id});
  return id;
}

/**
 * Returns the number of ids following an image operands mask.
 */
static uint32_t
get_num_image_operand_ids(uint32_t operands) {
  uint32_t count = 0;

  if (operands & spv::ImageOperandsBiasMask) {
    ++count;
  }
  if (operands & spv::ImageOperandsLodMask) {
    ++count;
  }
  if (operands & spv::ImageOperandsGradMask) {
    count += 2;
  }
  if (operands & spv::ImageOperandsConstOffsetMask) {
    ++count;
  }
  if (operands & spv::ImageOperandsOffsetMask) {
    ++count;
  }
  if (operands & spv::ImageOperandsConstOffsetsMask) {
    ++count;
  }
  if (operands & spv::ImageOperandsSampleMask) {
    ++count;
  }
  if (operands & spv::ImageOperandsMinLodMask) {
    ++count;
  }
  if (operands & spv::ImageOperandsMakeTexelAvailableMask) {
    ++count;
  }
  if (operands & spv::ImageOperandsMakeTexelVisibleMask) {
    ++count;
  }
  if (operands & spv::ImageOperandsOffsetsMask) {
    ++count;
  }

  return count;
}

/**
 * Inserts an OpImageSampleExplicitLod or OpImageSampleImplicitLod, depending
 * on whether a Lod or Grad is included in the given raw image operands mask.
 * The GLSL-flavored spelling without a mask is glsl::texture().
 */
SpirVId SpirVBuilder::
op_image_sample(Id sampled_image, Id coord, uint32_t operands,
                const uint32_t *operand_ids) {
  const ShaderType *image_type = _module.resolve_type(sampled_image);
  nassertr(image_type != nullptr, Id());
  const ShaderType::SampledImage *sampled_image_type = image_type->as_sampled_image();
  nassertr(sampled_image_type != nullptr, Id());

  const ShaderType *type = ShaderType::register_type(ShaderType::Vector(sampled_image_type->get_sampled_type(), 4));
  Id type_id = _module.define_type(type);

  spv::Op opcode;
  if (operands & (spv::ImageOperandsLodMask | spv::ImageOperandsGradMask)) {
    opcode = spv::OpImageSampleExplicitLod;
  } else {
    opcode = spv::OpImageSampleImplicitLod;
  }

  Id id = allocate_id();

  uint32_t num_ids = operand_ids != nullptr ? get_num_image_operand_ids(operands) : 0u;
  Instruction op(opcode, {type_id, id, sampled_image, coord, operands});
  if (num_ids > 0) {
    op.args.insert(op.args.end(), operand_ids, operand_ids + num_ids);
  }
  insert(std::move(op));
  return id;
}

/**
 * Inserts an OpLabel.
 */
SpirVId SpirVBuilder::
op_label() {
  Id id = allocate_id();
  insert(spv::OpLabel, {id});
  return id;
}

/**
 * Inserts an OpBranch.
 */
void SpirVBuilder::
op_branch(Id target) {
  insert(spv::OpBranch, {target});
}

/**
 * Inserts an OpReturn.
 */
void SpirVBuilder::
op_return() {
  insert(spv::OpReturn, {});
}

/**
 * Inserts an OpReturnValue.  The value type must match the return type of the
 * current function.
 */
void SpirVBuilder::
op_return_value(Id value) {
  Function *function = get_current_function();
  nassertv(function != nullptr);
  nassertv(value != 0u);

  Id return_type_id = _module.get_type_id(function->type_id);
  const Instruction *return_type_decl =
    _module.find_declaration(return_type_id);
  nassertv(return_type_decl != nullptr &&
           return_type_decl->opcode != spv::OpTypeVoid);
  nassertv(_module.get_type_id(value) == return_type_id);

  insert(spv::OpReturnValue, {value});
}

/**
 * Inserts an OpKill.
 */
void SpirVBuilder::
op_kill() {
  insert(spv::OpKill, {});
}

/**
 * Begins an "if" branch.
 * The return value should be passed to branch_endif().
 */
SpirVId SpirVBuilder::
branch_if(Id cond) {
  Id true_label = allocate_id();
  Id false_label = allocate_id();

  insert(spv::OpSelectionMerge, {false_label, (uint32_t)spv::SelectionControlMaskNone});
  insert(spv::OpBranchConditional, {cond, true_label, false_label});
  insert(spv::OpLabel, {true_label});
  return false_label;
}

/**
 * Ends an "if" branch.
 */
void SpirVBuilder::
branch_endif(Id false_label) {
  // The block being ended must not fall through into the merge label; if the
  // code inserted since branch_if did not end with its own terminator (such
  // as the OpKill of the alpha test), branch to the merge block explicitly.
  Function *function = get_current_function();
  nassertv(function != nullptr);

  spv::Op prev_opcode = spv::OpNop;
  for (size_t i = _index; i > 0; --i) {
    const Instruction &prev = function->instructions[i - 1];
    if (!prev.is_nop()) {
      prev_opcode = prev.opcode;
      break;
    }
  }
  if (!is_block_terminator(prev_opcode)) {
    op_branch(false_label);
  }

  insert(spv::OpLabel, {false_label});
}

/**
 * Emits a "discard" that executes when cond (a bool) is true.
 */
void SpirVBuilder::
kill_if(Id cond) {
  Id merge_label = branch_if(cond);
  op_kill();
  branch_endif(merge_label);
}
