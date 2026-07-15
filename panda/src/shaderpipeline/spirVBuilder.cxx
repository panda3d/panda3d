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

#ifndef NDEBUG
/**
 * Returns true if this is a floating point numeric type, used in asserts only.
 */
static bool
is_floating_point(const ShaderType *type) {
  if (type != nullptr) {
    ShaderType::ScalarType scalar_type;
    if (const ShaderType::Scalar *scalar = type->as_scalar()) {
      scalar_type = scalar->get_scalar_type();
    }
    else if (const ShaderType::Vector *vector = type->as_vector()) {
      scalar_type = vector->get_scalar_type();
    }
    else if (const ShaderType::Matrix *matrix = type->as_matrix()) {
      scalar_type = matrix->get_scalar_type();
    }
    else {
      return false;
    }
    return scalar_type == ShaderType::ST_float
        || scalar_type == ShaderType::ST_double;
  }
  return false;
}
#endif  // NDEBUG

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
 * Inserts an OpLoad from the given pointer id.
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
 * Inserts an OpStore to the given pointer id.
 */
void SpirVBuilder::
op_store(Id var_id, Id value) {
  insert(spv::OpStore, {var_id, value});
}

/**
 * Inserts an OpSelect.
 */
SpirVId SpirVBuilder::
op_select(Id cond, Id obj1, Id obj2) {
  Id type_id = _module.get_type_id(obj1);
  nassertr(type_id == _module.get_type_id(obj2), Id());

  Id id = allocate_id();
  insert(spv::OpSelect, {type_id, id, cond, obj1, obj2});
  return id;
}

/**
 * Inserts an OpAccessChain with the given base id (which must be a pointer)
 * and constant ids containing the various member/array indices.
 */
SpirVId SpirVBuilder::
op_access_chain(Id var_id, std::initializer_list<Id> chain) {
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
  for (auto index_id : chain) {
    const Instruction *type_decl = _module.find_declaration(type_id);
    nassertr(type_decl != nullptr, Id());

    if (type_decl->opcode == spv::OpTypeStruct) {
      uint32_t member_index = _module.resolve_constant(index_id);
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
  for (Id arg : chain) {
    op.args.push_back(arg);
  }
  insert(std::move(op));
  return id;
}

/**
 * Inserts an OpVectorShuffle, like a swizzle but may source from two vectors
 * at once, with the indices continuing to number into the second vector.
 * For a regular swizzle, pass the same vector twice.
 */
SpirVId SpirVBuilder::
op_vector_shuffle(Id vec1, Id vec2, const pvector<uint32_t> &components) {
  const ShaderType *vec1_shader_type = _module.resolve_type(vec1);
  const ShaderType *vec2_shader_type = _module.resolve_type(vec2);
  nassertr(vec1_shader_type != nullptr && vec2_shader_type != nullptr, Id());
  const ShaderType::Vector *vec1_type = vec1_shader_type->as_vector();
  const ShaderType::Vector *vec2_type = vec2_shader_type->as_vector();
  nassertr(vec1_type != nullptr && vec2_type != nullptr, Id());
  nassertr(vec1_type->get_scalar_type() == vec2_type->get_scalar_type(), Id());

  Id id = allocate_id();

  const ShaderType *result_type = ShaderType::register_type(ShaderType::Vector(vec1_type->get_scalar_type(), components.size()));
  Id type_id = _module.define_type(result_type);

  Instruction op(spv::OpVectorShuffle, {type_id, id, vec1, vec2});
  for (uint32_t component : components) {
    op.args.push_back(component);
  }
  insert(std::move(op));
  return id;
}

/**
 * Constructs a composite with the given type from the given constituents.
 */
SpirVId SpirVBuilder::
op_composite_construct(const ShaderType *type, const pvector<Id> &constituents) {
  Id id = allocate_id();
  Id type_id = _module.define_type(type);

  Instruction op(spv::OpCompositeConstruct, {type_id, id});
  for (Id arg : constituents) {
    op.args.push_back(arg);
  }
  insert(std::move(op));
  return id;
}

/**
 * Inserts an OpCompositeExtract.
 */
SpirVId SpirVBuilder::
op_composite_extract(Id obj_id, std::initializer_list<uint32_t> chain) {
  Id type_id = _module.get_type_id(obj_id);
  nassertr(_module.get_definition_type(type_id) != SpirVModule::DT_pointer_type, Id());

  for (auto index : chain) {
    type_id = _module.get_composite_member_type_id(type_id, index);
    nassertr(type_id != 0, Id());
  }

  Id id = allocate_id();
  Instruction op(spv::OpCompositeExtract, {type_id, id, obj_id});
  for (uint32_t arg : chain) {
    op.args.push_back(arg);
  }
  insert(std::move(op));
  return id;
}

/**
 * Inserts an OpCompositeInsert.
 */
SpirVId SpirVBuilder::
op_composite_insert(Id obj_id, Id composite_id, std::initializer_list<uint32_t> chain) {
  Id type_id = _module.get_type_id(composite_id);

  Id id = allocate_id();
  Instruction op(spv::OpCompositeInsert, {type_id, id, obj_id, composite_id});
  for (uint32_t arg : chain) {
    op.args.push_back(arg);
  }
  insert(std::move(op));
  return id;
}

/**
 * Inserts a comparison op, taking two operands and returning a bool.  Vector
 * operands produce a bool vector with the same number of components.
 */
SpirVId SpirVBuilder::
op_compare(spv::Op opcode, Id obj1, Id obj2) {
  const ShaderType *operand_type = _module.resolve_type(obj1);
  nassertr(operand_type != nullptr, Id());

  const ShaderType *bool_type = ShaderType::BOOL;
  if (const ShaderType::Vector *vector = operand_type->as_vector()) {
    bool_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_bool, vector->get_num_components()));
  }

  Id id = allocate_id();
  Id type_id = _module.define_type(bool_type);

  insert(opcode, {type_id, id, obj1, obj2});
  return id;
}

/**
 * Insert a conversion op to the given scalar type.  May be a scalar or vector.
 * If it is already of the given scalar type, does nothing.
 */
SpirVId SpirVBuilder::
op_convert(ShaderType::ScalarType new_scalar_type, Id value) {
  const ShaderType *value_type = _module.resolve_type(value);
  nassertr(value_type != nullptr, Id());

  const ShaderType *new_type = nullptr;
  ShaderType::ScalarType old_scalar_type = ShaderType::ST_float;

  if (const ShaderType::Scalar *scalar = value_type->as_scalar()) {
    old_scalar_type = scalar->get_scalar_type();
    if (old_scalar_type == new_scalar_type) {
      // No conversion needed.
      return value;
    }
    new_type = ShaderType::register_type(ShaderType::Scalar(new_scalar_type));
  }
  else if (const ShaderType::Vector *vector = value_type->as_vector()) {
    old_scalar_type = vector->get_scalar_type();
    if (old_scalar_type == new_scalar_type) {
      // No conversion needed.
      return value;
    }
    new_type = ShaderType::register_type(ShaderType::Vector(new_scalar_type, vector->get_num_components()));
  }
  else {
    nassertr_always(false, Id());
  }

  bool old_float = old_scalar_type == ShaderType::ST_float
                || old_scalar_type == ShaderType::ST_double;
  bool new_float = new_scalar_type == ShaderType::ST_float
                || new_scalar_type == ShaderType::ST_double;

  if (old_scalar_type == ShaderType::ST_bool && new_scalar_type != ShaderType::ST_bool) {
    Id true_value, false_value;
    if (new_scalar_type == ShaderType::ST_double) {
      true_value = _module.define_double_constant(1.0);
      false_value = _module.define_double_constant(0.0);
    } else if (new_float) {
      true_value = _module.define_float_constant(1.0f);
      false_value = _module.define_float_constant(0.0f);
    } else {
      const ShaderType *scalar_type = ShaderType::register_type(ShaderType::Scalar(new_scalar_type));
      true_value = _module.define_constant(scalar_type, 1u);
      false_value = _module.define_constant(scalar_type, 0u);
    }
    if (const ShaderType::Vector *vector = new_type->as_vector()) {
      true_value = op_composite_construct(new_type, pvector<Id>(vector->get_num_components(), true_value));
      false_value = op_composite_construct(new_type, pvector<Id>(vector->get_num_components(), false_value));
    }
    return op_select(value, true_value, false_value);
  }

  if (new_scalar_type == ShaderType::ST_bool && old_scalar_type != ShaderType::ST_bool) {
    // Conversion to bool is the same as != 0
    Id null_value = _module.define_null_constant(value_type);
    return op_compare(old_float ? spv::OpFUnordNotEqual : spv::OpINotEqual, value, null_value);
  }

  Id new_type_id = _module.define_type(new_type);

  // Determine which conversion instruction to use.
  spv::Op opcode;
  if (old_float && new_float) {
    opcode = spv::OpFConvert;
  }
  else if (old_float) {
    bool new_signed = new_scalar_type == ShaderType::ST_int;
    opcode = new_signed ? spv::OpConvertFToS : spv::OpConvertFToU;
  }
  else if (new_float) {
    bool old_signed = old_scalar_type == ShaderType::ST_int;
    opcode = old_signed ? spv::OpConvertSToF : spv::OpConvertUToF;
  }
  else {
    // Equal scalar types were already handled by the early returns above, so
    // this is int<->uint; assuming it's the same bit width, for now.
    opcode = spv::OpBitcast;
  }

  Id id = allocate_id();
  insert(opcode, {new_type_id, id, value});
  return id;
}

/**
 * Insert an OpBitcast.
 */
SpirVId SpirVBuilder::
op_bitcast(const ShaderType *new_type, Id value) {
  Id id = allocate_id();
  Id new_type_id = _module.define_type(new_type);

  insert(spv::OpBitcast, {new_type_id, id, value});
  return id;
}

/**
 * Inserts a matrix transpose instruction.
 */
SpirVId SpirVBuilder::
op_transpose(Id obj) {
  const ShaderType *type = _module.resolve_type(obj);
  Id type_id = _module.get_type_id(obj);

  Id id = allocate_id();

  if (const ShaderType::Matrix *matrix = type->as_matrix()) {
    if (matrix->get_num_rows() != matrix->get_num_columns()) {
      type = ShaderType::register_type(ShaderType::Matrix(matrix->get_scalar_type(), matrix->get_num_columns(), matrix->get_num_rows()));
      type_id = _module.define_type(type);
    }
  } else {
    nassert_raise("OpTranspose requires a matrix");
  }

  insert(spv::OpTranspose, {type_id, id, obj});
  return id;
}

/**
 * Inserts a scalar or vector addition instruction.
 */
SpirVId SpirVBuilder::
op_add(Id left, Id right) {
  nassertr(left != 0u, Id());
  nassertr(right != 0u, Id());
  const ShaderType *left_type = _module.resolve_type(left);
  nassertr(left_type != nullptr, Id());
#ifndef NDEBUG
  const ShaderType *right_type = _module.resolve_type(right);
  nassertr(right_type != nullptr, Id());
#endif

  ShaderType::ScalarType left_scalar_type;
#ifndef NDEBUG
  ShaderType::ScalarType right_scalar_type;
#endif
  if (const ShaderType::Scalar *scalar = left_type->as_scalar()) {
    left_scalar_type = scalar->get_scalar_type();

#ifndef NDEBUG
    const ShaderType::Scalar *right_scalar = right_type->as_scalar();
    nassertr(right_scalar != nullptr, Id());
    right_scalar_type = right_scalar->get_scalar_type();
#endif
  }
  else if (const ShaderType::Vector *vector = left_type->as_vector()) {
    left_scalar_type = vector->get_scalar_type();

#ifndef NDEBUG
    const ShaderType::Vector *right_vector = right_type->as_vector();
    nassertr(right_vector != nullptr, Id());
    right_scalar_type = right_vector->get_scalar_type();
#endif
  }
  else {
    nassertr(false, Id());
    return Id();
  }

  spv::Op opcode;
  if (left_scalar_type == ShaderType::ST_float || left_scalar_type == ShaderType::ST_double) {
    opcode = spv::OpFAdd;
#ifndef NDEBUG
    nassertr(right_scalar_type == left_scalar_type, Id());
#endif
  } else {
    opcode = spv::OpIAdd;
#ifndef NDEBUG
    nassertr(right_scalar_type != ShaderType::ST_float && right_scalar_type != ShaderType::ST_double, Id());
#endif
  }

  Id id = allocate_id();
  insert(opcode, {_module.get_type_id(left), id, left, right});
  return id;
}

/**
 * Inserts a scalar or vector subtraction instruction.
 */
SpirVId SpirVBuilder::
op_sub(Id left, Id right) {
  nassertr(left != 0u, Id());
  nassertr(right != 0u, Id());
  const ShaderType *left_type = _module.resolve_type(left);
  nassertr(left_type != nullptr, Id());
  nassertr(left_type == _module.resolve_type(right), Id());

  ShaderType::ScalarType scalar_type;
  if (const ShaderType::Scalar *scalar = left_type->as_scalar()) {
    scalar_type = scalar->get_scalar_type();
  }
  else if (const ShaderType::Vector *vector = left_type->as_vector()) {
    scalar_type = vector->get_scalar_type();
  }
  else {
    nassertr(false, Id());
    return Id();
  }

  spv::Op opcode;
  if (scalar_type == ShaderType::ST_float || scalar_type == ShaderType::ST_double) {
    opcode = spv::OpFSub;
  } else {
    opcode = spv::OpISub;
  }

  Id id = allocate_id();
  insert(opcode, {_module.get_type_id(left), id, left, right});
  return id;
}

/**
 * Inserts a scalar division instruction.  Floating-point only, for now; see
 * op_convert for producing a float operand.
 */
SpirVId SpirVBuilder::
op_div(Id left, Id right) {
  nassertr(left != 0u, Id());
  nassertr(right != 0u, Id());
#ifndef NDEBUG
  const ShaderType *left_type = _module.resolve_type(left);
  nassertr(left_type == _module.resolve_type(right), Id());
  nassertr(is_floating_point(left_type), Id());
#endif

  Id id = allocate_id();
  insert(spv::OpFDiv, {_module.get_type_id(left), id, left, right});
  return id;
}

/**
 * Inserts a vector dot instruction.
 */
SpirVId SpirVBuilder::
op_dot(Id left, Id right) {
  nassertr(left != 0u, Id());
  nassertr(right != 0u, Id());
  const ShaderType *left_type = _module.resolve_type(left);
  nassertr(left_type != nullptr, Id());
  nassertr(left_type == _module.resolve_type(right), Id());

  const ShaderType::Vector *vector = left_type->as_vector();
  nassertr(vector != nullptr, Id());
  nassertr(is_floating_point(vector), Id());

  const ShaderType *type = ShaderType::register_type(ShaderType::Scalar(vector->get_scalar_type()));
  Id type_id = _module.define_type(type);

  Id id = allocate_id();
  insert(spv::OpDot, {type_id, id, left, right});
  return id;
}

/**
 * Inserts a negation instruction.  Floating-point only, for now.
 */
SpirVId SpirVBuilder::
op_negate(Id value) {
  nassertr(value != 0u, Id());
  nassertr(is_floating_point(_module.resolve_type(value)), Id());

  Id id = allocate_id();
  insert(spv::OpFNegate, {_module.get_type_id(value), id, value});
  return id;
}

/**
 * Inserts a multiplication instruction.
 */
SpirVId SpirVBuilder::
op_multiply(Id left, Id right) {
  nassertr(left != 0u, Id());
  nassertr(right != 0u, Id());
  const ShaderType *left_type = _module.resolve_type(left);
  const ShaderType *right_type = _module.resolve_type(right);
  nassertr(left_type != nullptr, Id());
  nassertr(right_type != nullptr, Id());

  Id id = allocate_id();

  Id type_id;
  spv::Op opcode;

  if (const ShaderType::Scalar *lscalar = left_type->as_scalar()) {
    if (const ShaderType::Scalar *rscalar = right_type->as_scalar()) {
      nassertr(lscalar == rscalar, Id());
      // Floating-point only, for now.
      nassertr(is_floating_point(lscalar), Id());
      opcode = spv::OpFMul;
      type_id = _module.get_type_id(left);
    }
    else if (const ShaderType::Vector *rvector = right_type->as_vector()) {
      nassertr(lscalar->get_scalar_type() == rvector->get_scalar_type(), Id());
      opcode = spv::OpVectorTimesScalar;
      type_id = _module.get_type_id(right);
      std::swap(left, right);
    }
    else if (const ShaderType::Matrix *rmatrix = right_type->as_matrix()) {
      nassertr(lscalar->get_scalar_type() == rmatrix->get_scalar_type(), Id());
      opcode = spv::OpMatrixTimesScalar;
      type_id = _module.get_type_id(right);
      std::swap(left, right);
    }
    else {
      nassert_raise("right side of multiplication is not a scalar, vector or matrix");
      return Id();
    }
  }
  else if (const ShaderType::Vector *lvector = left_type->as_vector()) {
    if (const ShaderType::Scalar *rscalar = right_type->as_scalar()) {
      nassertr(lvector->get_scalar_type() == rscalar->get_scalar_type(), Id());
      opcode = spv::OpVectorTimesScalar;
      type_id = _module.get_type_id(left);
    }
    else if (right_type->as_vector() != nullptr) {
      nassert_raise("op_multiply for vector times vector is ambiguous");
      return Id();
    }
    else if (const ShaderType::Matrix *rmatrix = right_type->as_matrix()) {
      nassertr(lvector->get_scalar_type() == rmatrix->get_scalar_type(), Id());
      nassertr(lvector->get_num_components() == rmatrix->get_num_columns(), Id());
      opcode = spv::OpVectorTimesMatrix;
      const ShaderType *type = ShaderType::register_type(ShaderType::Vector(lvector->get_scalar_type(), rmatrix->get_num_rows()));
      type_id = _module.define_type(type);
    }
    else {
      nassert_raise("right side of multiplication is not a scalar, vector or matrix");
      return Id();
    }
  }
  else if (const ShaderType::Matrix *lmatrix = left_type->as_matrix()) {
    if (const ShaderType::Scalar *rscalar = right_type->as_scalar()) {
      nassertr(lmatrix->get_scalar_type() == rscalar->get_scalar_type(), Id());
      opcode = spv::OpMatrixTimesScalar;
      type_id = _module.get_type_id(left);
    }
    else if (const ShaderType::Vector *rvector = right_type->as_vector()) {
      nassertr(lmatrix->get_scalar_type() == rvector->get_scalar_type(), Id());
      nassertr(lmatrix->get_num_rows() == rvector->get_num_components(), Id());
      opcode = spv::OpMatrixTimesVector;
      const ShaderType *type = ShaderType::register_type(ShaderType::Vector(lmatrix->get_scalar_type(), lmatrix->get_num_columns()));
      type_id = _module.define_type(type);
    }
    else if (const ShaderType::Matrix *rmatrix = right_type->as_matrix()) {
      nassertr(lmatrix->get_scalar_type() == rmatrix->get_scalar_type(), Id());
      opcode = spv::OpMatrixTimesMatrix;
      const ShaderType *type = ShaderType::register_type(ShaderType::Matrix(lmatrix->get_scalar_type(), rmatrix->get_num_rows(), lmatrix->get_num_columns()));
      type_id = _module.define_type(type);
    }
    else {
      nassert_raise("right side of multiplication is not a scalar, vector or matrix");
      return Id();
    }
  }
  else {
    nassert_raise("left side of multiplication is not a scalar, vector or matrix");
    return Id();
  }

  insert(opcode, {type_id, id, left, right});
  return id;
}

/**
 * Inserts an OpImageSampleExplicitLod or OpImageSampleImplicitLod.
 */
SpirVId SpirVBuilder::
op_image_sample(Id image, Id coord, uint32_t operands, const uint32_t *ids) {
  Id id = allocate_id();

  const ShaderType *image_type = _module.resolve_type(image);
  nassertr(image_type != nullptr, Id());
  const ShaderType::SampledImage *sampled_image = image_type->as_sampled_image();
  nassertr(sampled_image != nullptr, Id());

  const ShaderType *type = ShaderType::register_type(ShaderType::Vector(sampled_image->get_sampled_type(), 4));
  Id type_id = _module.define_type(type);

  spv::Op opcode;
  if (operands & (spv::ImageOperandsLodMask | spv::ImageOperandsGradMask)) {
    opcode = spv::OpImageSampleExplicitLod;
  } else {
    opcode = spv::OpImageSampleImplicitLod;
  }

  uint32_t num_ids = ids != nullptr ? get_num_image_operand_ids(operands) : 0u;
  Instruction op(opcode, {type_id, id, image, coord, operands});
  for (uint32_t i = 0; i < num_ids; ++i) {
    op.args.push_back(ids[i]);
  }
  insert(std::move(op));
  return id;
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
