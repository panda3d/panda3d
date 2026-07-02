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
  enum : uint32_t {
    id_main = 1, id_void, id_fnvoid, id_float,
    id_ptr_uc_float, id_var, id_ptr_out_float, id_out,
    id_label, id_value,
    id_bound,
  };

  // uniform float u; out float o;  main() { o = u; }
  ModuleBuilder b;
  b.op(spv::OpCapability, {spv::CapabilityShader});
  b.op(spv::OpMemoryModel, {spv::AddressingModelLogical, spv::MemoryModelGLSL450});
  b.op(spv::OpEntryPoint, {spv::ExecutionModelFragment, id_main}, "main", {id_out});
  b.op(spv::OpExecutionMode, {id_main, spv::ExecutionModeOriginUpperLeft});
  b.op(spv::OpName, {id_var}, "u");
  b.op(spv::OpDecorate, {id_out, spv::DecorationLocation, 0});
  b.op(spv::OpTypeVoid, {id_void});
  b.op(spv::OpTypeFunction, {id_fnvoid, id_void});
  b.op(spv::OpTypeFloat, {id_float, 32});
  b.op(spv::OpTypePointer, {id_ptr_uc_float, spv::StorageClassUniformConstant, id_float});
  b.op(spv::OpTypePointer, {id_ptr_out_float, spv::StorageClassOutput, id_float});
  b.op(spv::OpVariable, {id_ptr_uc_float, id_var, spv::StorageClassUniformConstant});
  b.op(spv::OpVariable, {id_ptr_out_float, id_out, spv::StorageClassOutput});
  b.op(spv::OpFunction, {id_void, id_main, spv::FunctionControlMaskNone, id_fnvoid});
  b.op(spv::OpLabel, {id_label});
  b.op(spv::OpLoad, {id_float, id_value, id_var});
  b.op(spv::OpStore, {id_out, id_value});
  b.op(spv::OpReturn, {});
  b.op(spv::OpFunctionEnd, {});

  InstructionStream stream = b.build(id_bound);
  REQUIRE(stream.validate());

  SECTION("replacing float with int converts the loaded value") {
    SpirVTransformer transformer(stream);
    transformer.run(SpirVReplaceVariableTypePass(id_var, ShaderType::INT,
                                                 spv::StorageClassUniformConstant));
    CHECK(transformer.validate_db());

    InstructionStream result = transformer.get_result();
    CHECK(result.validate());

    // The variable is now an int, and the loaded value is converted back to
    // float where it is used.
    CHECK(transformer.get_db().get_definition(id_var)._type == ShaderType::INT);
    CHECK(count_op(result, spv::OpTypeInt) == 1);
    CHECK(count_op(result, spv::OpConvertSToF) == 1);
  }

  SECTION("replacing float with vec4 extracts the first component") {
    const ShaderType *vec4_type =
      ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

    SpirVTransformer transformer(stream);
    transformer.run(SpirVReplaceVariableTypePass(id_var, vec4_type,
                                                 spv::StorageClassUniformConstant));
    CHECK(transformer.validate_db());

    InstructionStream result = transformer.get_result();
    CHECK(result.validate());
    CHECK(transformer.get_db().get_definition(id_var)._type == vec4_type);
    CHECK(count_op(result, spv::OpCompositeExtract) == 1);
  }
}

TEST_CASE("SpirVReplaceVariableTypePass replicates a scalar to a vector", "[shaderpipeline]") {
  enum : uint32_t {
    id_main = 1, id_void, id_fnvoid, id_float, id_vec4,
    id_ptr_uc_vec4, id_var, id_ptr_out_vec4, id_out,
    id_label, id_value,
    id_bound,
  };

  // uniform vec4 u; out vec4 o;  main() { o = u; }
  ModuleBuilder b;
  b.op(spv::OpCapability, {spv::CapabilityShader});
  b.op(spv::OpMemoryModel, {spv::AddressingModelLogical, spv::MemoryModelGLSL450});
  b.op(spv::OpEntryPoint, {spv::ExecutionModelFragment, id_main}, "main", {id_out});
  b.op(spv::OpExecutionMode, {id_main, spv::ExecutionModeOriginUpperLeft});
  b.op(spv::OpName, {id_var}, "u");
  b.op(spv::OpDecorate, {id_out, spv::DecorationLocation, 0});
  b.op(spv::OpTypeVoid, {id_void});
  b.op(spv::OpTypeFunction, {id_fnvoid, id_void});
  b.op(spv::OpTypeFloat, {id_float, 32});
  b.op(spv::OpTypeVector, {id_vec4, id_float, 4});
  b.op(spv::OpTypePointer, {id_ptr_uc_vec4, spv::StorageClassUniformConstant, id_vec4});
  b.op(spv::OpTypePointer, {id_ptr_out_vec4, spv::StorageClassOutput, id_vec4});
  b.op(spv::OpVariable, {id_ptr_uc_vec4, id_var, spv::StorageClassUniformConstant});
  b.op(spv::OpVariable, {id_ptr_out_vec4, id_out, spv::StorageClassOutput});
  b.op(spv::OpFunction, {id_void, id_main, spv::FunctionControlMaskNone, id_fnvoid});
  b.op(spv::OpLabel, {id_label});
  b.op(spv::OpLoad, {id_vec4, id_value, id_var});
  b.op(spv::OpStore, {id_out, id_value});
  b.op(spv::OpReturn, {});
  b.op(spv::OpFunctionEnd, {});

  InstructionStream stream = b.build(id_bound);
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  transformer.run(SpirVReplaceVariableTypePass(id_var, ShaderType::FLOAT,
                                               spv::StorageClassUniformConstant));
  CHECK(transformer.validate_db());

  InstructionStream result = transformer.get_result();
  CHECK(result.validate());
  CHECK(transformer.get_db().get_definition(id_var)._type == ShaderType::FLOAT);
  CHECK(count_op(result, spv::OpCompositeConstruct) == 1);
}
