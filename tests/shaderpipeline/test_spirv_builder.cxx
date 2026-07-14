/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_builder.cxx
 * @author rdb
 * @date 2026-07-14
 */

#include "catch_amalgamated.hpp"

#include "spirv_test_utils.h"

TEST_CASE("SpirVBuilder op_convert produces bool vectors for vector operands", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  Id id_var = module.define_variable(vec3_type, spv::StorageClassPrivate);

  Id id_result;
  {
    SpirVBuilder builder = make_entry_point(module);
    Id id_value = builder.op_load(id_var);
    id_result = builder.op_convert(ShaderType::ST_bool, id_value);
    builder.op_return();
  }

  const ShaderType *result_type = module.resolve_type(id_result);
  REQUIRE(result_type != nullptr);
  CHECK(result_type == ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_bool, 3)));

  InstructionStream stream = module.emit();
  CHECK(stream.validate());
}

TEST_CASE("SpirVBuilder op_convert converts bool to double", "[shaderpipeline]") {
  SpirVModule module = make_module();
  module.add_capability(spv::CapabilityFloat64);

  const ShaderType *bvec2_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_bool, 2));
  Id id_scalar_var = module.define_variable(ShaderType::BOOL, spv::StorageClassPrivate);
  Id id_vector_var = module.define_variable(bvec2_type, spv::StorageClassPrivate);

  Id id_scalar_result, id_vector_result;
  {
    SpirVBuilder builder = make_entry_point(module);
    id_scalar_result = builder.op_convert(ShaderType::ST_double, builder.op_load(id_scalar_var));
    id_vector_result = builder.op_convert(ShaderType::ST_double, builder.op_load(id_vector_var));
    builder.op_return();
  }

  CHECK(module.resolve_type(id_scalar_result) == ShaderType::DOUBLE);
  CHECK(module.resolve_type(id_vector_result) == ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_double, 2)));

  InstructionStream stream = module.emit();
  CHECK(stream.validate());
}

TEST_CASE("SpirVModule parse resets existing contents", "[shaderpipeline]") {
  SpirVModule module = make_module();
  Id id_var = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  module.set_name(id_var, "one");
  {
    SpirVBuilder builder = make_entry_point(module);
    builder.op_return();
  }
  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  // Parsing a different stream into the module must not retain anything from
  // the contents it held before.
  SpirVModule other = make_module();
  other.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  other.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  {
    SpirVBuilder builder = make_entry_point(other);
    builder.op_return();
  }

  REQUIRE(other.parse(stream.get_data(), stream.get_data_size()));

  InstructionStream reparsed = other.emit();
  REQUIRE(reparsed.validate());
  CHECK(count_op(reparsed, spv::OpVariable) == 1);
  CHECK(count_op(reparsed, spv::OpEntryPoint) == 1);
  CHECK(other.get_name(id_var) == "one");
}
