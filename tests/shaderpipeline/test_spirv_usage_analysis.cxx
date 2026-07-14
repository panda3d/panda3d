/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_usage_analysis.cxx
 * @author rdb
 * @date 2026-07-07
 */

#include "spirVUsageAnalysis.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVUsageAnalysis tracks usedness through pointers", "[shaderpipeline]") {
  // One variable loaded through an access chain, one stored to, one whose
  // access chain result is never consumed, and one never referenced at all.
  SpirVModule module = make_module();
  const ShaderType *vec2_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 2));

  Id var_loaded = module.define_variable(vec2_type, spv::StorageClassPrivate);
  Id var_stored = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id var_chained = module.define_variable(vec2_type, spv::StorageClassPrivate);
  Id var_untouched = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);

  Id id_const_0 = module.define_int_constant(0);
  Id chain_consumed, chain_dead, loaded;
  {
    SpirVBuilder builder = make_entry_point(module);
    chain_consumed = builder.op_access_chain(var_loaded, {id_const_0});
    loaded = builder.op_load(chain_consumed);
    builder.op_store(var_stored, loaded);
    chain_dead = builder.op_access_chain(var_chained, {id_const_0});
    builder.op_return();
  }

  SpirVUsageAnalysis usage = module.analyze_usage();

  CHECK(usage.is_used(var_loaded));
  CHECK(usage.is_used(var_stored));
  // A chain that is never loaded or stored does not make its variable used.
  CHECK(!usage.is_used(var_chained));
  CHECK(!usage.is_used(var_untouched));

  // Origins chase back to the variable; a variable originates from itself,
  // and a loaded value carries the origin of its pointer.
  CHECK(usage.get_origin(var_loaded) == var_loaded);
  CHECK(usage.get_origin(chain_consumed) == var_loaded);
  CHECK(usage.get_origin(chain_dead) == var_chained);
  CHECK(usage.get_origin(loaded) == var_loaded);
}

TEST_CASE("SpirVUsageAnalysis marks call arguments and callees used", "[shaderpipeline]") {
  // void f(float *p);  main() { f(&v); }  Passing the pointer marks the
  // variable used, and the call marks the function used.
  SpirVModule module = make_module();

  Id ptr_tid = module.define_pointer_type(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id void_tid = module.define_type(nullptr);

  Id func_id, param_id;
  {
    SpirVBuilder builder = module.make_function(nullptr, {ptr_tid});
    func_id = builder.get_current_function_id();
    param_id = module.find_function(func_id)->get_parameters()[0];
    builder.op_return();
  }

  Id var_arg = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id var_other = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);

  {
    SpirVBuilder builder = make_entry_point(module);
    Id call_id = module.allocate_id();
    builder.insert(spv::OpFunctionCall, {void_tid, call_id, func_id, var_arg});
    builder.op_return();
  }

  SpirVUsageAnalysis usage = module.analyze_usage();
  CHECK(usage.is_used(var_arg));
  CHECK(!usage.is_used(var_other));
  CHECK(usage.is_used(func_id));
  CHECK(!usage.is_used(param_id));
}

TEST_CASE("SpirVUsageAnalysis distinguishes dref and non-dref sampling", "[shaderpipeline]") {
  SpirVModule module = make_module();
  const ShaderType *sampler_type = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float, false));
  const ShaderType *shadow_type = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float, true));
  const ShaderType *vec2_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 2));

  Id var_plain = module.define_variable(sampler_type, spv::StorageClassUniformConstant);
  Id var_shadow = module.define_variable(shadow_type, spv::StorageClassUniformConstant);

  Id float_tid = module.define_type(ShaderType::FLOAT);
  Id coord = module.define_null_constant(vec2_type);
  Id dref = module.define_float_constant(0.5f);

  {
    SpirVBuilder builder = make_entry_point(module);
    builder.op_image_sample(builder.op_load(var_plain), coord);

    Id loaded_shadow = builder.op_load(var_shadow);
    Id result = module.allocate_id();
    builder.insert(spv::OpImageSampleDrefImplicitLod,
                   {float_tid, result, loaded_shadow, coord, dref});
    builder.op_return();
  }

  SpirVUsageAnalysis usage = module.analyze_usage();
  CHECK(usage.is_non_dref_sampled(var_plain));
  CHECK(!usage.is_dref_sampled(var_plain));
  CHECK(usage.is_dref_sampled(var_shadow));
  CHECK(!usage.is_non_dref_sampled(var_shadow));
  CHECK(usage.is_used(var_plain));
  CHECK(usage.is_used(var_shadow));
}

TEST_CASE("SpirVUsageAnalysis detects dynamic indexing via constant expressions", "[shaderpipeline]") {
  // Three arrays: one indexed by a plain constant, one by an expression of
  // constants (still a constant expression), one by a loaded value.
  SpirVModule module = make_module();
  const ShaderType *array_type =
    ShaderType::register_type(ShaderType::Array(ShaderType::FLOAT, 4));

  Id var_const_indexed = module.define_variable(array_type, spv::StorageClassPrivate);
  Id var_expr_indexed = module.define_variable(array_type, spv::StorageClassPrivate);
  Id var_dyn_indexed = module.define_variable(array_type, spv::StorageClassPrivate);
  Id var_index = module.define_variable(ShaderType::INT, spv::StorageClassPrivate);

  Id int_tid = module.define_type(ShaderType::INT);
  Id id_c1 = module.define_int_constant(1);
  Id id_c2 = module.define_int_constant(2);

  {
    SpirVBuilder builder = make_entry_point(module);
    builder.op_load(builder.op_access_chain(var_const_indexed, {id_c1}));

    Id sum = module.allocate_id();
    builder.insert(spv::OpIAdd, {int_tid, sum, id_c1, id_c2});
    builder.op_load(builder.op_access_chain(var_expr_indexed, {Id(sum)}));

    Id index = builder.op_load(var_index);
    builder.op_load(builder.op_access_chain(var_dyn_indexed, {index}));
    builder.op_return();
  }

  SpirVUsageAnalysis usage = module.analyze_usage();
  CHECK(!usage.is_dynamically_indexed(var_const_indexed));
  CHECK(!usage.is_dynamically_indexed(var_expr_indexed));
  CHECK(usage.is_dynamically_indexed(var_dyn_indexed));

  CHECK(usage.is_constant_expression(id_c1));
  CHECK(!usage.is_constant_expression(var_index));
}

TEST_CASE("SpirVUsageAnalysis flags queried image sizes and sampled image values", "[shaderpipeline]") {
  // textureSize(tex, 0): load, OpImage, OpImageQuerySizeLod.  The query flag
  // must trace back to the variable, and the OpImage result must be marked
  // as coming from a sampled image.
  SpirVModule module = make_module();
  const ShaderType *sampler_type = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float, false));
  const ShaderType *ivec2_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_int, 2));

  Id var_tex = module.define_variable(sampler_type, spv::StorageClassUniformConstant);
  Id var_other = module.define_variable(sampler_type, spv::StorageClassUniformConstant);

  Id sampler_tid = module.find_type(sampler_type);
  REQUIRE(sampler_tid != 0);
  Id image_tid = module.get_type_id(sampler_tid);
  Id ivec2_tid = module.define_type(ivec2_type);
  Id id_c0 = module.define_int_constant(0);

  Id image_value;
  {
    SpirVBuilder builder = make_entry_point(module);
    Id loaded = builder.op_load(var_tex);
    image_value = module.allocate_id();
    builder.insert(spv::OpImage, {image_tid, image_value, loaded});
    Id size = module.allocate_id();
    builder.insert(spv::OpImageQuerySizeLod, {ivec2_tid, size, image_value, id_c0});
    builder.op_return();
  }

  SpirVUsageAnalysis usage = module.analyze_usage();
  CHECK(usage.was_size_or_levels_queried(var_tex));
  CHECK(!usage.was_size_or_levels_queried(var_other));
  CHECK(usage.is_sampled_image_value(image_value));
  CHECK(usage.get_origin(image_value) == var_tex);
}

TEST_CASE("SpirVUsageAnalysis treats spec constants as constant expressions", "[shaderpipeline]") {
  // Indexing an array with a spec constant (or an OpSpecConstantOp result)
  // is not dynamic indexing.
  SpirVModule module = make_module();
  const ShaderType *array_type =
    ShaderType::register_type(ShaderType::Array(ShaderType::FLOAT, 8));

  Id var_arr = module.define_variable(array_type, spv::StorageClassPrivate);
  Id id_spec = module.define_spec_constant(ShaderType::INT, 1);
  module.decorate(id_spec, spv::DecorationSpecId, 3u);

  Id int_tid = module.define_type(ShaderType::INT);
  Id id_c2 = module.define_int_constant(2);
  Id id_spec_op = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpSpecConstantOp,
    {int_tid, id_spec_op, (uint32_t)spv::OpIMul, id_spec, id_c2}));

  {
    SpirVBuilder builder = make_entry_point(module);
    builder.op_load(builder.op_access_chain(var_arr, {id_spec_op}));
    builder.op_return();
  }

  SpirVUsageAnalysis usage = module.analyze_usage();
  CHECK(usage.is_constant_expression(id_spec));
  CHECK(usage.is_constant_expression(id_spec_op));
  CHECK(!usage.is_dynamically_indexed(var_arr));
  CHECK(usage.is_used(var_arr));
}
