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
  // uniform float a; uniform vec4 b;  main() loads both.
  SpirVModule module = make_module();
  const ShaderType *vec4_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  Id id_var_a = module.define_variable(ShaderType::FLOAT, spv::StorageClassUniformConstant);
  module.set_name(id_var_a, "a");
  Id id_var_b = module.define_variable(vec4_type, spv::StorageClassUniformConstant);
  module.set_name(id_var_b, "b");

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    builder.op_load(id_var_a);
    builder.op_load(id_var_b);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  ShaderType::Struct struct_type;
  struct_type.add_member(ShaderType::FLOAT, "a");
  struct_type.add_member(vec4_type, "b");
  const ShaderType::Struct *block_type =
    ShaderType::register_type(std::move(struct_type));

  SpirVTransformer transformer(stream);
  SpirVMakeBlockPass pass(block_type, {id_var_a, id_var_b}, spv::StorageClassUniform, 1, 2);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // The loose variables are gone, replaced by a single block variable.
  CHECK(!has_variable(result, id_var_a));
  CHECK(!has_variable(result, id_var_b));
  REQUIRE(pass._block_var_id != 0);
  CHECK(has_variable(result, pass._block_var_id));

  const SpirVModule &out_module = transformer.get_module();
  CHECK(out_module.resolve_type(pass._block_var_id) == block_type);
  CHECK(out_module.get_storage_class(pass._block_var_id) == spv::StorageClassUniform);

  // Each load now goes through an access chain into the block.
  CHECK(count_op(result, spv::OpAccessChain) == 2);
}
