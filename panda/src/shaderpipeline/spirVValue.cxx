/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVValue.cxx
 * @author rdb
 * @date 2026-07-08
 */

#include "spirVValue.h"
#include "spirVBuilder.h"

#include "GLSL.std.450.h"

namespace glsl {

using Id = SpirVId;

/**
 * Returns the scalar type of a scalar, vector or matrix ShaderType, or
 * ST_unknown.
 */
static ShaderType::ScalarType
get_element_scalar_type(const ShaderType *type) {
  if (type != nullptr) {
    if (const ShaderType::Scalar *scalar = type->as_scalar()) {
      return scalar->get_scalar_type();
    }
    if (const ShaderType::Vector *vector = type->as_vector()) {
      return vector->get_scalar_type();
    }
    if (const ShaderType::Matrix *matrix = type->as_matrix()) {
      return matrix->get_scalar_type();
    }
  }
  return ShaderType::ST_unknown;
}

/**
 * Returns true if the given scalar type is a floating-point type.
 */
static bool
is_float_scalar_type(ShaderType::ScalarType scalar_type) {
  return scalar_type == ShaderType::ST_float ||
         scalar_type == ShaderType::ST_double;
}

/**
 * Shorthands for interning a scalar or vector type.
 */
static const ShaderType *
make_scalar(ShaderType::ScalarType scalar_type) {
  return ShaderType::register_type(ShaderType::Scalar(scalar_type));
}

static const ShaderType *
make_vector(ShaderType::ScalarType scalar_type, uint32_t num_components) {
  return ShaderType::register_type(
    ShaderType::Vector(scalar_type, num_components));
}

/**
 * Returns the type with the given type's scalar or vector shape but the given
 * scalar type, or null for any other shape.
 */
static const ShaderType *
with_scalar_type(const ShaderType *type, ShaderType::ScalarType scalar_type) {
  if (const ShaderType::Vector *vector = type->as_vector()) {
    return make_vector(scalar_type, vector->get_num_components());
  }
  if (type->as_scalar() != nullptr) {
    return make_scalar(scalar_type);
  }
  return nullptr;
}

/**
 * Asserts that the two operands were emitted into the same builder and
 * returns it (or null).
 */
static SpirVBuilder *
common_builder(Value left, Value right) {
  SpirVBuilder *builder = left.get_builder();
  nassertr(builder != nullptr, nullptr);
  nassertr(builder == right.get_builder(), nullptr);
  return builder;
}

/**
 * The shared tail of every function that emits a single value-producing
 * instruction: allocates the result id, inserts {type_id, id, operands...} at
 * the builder's cursor and wraps the result id as a Value.  An operand may be
 * a Value or an Id, contributing its id, or a plain uint32_t, contributing a
 * literal word (such as a component index or an extended-instruction number).
 */
static uint32_t
operand_word(Id id) {
  return id;
}

static uint32_t
operand_word(uint32_t word) {
  return word;
}

template<typename... Operands>
static Value
emit(SpirVBuilder *builder, spv::Op opcode, Id type_id, Operands... operands) {
  Id id = builder->allocate_id();
  builder->insert(opcode, {type_id, id, operand_word(operands)...});
  return Value(*builder, id);
}

/**
 * Turns a number into a module-scope constant of the scalar type of the given
 * operand: GLSL's rule that a literal takes the type of what it meets.  This
 * is the one deferral in the value layer, and the whole of it.
 */
static Value
materialize_scalar(Value like, double value) {
  SpirVBuilder *builder = like.get_builder();
  nassertr(builder != nullptr, Value());

  const ShaderType *type = like.get_type();
  nassertr(type != nullptr, Value());

  Id id = builder->get_module().define_scalar_constant(
    get_element_scalar_type(type), value);
  nassertr(id != 0u, Value());
  return Value(*builder, id);
}

/**
 * As materialize_scalar, but for an operation that needs the number to have
 * the given scalar or vector shape as well as its scalar type, splatting it
 * across a vector type's components.
 *
 * The splat is not redundant with broadcast_to(), which would reach the same
 * shape: this produces a module-scope OpConstantComposite, where letting a
 * scalar through to be broadcast would emit an OpCompositeConstruct into the
 * body on every evaluation.  A number is known at build time, so it should
 * cost nothing at run time.
 */
static Value
materialize(SpirVBuilder *builder, const ShaderType *type, double value) {
  nassertr(type != nullptr, Value());
  SpirVModule &module = builder->get_module();

  Id id = module.define_scalar_constant(get_element_scalar_type(type), value);
  nassertr(id != 0u, Value());

  const ShaderType::Vector *vector = type->as_vector();
  if (vector == nullptr) {
    // A number cannot take the shape of a matrix operand.
    nassertr(type->as_scalar() != nullptr, Value());
    return Value(*builder, id);
  }
  return Value(*builder, module.define_composite_constant(type,
    pvector<Id>(vector->get_num_components(), id)));
}

/**
 * The common case: the shape and scalar type are those of the given operand,
 * GLSL's rule for a literal meeting an expression.
 */
static Value
materialize(Value like, double value) {
  SpirVBuilder *builder = like.get_builder();
  nassertr(builder != nullptr, Value());
  return materialize(builder, like.get_type(), value);
}

/**
 * Applies GLSL's rule that a scalar operand meeting a vector one is broadcast
 * to its component count.  Any other combination is returned unchanged, so
 * that the operation's own type check reports a genuine mismatch rather than
 * this silently papering over it.
 */
static Value
broadcast_to(const ShaderType *type, Value value) {
  const ShaderType::Vector *vector =
    (type != nullptr) ? type->as_vector() : nullptr;
  if (vector == nullptr) {
    return value;
  }
  const ShaderType *value_type = value.get_type();
  const ShaderType::Scalar *scalar =
    (value_type != nullptr) ? value_type->as_scalar() : nullptr;
  if (scalar == nullptr ||
      scalar->get_scalar_type() != vector->get_scalar_type()) {
    return value;
  }

  SpirVBuilder *builder = value.get_builder();
  nassertr(builder != nullptr, Value());

  return builder->construct(
    make_vector(scalar->get_scalar_type(), vector->get_num_components()), value);
}

/**
 * Broadcasts whichever operand of a component-wise operation is the scalar
 * one, so that a scalar expression follows the same rule a scalar literal
 * already does.
 */
static void
broadcast_scalars(Value &left, Value &right) {
  const ShaderType *left_type = left.get_type();
  const ShaderType *right_type = right.get_type();
  if (left_type == right_type) {
    return;
  }
  // Both use the types read before either was replaced, so that at most one
  // of the two broadcasts can apply.
  right = broadcast_to(left_type, right);
  left = broadcast_to(right_type, left);
}

// Stands for a form that GLSL.std.450 does not have, such as an unsigned
// abs(); glsl_std_450() reports it rather than emitting something else.
static const uint32_t no_instruction = (uint32_t)-1;

/**
 * Picks the GLSL.std.450 instruction matching the operand type, for the
 * functions that have separate floating-point, signed and unsigned forms.
 * Pass no_instruction for a form GLSL does not have; the result is then
 * no_instruction as well.
 */
static uint32_t
select_std_450(const ShaderType *type, uint32_t float_instruction,
               uint32_t signed_instruction, uint32_t unsigned_instruction) {
  switch (get_element_scalar_type(type)) {
  case ShaderType::ST_float:
  case ShaderType::ST_double:
    return float_instruction;
  case ShaderType::ST_int:
    return signed_instruction;
  case ShaderType::ST_uint:
    return unsigned_instruction;
  default:
    return no_instruction;
  }
}

/**
 * As select_std_450, but for core instructions with separate floating-point,
 * signed, unsigned and bool forms.  Pass spv::OpNop for a form the operation
 * does not have; that is also the result for any other operand type, so the
 * caller reports it with its own message.
 */
static spv::Op
select_op(const ShaderType *type, spv::Op float_opcode, spv::Op signed_opcode,
          spv::Op unsigned_opcode = spv::OpNop, spv::Op bool_opcode = spv::OpNop) {
  switch (get_element_scalar_type(type)) {
  case ShaderType::ST_float:
  case ShaderType::ST_double:
    return float_opcode;
  case ShaderType::ST_int:
    return signed_opcode;
  case ShaderType::ST_uint:
    return unsigned_opcode;
  case ShaderType::ST_bool:
    return bool_opcode;
  default:
    return spv::OpNop;
  }
}

/**
 * Emits a scalar or vector addition or subtraction instruction.
 */
static Value
add_or_subtract(Value left, Value right, spv::Op float_opcode, spv::Op int_opcode) {
  SpirVBuilder *builder = common_builder(left, right);
  nassertr(builder != nullptr, Value());

  broadcast_scalars(left, right);

  const ShaderType *left_type = left.get_type();
  nassertr(left_type != nullptr, Value());
  nassertr(left_type == right.get_type(), Value());
  nassertr(left_type->as_scalar() != nullptr || left_type->as_vector() != nullptr, Value());

  spv::Op opcode = select_op(left_type, float_opcode, int_opcode, int_opcode);
  nassertr(opcode != spv::OpNop, Value());
  return emit(builder, opcode, builder->get_module().get_type_id(left),
              left, right);
}

/**
 * Emits a comparison instruction, dispatched on the operand scalar type,
 * producing a bool (or, for vector operands, a bvec).
 */
static Value
compare(Value left, Argument right_arg, spv::Op float_opcode,
        spv::Op signed_opcode, spv::Op unsigned_opcode,
        spv::Op bool_opcode = spv::OpNop) {
  Value right = right_arg.resolve(left);
  SpirVBuilder *builder = common_builder(left, right);
  nassertr(builder != nullptr, Value());
  SpirVModule &module = builder->get_module();

  broadcast_scalars(left, right);

  const ShaderType *left_type = left.get_type();
  nassertr(left_type != nullptr, Value());
  nassertr(left_type == right.get_type(), Value());

  spv::Op opcode = select_op(left_type, float_opcode, signed_opcode,
                             unsigned_opcode, bool_opcode);
  if (opcode == spv::OpNop) {
    nassert_raise("no such comparison for this operand type");
    return Value();
  }

  const ShaderType *result_type = with_scalar_type(left_type, ShaderType::ST_bool);
  nassertr(result_type != nullptr, Value());
  return emit(builder, opcode, module.define_type(result_type), left, right);
}

/**
 * Emits a GLSL.std.450 extended instruction whose result type equals the type
 * of its first operand.  All operands must have that same scalar or vector
 * type.  The instruction is the one select_std_450() picked, so that a scalar
 * type for which GLSL has no such function is reported here rather than
 * silently emitting the floating-point form.
 */
template<typename... Operands>
static Value
glsl_std_450(uint32_t instruction, Value first, Operands... rest) {
  if (instruction == no_instruction) {
    nassert_raise("no GLSL.std.450 function of this name for this scalar type");
    return Value();
  }
  SpirVBuilder *builder = first.get_builder();
  nassertr(builder != nullptr, Value());
  SpirVModule &module = builder->get_module();

#ifndef NDEBUG
  const ShaderType *type = first.get_type();
  for (Value operand : {first, rest...}) {
    nassertr(operand.get_builder() == builder, Value());
    nassertr(operand.get_type() == type, Value());
  }
#endif

  return emit(builder, spv::OpExtInst, module.get_type_id(first),
              module.add_ext_inst_import("GLSL.std.450"), instruction,
              first, rest...);
}

/**
 * Emits an OpVectorShuffle sourcing from two vectors at once, with the
 * indices continuing to number into the second vector.
 */
static Value
vector_shuffle(Value vec1, Value vec2, const pvector<uint32_t> &components) {
  SpirVBuilder *builder = common_builder(vec1, vec2);
  nassertr(builder != nullptr, Value());
  SpirVModule &module = builder->get_module();

  const ShaderType *vec1_shader_type = vec1.get_type();
  const ShaderType *vec2_shader_type = vec2.get_type();
  nassertr(vec1_shader_type != nullptr && vec2_shader_type != nullptr, Value());
  const ShaderType::Vector *vec1_type = vec1_shader_type->as_vector();
  const ShaderType::Vector *vec2_type = vec2_shader_type->as_vector();
  nassertr(vec1_type != nullptr && vec2_type != nullptr, Value());
  nassertr(vec1_type->get_scalar_type() == vec2_type->get_scalar_type(), Value());

  const ShaderType *result_type =
    make_vector(vec1_type->get_scalar_type(), components.size());
  Id type_id = module.define_type(result_type);

  // Not emit(): the component words are a runtime-sized list.
  Id id = builder->allocate_id();
  SpirVModule::Instruction op(spv::OpVectorShuffle, {type_id, id, (Id)vec1, (Id)vec2});
  op.args.insert(op.args.end(), components.begin(), components.end());
  builder->insert(std::move(op));
  return Value(*builder, id);
}

/**
 * Returns the builder that the first expression among the given arguments was
 * emitted into, asserting that any others agree.  This is the one thing the
 * vecN and matN constructors must do that SpirVBuilder::construct() is simply
 * given: with no expression at all there is no module for a constructor of
 * numbers only to put its constant in.
 */
static SpirVBuilder *
find_builder(const Argument *args, size_t num_args) {
  SpirVBuilder *builder = nullptr;
  for (size_t i = 0; i < num_args; ++i) {
    if (!args[i].is_literal()) {
      SpirVBuilder *arg_builder = args[i].get_builder();
      nassertr(arg_builder != nullptr, nullptr);
      nassertr(builder == nullptr || builder == arg_builder, nullptr);
      builder = arg_builder;
    }
  }
  if (builder == nullptr) {
    nassert_raise("a vecN or matN constructor needs at least one expression "
                  "argument; for one of numbers only, name the type and use "
                  "SpirVBuilder::construct");
  }
  return builder;
}

/**
 * Implements the vecN constructors, which name their scalar type and
 * component count but must find their builder among their arguments.
 */
static Value
vector_construct(ShaderType::ScalarType scalar_type, uint32_t num_components,
                 std::initializer_list<Argument> args) {
  SpirVBuilder *builder = find_builder(args.begin(), args.size());
  nassertr(builder != nullptr, Value());

  return builder->construct(make_vector(scalar_type, num_components),
                            args.begin(), args.size());
}

/**
 * As above, but for the single-argument vecN constructors, which in GLSL both
 * splat a scalar and truncate a longer vector.
 */
static Value
vector_construct(ShaderType::ScalarType scalar_type, uint32_t num_components,
                 Value value) {
  const ShaderType *type = value.get_type();
  nassertr(type != nullptr, Value());

  if (const ShaderType::Vector *vector = type->as_vector()) {
    // truncate() keeps the value's own type, so the constructor's scalar type
    // must be pinned here; it is not a conversion.
    nassertr(vector->get_scalar_type() == scalar_type, Value());
    return value.truncate(num_components);
  }

  SpirVBuilder *builder = value.get_builder();
  nassertr(builder != nullptr, Value());
  return builder->construct(make_vector(scalar_type, num_components), value);
}

/**
 * Implements the matrix constructors taking one vector per row.  Unlike vecN,
 * matN does not name its scalar type, so that comes from the rows; construct()
 * checks that they all match it and have the right length.
 */
static Value
matrix_construct(uint32_t num_rows, uint32_t num_columns,
                 std::initializer_list<Value> rows) {
  nassertr(rows.size() == num_rows, Value());
  SpirVBuilder *builder = rows.begin()->get_builder();
  nassertr(builder != nullptr, Value());

  const ShaderType *row_type = rows.begin()->get_type();
  nassertr(row_type != nullptr, Value());
  const ShaderType::Vector *row_vector = row_type->as_vector();
  nassertr(row_vector != nullptr, Value());

  const ShaderType *type = ShaderType::register_type(ShaderType::Matrix(
    row_vector->get_scalar_type(), num_rows, num_columns));
  return builder->construct(type, pvector<Value>(rows.begin(), rows.end()));
}

/**
 * Emits an image sample instruction of the given opcode whose result is a
 * scalar of the image's sampled type (the Dref forms).
 */
static Value
image_sample_dref(spv::Op opcode, Value sampler, Value coord, Value dref) {
  SpirVBuilder *builder = common_builder(sampler, coord);
  nassertr(builder != nullptr, Value());
  nassertr(dref.get_builder() == builder, Value());

  const ShaderType *image_type = sampler.get_type();
  nassertr(image_type != nullptr, Value());
  const ShaderType::SampledImage *sampled_image = image_type->as_sampled_image();
  nassertr(sampled_image != nullptr, Value());

  const ShaderType *type = make_scalar(sampled_image->get_sampled_type());
  return emit(builder, opcode, builder->get_module().define_type(type),
              sampler, coord, dref);
}

/**
 * Returns the type of this value.
 */
const ShaderType *Value::
get_type() const {
  if (_id != 0) {
    nassertr(_builder != nullptr, nullptr);
    return _builder->get_module().resolve_type(_id);
  }
  return nullptr;
}

/**
 * Extracts a single component of a vector, or a row of a matrix (a row in
 * Panda's row-vector convention; a column in SPIR-V terms).
 */
Value Value::
operator [](uint32_t index) const {
  nassertr(_builder != nullptr, Value());
  SpirVModule &module = _builder->get_module();

  Id type_id = module.get_type_id(_id);
  nassertr(module.get_definition_type(type_id) != SpirVModule::DT_pointer_type, Value());
  type_id = module.get_composite_member_type_id(type_id, index);
  nassertr(type_id != 0, Value());

  return emit(_builder, spv::OpCompositeExtract, type_id, _id, index);
}

/**
 * Emits a conversion to the given scalar type.  May be a scalar or vector.
 * If it is already of the given scalar type, does nothing.
 */
Value Value::
convert(ShaderType::ScalarType new_scalar_type) const {
  SpirVBuilder *builder = _builder;
  nassertr(builder != nullptr, Value());
  SpirVModule &module = builder->get_module();

  const ShaderType *value_type = get_type();
  nassertr(value_type != nullptr, Value());

  ShaderType::ScalarType old_scalar_type = get_element_scalar_type(value_type);
  if (old_scalar_type == new_scalar_type) {
    // No conversion needed.
    return *this;
  }
  const ShaderType *new_type = with_scalar_type(value_type, new_scalar_type);
  nassertr_always(new_type != nullptr, Value());

  if (old_scalar_type == ShaderType::ST_bool) {
    // GLSL's bool-to-number conversion: select between a materialized 0 and 1.
    return mix(materialize(builder, new_type, 0.0),
               materialize(builder, new_type, 1.0), *this);
  }

  if (new_scalar_type == ShaderType::ST_bool) {
    // Conversion to bool is the same as != 0
    Value null_value(*builder, module.define_null_constant(value_type));
    return not_equal(*this, null_value);
  }

  bool old_float = is_float_scalar_type(old_scalar_type);
  bool new_float = is_float_scalar_type(new_scalar_type);

  // Determine which conversion instruction to use.
  spv::Op opcode;
  if (old_float && new_float) {
    opcode = spv::OpFConvert;
  }
  else if (old_float) {
    opcode = (new_scalar_type == ShaderType::ST_int)
      ? spv::OpConvertFToS : spv::OpConvertFToU;
  }
  else if (new_float) {
    opcode = (old_scalar_type == ShaderType::ST_int)
      ? spv::OpConvertSToF : spv::OpConvertUToF;
  }
  else {
    // Equal scalar types were already handled by the early return above, so
    // this is int<->uint; assuming it's the same bit width, for now.
    opcode = spv::OpBitcast;
  }

  return emit(builder, opcode, module.define_type(new_type), _id);
}

/**
 * Emits an OpBitcast to reinterpret the bits of the value as another type,
 * matching GLSL functions like floatBitsToInt.
 */
Value Value::
bitcast(const ShaderType *type) const {
  nassertr(_builder != nullptr, Value());
  return emit(_builder, spv::OpBitcast,
              _builder->get_module().define_type(type), _id);
}

/**
 * Extracts the given components of the vector, in the given order, into a
 * new vector (or a scalar, if only one component is given).
 */
Value Value::
swizzle(const pvector<uint32_t> &components) const {
  nassertr(components.size() > 0, Value());
  if (components.size() == 1) {
    return (*this)[components[0]];
  }
  return vector_shuffle(*this, *this, components);
}

/**
 *
 */
Value Value::
swizzle(std::initializer_list<uint32_t> components) const {
  return swizzle(pvector<uint32_t>(components.begin(), components.end()));
}

/**
 * Takes the first components of a vector, returning it unchanged if it
 * already has exactly the requested number.
 */
Value Value::
truncate(uint32_t num_components) const {
  nassertr(num_components > 0, Value());

  const ShaderType *type = get_type();
  nassertr(type != nullptr, Value());
  const ShaderType::Vector *vector_type = type->as_vector();
  nassertr(vector_type != nullptr, Value());
  nassertr(vector_type->get_num_components() >= num_components, Value());

  if (vector_type->get_num_components() == num_components) {
    return *this;
  }
  pvector<uint32_t> components;
  components.reserve(num_components);
  for (uint32_t i = 0; i < num_components; ++i) {
    components.push_back(i);
  }
  return swizzle(components);
}

/**
 * Rebinds this value to a copy with the component at the given index
 * replaced by the given value.  A literal takes the replaced part's type:
 * the scalar type for a vector, the row type for a matrix.
 */
void Value::
set_component(uint32_t index, Argument value_arg) {
  nassertv(_builder != nullptr);

  const ShaderType *type = get_type();
  nassertv(type != nullptr);

  const ShaderType *member_type = nullptr;
  if (const ShaderType::Vector *vector = type->as_vector()) {
    member_type = make_scalar(vector->get_scalar_type());
  }
  else if (const ShaderType::Matrix *matrix = type->as_matrix()) {
    member_type = make_vector(matrix->get_scalar_type(),
                              matrix->get_num_columns());
  }
  nassertv(member_type != nullptr);

  Value value = value_arg.resolve(*_builder, member_type);
  nassertv(value.get_builder() == _builder);
  nassertv(value.get_type() == member_type);

  Id type_id = _builder->get_module().get_type_id(_id);
  *this = emit(_builder, spv::OpCompositeInsert, type_id, value, _id, index);
}

/**
 * Rebinds this vector value to a copy with the first three components
 * replaced by the given 3-component vector.  A literal materializes as a
 * vec3-shaped constant of this vector's scalar type.
 */
void Value::
set_xyz(Argument new_xyz_arg) {
  nassertv(_builder != nullptr);

  const ShaderType *type = get_type();
  nassertv(type != nullptr);
  const ShaderType::Vector *vector = type->as_vector();
  nassertv(vector != nullptr);

  uint32_t num_components = vector->get_num_components();
  nassertv(num_components >= 3);

  Value new_xyz = new_xyz_arg.resolve(
    *_builder, make_vector(vector->get_scalar_type(), 3));
  const ShaderType *new_type = new_xyz.get_type();
  nassertv(new_type != nullptr);
  const ShaderType::Vector *new_vector = new_type->as_vector();
  nassertv(new_vector != nullptr && new_vector->get_num_components() == 3);

  // The shuffle numbers the second vector's components after the first's, so
  // the replacement components are num_components..num_components + 2 and the
  // components past the third are kept from this vector.
  pvector<uint32_t> components;
  components.reserve(num_components);
  for (uint32_t i = 0; i < 3; ++i) {
    components.push_back(num_components + i);
  }
  for (uint32_t i = 3; i < num_components; ++i) {
    components.push_back(i);
  }
  *this = vector_shuffle(*this, new_xyz, components);
}

/**
 * Outputs the value's name (or, as values are rarely named, its %id),
 * followed by its type, such as "%57:float3".
 */
void Value::
output(std::ostream &out) const {
  if (_builder == nullptr || _id == 0) {
    out << '%' << _id;
    return;
  }
  SpirVModule &module = _builder->get_module();

  const std::string &name = module.get_name(_id);
  if (!name.empty()) {
    out << name;
  } else {
    out << '%' << _id;
  }

  // Prefer the type's debug name (which a struct type usually has) over
  // spelling out the whole type.
  const std::string &type_name = module.get_name(module.get_type_id(_id));
  if (!type_name.empty()) {
    out << ':' << type_name;
  }
  else if (const ShaderType *type = module.resolve_type(_id)) {
    out << ':' << *type;
  }
}

/**
 * Turns this argument into a Value against an anchoring type: a literal
 * materializes with the type's scalar type and shape, while an expression is
 * checked to have come from the given builder and broadcast if it is a scalar
 * meeting a vector anchor.  This form is for the sinks that know their type
 * without an operand: SpirVPointer::store and the lvalue-swizzle stand-ins.
 */
Value Argument::
resolve(SpirVBuilder &builder, const ShaderType *type) const {
  if (_is_literal) {
    return materialize(&builder, type, _literal);
  } else {
    nassertr(_value.get_builder() == &builder, Value());
    return broadcast_to(type, _value);
  }
}

/**
 * As above, but anchored on an operand (which is always the function's first
 * parameter, and is therefore known to be an expression).
 */
Value Argument::
resolve(Value anchor) const {
  SpirVBuilder *builder = anchor.get_builder();
  nassertr(builder != nullptr, Value());
  return resolve(*builder, anchor.get_type());
}

/**
 * Emits a scalar or vector addition instruction.
 */
Value
operator + (Value left, Value right) {
  return add_or_subtract(left, right, spv::OpFAdd, spv::OpIAdd);
}

/**
 *
 */
Value
operator + (Value left, double right) {
  return left + materialize(left, right);
}

/**
 *
 */
Value
operator + (double left, Value right) {
  return materialize(right, left) + right;
}

/**
 * Emits a scalar or vector subtraction instruction.
 */
Value
operator - (Value left, Value right) {
  return add_or_subtract(left, right, spv::OpFSub, spv::OpISub);
}

/**
 *
 */
Value
operator - (Value left, double right) {
  return left - materialize(left, right);
}

/**
 *
 */
Value
operator - (double left, Value right) {
  return materialize(right, left) - right;
}

/**
 * Emits a multiplication instruction, dispatched on the operand shapes.
 */
Value
operator * (Value left, Value right) {
  SpirVBuilder *builder = common_builder(left, right);
  nassertr(builder != nullptr, Value());
  SpirVModule &module = builder->get_module();

  const ShaderType *left_type = left.get_type();
  const ShaderType *right_type = right.get_type();
  nassertr(left_type != nullptr, Value());
  nassertr(right_type != nullptr, Value());

  Id left_id = left;
  Id right_id = right;
  Id type_id;
  spv::Op opcode;

  if (const ShaderType::Scalar *lscalar = left_type->as_scalar()) {
    if (const ShaderType::Scalar *rscalar = right_type->as_scalar()) {
      nassertr(lscalar == rscalar, Value());
      opcode = is_float_scalar_type(lscalar->get_scalar_type())
        ? spv::OpFMul : spv::OpIMul;
      type_id = module.get_type_id(left_id);
    }
    else if (const ShaderType::Vector *rvector = right_type->as_vector()) {
      nassertr(lscalar->get_scalar_type() == rvector->get_scalar_type(), Value());
      type_id = module.get_type_id(right_id);
      if (is_float_scalar_type(lscalar->get_scalar_type())) {
        // OpVectorTimesScalar is float-only per the spec.
        opcode = spv::OpVectorTimesScalar;
        std::swap(left_id, right_id);
      } else {
        // Integers have no scalar form; splat and multiply component-wise.
        opcode = select_op(right_type, spv::OpNop, spv::OpIMul, spv::OpIMul);
        nassertr(opcode != spv::OpNop, Value());
        left_id = broadcast_to(right_type, left);
      }
    }
    else if (const ShaderType::Matrix *rmatrix = right_type->as_matrix()) {
      nassertr(lscalar->get_scalar_type() == rmatrix->get_scalar_type(), Value());
      opcode = spv::OpMatrixTimesScalar;
      type_id = module.get_type_id(right_id);
      std::swap(left_id, right_id);
    }
    else {
      nassert_raise("right side of multiplication is not a scalar, vector or matrix");
      return Value();
    }
  }
  else if (const ShaderType::Vector *lvector = left_type->as_vector()) {
    if (const ShaderType::Scalar *rscalar = right_type->as_scalar()) {
      nassertr(lvector->get_scalar_type() == rscalar->get_scalar_type(), Value());
      type_id = module.get_type_id(left_id);
      if (is_float_scalar_type(rscalar->get_scalar_type())) {
        // OpVectorTimesScalar is float-only per the spec.
        opcode = spv::OpVectorTimesScalar;
      } else {
        // Integers have no scalar form; splat and multiply component-wise.
        opcode = select_op(left_type, spv::OpNop, spv::OpIMul, spv::OpIMul);
        nassertr(opcode != spv::OpNop, Value());
        right_id = broadcast_to(left_type, right);
      }
    }
    else if (const ShaderType::Vector *rvector = right_type->as_vector()) {
      // Component-wise multiplication, with GLSL/Cg semantics.  For a dot
      // product, use dot() instead.
      nassertr(left_type == right_type, Value());
      (void)rvector;
      opcode = is_float_scalar_type(lvector->get_scalar_type())
        ? spv::OpFMul : spv::OpIMul;
      type_id = module.get_type_id(left_id);
    }
    else if (const ShaderType::Matrix *rmatrix = right_type->as_matrix()) {
      nassertr(lvector->get_scalar_type() == rmatrix->get_scalar_type(), Value());
      nassertr(lvector->get_num_components() == rmatrix->get_num_columns(), Value());
      opcode = spv::OpVectorTimesMatrix;
      const ShaderType *type = ShaderType::register_type(ShaderType::Vector(lvector->get_scalar_type(), rmatrix->get_num_rows()));
      type_id = module.define_type(type);
    }
    else {
      nassert_raise("right side of multiplication is not a scalar, vector or matrix");
      return Value();
    }
  }
  else if (const ShaderType::Matrix *lmatrix = left_type->as_matrix()) {
    if (const ShaderType::Scalar *rscalar = right_type->as_scalar()) {
      nassertr(lmatrix->get_scalar_type() == rscalar->get_scalar_type(), Value());
      opcode = spv::OpMatrixTimesScalar;
      type_id = module.get_type_id(left_id);
    }
    else if (const ShaderType::Vector *rvector = right_type->as_vector()) {
      nassertr(lmatrix->get_scalar_type() == rvector->get_scalar_type(), Value());
      nassertr(lmatrix->get_num_rows() == rvector->get_num_components(), Value());
      opcode = spv::OpMatrixTimesVector;
      const ShaderType *type = ShaderType::register_type(ShaderType::Vector(lmatrix->get_scalar_type(), lmatrix->get_num_columns()));
      type_id = module.define_type(type);
    }
    else if (const ShaderType::Matrix *rmatrix = right_type->as_matrix()) {
      nassertr(lmatrix->get_scalar_type() == rmatrix->get_scalar_type(), Value());
      // Each constituent of the left matrix is combined with a whole
      // constituent of the right one, so the left's constituent count must
      // match the right's constituent length.
      nassertr(lmatrix->get_num_rows() == rmatrix->get_num_columns(), Value());
      opcode = spv::OpMatrixTimesMatrix;
      const ShaderType *type = ShaderType::register_type(ShaderType::Matrix(lmatrix->get_scalar_type(), rmatrix->get_num_rows(), lmatrix->get_num_columns()));
      type_id = module.define_type(type);
    }
    else {
      nassert_raise("right side of multiplication is not a scalar, vector or matrix");
      return Value();
    }
  }
  else {
    nassert_raise("left side of multiplication is not a scalar, vector or matrix");
    return Value();
  }

  return emit(builder, opcode, type_id, left_id, right_id);
}

/**
 *
 */
Value
operator * (Value left, double right) {
  // No splat here: the float scalar forms (OpVectorTimesScalar,
  // OpMatrixTimesScalar) take the literal as a scalar directly, and
  // operator * itself splats for the integer case.
  return left * materialize_scalar(left, right);
}

/**
 *
 */
Value
operator * (double left, Value right) {
  return materialize_scalar(right, left) * right;
}

/**
 * Emits a component-wise division instruction.
 */
Value
operator / (Value left, Value right) {
  SpirVBuilder *builder = common_builder(left, right);
  nassertr(builder != nullptr, Value());

  broadcast_scalars(left, right);

  const ShaderType *left_type = left.get_type();
  nassertr(left_type != nullptr, Value());
  nassertr(left_type == right.get_type(), Value());

  spv::Op opcode = select_op(left_type, spv::OpFDiv, spv::OpSDiv, spv::OpUDiv);
  if (opcode == spv::OpNop) {
    nassert_raise("division requires a float or integer typed operand");
    return Value();
  }
  return emit(builder, opcode, builder->get_module().get_type_id(left),
              left, right);
}

/**
 *
 */
Value
operator / (Value left, double right) {
  return left / materialize(left, right);
}

/**
 *
 */
Value
operator / (double left, Value right) {
  return materialize(right, left) / right;
}

/**
 * Emits a negation instruction.
 */
Value
operator - (Value value) {
  SpirVBuilder *builder = value.get_builder();
  nassertr(builder != nullptr, Value());

  spv::Op opcode = select_op(value.get_type(), spv::OpFNegate, spv::OpSNegate);
  if (opcode == spv::OpNop) {
    nassert_raise("negation requires a float or signed integer typed operand");
    return Value();
  }
  return emit(builder, opcode, builder->get_module().get_type_id(value), value);
}

/**
 *
 */
Value
equal(Value left, Argument right) {
  return compare(left, right, spv::OpFOrdEqual, spv::OpIEqual,
                 spv::OpIEqual, spv::OpLogicalEqual);
}

/**
 *
 */
Value
not_equal(Value left, Argument right) {
  return compare(left, right, spv::OpFUnordNotEqual,
                 spv::OpINotEqual, spv::OpINotEqual, spv::OpLogicalNotEqual);
}

/**
 *
 */
Value
less_than(Value left, Argument right) {
  return compare(left, right, spv::OpFOrdLessThan,
                 spv::OpSLessThan, spv::OpULessThan);
}

/**
 *
 */
Value
less_than_equal(Value left, Argument right) {
  return compare(left, right, spv::OpFOrdLessThanEqual,
                 spv::OpSLessThanEqual, spv::OpULessThanEqual);
}

/**
 *
 */
Value
greater_than(Value left, Argument right) {
  return compare(left, right, spv::OpFOrdGreaterThan,
                 spv::OpSGreaterThan, spv::OpUGreaterThan);
}

/**
 *
 */
Value
greater_than_equal(Value left, Argument right) {
  return compare(left, right, spv::OpFOrdGreaterThanEqual,
                 spv::OpSGreaterThanEqual, spv::OpUGreaterThanEqual);
}

/**
 * The shared body of any() and all(): collapses a bvec to a single bool, and
 * passes a bool through unchanged.
 */
static Value
any_or_all(Value value, spv::Op opcode) {
  SpirVBuilder *builder = value.get_builder();
  nassertr(builder != nullptr, Value());

  const ShaderType *type = value.get_type();
  nassertr(get_element_scalar_type(type) == ShaderType::ST_bool, Value());
  if (type->as_scalar() != nullptr) {
    return value;
  }
  nassertr(type->as_vector() != nullptr, Value());

  return emit(builder, opcode,
              builder->get_module().define_type(ShaderType::BOOL), value);
}

/**
 * Emits an instruction producing true if any component of the given bvec is
 * true.  A scalar bool is returned unchanged.
 */
Value
any(Value value) {
  return any_or_all(value, spv::OpAny);
}

/**
 * Emits an instruction producing true if all components of the given bvec are
 * true.  A scalar bool is returned unchanged.
 */
Value
all(Value value) {
  return any_or_all(value, spv::OpAll);
}

/**
 * Emits an absolute value instruction.  GLSL has no unsigned form.
 */
Value
abs(Value value) {
  uint32_t op = select_std_450(value.get_type(), GLSLstd450FAbs, GLSLstd450SAbs, no_instruction);
  return glsl_std_450(op, value);
}

/**
 * Emits a base-2 exponential instruction.
 */
Value
exp2(Value value) {
  uint32_t op = select_std_450(value.get_type(), GLSLstd450Exp2, no_instruction, no_instruction);
  return glsl_std_450(op, value);
}

/**
 * Emits a power instruction.
 */
Value
pow(Value base, Argument exponent) {
  uint32_t op = select_std_450(base.get_type(), GLSLstd450Pow, no_instruction, no_instruction);
  return glsl_std_450(op, base, exponent.resolve(base));
}

/**
 * Emits a power instruction with a constant base, such as pow(10.0, x); the
 * base takes the exponent's type and shape.
 */
Value
pow(double base, Value exponent) {
  return pow(materialize(exponent, base), exponent);
}

/**
 * Emits a minimum instruction.
 */
Value
min(Value a, Argument b) {
  uint32_t op = select_std_450(a.get_type(), GLSLstd450FMin, GLSLstd450SMin, GLSLstd450UMin);
  return glsl_std_450(op, a, b.resolve(a));
}

/**
 * Emits a maximum instruction.
 */
Value
max(Value a, Argument b) {
  uint32_t op = select_std_450(a.get_type(), GLSLstd450FMax, GLSLstd450SMax, GLSLstd450UMax);
  return glsl_std_450(op, a, b.resolve(a));
}

/**
 * Emits a clamp instruction.
 */
Value
clamp(Value value, Argument min_value, Argument max_value) {
  uint32_t op = select_std_450(value.get_type(), GLSLstd450FClamp, GLSLstd450SClamp, GLSLstd450UClamp);
  return glsl_std_450(op, value, min_value.resolve(value), max_value.resolve(value));
}

/**
 * Emits a linear interpolation instruction.  All three operands must have
 * the same type, except that, as in GLSL, t may instead be a bool (or
 * shape-matched bvec), in which case this selects b where t is true.
 */
Value
mix(Value a, Argument b_arg, Argument t_arg) {
  Value b = b_arg.resolve(a);
  Value t = t_arg.resolve(a);

  SpirVBuilder *builder = common_builder(a, b);
  nassertr(builder != nullptr, Value());
  nassertr(t.get_builder() == builder, Value());
  SpirVModule &module = builder->get_module();

  const ShaderType *t_type = t.get_type();
  if (get_element_scalar_type(t_type) == ShaderType::ST_bool) {
    Id type_id = module.get_type_id(a);
    nassertr(type_id == module.get_type_id(b), Value());

#ifndef NDEBUG
    const ShaderType *a_type = a.get_type();
    const ShaderType::Vector *a_vector = a_type->as_vector();
    const ShaderType::Vector *t_vector = t_type->as_vector();
    nassertr((a_vector == nullptr) == (t_vector == nullptr), Value());
    nassertr(a_vector == nullptr || a_vector->get_num_components() ==
                                    t_vector->get_num_components(), Value());
#endif

    // OpSelect returns its second operand where the condition is true, so
    // the operand order swaps: mix returns b where t is true.
    return emit(builder, spv::OpSelect, type_id, t, b, a);
  }
  uint32_t op = select_std_450(a.get_type(), GLSLstd450FMix, no_instruction, no_instruction);
  return glsl_std_450(op, a, b, t);
}

/**
 * Emits a vector dot product instruction, producing a scalar.
 */
Value
dot(Value a, Argument b_arg) {
  Value b = b_arg.resolve(a);

  SpirVBuilder *builder = common_builder(a, b);
  nassertr(builder != nullptr, Value());
  SpirVModule &module = builder->get_module();

  const ShaderType *left_type = a.get_type();
  nassertr(left_type != nullptr, Value());
  nassertr(left_type == b.get_type(), Value());

  const ShaderType::Vector *vector = left_type->as_vector();
  nassertr(vector != nullptr, Value());
  nassertr(is_float_scalar_type(vector->get_scalar_type()), Value());

  const ShaderType *type = make_scalar(vector->get_scalar_type());
  return emit(builder, spv::OpDot, module.define_type(type), a, b);
}

/**
 * Emits an instruction taking the length of the given vector, producing a
 * scalar.
 */
Value
length(Value vector) {
  SpirVBuilder *builder = vector.get_builder();
  nassertr(builder != nullptr, Value());
  SpirVModule &module = builder->get_module();

  const ShaderType *type = vector.get_type();
  nassertr(type != nullptr, Value());
  const ShaderType::Vector *vector_type = type->as_vector();
  nassertr(vector_type != nullptr, Value());
  nassertr(is_float_scalar_type(vector_type->get_scalar_type()), Value());

  const ShaderType *scalar_type = make_scalar(vector_type->get_scalar_type());
  return emit(builder, spv::OpExtInst, module.define_type(scalar_type),
              module.add_ext_inst_import("GLSL.std.450"),
              (uint32_t)GLSLstd450Length, vector);
}

/**
 * Emits a vector normalization instruction.
 */
Value
normalize(Value vector) {
  uint32_t op = select_std_450(vector.get_type(), GLSLstd450Normalize, no_instruction, no_instruction);
  return glsl_std_450(op, vector);
}

/**
 * Emits a matrix transpose instruction.
 */
Value
transpose(Value matrix) {
  SpirVBuilder *builder = matrix.get_builder();
  nassertr(builder != nullptr, Value());
  SpirVModule &module = builder->get_module();

  Id matrix_id = matrix;
  Id type_id = module.get_type_id(matrix_id);

  const ShaderType *type = matrix.get_type();
  if (const ShaderType::Matrix *matrix_type = type->as_matrix()) {
    if (matrix_type->get_num_rows() != matrix_type->get_num_columns()) {
      type = ShaderType::register_type(ShaderType::Matrix(matrix_type->get_scalar_type(), matrix_type->get_num_columns(), matrix_type->get_num_rows()));
      type_id = module.define_type(type);
    }
  } else {
    nassert_raise("transpose requires a matrix");
    return Value();
  }

  return emit(builder, spv::OpTranspose, type_id, matrix_id);
}

/**
 * Splats a scalar or truncates a longer vector into a vec2.
 */
Value
vec2(Value scalar_or_vector) {
  return vector_construct(ShaderType::ST_float, 2, scalar_or_vector);
}

/**
 * Splats a scalar or truncates a longer vector into a vec3.
 */
Value
vec3(Value scalar_or_vector) {
  return vector_construct(ShaderType::ST_float, 3, scalar_or_vector);
}

/**
 * Splats a scalar into a vec4, or returns a vec4 unchanged.
 */
Value
vec4(Value scalar_or_vector) {
  return vector_construct(ShaderType::ST_float, 4, scalar_or_vector);
}

/**
 * Constructs a vec2.
 */
Value
vec2(Argument x, Argument y) {
  return vector_construct(ShaderType::ST_float, 2, {x, y});
}

/**
 * Constructs a vec3 from a vec2 and a scalar.
 */
Value
vec3(Argument x, Argument y) {
  return vector_construct(ShaderType::ST_float, 3, {x, y});
}

/**
 * Constructs a vec3.
 */
Value
vec3(Argument x, Argument y, Argument z) {
  return vector_construct(ShaderType::ST_float, 3, {x, y, z});
}

/**
 * Constructs a vec4 from a vec3 and a scalar, or two vec2s.
 */
Value
vec4(Argument x, Argument y) {
  return vector_construct(ShaderType::ST_float, 4, {x, y});
}

/**
 * Constructs a vec4 from a vec2 and two scalars.
 */
Value
vec4(Argument x, Argument y, Argument z) {
  return vector_construct(ShaderType::ST_float, 4, {x, y, z});
}

/**
 * Constructs a vec4.
 */
Value
vec4(Argument x, Argument y, Argument z, Argument w) {
  return vector_construct(ShaderType::ST_float, 4, {x, y, z, w});
}

/**
 * Extracts the upper-left 3x3 part of a 3x4 or 4x4 matrix, like GLSL's
 * mat3(mat) constructor or Cg's (float3x3)mat cast.
 */
Value
mat3(Value matrix) {
  return mat3(matrix[0].get_xyz(), matrix[1].get_xyz(), matrix[2].get_xyz());
}

/**
 * Constructs a mat3 from three vec3 rows.
 */
Value
mat3(Value r0, Value r1, Value r2) {
  return matrix_construct(3, 3, {r0, r1, r2});
}

/**
 * Constructs a mat4 from four vec4 rows.
 */
Value
mat4(Value r0, Value r1, Value r2, Value r3) {
  return matrix_construct(4, 4, {r0, r1, r2, r3});
}

/**
 * Constructs a 3x4 matrix from three vec4 rows.
 */
Value
mat3x4(Value r0, Value r1, Value r2) {
  return matrix_construct(3, 4, {r0, r1, r2});
}

/**
 * Emits an OpImageSampleImplicitLod.
 */
Value
texture(Value sampler, Value coord) {
  SpirVBuilder *builder = common_builder(sampler, coord);
  nassertr(builder != nullptr, Value());
  return Value(*builder, builder->op_image_sample(sampler, coord, 0u, nullptr));
}

/**
 * Emits an OpImageSampleDrefImplicitLod, which performs a depth comparison
 * against the given reference value.  The result is a scalar.
 */
Value
texture_dref(Value sampler, Value coord, Value dref) {
  return image_sample_dref(spv::OpImageSampleDrefImplicitLod, sampler, coord, dref);
}

/**
 * Emits an OpImageSampleProjImplicitLod, which divides the coordinate by its
 * last component before sampling.
 */
Value
texture_proj(Value sampler, Value coord) {
  SpirVBuilder *builder = common_builder(sampler, coord);
  nassertr(builder != nullptr, Value());

  const ShaderType *image_type = sampler.get_type();
  nassertr(image_type != nullptr, Value());
  const ShaderType::SampledImage *sampled_image = image_type->as_sampled_image();
  nassertr(sampled_image != nullptr, Value());

  const ShaderType *type = make_vector(sampled_image->get_sampled_type(), 4);
  return emit(builder, spv::OpImageSampleProjImplicitLod,
              builder->get_module().define_type(type), sampler, coord);
}

/**
 * Like texture_dref, but the coordinate and reference value are divided by
 * the coordinate's last component first.
 *
 * This emits the division explicitly rather than using the ProjDref
 * instruction, which the NVIDIA driver fails to translate ("Can't convert to
 * expr: textureProj").  Since drivers lower ProjDref to an explicit division
 * anyway, this costs nothing on conforming drivers.
 */
Value
texture_proj_dref(Value sampler, Value coord, Value dref) {
  const ShaderType *coord_type = coord.get_type();
  nassertr(coord_type != nullptr, Value());
  const ShaderType::Vector *coord_vector = coord_type->as_vector();
  nassertr(coord_vector != nullptr, Value());
  uint32_t last = coord_vector->get_num_components() - 1;

  pvector<uint32_t> components(last);
  for (uint32_t i = 0; i < last; ++i) {
    components[i] = i;
  }
  Value q = coord[last];
  return texture_dref(sampler, coord.swizzle(components) / q, dref / q);
}

/**
 * Emits an OpImageFetch, which reads a single texel using an (integer)
 * coordinate, without filtering.  The given image may be a sampled image, in
 * which case the image is first extracted from it.
 */
Value
texel_fetch(Value image, Value coord) {
  SpirVBuilder *builder = common_builder(image, coord);
  nassertr(builder != nullptr, Value());
  SpirVModule &module = builder->get_module();

  const ShaderType *image_type = image.get_type();
  nassertr(image_type != nullptr, Value());

  ShaderType::ScalarType sampled_type;
  if (const ShaderType::SampledImage *sampled_image = image_type->as_sampled_image()) {
    // Extract the image from the sampled image.
    const SpirVModule::Instruction *decl = module.find_declaration(module.get_type_id(image));
    nassertr(decl != nullptr && decl->opcode == spv::OpTypeSampledImage, Value());
    Id image_type_id(decl->args[1]);
    image = emit(builder, spv::OpImage, image_type_id, image);

    sampled_type = sampled_image->get_sampled_type();
  }
  else if (const ShaderType::Image *plain_image = image_type->as_image()) {
    sampled_type = plain_image->get_sampled_type();
  }
  else {
    nassert_raise("texel_fetch requires an image or sampled image");
    return Value();
  }

  const ShaderType *type = make_vector(sampled_type, 4);
  return emit(builder, spv::OpImageFetch, module.define_type(type), image, coord);
}

}  // namespace glsl
