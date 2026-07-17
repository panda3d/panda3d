/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_replace_variable_type_pass.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "spirVReplaceVariableTypePass.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVReplaceVariableTypePass converts loads to the old type", "[shaderpipeline]") {
  // uniform float u; out float o;  main() { o = u; }
  SpirVModule module = make_module();

  Id id_var = module.define_variable(ShaderType::FLOAT, spv::StorageClassUniformConstant);
  module.set_name(id_var, "u");

  Id id_out = module.define_variable(ShaderType::FLOAT, spv::StorageClassOutput);
  module.decorate(id_out, spv::DecorationLocation, 0u);

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    Id id_value = builder.op_load(id_var);
    builder.op_store(id_out, id_value);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SECTION("replacing float with int converts the loaded value") {
    SpirVTransformer transformer(stream);
    transformer.run(SpirVReplaceVariableTypePass(id_var, ShaderType::INT,
                                                 spv::StorageClassUniformConstant));
    CHECK(transformer.get_module().validate());

    InstructionStream result = transformer.get_result();

    // The variable is now an int, and the loaded value is converted back to
    // float where it is used.
    CHECK(transformer.get_module().resolve_type(id_var) == ShaderType::INT);
    CHECK(count_op(result, spv::OpTypeInt) == 1);
    CHECK(count_op(result, spv::OpConvertSToF) == 1);
  }

  SECTION("replacing float with vec4 extracts the first component") {
    const ShaderType *vec4_type =
      ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

    SpirVTransformer transformer(stream);
    transformer.run(SpirVReplaceVariableTypePass(id_var, vec4_type,
                                                 spv::StorageClassUniformConstant));
    CHECK(transformer.get_module().validate());

    InstructionStream result = transformer.get_result();
    CHECK(transformer.get_module().resolve_type(id_var) == vec4_type);
    CHECK(count_op(result, spv::OpCompositeExtract) == 1);
  }
}

TEST_CASE("SpirVReplaceVariableTypePass replicates a scalar to a vector", "[shaderpipeline]") {
  // uniform vec4 u; out vec4 o;  main() { o = u; }
  SpirVModule module = make_module();
  const ShaderType *vec4_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  Id id_var = module.define_variable(vec4_type, spv::StorageClassUniformConstant);
  module.set_name(id_var, "u");

  Id id_out = module.define_variable(vec4_type, spv::StorageClassOutput);
  module.decorate(id_out, spv::DecorationLocation, 0u);

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    Id id_value = builder.op_load(id_var);
    builder.op_store(id_out, id_value);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  transformer.run(SpirVReplaceVariableTypePass(id_var, ShaderType::FLOAT,
                                               spv::StorageClassUniformConstant));
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();
  CHECK(transformer.get_module().resolve_type(id_var) == ShaderType::FLOAT);
  CHECK(count_op(result, spv::OpCompositeConstruct) == 1);
}
