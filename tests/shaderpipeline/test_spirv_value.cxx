/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_value.cxx
 * @author rdb
 * @date 2026-07-27
 */

#include "catch_amalgamated.hpp"

#include "spirv_test_utils.h"

TEST_CASE("SpirVValue convert produces bool vectors for vector operands", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));

  Id id_result;
  {
    SpirVBuilder builder = make_entry_point(module);
    id_result = builder.define_private(vec3_type).load().convert(ShaderType::ST_bool);
    builder.op_return();
  }

  const ShaderType *result_type = module.resolve_type(id_result);
  REQUIRE(result_type != nullptr);
  CHECK(result_type == ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_bool, 3)));

  InstructionStream stream = module.emit();
  CHECK(stream.validate());
  CHECK(count_op(stream, spv::OpFUnordNotEqual) == 1);
}

TEST_CASE("SpirVValue convert converts bool to double", "[shaderpipeline]") {
  SpirVModule module = make_module();
  module.add_capability(spv::CapabilityFloat64);

  const ShaderType *bvec2_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_bool, 2));

  Id id_scalar_result, id_vector_result;
  {
    SpirVBuilder builder = make_entry_point(module);
    id_scalar_result = builder.define_private(ShaderType::BOOL).load().convert(ShaderType::ST_double);
    id_vector_result = builder.define_private(bvec2_type).load().convert(ShaderType::ST_double);
    builder.op_return();
  }

  CHECK(module.resolve_type(id_scalar_result) == ShaderType::DOUBLE);
  CHECK(module.resolve_type(id_vector_result) == ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_double, 2)));

  InstructionStream stream = module.emit();
  CHECK(stream.validate());
  CHECK(count_op(stream, spv::OpSelect) == 2);
}

TEST_CASE("SpirVValue broadcasts a scalar operand to a vector one", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));

  Id id_sum, id_quotient, id_reversed;
  {
    SpirVBuilder builder = make_entry_point(module);
    Value vector = builder.define_private(vec3_type).load();
    Value scalar = builder.define_private(ShaderType::FLOAT).load();
    id_sum = vector + scalar;
    id_quotient = vector / scalar;
    // The scalar on the left broadcasts just the same.
    id_reversed = scalar - vector;
    builder.op_return();
  }

  CHECK(module.resolve_type(id_sum) == vec3_type);
  CHECK(module.resolve_type(id_quotient) == vec3_type);
  CHECK(module.resolve_type(id_reversed) == vec3_type);

  InstructionStream stream = module.emit();
  // One splat per operation, and no more: an operand that already matches is
  // left alone.
  CHECK(count_op(stream, spv::OpCompositeConstruct) == 3);
  CHECK(stream.validate());
}

TEST_CASE("SpirVValue vecN truncates a longer vector", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec2_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 2));
  const ShaderType *vec4_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 4));

  Id id_xy, id_same, id_splat;
  {
    SpirVBuilder builder = make_entry_point(module);
    Value value = builder.define_private(vec4_type).load();
    id_xy = vec2(value);
    // Asking for the count it already has is a no-op, not an instruction.
    id_same = vec4(value);
    // A scalar still splats.
    id_splat = vec2(value[0]);
    builder.op_return();
  }

  CHECK(module.resolve_type(id_xy) == vec2_type);
  CHECK(module.resolve_type(id_same) == vec4_type);
  CHECK(module.resolve_type(id_splat) == vec2_type);

  InstructionStream stream = module.emit();
  // The truncation is a shuffle, the splat a construct, and vec4() of a vec4
  // emitted neither.
  CHECK(count_op(stream, spv::OpVectorShuffle) == 1);
  CHECK(count_op(stream, spv::OpCompositeConstruct) == 1);
  CHECK(stream.validate());
}

TEST_CASE("SpirVValue min and max dispatch on the operand scalar type", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *ivec2_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_int, 2));

  Id id_signed, id_unsigned, id_float;
  {
    SpirVBuilder builder = make_entry_point(module);
    // The literal materializes as an int and splats to the vector's shape.
    id_signed = min(builder.define_private(ivec2_type).load(), 3);
    id_unsigned = max(builder.define_private(ShaderType::UINT).load(), 3);
    id_float = clamp(builder.define_private(ShaderType::FLOAT).load(), 0.0f, 1.0f);
    builder.op_return();
  }

  CHECK(module.resolve_type(id_signed) == ivec2_type);
  CHECK(module.resolve_type(id_unsigned) == ShaderType::UINT);
  CHECK(module.resolve_type(id_float) == ShaderType::FLOAT);

  InstructionStream stream = module.emit();
  CHECK(stream.validate());
}

TEST_CASE("SpirVValue any and all collapse a bvec to a bool", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));

  Id id_any, id_all, id_passthrough, id_scalar_cond;
  {
    SpirVBuilder builder = make_entry_point(module);
    Value vector = builder.define_private(vec3_type).load();
    id_any = any(less_than(vector, 0.5));
    id_all = all(greater_than(vector, 0.5));

    // A scalar bool passes through unchanged, without an instruction.
    Value cond = equal(builder.define_private(ShaderType::FLOAT).load(), 0.0);
    id_scalar_cond = cond;
    id_passthrough = all(cond);
    builder.op_return();
  }

  CHECK(module.resolve_type(id_any) == ShaderType::BOOL);
  CHECK(module.resolve_type(id_all) == ShaderType::BOOL);
  CHECK(id_passthrough == id_scalar_cond);

  InstructionStream stream = module.emit();
  CHECK(count_op(stream, spv::OpAny) == 1);
  CHECK(count_op(stream, spv::OpAll) == 1);
  CHECK(stream.validate());
}

TEST_CASE("SpirVValue setters and pow accept literal arguments", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec4_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 4));

  Id id_pow;
  {
    SpirVBuilder builder = make_entry_point(module);
    Pointer vec_ptr = builder.define_private(vec4_type);
    Pointer float_ptr = builder.define_private(ShaderType::FLOAT);

    // A stored literal takes the pointee's scalar type and shape.
    vec_ptr.store(0.0);
    float_ptr.store(0.5);

    // A literal lvalue swizzle takes the replaced part's type.
    Value value = vec_ptr.load();
    value.set_w(1.0);
    value.set_xyz(0.25);
    vec_ptr.store(value);

    // pow with a constant base anchors on the exponent.
    id_pow = pow(2.0, float_ptr.load());
    builder.op_return();
  }

  CHECK(module.resolve_type(id_pow) == ShaderType::FLOAT);

  InstructionStream stream = module.emit();
  // Every literal materialized as a module-scope constant: no in-body splats.
  CHECK(count_op(stream, spv::OpCompositeConstruct) == 0);
  CHECK(count_op(stream, spv::OpStore) == 3);
  CHECK(count_op(stream, spv::OpCompositeInsert) == 1);
  CHECK(count_op(stream, spv::OpVectorShuffle) == 1);
  CHECK(stream.validate());
}

TEST_CASE("SpirVValue multiplies an integer vector by a scalar component-wise", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *ivec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_int, 3));

  Id id_literal, id_scalar_left;
  {
    SpirVBuilder builder = make_entry_point(module);
    Value vector = builder.define_private(ivec3_type).load();
    id_literal = vector * 2;
    id_scalar_left = builder.const_int(3) * vector;
    builder.op_return();
  }

  CHECK(module.resolve_type(id_literal) == ivec3_type);
  CHECK(module.resolve_type(id_scalar_left) == ivec3_type);

  InstructionStream stream = module.emit();
  // OpVectorTimesScalar is float-only, so the scalar splats instead.
  CHECK(count_op(stream, spv::OpVectorTimesScalar) == 0);
  CHECK(count_op(stream, spv::OpIMul) == 2);
  CHECK(count_op(stream, spv::OpCompositeConstruct) == 2);
  CHECK(stream.validate());
}

TEST_CASE("SpirVValue set_xyz keeps the components past the third", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *vec4_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 4));

  Id id_result;
  {
    SpirVBuilder builder = make_entry_point(module);
    Value value = builder.define_private(vec4_type).load();
    value.set_xyz(builder.define_private(vec3_type).load());
    id_result = value;
    builder.op_return();
  }

  CHECK(module.resolve_type(id_result) == vec4_type);

  InstructionStream stream = module.emit();
  Instruction shuffle = find_op(stream, spv::OpVectorShuffle);
  REQUIRE(shuffle.nargs == 8);
  // The first three come from the second vector, the w from the first.
  CHECK(shuffle.args[4] == 4);
  CHECK(shuffle.args[5] == 5);
  CHECK(shuffle.args[6] == 6);
  CHECK(shuffle.args[7] == 3);
  CHECK(stream.validate());
}

TEST_CASE("SpirVValue prints its id, name and type", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));

  ShaderType::Struct struct_type;
  struct_type.add_member(vec3_type, "position");
  const ShaderType *light_type = ShaderType::register_type(std::move(struct_type));
  module.set_name(module.define_type(light_type), "Light");

  {
    SpirVBuilder builder = make_entry_point(module);

    // An unnamed value prints as %id with its type spelled out.
    Value value = builder.define_private(vec3_type).load();
    std::ostringstream unnamed;
    unnamed << value;
    std::ostringstream expected;
    expected << '%' << (Id)value << ":float3";
    CHECK(unnamed.str() == expected.str());

    // A debug name replaces the id, and a type's name is preferred over
    // spelling the type out.
    Value light = builder.define_private(light_type).load();
    module.set_name(light, "light");
    std::ostringstream named;
    named << light;
    CHECK(named.str() == "light:Light");

    builder.op_return();
  }

  // A default-constructed value has no module to consult.
  std::ostringstream empty;
  empty << Value();
  CHECK(empty.str() == "%0");
}
