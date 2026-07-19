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

TEST_CASE("SpirVTransformer::assign_procedural_names", "[shaderpipeline]") {
  // Modelled after a geometry shader: an arrayed input block, an output
  // block, a gl_PerVertex-like built-in block, a loose varying and a uniform.
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

  // The output block gets unnamed members, to test member name assignment.
  // The last member is a nested struct, which is named after its signature.
  ShaderType::Struct out_nested_struct;
  out_nested_struct.add_member(vec4_type, "x");
  const ShaderType *out_nested_type =
    ShaderType::register_type(std::move(out_nested_struct));

  ShaderType::Struct out_struct;
  out_struct.add_member(vec4_type, "");
  out_struct.add_member(ShaderType::FLOAT, "");
  out_struct.add_member(out_nested_type, "");
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
  Id out_nested_type_id = module.get_member_type_id(out_type, 2);

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

  // A loose (non-block) varying of struct type; its type is also named.
  const ShaderType *vec2_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 2));
  ShaderType::Struct vary_struct;
  vary_struct.add_member(vec2_type, "");
  const ShaderType *vary_struct_type =
    ShaderType::register_type(std::move(vary_struct));

  Id vary_var =
    module.define_variable(vary_struct_type, spv::StorageClassOutput);
  Id vary_type_id = module.unwrap_pointer_type(module.get_type_id(vary_var));
  module.set_location(vary_var, 4);

  Id uniform_var =
    module.define_variable(vec4_type, spv::StorageClassUniformConstant);

  // An array-of-struct uniform, to test uniform struct type naming.  One
  // member is itself a struct, and one member already has a name.
  ShaderType::Struct nested_struct;
  nested_struct.add_member(ShaderType::FLOAT, "");
  const ShaderType *nested_struct_type =
    ShaderType::register_type(std::move(nested_struct));

  ShaderType::Struct uniform_struct;
  uniform_struct.add_member(vec4_type, "");
  uniform_struct.add_member(nested_struct_type, "");
  uniform_struct.add_member(ShaderType::FLOAT, "shininess");
  const ShaderType *uniform_struct_type =
    ShaderType::register_type(std::move(uniform_struct));
  const ShaderType *uniform_array_type =
    ShaderType::register_type(ShaderType::Array(uniform_struct_type, 2));

  Id struct_uniform_var = module.define_variable(
    uniform_array_type, spv::StorageClassUniformConstant);
  Id struct_uniform_type = module.get_type_id(
    module.unwrap_pointer_type(module.get_type_id(struct_uniform_var)));
  Id nested_struct_type_id =
    module.get_member_type_id(struct_uniform_type, 1);

  {
    SpirVBuilder builder = make_entry_point(module,
      spv::ExecutionModelGeometry);
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
  pmap<uint32_t, int> uniform_locations;
  uniform_locations[uniform_var] = 7;
  uniform_locations[struct_uniform_var] = 10;
  transformer.assign_procedural_names(uniform_locations, 1);
  CHECK(transformer.get_module().validate());

  const SpirVModule &result = transformer.get_module();

  // The unnamed blocks are named after their locations.
  CHECK(result.get_name(in_type) == "b0");
  CHECK(result.get_name(out_type) == "b1");

  // The built-in block has no location and remains unnamed.
  CHECK(result.get_name(builtin_type).empty());
  CHECK(result.get_name(builtin_var).empty());

  // The block that still had a name keeps it.
  CHECK(result.get_name(named_type) == "vData");

  // The loose varying is not a block, so its type is not named.
  CHECK(result.get_name(loose_type).empty());

  // Interface variables are named after the stage index and location, with
  // outputs getting the index of the next stage.
  CHECK(result.get_name(in_var) == "i1_0");
  CHECK(result.get_name(out_var) == "i2_1");
  CHECK(result.get_name(loose_var) == "i2_3");
  CHECK(result.get_name(named_var) == "i1_5");

  // Unnamed block members get procedural names, named ones keep theirs.
  CHECK(result.get_member_name(out_type, 0) == "m0");
  CHECK(result.get_member_name(out_type, 1) == "m1");
  CHECK(result.get_member_name(out_type, 2) == "m2");
  CHECK(result.get_member_name(in_type, 0) == "a");
  CHECK(result.get_member_name(in_type, 1) == "b");

  // A struct nested in a block member is named after its type signature,
  // with its member names overwritten.
  CHECK(result.get_name(out_nested_type_id) == "Sf4_");
  CHECK(result.get_member_name(out_nested_type_id, 0) == "m0");

  // A loose struct varying's type is named after its signature as well.
  CHECK(result.get_name(vary_var) == "i2_4");
  CHECK(result.get_name(vary_type_id) == "Sf2_");
  CHECK(result.get_member_name(vary_type_id, 0) == "m0");

  // The uniform is named after its assigned location.
  CHECK(result.get_name(uniform_var) == "p7");

  // The struct uniform's type is named after its type signature (matching
  // the naming in the SPIRV-Cross fallback path), including nested structs.
  // Member names are not part of the signature, so they are overwritten
  // even if already named.
  CHECK(result.get_name(struct_uniform_var) == "p10");
  CHECK(result.get_name(struct_uniform_type) == "Sf4Sf_f_");
  CHECK(result.get_member_name(struct_uniform_type, 0) == "m0");
  CHECK(result.get_member_name(struct_uniform_type, 1) == "m1");
  CHECK(result.get_member_name(struct_uniform_type, 2) == "m2");
  CHECK(result.get_name(nested_struct_type_id) == "Sf_");
  CHECK(result.get_member_name(nested_struct_type_id, 0) == "m0");
}

TEST_CASE("SpirVTransformer::assign_interface_locations", "[shaderpipeline]") {
  const ShaderType *vec4_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  SECTION("geometry per-vertex input arrays occupy element locations") {
    SpirVModule module = make_module();
    module.add_capability(spv::CapabilityGeometry);

    ShaderType::Struct block_struct;
    block_struct.add_member(ShaderType::FLOAT, "a");
    block_struct.add_member(vec4_type, "b");
    const ShaderType *block_type =
      ShaderType::register_type(std::move(block_struct));
    const ShaderType *block_array_type =
      ShaderType::register_type(ShaderType::Array(block_type, 3));
    const ShaderType *vec4_array_type =
      ShaderType::register_type(ShaderType::Array(vec4_type, 3));

    Id block_var =
      module.define_variable(block_array_type, spv::StorageClassInput);
    Id block_type_id = module.get_type_id(
      module.unwrap_pointer_type(module.get_type_id(block_var)));
    module.decorate(block_type_id, spv::DecorationBlock);

    Id loose_var =
      module.define_variable(vec4_array_type, spv::StorageClassInput);

    Id out_var = module.define_variable(vec4_type, spv::StorageClassOutput);

    {
      SpirVBuilder builder = make_entry_point(module,
        spv::ExecutionModelGeometry);
      Id function_id = builder.get_current_function_id();
      module.add_execution_mode(function_id, spv::ExecutionModeTriangles);
      module.add_execution_mode(function_id, spv::ExecutionModeInvocations, {1});
      module.add_execution_mode(function_id, spv::ExecutionModeOutputPoints);
      module.add_execution_mode(function_id, spv::ExecutionModeOutputVertices, {1});
      builder.op_return();
    }

    InstructionStream stream = module.emit();
    REQUIRE(stream.validate());

    SpirVTransformer transformer(stream);
    transformer.assign_interface_locations(ShaderModule::Stage::GEOMETRY);
    CHECK(transformer.get_module().validate());

    const SpirVModule &result = transformer.get_module();

    // The outer per-vertex array dimension is ignored, so the block takes up
    // only two locations, not six.
    CHECK(result.get_location(block_var) == 0);
    CHECK(result.get_location(loose_var) == 2);
    CHECK(result.get_location(out_var) == 0);
  }

  SECTION("tess control outputs are per-vertex unless patch") {
    SpirVModule module = make_module();
    module.add_capability(spv::CapabilityTessellation);

    const ShaderType *in_array_type =
      ShaderType::register_type(ShaderType::Array(vec4_type, 32));
    const ShaderType *out_array_type =
      ShaderType::register_type(ShaderType::Array(vec4_type, 4));
    const ShaderType *patch_array_type =
      ShaderType::register_type(ShaderType::Array(vec4_type, 2));

    Id in_var = module.define_variable(in_array_type, spv::StorageClassInput);
    Id out_var = module.define_variable(out_array_type, spv::StorageClassOutput);

    // An explicitly arrayed per-patch output, which is not per-vertex, so its
    // array dimension does count.
    Id patch_var =
      module.define_variable(patch_array_type, spv::StorageClassOutput);
    module.decorate(patch_var, spv::DecorationPatch);

    Id out_var2 = module.define_variable(out_array_type, spv::StorageClassOutput);

    {
      SpirVBuilder builder = make_entry_point(module,
        spv::ExecutionModelTessellationControl);
      Id function_id = builder.get_current_function_id();
      module.add_execution_mode(function_id, spv::ExecutionModeOutputVertices, {4});
      builder.op_return();
    }

    InstructionStream stream = module.emit();
    REQUIRE(stream.validate());

    SpirVTransformer transformer(stream);
    transformer.assign_interface_locations(ShaderModule::Stage::TESS_CONTROL);
    CHECK(transformer.get_module().validate());

    const SpirVModule &result = transformer.get_module();

    // The per-vertex input array counts as a single location, despite being
    // declared with gl_MaxPatchVertices entries.
    CHECK(result.get_location(in_var) == 0);

    // The first per-vertex output takes one location, the patch output takes
    // two (its array dimension is real), so the last output lands on 3.
    CHECK(result.get_location(out_var) == 0);
    CHECK(result.get_location(patch_var) == 1);
    CHECK(result.get_location(out_var2) == 3);
  }
}
