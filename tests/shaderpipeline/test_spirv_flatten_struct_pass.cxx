/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_flatten_struct_pass.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "spirVFlattenStructPass.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVFlattenStructPass turns struct members into variables", "[shaderpipeline]") {
  // struct S { float a; vec2 b; }; uniform S s;  main() loads s.a.
  SpirVModule module = make_module();
  const ShaderType *vec2_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 2));

  ShaderType::Struct st;
  st.add_member(ShaderType::FLOAT, "a");
  st.add_member(vec2_type, "b");
  const ShaderType *struct_type = ShaderType::register_type(std::move(st));

  Id id_struct = module.define_type(struct_type);
  module.set_name(id_struct, "S");
  module.set_member_name(id_struct, 0, "a");
  module.set_member_name(id_struct, 1, "b");

  Id id_var = module.define_variable(struct_type, spv::StorageClassUniformConstant);
  module.set_name(id_var, "s");

  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_const_0 = module.define_int_constant(0);

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    Id chain = builder.op_access_chain(id_var, {id_const_0});
    builder.op_load(chain);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  transformer.run(SpirVFlattenStructPass(id_struct));
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();
  CHECK(count_op(result, spv::OpTypeStruct) == 0);
  CHECK(!has_variable(result, id_var));

  // The members have become individual variables with the member names.
  const SpirVModule &out_module = transformer.get_module();
  Id var_a, var_b;
  for (uint32_t word = 0; word < transformer.get_id_bound(); ++word) {
    Id id(word);
    if (out_module.get_name(id) == "a") {
      var_a = id;
    }
    else if (out_module.get_name(id) == "b") {
      var_b = id;
    }
  }
  REQUIRE(var_a != 0);
  REQUIRE(var_b != 0);
  CHECK(out_module.resolve_type(var_a) == ShaderType::FLOAT);
  CHECK(out_module.resolve_type(var_b) == vec2_type);
  CHECK(out_module.get_name(var_a) == "a");
  CHECK(out_module.get_name(var_b) == "b");

  // The access chain is gone; the load reads the new variable directly.
  CHECK(count_op(result, spv::OpAccessChain) == 0);
  CHECK(find_load_pointer(result, id_float) == var_a);
}

TEST_CASE("SpirVFlattenStructPass rebases deeper access chains", "[shaderpipeline]") {
  // struct S { float a; vec2 b; }; uniform S s;  main() loads s.b.y, via a
  // two-index chain (which is shortened in place rather than deleted) and
  // via a chain of chains (whose deleted base must be mapped to the new
  // variable).
  SpirVModule module = make_module();
  const ShaderType *vec2_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 2));

  ShaderType::Struct st;
  st.add_member(ShaderType::FLOAT, "a");
  st.add_member(vec2_type, "b");
  const ShaderType *struct_type = ShaderType::register_type(std::move(st));

  Id id_struct = module.define_type(struct_type);
  module.set_member_name(id_struct, 0, "a");
  module.set_member_name(id_struct, 1, "b");

  Id id_var = module.define_variable(struct_type, spv::StorageClassUniformConstant);
  module.set_name(id_var, "s");

  Id id_const_1 = module.define_int_constant(1);

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    // One chain straight to the component...
    Id deep_chain = builder.op_access_chain(id_var, {id_const_1, id_const_1});
    builder.op_load(deep_chain);
    // ...and one that goes through an intermediate chain to the member.
    Id member_chain = builder.op_access_chain(id_var, {id_const_1});
    Id component_chain = builder.op_access_chain(member_chain, {id_const_1});
    builder.op_load(component_chain);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  transformer.run(SpirVFlattenStructPass(id_struct));
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();
  CHECK(count_op(result, spv::OpTypeStruct) == 0);
  CHECK(!has_variable(result, id_var));

  const SpirVModule &out_module = transformer.get_module();
  Id var_b;
  for (uint32_t word = 0; word < transformer.get_id_bound(); ++word) {
    if (out_module.get_name(Id(word)) == "b") {
      var_b = Id(word);
    }
  }
  REQUIRE(var_b != 0);

  // Both loads go through single-index chains based on the new variable,
  // whose result pointer has the UniformConstant storage class.
  CHECK(count_op(result, spv::OpAccessChain) == 2);
  for (int i = 0; i < 2; ++i) {
    Instruction chain = find_op(result, spv::OpAccessChain, i);
    REQUIRE(chain.nargs == 4);
    CHECK(chain.args[2] == var_b);
    CHECK(chain.args[3] == id_const_1);
    CHECK(out_module.get_storage_class(Id(chain.args[0])) == spv::StorageClassUniformConstant);
  }
  CHECK(count_op(result, spv::OpLoad) == 2);
}

TEST_CASE("SpirVFlattenStructPass spreads the struct location over the members", "[shaderpipeline]") {
  // A struct variable with a location; each member variable gets a
  // consecutive location starting at the struct's.
  SpirVModule module = make_module();
  const ShaderType *vec2_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 2));

  ShaderType::Struct st;
  st.add_member(ShaderType::FLOAT, "a");
  st.add_member(vec2_type, "b");
  const ShaderType *struct_type = ShaderType::register_type(std::move(st));

  Id id_struct = module.define_type(struct_type);
  module.set_member_name(id_struct, 0, "a");
  module.set_member_name(id_struct, 1, "b");

  Id id_var = module.define_variable(struct_type, spv::StorageClassUniformConstant);
  module.set_name(id_var, "s");
  module.decorate(id_var, spv::DecorationLocation, 5u);

  Id id_const_0 = module.define_int_constant(0);
  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    builder.op_load(builder.op_access_chain(id_var, {id_const_0}));
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  transformer.run(SpirVFlattenStructPass(id_struct));
  CHECK(transformer.get_module().validate());

  const SpirVModule &out_module = transformer.get_module();
  Id var_a, var_b;
  for (uint32_t word = 0; word < transformer.get_id_bound(); ++word) {
    if (out_module.get_name(Id(word)) == "a") {
      var_a = Id(word);
    }
    else if (out_module.get_name(Id(word)) == "b") {
      var_b = Id(word);
    }
  }
  REQUIRE(var_a != 0);
  REQUIRE(var_b != 0);
  CHECK(out_module.get_location(var_a) == 5);
  CHECK(out_module.get_location(var_b) == 6);
}
