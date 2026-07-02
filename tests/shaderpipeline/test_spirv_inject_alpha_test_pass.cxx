/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_inject_alpha_test_pass.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "spirVInjectAlphaTestPass.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVInjectAlphaTestPass injects a comparison", "[shaderpipeline]") {
  enum : uint32_t {
    id_main = 1, id_void, id_fnvoid, id_float, id_vec4,
    id_ptr_out, id_color, id_const_half, id_const_color,
    id_label,
    id_bound,
  };

  // out vec4 p3d_FragColor;  main() { p3d_FragColor = vec4(0.5); }
  ModuleBuilder b;
  b.op(spv::OpCapability, {spv::CapabilityShader});
  b.op(spv::OpMemoryModel, {spv::AddressingModelLogical, spv::MemoryModelGLSL450});
  b.op(spv::OpEntryPoint, {spv::ExecutionModelFragment, id_main}, "main", {id_color});
  b.op(spv::OpExecutionMode, {id_main, spv::ExecutionModeOriginUpperLeft});
  b.op(spv::OpDecorate, {id_color, spv::DecorationLocation, 0});
  b.op(spv::OpTypeVoid, {id_void});
  b.op(spv::OpTypeFunction, {id_fnvoid, id_void});
  b.op(spv::OpTypeFloat, {id_float, 32});
  b.op(spv::OpTypeVector, {id_vec4, id_float, 4});
  b.op(spv::OpTypePointer, {id_ptr_out, spv::StorageClassOutput, id_vec4});
  b.op(spv::OpVariable, {id_ptr_out, id_color, spv::StorageClassOutput});
  b.op(spv::OpConstant, {id_float, id_const_half, 0x3f000000u});
  b.op(spv::OpConstantComposite, {id_vec4, id_const_color, id_const_half, id_const_half, id_const_half, id_const_half});
  b.op(spv::OpFunction, {id_void, id_main, spv::FunctionControlMaskNone, id_fnvoid});
  b.op(spv::OpLabel, {id_label});
  b.op(spv::OpStore, {id_color, id_const_color});
  b.op(spv::OpReturn, {});
  b.op(spv::OpFunctionEnd, {});

  InstructionStream stream = b.build(id_bound);
  REQUIRE(stream.validate());

  SECTION("greater-than mode kills fragments conditionally") {
    SpirVTransformer transformer(stream);
    SpirVInjectAlphaTestPass pass(SpirVInjectAlphaTestPass::M_greater, 4);
    transformer.run(pass);
    CHECK(transformer.validate_db());

    InstructionStream result = transformer.get_result();
    CHECK(result.validate());

    // alpha > ref is tested by killing when alpha <= ref.
    CHECK(count_op(result, spv::OpFOrdLessThanEqual) == 1);
    CHECK(count_op(result, spv::OpKill) == 1);
    CHECK(count_op(result, spv::OpReturn) == 1);

    // A uniform was added to hold the reference alpha value.
    REQUIRE(pass._alpha_ref_var_id != 0);
    const SpirVResultDatabase::Definition &ref_def =
      transformer.get_db().get_definition(pass._alpha_ref_var_id);
    CHECK(ref_def._type == ShaderType::FLOAT);
    CHECK(ref_def._storage_class == spv::StorageClassUniformConstant);
    CHECK(ref_def._location == 4);
  }

  SECTION("never mode replaces the return with a kill") {
    SpirVTransformer transformer(stream);
    transformer.run(SpirVInjectAlphaTestPass(SpirVInjectAlphaTestPass::M_never));
    CHECK(transformer.validate_db());

    InstructionStream result = transformer.get_result();
    CHECK(result.validate());
    CHECK(count_op(result, spv::OpKill) == 1);
    CHECK(count_op(result, spv::OpReturn) == 0);
  }
}
