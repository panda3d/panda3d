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
  // out vec4 p3d_FragColor;  main() { p3d_FragColor = vec4(0); }
  SpirVModule module = make_module();
  const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  Id color = module.define_variable(vec4_type, spv::StorageClassOutput);
  module.decorate(color, spv::DecorationLocation, 0u);

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    builder.op_store(color, module.define_null_constant(vec4_type));
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SECTION("greater-than mode kills fragments conditionally") {
    SpirVTransformer transformer(stream);
    SpirVInjectAlphaTestPass pass(SpirVInjectAlphaTestPass::M_greater, 4);
    transformer.run(pass);
    CHECK(transformer.get_module().validate());

    InstructionStream result = transformer.get_result();

    // alpha > ref is tested by killing when alpha <= ref.
    CHECK(count_op(result, spv::OpFOrdLessThanEqual) == 1);
    CHECK(count_op(result, spv::OpKill) == 1);
    CHECK(count_op(result, spv::OpReturn) == 1);

    // A uniform was added to hold the reference alpha value.
    REQUIRE(pass._alpha_ref_var_id != 0);
    const SpirVModule &out_module = transformer.get_module();
    CHECK(out_module.resolve_type(pass._alpha_ref_var_id) == ShaderType::FLOAT);
    CHECK(out_module.get_storage_class(pass._alpha_ref_var_id) == spv::StorageClassUniformConstant);
    CHECK(out_module.get_location(pass._alpha_ref_var_id) == 4);

    // The comparison is locatable by its recorded result id.
    REQUIRE(pass._compare_op_ids.size() == 1);
    Instruction cmp = find_op(result, spv::OpFOrdLessThanEqual);
    REQUIRE(cmp.opcode == spv::OpFOrdLessThanEqual);
    CHECK(cmp.args[1] == pass._compare_op_ids[0]);
  }

  SECTION("never mode replaces the return with a kill") {
    SpirVTransformer transformer(stream);
    transformer.run(SpirVInjectAlphaTestPass(SpirVInjectAlphaTestPass::M_never));
    CHECK(transformer.get_module().validate());

    InstructionStream result = transformer.get_result();
    CHECK(count_op(result, spv::OpKill) == 1);
    CHECK(count_op(result, spv::OpReturn) == 0);
  }
}

TEST_CASE("SpirVInjectAlphaTestPass ignores non-outputs at location 0", "[shaderpipeline]") {
  // in vec2 texcoord;  out vec4 p3d_FragColor;  main() { p3d_FragColor = vec4(0); }
  // The input also has location 0 and is listed before the output in the
  // entry point interface; the alpha test must be done on the output.
  SpirVModule module = make_module();
  const ShaderType *vec2_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 2));
  const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  Id texcoord = module.define_variable(vec2_type, spv::StorageClassInput);
  module.decorate(texcoord, spv::DecorationLocation, 0u);

  Id color = module.define_variable(vec4_type, spv::StorageClassOutput);
  module.decorate(color, spv::DecorationLocation, 0u);

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    builder.op_store(color, module.define_null_constant(vec4_type));
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVInjectAlphaTestPass pass(SpirVInjectAlphaTestPass::M_greater, 4);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();
  CHECK(count_op(result, spv::OpFOrdLessThanEqual) == 1);
  CHECK(count_op(result, spv::OpKill) == 1);

  // The alpha is fetched from the output variable, not the input.
  Instruction chain = find_op(result, spv::OpAccessChain);
  REQUIRE(chain.opcode == spv::OpAccessChain);
  CHECK(chain.args[2] == (uint32_t)color);
}

TEST_CASE("SpirVInjectAlphaTestPass tests alpha of 1 for vec3 output", "[shaderpipeline]") {
  // out vec3 p3d_FragColor;  main() { p3d_FragColor = vec3(0); }
  // An RGB output has an implicit alpha of 1, which must be tested against
  // the reference value rather than reading a nonexistent fourth component.
  SpirVModule module = make_module();
  const ShaderType *vec3_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 3));

  Id color = module.define_variable(vec3_type, spv::StorageClassOutput);
  module.decorate(color, spv::DecorationLocation, 0u);

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    builder.op_store(color, module.define_null_constant(vec3_type));
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVInjectAlphaTestPass pass(SpirVInjectAlphaTestPass::M_greater, 4);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();
  CHECK(count_op(result, spv::OpFOrdLessThanEqual) == 1);
  CHECK(count_op(result, spv::OpKill) == 1);
  CHECK(count_op(result, spv::OpAccessChain) == 0);

  // The first comparison operand is the constant 1.0f.
  Instruction cmp = find_op(result, spv::OpFOrdLessThanEqual);
  REQUIRE(cmp.opcode == spv::OpFOrdLessThanEqual);
  bool found_constant = false;
  for (Instruction op : result) {
    if (op.opcode == spv::OpConstant && op.args[1] == cmp.args[2]) {
      CHECK(op.args[2] == 0x3f800000u);
      found_constant = true;
    }
  }
  CHECK(found_constant);
}

TEST_CASE("SpirVInjectAlphaTestPass never mode without color output", "[shaderpipeline]") {
  // A depth-only fragment shader without any color output must still be
  // killed in never mode.
  SpirVModule module = make_module();

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  transformer.run(SpirVInjectAlphaTestPass(SpirVInjectAlphaTestPass::M_never));
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();
  CHECK(count_op(result, spv::OpKill) == 1);
  CHECK(count_op(result, spv::OpReturn) == 0);
}

TEST_CASE("SpirVInjectAlphaTestPass tests before every return", "[shaderpipeline]") {
  // main() { p3d_FragColor = vec4(0); if (true) return; return; }
  // Both returns must receive their own alpha test.
  SpirVModule module = make_module();
  const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  Id color = module.define_variable(vec4_type, spv::StorageClassOutput);
  module.decorate(color, spv::DecorationLocation, 0u);

  Id main_id;
  {
    SpirVBuilder builder = module.make_function(nullptr);
    builder.op_store(color, module.define_null_constant(vec4_type));
    Id merge = builder.branch_if(module.define_constant(ShaderType::BOOL, 1));
    builder.op_return();
    builder.branch_endif(merge);
    builder.op_return();
    main_id = builder.get_current_function_id();
  }
  module.add_entry_point(spv::ExecutionModelFragment, main_id, "main");
  module.add_execution_mode(main_id, spv::ExecutionModeOriginUpperLeft);

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVInjectAlphaTestPass pass(SpirVInjectAlphaTestPass::M_less, 4);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // alpha < ref is tested by killing when alpha >= ref, once per return.
  CHECK(count_op(result, spv::OpFOrdGreaterThanEqual) == 2);
  CHECK(count_op(result, spv::OpKill) == 2);
  CHECK(count_op(result, spv::OpReturn) == 2);

  // Both comparisons were recorded, in stream order, and each id identifies
  // one of the comparison instructions in the result.
  REQUIRE(pass._compare_op_ids.size() == 2);
  for (int i = 0; i < 2; ++i) {
    Instruction cmp = find_op(result, spv::OpFOrdGreaterThanEqual, i);
    REQUIRE(cmp.opcode == spv::OpFOrdGreaterThanEqual);
    CHECK(cmp.args[1] == pass._compare_op_ids[i]);
  }
}
