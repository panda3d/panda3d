/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_remove_unused_variables_pass.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "spirVRemoveUnusedVariablesPass.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVRemoveUnusedVariablesPass removes unused variables", "[shaderpipeline]") {
  enum : uint32_t {
    id_main = 1, id_void, id_fnvoid, id_float, id_vec4,
    id_ptr_in, id_ptr_out, id_ptr_uc,
    id_in_color, id_out_color, id_unused,
    id_label, id_tmp,
    id_bound,
  };

  ModuleBuilder b;
  b.op(spv::OpCapability, {spv::CapabilityShader});
  b.op(spv::OpMemoryModel, {spv::AddressingModelLogical, spv::MemoryModelGLSL450});
  b.op(spv::OpEntryPoint, {spv::ExecutionModelFragment, id_main}, "main", {id_in_color, id_out_color});
  b.op(spv::OpExecutionMode, {id_main, spv::ExecutionModeOriginUpperLeft});
  b.op(spv::OpName, {id_unused}, "unused");
  b.op(spv::OpDecorate, {id_in_color, spv::DecorationLocation, 0});
  b.op(spv::OpDecorate, {id_out_color, spv::DecorationLocation, 0});
  b.op(spv::OpTypeVoid, {id_void});
  b.op(spv::OpTypeFunction, {id_fnvoid, id_void});
  b.op(spv::OpTypeFloat, {id_float, 32});
  b.op(spv::OpTypeVector, {id_vec4, id_float, 4});
  b.op(spv::OpTypePointer, {id_ptr_in, spv::StorageClassInput, id_vec4});
  b.op(spv::OpTypePointer, {id_ptr_out, spv::StorageClassOutput, id_vec4});
  b.op(spv::OpTypePointer, {id_ptr_uc, spv::StorageClassUniformConstant, id_float});
  b.op(spv::OpVariable, {id_ptr_in, id_in_color, spv::StorageClassInput});
  b.op(spv::OpVariable, {id_ptr_out, id_out_color, spv::StorageClassOutput});
  b.op(spv::OpVariable, {id_ptr_uc, id_unused, spv::StorageClassUniformConstant});
  b.op(spv::OpFunction, {id_void, id_main, spv::FunctionControlMaskNone, id_fnvoid});
  b.op(spv::OpLabel, {id_label});
  b.op(spv::OpLoad, {id_vec4, id_tmp, id_in_color});
  b.op(spv::OpStore, {id_out_color, id_tmp});
  b.op(spv::OpReturn, {});
  b.op(spv::OpFunctionEnd, {});

  InstructionStream stream = b.build(id_bound);
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  transformer.run(SpirVRemoveUnusedVariablesPass());
  CHECK(transformer.validate_db());

  InstructionStream result = transformer.get_result();
  CHECK(result.validate());
  CHECK(!has_variable(result, id_unused));
  CHECK(has_variable(result, id_in_color));
  CHECK(has_variable(result, id_out_color));
  CHECK(!transformer.get_db().get_definition(id_unused).is_variable());
}
