/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_pointer.cxx
 * @author rdb
 * @date 2026-07-27
 */

#include "catch_amalgamated.hpp"

#include "spirv_test_utils.h"

TEST_CASE("SpirVPointer emits nothing until a memory operation", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec4_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 4));
  const ShaderType *array_type = ShaderType::register_type(
    ShaderType::Array(vec4_type, 4));

  {
    SpirVBuilder builder = make_entry_point(module);
    Pointer pointer = builder.define_private(array_type);
    Pointer element = pointer[2][1];
    (void)element;
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  CHECK(count_op(stream, spv::OpAccessChain) == 0);
  CHECK(count_op(stream, spv::OpLoad) == 0);
  CHECK(stream.validate());
}

TEST_CASE("SpirVPointer indexing emits one access chain per memory op", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec4_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 4));
  const ShaderType *array_type = ShaderType::register_type(
    ShaderType::Array(vec4_type, 4));
  Id id_var = module.define_variable(array_type, spv::StorageClassPrivate);

  Id id_result;
  {
    SpirVBuilder builder = make_entry_point(module);
    Pointer element = Pointer(builder, id_var)[2][1];
    id_result = element.load();
    element.store(Value(builder, id_result) + 1.0f);
    builder.op_return();
  }

  CHECK(module.resolve_type(id_result) == ShaderType::FLOAT);

  InstructionStream stream = module.emit();
  // One chain for the load, one for the store; each with both indices.
  CHECK(count_op(stream, spv::OpAccessChain) == 2);
  Instruction chain = find_op(stream, spv::OpAccessChain);
  REQUIRE(chain.nargs == 5);
  CHECK(chain.args[2] == (uint32_t)id_var);

  // The load reads through the chain, not the base variable.
  Instruction load = find_op(stream, spv::OpLoad);
  REQUIRE(load.nargs >= 3);
  CHECK(load.args[2] == chain.args[1]);

  CHECK(stream.validate());
}

TEST_CASE("SpirVPointer without indices accesses the variable directly", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec4_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 4));
  Id id_var = module.define_variable(vec4_type, spv::StorageClassPrivate);

  {
    SpirVBuilder builder = make_entry_point(module);
    Pointer pointer(builder, id_var);
    pointer.store(pointer.load() * 2.0f);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  CHECK(count_op(stream, spv::OpAccessChain) == 0);
  Instruction load = find_op(stream, spv::OpLoad);
  CHECK(load.args[2] == (uint32_t)id_var);
  Instruction store = find_op(stream, spv::OpStore);
  CHECK(store.args[0] == (uint32_t)id_var);
  CHECK(stream.validate());
}

TEST_CASE("SpirVPointer accepts dynamic indices and struct members", "[shaderpipeline]") {
  // struct S { float a; vec2 b[3]; }, indexed as s.b[i].
  SpirVModule module = make_module();

  const ShaderType *vec2_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 2));
  const ShaderType *array_type = ShaderType::register_type(
    ShaderType::Array(vec2_type, 3));

  ShaderType::Struct st;
  st.add_member(ShaderType::FLOAT, "a");
  st.add_member(array_type, "b");
  const ShaderType *struct_type = ShaderType::register_type(std::move(st));

  Id id_result, id_index;
  {
    SpirVBuilder builder = make_entry_point(module);
    Value index = builder.define_private(ShaderType::UINT).load();
    id_index = index;
    id_result = builder.define_private(struct_type)[1][index].load();
    builder.op_return();
  }

  CHECK(module.resolve_type(id_result) == vec2_type);

  InstructionStream stream = module.emit();
  Instruction chain = find_op(stream, spv::OpAccessChain);
  REQUIRE(chain.nargs == 5);
  CHECK(chain.args[4] == (uint32_t)id_index);
  CHECK(stream.validate());
}

TEST_CASE("SpirVPointer atomics operate through the pointer", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *array_type = ShaderType::register_type(
    ShaderType::Array(ShaderType::UINT, 4));
  Id id_counter_var = module.define_variable(ShaderType::UINT, spv::StorageClassWorkgroup);
  Id id_array_var = module.define_variable(array_type, spv::StorageClassWorkgroup);

  Id id_old_count, id_old_element;
  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelGLCompute);
    module.add_execution_mode(builder.get_current_function_id(),
                              spv::ExecutionModeLocalSize, {1, 1, 1});
    id_old_count = Pointer(builder, id_counter_var).atomic_increment();
    // The literal materializes against the pointed-to type (a uint).
    id_old_element = Pointer(builder, id_array_var)[2].atomic_add(5);
    builder.op_return();
  }

  // Both return the value the memory held before, of the pointed-to type.
  CHECK(module.resolve_type(id_old_count) == ShaderType::UINT);
  CHECK(module.resolve_type(id_old_element) == ShaderType::UINT);

  InstructionStream stream = module.emit();

  // The unindexed atomic operates on the variable directly...
  Instruction increment = find_op(stream, spv::OpAtomicIIncrement);
  REQUIRE(increment.nargs == 5);
  CHECK(increment.args[2] == (uint32_t)id_counter_var);

  // ...and the indexed one through its own access chain.
  Instruction chain = find_op(stream, spv::OpAccessChain);
  Instruction add = find_op(stream, spv::OpAtomicIAdd);
  REQUIRE(add.nargs == 6);
  CHECK(add.args[2] == chain.args[1]);

  CHECK(stream.validate());
}

TEST_CASE("SpirVPointer prints its base and indices", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *vec4_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 4));
  const ShaderType *array_type = ShaderType::register_type(
    ShaderType::Array(vec4_type, 4));
  Id id_var = module.define_variable(array_type, spv::StorageClassPrivate);
  module.set_name(id_var, "verts");

  {
    SpirVBuilder builder = make_entry_point(module);

    // Constant indices print as their literal value.
    std::ostringstream constant;
    constant << Pointer(builder, id_var)[2][1];
    CHECK(constant.str() == "&verts[2][1]");

    // A dynamic index prints as the index value's %id.
    Value index = builder.define_private(ShaderType::INT).load();
    std::ostringstream dynamic;
    dynamic << Pointer(builder, id_var)[index];
    std::ostringstream expected;
    expected << "&verts[%" << (Id)index << ']';
    CHECK(dynamic.str() == expected.str());

    builder.op_return();
  }
}
