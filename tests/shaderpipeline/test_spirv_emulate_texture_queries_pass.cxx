/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_emulate_texture_queries_pass.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "spirVEmulateTextureQueriesPass.h"
#include "spirVRemoveUnusedVariablesPass.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

static const ShaderType *
cube_sampler_type(bool shadow) {
  return ShaderType::register_type(ShaderType::SampledImage(
    Texture::TT_cube_map, ShaderType::ST_float, shadow));
}

TEST_CASE("SpirVEmulateTextureQueriesPass replaces textureSize", "[shaderpipeline]") {
  // uniform sampler2D tex;  main() calls textureSize(tex, 0).
  SpirVModule module = make_module();
  module.add_capability(spv::CapabilityImageQuery);

  const ShaderType *sampler_type = ShaderType::register_type(ShaderType::SampledImage(
    Texture::TT_2d_texture, ShaderType::ST_float, false));
  const ShaderType *ivec2_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_int, 2));

  Id id_tex = module.define_variable(sampler_type, spv::StorageClassUniformConstant);
  module.set_name(id_tex, "tex");

  Id id_sampled_image = module.unwrap_pointer_type(module.get_type_id(id_tex));
  Id id_image = module.get_type_id(id_sampled_image);
  Id id_ivec2 = module.define_type(ivec2_type);
  Id id_const_0 = module.define_int_constant(0);

  {
    SpirVBuilder b = make_entry_point(module);
    Id load = b.op_load(id_tex);
    Id img = b.allocate_id();
    b.insert(spv::OpImage, {id_image, img, load});
    b.insert(spv::OpImageQuerySizeLod, {id_ivec2, b.allocate_id(), img, id_const_0});
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVEmulateTextureQueriesPass pass(Shader::C_texture_query_size);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // The query is replaced with a read from a new vec4 uniform that holds the
  // texture size, converted to the int vector the query returned.
  CHECK(count_op(result, spv::OpImageQuerySizeLod) == 0);
  CHECK(count_op(result, spv::OpConvertFToS) == 1);

  REQUIRE(pass._size_var_ids.size() == 1);
  const auto &item = *pass._size_var_ids.begin();
  CHECK(item.first._var_id == id_tex);
  CHECK(item.first.size() == 0);

  CHECK(transformer.get_module().resolve_type(item.second) ==
        ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4)));
  CHECK(transformer.get_module().get_storage_class(item.second) ==
        spv::StorageClassUniformConstant);
}

TEST_CASE("SpirVEmulateTextureQueriesPass emulates samplerCubeShadow", "[shaderpipeline]") {
  // uniform samplerCubeShadow tex;  main() does one Dref sample from it.
  SpirVModule module = make_module();
  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *sampler_cube = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float));
  const ShaderType *sampler_cube_shadow = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float, true));

  Id id_tex = module.define_variable(sampler_cube_shadow, spv::StorageClassUniformConstant);
  module.set_name(id_tex, "tex");

  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_coord = module.define_null_constant(vec3_type);
  Id id_dref = module.define_float_constant(0.5f);

  {
    SpirVBuilder b = make_entry_point(module);
    Id load = b.op_load(id_tex);
    b.insert(spv::OpImageSampleDrefImplicitLod,
             {id_float, b.allocate_id(), load, id_coord, id_dref});
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVEmulateTextureQueriesPass pass(Shader::C_sampler_cube_shadow);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // The Dref sample is replaced with a regular sample followed by a manual
  // comparison against the reference value.
  CHECK(count_op(result, spv::OpImageSampleDrefImplicitLod) == 0);
  CHECK(count_op(result, spv::OpImageSampleImplicitLod) == 1);
  CHECK(count_op(result, spv::OpFOrdGreaterThan) == 1);
  CHECK(count_op(result, spv::OpSelect) == 1);

  // The image type has lost its depth flag, and there is only one of it.
  REQUIRE(count_op(result, spv::OpTypeImage) == 1);
  Instruction image_op = find_op(result, spv::OpTypeImage);
  CHECK(image_op.args[2] == (uint32_t)spv::DimCube);
  CHECK(image_op.args[3] == 0);

  // The sampler variable is still there, and it's now considered a non-shadow
  // sampler type.
  CHECK(has_variable(result, id_tex));
  CHECK(transformer.get_module().resolve_type(id_tex) == sampler_cube);
}

TEST_CASE("SpirVEmulateTextureQueriesPass leaves non-cube shadow samples alone", "[shaderpipeline]") {
  SpirVModule module = make_module();
  const ShaderType *sampler_2d_shadow = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float, true));
  const ShaderType *sampler_cube = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float));
  const ShaderType *sampler_cube_shadow = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float, true));
  const ShaderType *vec2_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 2));
  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));

  Id id_tex_2d = module.define_variable(
    sampler_2d_shadow, spv::StorageClassUniformConstant);
  Id id_tex_cube = module.define_variable(
    sampler_cube_shadow, spv::StorageClassUniformConstant);
  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_coord_2d = module.define_null_constant(vec2_type);
  Id id_coord_cube = module.define_null_constant(vec3_type);
  Id id_dref = module.define_float_constant(0.5f);

  {
    SpirVBuilder b = make_entry_point(module);
    Id load_2d = b.op_load(id_tex_2d);
    b.insert(spv::OpImageSampleDrefImplicitLod,
             {id_float, b.allocate_id(), load_2d, id_coord_2d, id_dref});
    Id load_cube = b.op_load(id_tex_cube);
    b.insert(spv::OpImageSampleDrefImplicitLod,
             {id_float, b.allocate_id(), load_cube, id_coord_cube, id_dref});
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  transformer.run(SpirVEmulateTextureQueriesPass(Shader::C_sampler_cube_shadow));
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();
  CHECK(count_op(result, spv::OpImageSampleDrefImplicitLod) == 1);
  CHECK(count_op(result, spv::OpImageSampleImplicitLod) == 1);
  CHECK(count_op(result, spv::OpFOrdGreaterThan) == 1);
  CHECK(count_op(result, spv::OpSelect) == 1);

  CHECK(transformer.get_module().resolve_type(id_tex_2d) ==
        sampler_2d_shadow);
  CHECK(transformer.get_module().resolve_type(id_tex_cube) ==
        sampler_cube);
}

TEST_CASE("SpirVEmulateTextureQueriesPass preserves gradient operands", "[shaderpipeline]") {
  SpirVModule module = make_module();
  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *sampler_cube_shadow = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float, true));

  Id id_tex = module.define_variable(
    sampler_cube_shadow, spv::StorageClassUniformConstant);
  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_coord = module.define_null_constant(vec3_type);
  Id id_dref = module.define_float_constant(0.5f);
  Id id_dx = module.define_null_constant(vec3_type);
  Id id_dy = module.define_null_constant(vec3_type);

  {
    SpirVBuilder b = make_entry_point(module);
    Id load = b.op_load(id_tex);
    b.insert(spv::OpImageSampleDrefExplicitLod,
             {id_float, b.allocate_id(), load, id_coord, id_dref,
              (uint32_t)spv::ImageOperandsGradMask, id_dx, id_dy});
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  transformer.run(SpirVEmulateTextureQueriesPass(Shader::C_sampler_cube_shadow));
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();
  CHECK(count_op(result, spv::OpImageSampleDrefExplicitLod) == 0);
  CHECK(count_op(result, spv::OpImageSampleImplicitLod) == 0);
  REQUIRE(count_op(result, spv::OpImageSampleExplicitLod) == 1);

  Instruction sample = find_op(result, spv::OpImageSampleExplicitLod);
  REQUIRE(sample.nargs == 7);
  CHECK(sample.args[4] == (uint32_t)spv::ImageOperandsGradMask);
  CHECK(sample.args[5] == id_dx);
  CHECK(sample.args[6] == id_dy);
}

TEST_CASE("SpirVEmulateTextureQueriesPass dedupes cube types (shadow last)", "[shaderpipeline]") {
  // uniform samplerCube tex0;  uniform samplerCubeShadow tex1;
  // main() samples both.  After the pass replaces the depth cube image type
  // with a non-depth one, it is identical to the type of tex0, so the types
  // must be deduplicated all the way up through the pointer type, since
  // SPIR-V does not permit duplicate non-aggregate type declarations.
  SpirVModule module = make_module();
  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *sampler_cube = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float));
  const ShaderType *sampler_cube_shadow = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float, true));

  Id id_tex0 = module.define_variable(sampler_cube, spv::StorageClassUniformConstant);
  module.set_name(id_tex0, "tex0");
  Id id_tex1 = module.define_variable(sampler_cube_shadow, spv::StorageClassUniformConstant);
  module.set_name(id_tex1, "tex1");

  Id id_sampled_image1 = module.unwrap_pointer_type(module.get_type_id(id_tex1));
  module.set_name(id_sampled_image1, "samplerCubeShadow");

  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_coord = module.define_null_constant(vec3_type);
  Id id_dref = module.define_float_constant(0.5f);

  {
    SpirVBuilder b = make_entry_point(module);
    Id load0 = b.op_load(id_tex0);
    b.op_image_sample(load0, id_coord);
    Id load1 = b.op_load(id_tex1);
    b.insert(spv::OpImageSampleDrefImplicitLod,
             {id_float, b.allocate_id(), load1, id_coord, id_dref});
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVEmulateTextureQueriesPass pass(Shader::C_sampler_cube_shadow);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // Only one cube image type remains, without depth flag, and the sampled
  // image and pointer types wrapping it have been deduplicated as well.
  REQUIRE(count_op(result, spv::OpTypeImage) == 1);
  REQUIRE(count_op(result, spv::OpTypeSampledImage) == 1);
  REQUIRE(count_op(result, spv::OpTypePointer) == 1);
  CHECK(find_op(result, spv::OpTypeImage).args[3] == 0);

  // Both variables remain, now sharing the same pointer type, and both loads
  // now produce the same sampled image type.
  uint32_t si_id = find_op(result, spv::OpTypeSampledImage).args[0];
  uint32_t ptr_id = find_op(result, spv::OpTypePointer).args[0];
  CHECK(has_variable(result, id_tex0));
  CHECK(has_variable(result, id_tex1));
  for (Instruction op : result) {
    if (op.opcode == spv::OpVariable) {
      CHECK(op.args[0] == ptr_id);
    }
    if (op.opcode == spv::OpLoad) {
      CHECK(op.args[0] == si_id);
    }
  }

  // The Dref sample is rewritten; the regular sample is left alone.
  CHECK(count_op(result, spv::OpImageSampleDrefImplicitLod) == 0);
  CHECK(count_op(result, spv::OpImageSampleImplicitLod) == 2);
  CHECK(count_op(result, spv::OpSelect) == 1);

  // The name of the replaced sampled image type is removed along with it,
  // but the variable names remain.
  CHECK(count_op(result, spv::OpName) == 2);

  // Both variables now resolve to the same non-shadow sampler type.
  const ShaderType *expected_type = sampler_cube;
  CHECK(transformer.get_module().resolve_type(id_tex0) == expected_type);
  CHECK(transformer.get_module().resolve_type(id_tex1) == expected_type);
}

TEST_CASE("SpirVEmulateTextureQueriesPass dedupes cube types (shadow first)", "[shaderpipeline]") {
  // Same as above, but with the shadow sampler types declared first, so that
  // the replacement image type has to be reused from a declaration that
  // occurs later in the module.
  SpirVModule module = make_module();
  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *sampler_cube = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float));
  const ShaderType *sampler_cube_shadow = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float, true));

  Id id_tex1 = module.define_variable(sampler_cube_shadow, spv::StorageClassUniformConstant);
  module.set_name(id_tex1, "tex1");
  Id id_tex0 = module.define_variable(sampler_cube, spv::StorageClassUniformConstant);
  module.set_name(id_tex0, "tex0");

  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_coord = module.define_null_constant(vec3_type);
  Id id_dref = module.define_float_constant(0.5f);

  {
    SpirVBuilder b = make_entry_point(module);
    Id load1 = b.op_load(id_tex1);
    b.insert(spv::OpImageSampleDrefImplicitLod,
             {id_float, b.allocate_id(), load1, id_coord, id_dref});
    Id load0 = b.op_load(id_tex0);
    b.op_image_sample(load0, id_coord);
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVEmulateTextureQueriesPass pass(Shader::C_sampler_cube_shadow);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  REQUIRE(count_op(result, spv::OpTypeImage) == 1);
  REQUIRE(count_op(result, spv::OpTypeSampledImage) == 1);
  REQUIRE(count_op(result, spv::OpTypePointer) == 1);
  CHECK(find_op(result, spv::OpTypeImage).args[3] == 0);

  uint32_t si_id = find_op(result, spv::OpTypeSampledImage).args[0];
  uint32_t ptr_id = find_op(result, spv::OpTypePointer).args[0];
  CHECK(has_variable(result, id_tex0));
  CHECK(has_variable(result, id_tex1));
  for (Instruction op : result) {
    if (op.opcode == spv::OpVariable) {
      CHECK(op.args[0] == ptr_id);
    }
    if (op.opcode == spv::OpLoad) {
      CHECK(op.args[0] == si_id);
    }
  }

  CHECK(count_op(result, spv::OpImageSampleDrefImplicitLod) == 0);
  CHECK(count_op(result, spv::OpImageSampleImplicitLod) == 2);
  CHECK(count_op(result, spv::OpSelect) == 1);
}

TEST_CASE("SpirVEmulateTextureQueriesPass dedupes function signatures", "[shaderpipeline]") {
  // A helper function takes the samplerCubeShadow as a parameter.  When the
  // pointer type is deduplicated onto the samplerCube pointer type, the
  // helper's function type becomes identical to another function type and
  // must be deduplicated as well, which in turn requires rewriting the
  // OpFunction header and the OpFunctionParameter.
  SpirVModule module = make_module();
  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *sampler_cube = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float));
  const ShaderType *sampler_cube_shadow = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float, true));

  Id id_tex0 = module.define_variable(sampler_cube, spv::StorageClassUniformConstant);
  module.set_name(id_tex0, "tex0");
  Id id_tex1 = module.define_variable(sampler_cube_shadow, spv::StorageClassUniformConstant);
  module.set_name(id_tex1, "tex1");

  Id id_ptr0 = module.get_type_id(id_tex0);
  Id id_ptr1 = module.get_type_id(id_tex1);

  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_coord = module.define_null_constant(vec3_type);
  Id id_dref = module.define_float_constant(0.5f);

  // The decoy float(samplerCube) type: no function uses it initially, but the
  // helper function's type must merge onto it after the pointer types collapse.
  Id id_fnf_a = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpTypeFunction, {id_fnf_a, id_float, id_ptr0}));

  // float f(samplerCubeShadow p) { return texture(p, ...); }
  Id id_func;
  {
    SpirVBuilder builder = module.make_function(ShaderType::FLOAT, {id_ptr1});
    id_func = builder.get_current_function_id();
    Id id_param = module.find_function(id_func)->get_parameters()[0];
    Id id_fload = builder.op_load(id_param);
    Id id_fresult = module.allocate_id();
    builder.insert(spv::OpImageSampleDrefImplicitLod,
                   {id_float, id_fresult, id_fload, id_coord, id_dref});
    builder.op_return_value(id_fresult);
  }

  // main() calls f(tex1).
  {
    SpirVBuilder b = make_entry_point(module);
    b.insert(spv::OpFunctionCall, {id_float, b.allocate_id(), id_func, id_tex1});
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVEmulateTextureQueriesPass pass(Shader::C_sampler_cube_shadow);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // The whole cube sampler type chain was deduplicated.
  REQUIRE(count_op(result, spv::OpTypeImage) == 1);
  REQUIRE(count_op(result, spv::OpTypeSampledImage) == 1);
  REQUIRE(count_op(result, spv::OpTypePointer) == 1);
  CHECK(find_op(result, spv::OpTypeImage).args[3] == 0);

  // The two float(sampler) function types collapsed into one, and the helper
  // function and its parameter now use the surviving types.
  CHECK(count_op(result, spv::OpTypeFunction) == 2);  // void(), float(sampler)
  uint32_t ptr_id = find_op(result, spv::OpTypePointer).args[0];
  Instruction func_op = find_op(result, spv::OpFunction, 0);
  REQUIRE(func_op.opcode == spv::OpFunction);
  CHECK(func_op.args[1] == id_func);
  CHECK(func_op.args[3] == id_fnf_a);
  Instruction param_op = find_op(result, spv::OpFunctionParameter);
  REQUIRE(param_op.opcode == spv::OpFunctionParameter);
  CHECK(param_op.args[0] == ptr_id);

  // The Dref sample in the helper is rewritten.
  CHECK(count_op(result, spv::OpImageSampleDrefImplicitLod) == 0);
  CHECK(count_op(result, spv::OpImageSampleImplicitLod) == 1);
  CHECK(count_op(result, spv::OpSelect) == 1);
}

TEST_CASE("SpirVEmulateTextureQueriesPass leaves unaffected modules untouched", "[shaderpipeline]") {
  // uniform samplerCube tex;  main() does a regular (non-Dref) sample.
  // With the cube shadow emulation enabled but nothing to emulate, the module
  // must come out byte-for-byte identical: the always-on driver machinery
  // must be invisible when there is nothing to do.
  SpirVModule module = make_module();
  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *sampler_cube = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float));

  Id id_tex = module.define_variable(sampler_cube, spv::StorageClassUniformConstant);
  module.set_name(id_tex, "tex");

  Id id_coord = module.define_null_constant(vec3_type);

  {
    SpirVBuilder b = make_entry_point(module);
    Id load = b.op_load(id_tex);
    b.op_image_sample(load, id_coord);
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVEmulateTextureQueriesPass pass(Shader::C_sampler_cube_shadow);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();
  CHECK(result._words == stream._words);
}

TEST_CASE("SpirVEmulateTextureQueriesPass dedupes unknown-depth cube types", "[shaderpipeline]") {
  // Like the "shadow last" test, but the Dref-sampled image declares depth=2
  // ("unknown depth", as emitted by DXC) rather than depth=1.  It must also
  // be flipped to a non-depth image and deduplicated onto the regular one.
  SpirVModule module = make_module();
  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *sampler_cube = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float));

  Id id_tex0 = module.define_variable(sampler_cube, spv::StorageClassUniformConstant);
  module.set_name(id_tex0, "tex0");

  // No define helper produces a sampled image with an unknown-depth image
  // type, so declare that chain by hand.
  const ShaderType *image_type = ShaderType::register_type(ShaderType::Image(
    Texture::TT_cube_map, ShaderType::ST_float, ShaderType::Access::NONE));
  Id id_image1 = module.define_image_type(image_type, 2, 1, spv::ImageFormatUnknown);
  Id id_sampled_image1 = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpTypeSampledImage,
                                             {id_sampled_image1, id_image1}));
  Id id_ptr1 = module.define_pointer_type(id_sampled_image1, spv::StorageClassUniformConstant);
  Id id_tex1 = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpVariable,
    {id_ptr1, id_tex1, (uint32_t)spv::StorageClassUniformConstant}));
  module.set_name(id_tex1, "tex1");

  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_coord = module.define_null_constant(vec3_type);
  Id id_dref = module.define_float_constant(0.5f);

  {
    SpirVBuilder b = make_entry_point(module);
    Id load0 = b.op_load(id_tex0);
    b.op_image_sample(load0, id_coord);
    Id load1 = b.op_load(id_tex1);
    b.insert(spv::OpImageSampleDrefImplicitLod,
             {id_float, b.allocate_id(), load1, id_coord, id_dref});
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVEmulateTextureQueriesPass pass(Shader::C_sampler_cube_shadow);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  REQUIRE(count_op(result, spv::OpTypeImage) == 1);
  REQUIRE(count_op(result, spv::OpTypeSampledImage) == 1);
  REQUIRE(count_op(result, spv::OpTypePointer) == 1);
  CHECK(find_op(result, spv::OpTypeImage).args[3] == 0);

  CHECK(count_op(result, spv::OpImageSampleDrefImplicitLod) == 0);
  CHECK(count_op(result, spv::OpImageSampleImplicitLod) == 2);
  CHECK(count_op(result, spv::OpSelect) == 1);
}

TEST_CASE("SpirVEmulateTextureQueriesPass dedupes cube sampler arrays", "[shaderpipeline]") {
  // uniform samplerCube tex0[2];  uniform samplerCubeShadow tex1[2];
  // The element types must be deduplicated, and so must the pointer types
  // produced by the access chains.  The two array types become identical, but
  // both must survive: SPIR-V explicitly permits duplicate aggregate type
  // declarations, and over-deduplicating them would be wrong.
  SpirVModule module = make_module();
  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *sampler_cube = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float));
  const ShaderType *sampler_cube_shadow = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float, true));
  const ShaderType *array0_type = ShaderType::register_type(
    ShaderType::Array(sampler_cube, 2));
  const ShaderType *array1_type = ShaderType::register_type(
    ShaderType::Array(sampler_cube_shadow, 2));

  Id id_tex0 = module.define_variable(array0_type, spv::StorageClassUniformConstant);
  module.set_name(id_tex0, "tex0");
  Id id_tex1 = module.define_variable(array1_type, spv::StorageClassUniformConstant);
  module.set_name(id_tex1, "tex1");

  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_coord = module.define_null_constant(vec3_type);
  Id id_dref = module.define_float_constant(0.5f);
  Id id_const_0 = module.define_int_constant(0);
  Id id_const_1 = module.define_int_constant(1);

  uint32_t id_ptr_si0;
  {
    SpirVBuilder b = make_entry_point(module);
    Id chain0 = b.op_access_chain(id_tex0, {id_const_0});
    id_ptr_si0 = module.get_type_id(chain0);
    Id load0 = b.op_load(chain0);
    b.op_image_sample(load0, id_coord);
    Id chain1 = b.op_access_chain(id_tex1, {id_const_1});
    Id load1 = b.op_load(chain1);
    b.insert(spv::OpImageSampleDrefImplicitLod,
             {id_float, b.allocate_id(), load1, id_coord, id_dref});
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVEmulateTextureQueriesPass pass(Shader::C_sampler_cube_shadow);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // The image and sampled image types are deduplicated.
  REQUIRE(count_op(result, spv::OpTypeImage) == 1);
  REQUIRE(count_op(result, spv::OpTypeSampledImage) == 1);
  CHECK(find_op(result, spv::OpTypeImage).args[3] == 0);

  // Both arrays remain, as (now identical) declarations over the deduped
  // element type, each keeping its own pointer type; the element pointer
  // types used by the access chains are deduplicated.
  REQUIRE(count_op(result, spv::OpTypeArray) == 2);
  uint32_t si_id = find_op(result, spv::OpTypeSampledImage).args[0];
  CHECK(find_op(result, spv::OpTypeArray, 0).args[1] == si_id);
  CHECK(find_op(result, spv::OpTypeArray, 1).args[1] == si_id);
  CHECK(count_op(result, spv::OpTypePointer) == 3);

  // Both access chains now produce the same (deduplicated) pointer type, and
  // both loads the same sampled image type.
  for (Instruction op : result) {
    if (op.opcode == spv::OpAccessChain) {
      CHECK(op.args[0] == id_ptr_si0);
    }
    if (op.opcode == spv::OpLoad) {
      CHECK(op.args[0] == si_id);
    }
  }

  CHECK(has_variable(result, id_tex0));
  CHECK(has_variable(result, id_tex1));
  CHECK(count_op(result, spv::OpImageSampleDrefImplicitLod) == 0);
  CHECK(count_op(result, spv::OpImageSampleImplicitLod) == 2);
  CHECK(count_op(result, spv::OpSelect) == 1);
}

TEST_CASE("SpirVEmulateTextureQueriesPass composes with variable removal", "[shaderpipeline]") {
  // uniform samplerCube tex0;  // never used
  // uniform samplerCubeShadow tex1;  // Dref-sampled
  // After the emulation pass dedupes the type chains, a second pass runs on
  // the same transformer to remove the unused variable.  This exercises the
  // handoff between passes: the definitions and unique type records of the
  // replaced ids must have been cleaned up when the first pass ended.
  SpirVModule module = make_module();
  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *sampler_cube = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float));
  const ShaderType *sampler_cube_shadow = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float, true));

  Id id_tex0 = module.define_variable(sampler_cube, spv::StorageClassUniformConstant);
  module.set_name(id_tex0, "tex0");
  Id id_tex1 = module.define_variable(sampler_cube_shadow, spv::StorageClassUniformConstant);
  module.set_name(id_tex1, "tex1");

  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_coord = module.define_null_constant(vec3_type);
  Id id_dref = module.define_float_constant(0.5f);

  {
    SpirVBuilder b = make_entry_point(module);
    Id load1 = b.op_load(id_tex1);
    b.insert(spv::OpImageSampleDrefImplicitLod,
             {id_float, b.allocate_id(), load1, id_coord, id_dref});
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVEmulateTextureQueriesPass pass(Shader::C_sampler_cube_shadow);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());
  transformer.run(SpirVRemoveUnusedVariablesPass());
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // The unused regular sampler is gone; the shadow sampler remains, using
  // the deduplicated type chain.
  CHECK(!has_variable(result, id_tex0));
  CHECK(has_variable(result, id_tex1));
  REQUIRE(count_op(result, spv::OpTypeImage) == 1);
  REQUIRE(count_op(result, spv::OpTypeSampledImage) == 1);
  CHECK(find_op(result, spv::OpTypeImage).args[3] == 0);

  CHECK(count_op(result, spv::OpImageSampleDrefImplicitLod) == 0);
  CHECK(count_op(result, spv::OpImageSampleImplicitLod) == 1);
  CHECK(count_op(result, spv::OpSelect) == 1);
}

TEST_CASE("SpirVEmulateTextureQueriesPass combines shadow and query emulation", "[shaderpipeline]") {
  // uniform samplerCubeShadow tex;  main() Dref-samples it AND queries its
  // level count, with both emulations enabled at once (the actual DX9
  // configuration).  The type replacement and the size-variable machinery
  // must not interfere with each other.
  SpirVModule module = make_module();
  module.add_capability(spv::CapabilityImageQuery);
  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *sampler_cube_shadow = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float, true));

  Id id_tex = module.define_variable(sampler_cube_shadow, spv::StorageClassUniformConstant);
  module.set_name(id_tex, "tex");

  Id id_sampled_image = module.unwrap_pointer_type(module.get_type_id(id_tex));
  Id id_image = module.get_type_id(id_sampled_image);
  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_int = module.define_type(ShaderType::INT);
  Id id_coord = module.define_null_constant(vec3_type);
  Id id_dref = module.define_float_constant(0.5f);

  {
    SpirVBuilder b = make_entry_point(module);
    Id load = b.op_load(id_tex);
    b.insert(spv::OpImageSampleDrefImplicitLod,
             {id_float, b.allocate_id(), load, id_coord, id_dref});
    Id img = b.allocate_id();
    b.insert(spv::OpImage, {id_image, img, load});
    b.insert(spv::OpImageQueryLevels, {id_int, b.allocate_id(), img});
    b.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVEmulateTextureQueriesPass pass(
    Shader::C_sampler_cube_shadow | Shader::C_texture_query_levels);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // The Dref sample was rewritten to a manual comparison, and the level query
  // was replaced by a read from a new size uniform.
  CHECK(count_op(result, spv::OpImageSampleDrefImplicitLod) == 0);
  CHECK(count_op(result, spv::OpImageSampleImplicitLod) == 1);
  CHECK(count_op(result, spv::OpSelect) == 1);
  CHECK(count_op(result, spv::OpImageQueryLevels) == 0);
  CHECK(count_op(result, spv::OpConvertFToS) == 1);

  REQUIRE(count_op(result, spv::OpTypeImage) == 1);
  CHECK(find_op(result, spv::OpTypeImage).args[3] == 0);

  REQUIRE(pass._size_var_ids.size() == 1);
  const auto &item = *pass._size_var_ids.begin();
  CHECK(item.first._var_id == id_tex);
  CHECK(item.first.size() == 0);

  CHECK(transformer.get_module().resolve_type(item.second) ==
        ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4)));
  CHECK(transformer.get_module().get_storage_class(item.second) ==
        spv::StorageClassUniformConstant);
}
