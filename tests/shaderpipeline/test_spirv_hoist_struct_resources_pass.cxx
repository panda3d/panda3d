/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_hoist_struct_resources_pass.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "spirVHoistStructResourcesPass.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVHoistStructResourcesPass moves resources out of structs", "[shaderpipeline]") {
  // struct S { sampler2D tex; float f; }; uniform S s;  main() loads both.
  SpirVModule module = make_module();
  const ShaderType *sampler_type = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float));

  ShaderType::Struct struct_def_builder;
  struct_def_builder.add_member(sampler_type, "tex");
  struct_def_builder.add_member(ShaderType::FLOAT, "f");
  const ShaderType *struct_type = ShaderType::register_type(std::move(struct_def_builder));

  Id id_sampled_image = module.define_type(sampler_type);
  Id id_struct = module.define_type(struct_type);
  module.set_name(id_struct, "S");

  Id id_var = module.define_variable(struct_type, spv::StorageClassUniformConstant);
  module.set_name(id_var, "s");

  Id id_const_0 = module.define_int_constant(0);
  Id id_const_1 = module.define_int_constant(1);

  Id id_chain_tex, id_chain_f;
  {
    SpirVBuilder builder = make_entry_point(module);
    id_chain_tex = builder.op_access_chain(id_var, {id_const_0});
    builder.op_load(id_chain_tex);
    id_chain_f = builder.op_access_chain(id_var, {id_const_1});
    builder.op_load(id_chain_f);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVHoistStructResourcesPass pass(true);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // The sampler was hoisted into its own variable.
  REQUIRE(pass._hoisted_vars.size() == 1);
  const auto &hoisted = *pass._hoisted_vars.begin();
  CHECK(hoisted.first._var_id == id_var);
  REQUIRE(hoisted.first.size() == 1);
  CHECK(hoisted.first[0] == 0);

  Id hoisted_var = hoisted.second;
  CHECK(has_variable(result, hoisted_var));

  // The access chain to the sampler member was rebased onto the new variable,
  // with the struct member index dropped.
  bool found_chain = false;
  for (Instruction op : result) {
    if (op.opcode == spv::OpAccessChain && op.args[1] == id_chain_tex) {
      found_chain = true;
      CHECK(op.args[2] == hoisted_var);
      CHECK(op.nargs == 3);
    }
  }
  CHECK(found_chain);
  CHECK(find_load_pointer(result, id_sampled_image) == id_chain_tex);

  // The struct itself remains, with only the float member.
  const ShaderType *result_type = transformer.get_module().resolve_type(id_struct);
  REQUIRE(result_type != nullptr);
  const ShaderType::Struct *result_struct_type = result_type->as_struct();
  REQUIRE(result_struct_type != nullptr);
  REQUIRE(result_struct_type->get_num_members() == 1);
  CHECK(result_struct_type->get_member(0).type == ShaderType::FLOAT);
}

TEST_CASE("SpirVHoistStructResourcesPass empties structs with only resources", "[shaderpipeline]") {
  // struct S { sampler2D tex; }; uniform S s;  main() loads the sampler.
  // With remove_empty_structs = false, the emptied struct declaration is
  // kept (it may be a member of an enclosing struct, where removing it would
  // change the member numbering), but the pointer type and variable serve no
  // further purpose and must be removed.
  SpirVModule module = make_module();
  const ShaderType *sampler_type = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float));

  ShaderType::Struct struct_def_builder;
  struct_def_builder.add_member(sampler_type, "tex");
  const ShaderType *struct_type = ShaderType::register_type(std::move(struct_def_builder));

  Id id_sampled_image = module.define_type(sampler_type);
  Id id_ptr_uc_image = module.define_pointer_type(sampler_type, spv::StorageClassUniformConstant);
  Id id_struct = module.define_type(struct_type);
  module.set_name(id_struct, "S");

  Id id_var = module.define_variable(struct_type, spv::StorageClassUniformConstant);
  module.set_name(id_var, "s");

  Id id_const_0 = module.define_int_constant(0);

  Id id_chain;
  {
    SpirVBuilder builder = make_entry_point(module);
    id_chain = builder.op_access_chain(id_var, {id_const_0});
    builder.op_load(id_chain);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVHoistStructResourcesPass pass(false);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // The sampler was hoisted, and the chain rebased onto the new variable.
  REQUIRE(pass._hoisted_vars.size() == 1);
  const auto &hoisted = *pass._hoisted_vars.begin();
  CHECK(hoisted.first._var_id == id_var);
  REQUIRE(hoisted.first.size() == 1);
  CHECK(hoisted.first[0] == 0);

  Id hoisted_var = hoisted.second;
  CHECK(has_variable(result, hoisted_var));

  bool found_chain = false;
  for (Instruction op : result) {
    if (op.opcode == spv::OpAccessChain && op.args[1] == id_chain) {
      found_chain = true;
      CHECK(op.args[2] == hoisted_var);
      CHECK(op.nargs == 3);
    }
  }
  CHECK(found_chain);
  CHECK(find_load_pointer(result, id_sampled_image) == id_chain);

  // The struct declaration remains, but with no members left.
  REQUIRE(count_op(result, spv::OpTypeStruct) == 1);
  CHECK(find_op(result, spv::OpTypeStruct).nargs == 1);

  // The old variable is gone, along with its pointer type; the only pointer
  // type left is the one to the sampler (reused for the hoisted variable).
  CHECK(!has_variable(result, id_var));
  CHECK(count_op(result, spv::OpTypePointer) == 1);
  CHECK(find_op(result, spv::OpTypePointer).args[0] == id_ptr_uc_image);

  // The hoisted variable took over the name of the old variable.
  CHECK(transformer.get_module().get_name(hoisted_var) == "s_m0");
}

TEST_CASE("SpirVHoistStructResourcesPass removes structs with only resources", "[shaderpipeline]") {
  // Same module as above, but with remove_empty_structs = true, the struct
  // and everything referencing it must be removed altogether.
  SpirVModule module = make_module();
  const ShaderType *sampler_type = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float));

  ShaderType::Struct struct_def_builder;
  struct_def_builder.add_member(sampler_type, "tex");
  const ShaderType *struct_type = ShaderType::register_type(std::move(struct_def_builder));

  Id id_sampled_image = module.define_type(sampler_type);
  Id id_ptr_uc_image = module.define_pointer_type(sampler_type, spv::StorageClassUniformConstant);
  Id id_struct = module.define_type(struct_type);
  module.set_name(id_struct, "S");

  Id id_var = module.define_variable(struct_type, spv::StorageClassUniformConstant);
  module.set_name(id_var, "s");

  Id id_const_0 = module.define_int_constant(0);

  Id id_chain;
  {
    SpirVBuilder builder = make_entry_point(module);
    id_chain = builder.op_access_chain(id_var, {id_const_0});
    builder.op_load(id_chain);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVHoistStructResourcesPass pass(true);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  REQUIRE(pass._hoisted_vars.size() == 1);
  Id hoisted_var = pass._hoisted_vars.begin()->second;
  CHECK(has_variable(result, hoisted_var));
  CHECK(!has_variable(result, id_var));

  // The struct is gone entirely.
  CHECK(count_op(result, spv::OpTypeStruct) == 0);
  CHECK(count_op(result, spv::OpTypePointer) == 1);
  CHECK(find_op(result, spv::OpTypePointer).args[0] == id_ptr_uc_image);

  CHECK(find_load_pointer(result, id_sampled_image) == id_chain);
}

TEST_CASE("SpirVHoistStructResourcesPass keeps emptied structs as members", "[shaderpipeline]") {
  // struct Inner { sampler2D t; }; struct Outer { Inner i; float f; };
  // uniform Outer o;  With remove_empty_structs = false, the emptied Inner
  // must remain as a member of Outer, so that the member numbering of Outer
  // is unchanged.
  SpirVModule module = make_module();
  const ShaderType *sampler_type = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float));

  ShaderType::Struct inner_def_builder;
  inner_def_builder.add_member(sampler_type, "t");
  const ShaderType *inner_struct_type = ShaderType::register_type(std::move(inner_def_builder));

  ShaderType::Struct outer_def_builder;
  outer_def_builder.add_member(inner_struct_type, "i");
  outer_def_builder.add_member(ShaderType::FLOAT, "f");
  const ShaderType *outer_struct_type = ShaderType::register_type(std::move(outer_def_builder));

  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_inner = module.define_type(inner_struct_type);
  module.set_name(id_inner, "Inner");
  Id id_outer = module.define_type(outer_struct_type);
  module.set_name(id_outer, "Outer");

  Id id_var = module.define_variable(outer_struct_type, spv::StorageClassUniformConstant);
  module.set_name(id_var, "o");

  Id id_const_0 = module.define_int_constant(0);
  Id id_const_1 = module.define_int_constant(1);

  Id id_chain_t, id_chain_f;
  {
    SpirVBuilder builder = make_entry_point(module);
    id_chain_t = builder.op_access_chain(id_var, {id_const_0, id_const_0});
    builder.op_load(id_chain_t);
    id_chain_f = builder.op_access_chain(id_var, {id_const_1});
    builder.op_load(id_chain_f);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVHoistStructResourcesPass pass(false);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

  // The sampler was hoisted from o.i.t.
  REQUIRE(pass._hoisted_vars.size() == 1);
  const auto &hoisted = *pass._hoisted_vars.begin();
  CHECK(hoisted.first._var_id == id_var);
  REQUIRE(hoisted.first.size() == 2);
  CHECK(hoisted.first[0] == 0);
  CHECK(hoisted.first[1] == 0);
  Id hoisted_var = hoisted.second;
  CHECK(has_variable(result, hoisted_var));

  // Inner is emptied but remains a member of Outer, which keeps its member
  // numbering: f is still member 1.
  REQUIRE(count_op(result, spv::OpTypeStruct) == 2);
  Instruction inner_op = find_op(result, spv::OpTypeStruct, 0);
  CHECK(inner_op.args[0] == id_inner);
  CHECK(inner_op.nargs == 1);
  Instruction outer_op = find_op(result, spv::OpTypeStruct, 1);
  CHECK(outer_op.args[0] == id_outer);
  REQUIRE(outer_op.nargs == 3);
  CHECK(outer_op.args[1] == id_inner);
  CHECK(outer_op.args[2] == id_float);

  // The outer variable remains, and the chain to o.f still uses index 1.
  CHECK(has_variable(result, id_var));
  bool found_chain_f = false;
  for (Instruction op : result) {
    if (op.opcode == spv::OpAccessChain && op.args[1] == id_chain_f) {
      found_chain_f = true;
      CHECK(op.args[2] == id_var);
      REQUIRE(op.nargs == 4);
      CHECK(op.args[3] == id_const_1);
    }
    if (op.opcode == spv::OpAccessChain && op.args[1] == id_chain_t) {
      // The sampler chain was rebased onto the hoisted variable.
      CHECK(op.args[2] == hoisted_var);
      CHECK(op.nargs == 3);
    }
  }
  CHECK(found_chain_f);

  // The resolved type reflects the emptied inner struct.
  const ShaderType::Struct *outer_type =
    transformer.get_module().resolve_type(id_outer)->as_struct();
  REQUIRE(outer_type != nullptr);
  REQUIRE(outer_type->get_num_members() == 2);
  const ShaderType::Struct *inner_type = outer_type->get_member(0).type->as_struct();
  REQUIRE(inner_type != nullptr);
  CHECK(inner_type->get_num_members() == 0);
  CHECK(outer_type->get_member(1).type == ShaderType::FLOAT);
}
