/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_make_block_pass.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "spirVMakeBlockPass.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVMakeBlockPass gathers variables into a block", "[shaderpipeline]") {
  enum : uint32_t {
    id_main = 1, id_void, id_fnvoid, id_float, id_vec4,
    id_ptr_uc_float, id_ptr_uc_vec4, id_var_a, id_var_b,
    id_label, id_load_a, id_load_b,
    id_bound,
  };

  // uniform float a; uniform vec4 b;  main() loads both.
  ModuleBuilder b;
  b.op(spv::OpCapability, {spv::CapabilityShader});
  b.op(spv::OpMemoryModel, {spv::AddressingModelLogical, spv::MemoryModelGLSL450});
  b.op(spv::OpEntryPoint, {spv::ExecutionModelFragment, id_main}, "main");
  b.op(spv::OpExecutionMode, {id_main, spv::ExecutionModeOriginUpperLeft});
  b.op(spv::OpName, {id_var_a}, "a");
  b.op(spv::OpName, {id_var_b}, "b");
  b.op(spv::OpTypeVoid, {id_void});
  b.op(spv::OpTypeFunction, {id_fnvoid, id_void});
  b.op(spv::OpTypeFloat, {id_float, 32});
  b.op(spv::OpTypeVector, {id_vec4, id_float, 4});
  b.op(spv::OpTypePointer, {id_ptr_uc_float, spv::StorageClassUniformConstant, id_float});
  b.op(spv::OpTypePointer, {id_ptr_uc_vec4, spv::StorageClassUniformConstant, id_vec4});
  b.op(spv::OpVariable, {id_ptr_uc_float, id_var_a, spv::StorageClassUniformConstant});
  b.op(spv::OpVariable, {id_ptr_uc_vec4, id_var_b, spv::StorageClassUniformConstant});
  b.op(spv::OpFunction, {id_void, id_main, spv::FunctionControlMaskNone, id_fnvoid});
  b.op(spv::OpLabel, {id_label});
  b.op(spv::OpLoad, {id_float, id_load_a, id_var_a});
  b.op(spv::OpLoad, {id_vec4, id_load_b, id_var_b});
  b.op(spv::OpReturn, {});
  b.op(spv::OpFunctionEnd, {});

  InstructionStream stream = b.build(id_bound);
  REQUIRE(stream.validate());

  ShaderType::Struct struct_type;
  struct_type.add_member(ShaderType::FLOAT, "a");
  struct_type.add_member(
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4)), "b");
  const ShaderType::Struct *block_type =
    ShaderType::register_type(std::move(struct_type));

  SpirVTransformer transformer(stream);
  SpirVMakeBlockPass pass(block_type, {id_var_a, id_var_b}, spv::StorageClassUniform, 1, 2);
  transformer.run(pass);
  CHECK(transformer.validate_db());

  InstructionStream result = transformer.get_result();
  CHECK(result.validate());

  // The loose variables are gone, replaced by a single block variable.
  CHECK(!has_variable(result, id_var_a));
  CHECK(!has_variable(result, id_var_b));
  REQUIRE(pass._block_var_id != 0);
  CHECK(has_variable(result, pass._block_var_id));

  const SpirVResultDatabase::Definition &def =
    transformer.get_db().get_definition(pass._block_var_id);
  CHECK(def._type == block_type);
  CHECK(def._storage_class == spv::StorageClassUniform);

  // Each load now goes through an access chain into the block.
  CHECK(count_op(result, spv::OpAccessChain) == 2);
}
