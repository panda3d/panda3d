/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_transformer.cxx
 * @author rdb
 * @date 2026-07-14
 */

#include "spirVTransformer.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVTransformer::assign_procedural_block_names", "[shaderpipeline]") {
  // Modelled after a geometry shader: an arrayed input block, an output
  // block, a gl_PerVertex-like built-in block and a loose varying.
  SpirVModule module = make_module();
  module.add_capability(spv::CapabilityGeometry);

  const ShaderType *vec4_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  // Use different member layouts so each block gets a distinct struct type.
  ShaderType::Struct in_struct;
  in_struct.add_member(ShaderType::FLOAT, "a");
  in_struct.add_member(vec4_type, "b");
  const ShaderType *in_block_type =
    ShaderType::register_type(std::move(in_struct));
  const ShaderType *in_array_type =
    ShaderType::register_type(ShaderType::Array(in_block_type, 1));

  ShaderType::Struct out_struct;
  out_struct.add_member(vec4_type, "b");
  out_struct.add_member(ShaderType::FLOAT, "a");
  const ShaderType *out_block_type =
    ShaderType::register_type(std::move(out_struct));

  ShaderType::Struct builtin_struct;
  builtin_struct.add_member(vec4_type, "gl_Position");
  const ShaderType *builtin_block_type =
    ShaderType::register_type(std::move(builtin_struct));

  ShaderType::Struct named_struct;
  named_struct.add_member(ShaderType::FLOAT, "a");
  const ShaderType *named_block_type =
    ShaderType::register_type(std::move(named_struct));

  Id in_var = module.define_variable(in_array_type, spv::StorageClassInput);
  Id in_type = module.get_type_id(
    module.unwrap_pointer_type(module.get_type_id(in_var)));
  module.decorate(in_type, spv::DecorationBlock);
  module.set_location(in_var, 0);

  Id out_var = module.define_variable(out_block_type, spv::StorageClassOutput);
  Id out_type = module.unwrap_pointer_type(module.get_type_id(out_var));
  module.decorate(out_type, spv::DecorationBlock);
  module.set_location(out_var, 1);

  Id builtin_var =
    module.define_variable(builtin_block_type, spv::StorageClassOutput);
  Id builtin_type = module.unwrap_pointer_type(module.get_type_id(builtin_var));
  module.decorate(builtin_type, spv::DecorationBlock);
  module.decorate_member(builtin_type, 0, spv::DecorationBuiltIn,
                         spv::BuiltInPosition);

  Id named_var =
    module.define_variable(named_block_type, spv::StorageClassInput);
  Id named_type = module.unwrap_pointer_type(module.get_type_id(named_var));
  module.decorate(named_type, spv::DecorationBlock);
  module.set_location(named_var, 5);
  module.set_name(named_type, "vData");

  Id loose_var = module.define_variable(vec4_type, spv::StorageClassOutput);
  Id loose_type = module.unwrap_pointer_type(module.get_type_id(loose_var));
  module.set_location(loose_var, 3);

  {
    SpirVBuilder builder = make_entry_point(module,
      spv::ExecutionModelGeometry,
      {in_var, out_var, builtin_var, named_var, loose_var});
    Id function_id = builder.get_current_function_id();
    module.add_execution_mode(function_id, spv::ExecutionModeInputPoints);
    module.add_execution_mode(function_id, spv::ExecutionModeInvocations, {1});
    module.add_execution_mode(function_id, spv::ExecutionModeOutputPoints);
    module.add_execution_mode(function_id, spv::ExecutionModeOutputVertices, {1});
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  transformer.assign_procedural_block_names("b");
  CHECK(transformer.get_module().validate());

  const SpirVModule &result = transformer.get_module();

  // The unnamed blocks are named after their locations.
  CHECK(result.get_name(in_type) == "b0");
  CHECK(result.get_name(out_type) == "b1");

  // The built-in block has no location and remains unnamed.
  CHECK(result.get_name(builtin_type).empty());

  // The block that still had a name keeps it.
  CHECK(result.get_name(named_type) == "vData");

  // The loose varying is not a block, so its type is not named.
  CHECK(result.get_name(loose_type).empty());
}
