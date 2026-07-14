/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_module.cxx
 * @author rdb
 * @date 2026-07-07
 */

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

struct TestFunction {
  Id type_id;
  Id func_id;
  pvector<Id> param_ids;
};

/**
 * Builds a function with the given parameter types that returns its last
 * parameter (which must be of the return type), for testing parameter
 * removal.
 */
static TestFunction
make_float_function(SpirVModule &module, const pvector<Id> &param_type_ids) {
  SpirVBuilder builder = module.make_function(ShaderType::FLOAT, param_type_ids);

  TestFunction fn;
  fn.func_id = builder.get_current_function_id();
  fn.type_id = module.get_type_id(fn.func_id);
  fn.param_ids = module.find_function(fn.func_id)->get_parameters();

  builder.op_return_value(fn.param_ids.back());
  return fn;
}

TEST_CASE("SpirVModule prunes deleted variables from entry point interfaces", "[shaderpipeline]") {
  // Three unused inputs; the first two (adjacent in the interface) get
  // deleted.  Their declarations, names, decorations and interface entries
  // must all disappear.
  SpirVModule module = make_module();

  Id id_in1 = module.define_variable(ShaderType::FLOAT, spv::StorageClassInput);
  Id id_in2 = module.define_variable(ShaderType::FLOAT, spv::StorageClassInput);
  Id id_in3 = module.define_variable(ShaderType::FLOAT, spv::StorageClassInput);
  module.set_name(id_in1, "in1");
  module.set_name(id_in2, "in2");
  module.set_name(id_in3, "in3");
  module.decorate(id_in1, spv::DecorationLocation, 0u);
  module.decorate(id_in2, spv::DecorationLocation, 1u);
  module.decorate(id_in3, spv::DecorationLocation, 2u);
  {
    SpirVBuilder builder = make_entry_point(
      module, spv::ExecutionModelFragment, {id_in1, id_in2, id_in3});
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  // Delete the given unused variables.
  SpirVModule transformed(stream);
  pset<Id> dead = {id_in1, id_in2};
  for (Id id : dead) {
    transformed.delete_id(id);
  }
  transformed.delete_dead_code(dead);
  transformed.deduplicate_types();
  CHECK(transformed.validate());

  InstructionStream result = transformed.emit();

  // The interface lists only in3, and the word count is consistent with that.
  Instruction ep = find_op(result, spv::OpEntryPoint);
  REQUIRE(ep.opcode == spv::OpEntryPoint);
  REQUIRE(ep.nargs == 5);  // model, main, "main" (two words), in3
  CHECK(ep.args[4] == id_in3);

  // The variables, their names and their decorations are gone as well.
  CHECK(count_op(result, spv::OpVariable) == 1);
  CHECK(has_variable(result, id_in3));
  CHECK(count_op(result, spv::OpName) == 1);
  CHECK(count_op(result, spv::OpDecorate) == 1);
}

TEST_CASE("SpirVModule strips call arguments when removing function parameters", "[shaderpipeline]") {
  // float f(sampler2D t, float x) and float g(float y).  Removing f's
  // sampler parameter makes f's function type identical to g's; the
  // deduplication merges the two.  The call to f must lose its sampler
  // argument, and the call to g (whose type the merge joins with f's, even
  // though g never had any parameters removed) must keep its argument.
  SpirVModule module = make_module();

  const ShaderType *sampler_type = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float, false));
  Id float_tid = module.define_type(ShaderType::FLOAT);
  Id ptr_tid = module.define_pointer_type(sampler_type, spv::StorageClassUniformConstant);
  Id id_tex = module.define_variable(sampler_type, spv::StorageClassUniformConstant);
  Id id_c1 = module.define_float_constant(1.0f);

  TestFunction fn_f = make_float_function(module, {ptr_tid, float_tid});
  TestFunction fn_g = make_float_function(module, {float_tid});

  Id id_call1, id_call2;
  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    id_call1 = module.allocate_id();
    builder.insert(spv::OpFunctionCall, {float_tid, id_call1, fn_f.func_id, id_tex, id_c1});
    id_call2 = module.allocate_id();
    builder.insert(spv::OpFunctionCall, {float_tid, id_call2, fn_g.func_id, id_c1});
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  // Remove the sampler parameter from f.
  SpirVModule transformed(stream);
  transformed.remove_function_parameters(fn_f.type_id, {0u});
  transformed.deduplicate_types();
  CHECK(transformed.validate());

  InstructionStream result = transformed.emit();

  // The rebuilt type merged onto the identical other declaration.
  CHECK(count_op(result, spv::OpTypeFunction) == 2);  // void(), float(float)
  CHECK(count_op(result, spv::OpFunctionParameter) == 2);

  bool found_call1 = false, found_call2 = false;
  for (Instruction op : result) {
    if (op.opcode == spv::OpFunctionCall && op.args[1] == id_call1) {
      REQUIRE(op.nargs == 4);
      CHECK(op.args[2] == fn_f.func_id);
      CHECK(op.args[3] == id_c1);
      found_call1 = true;
    }
    else if (op.opcode == spv::OpFunctionCall && op.args[1] == id_call2) {
      REQUIRE(op.nargs == 4);
      CHECK(op.args[2] == fn_g.func_id);
      CHECK(op.args[3] == id_c1);
      found_call2 = true;
    }
  }
  CHECK(found_call1);
  CHECK(found_call2);

  // The parameter list of f reflects the removal; g is untouched.
  SpirVModule::Function *f_func = transformed.find_function(fn_f.func_id);
  REQUIRE(f_func != nullptr);
  pvector<Id> f_params = f_func->get_parameters();
  REQUIRE(f_params.size() == 1);
  CHECK(f_params[0] == fn_f.param_ids[1]);

  SpirVModule::Function *g_func = transformed.find_function(fn_g.func_id);
  REQUIRE(g_func != nullptr);
  pvector<Id> g_params = g_func->get_parameters();
  REQUIRE(g_params.size() == 1);
  CHECK(g_params[0] == fn_g.param_ids[0]);
}

TEST_CASE("SpirVModule cascades body code based on removed parameters", "[shaderpipeline]") {
  // float f(vec4 *p, float x) contains an access chain based on p, and a
  // pointer copy of that chain, but never loads or stores through either, so
  // p is removable.  Removing the parameter must also delete the chain and
  // the copy, which would otherwise be left referencing the deleted id.
  SpirVModule module = make_module();

  const ShaderType *vec4_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 4));
  Id float_tid = module.define_type(ShaderType::FLOAT);
  Id ptr_tid = module.define_pointer_type(vec4_type, spv::StorageClassPrivate);
  Id float_ptr_tid = module.define_pointer_type(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id id_var = module.define_variable(vec4_type, spv::StorageClassPrivate);
  Id id_c0 = module.define_int_constant(0);
  Id id_c1 = module.define_float_constant(1.0f);

  TestFunction fn = make_float_function(module, {ptr_tid, float_tid});
  {
    // Position just before the OpReturnValue at the end of the body.
    SpirVModule::Function *function = module.find_function(fn.func_id);
    REQUIRE(function != nullptr);
    SpirVBuilder builder(module, fn.func_id, function->instructions.size() - 1);
    Id id_chain = module.allocate_id();
    builder.insert(spv::OpAccessChain, {float_ptr_tid, id_chain, fn.param_ids[0], id_c0});
    Id id_copy = module.allocate_id();
    builder.insert(spv::OpCopyObject, {float_ptr_tid, id_copy, id_chain});
  }

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    Id id_call = module.allocate_id();
    builder.insert(spv::OpFunctionCall, {float_tid, id_call, fn.func_id, id_var, id_c1});
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  // Remove the pointer parameter from f.
  SpirVModule transformed(stream);
  transformed.remove_function_parameters(fn.type_id, {0u});
  transformed.deduplicate_types();
  CHECK(transformed.validate());

  InstructionStream result = transformed.emit();

  // The parameter is gone, and with it the chain and the pointer copy.
  CHECK(count_op(result, spv::OpFunctionParameter) == 1);
  CHECK(count_op(result, spv::OpAccessChain) == 0);
  CHECK(count_op(result, spv::OpCopyObject) == 0);

  // The call lost its pointer argument but kept the float argument.
  Instruction call = find_op(result, spv::OpFunctionCall);
  REQUIRE(call.opcode == spv::OpFunctionCall);
  REQUIRE(call.nargs == 4);
  CHECK(call.args[2] == fn.func_id);
  CHECK(call.args[3] == id_c1);
}

TEST_CASE("SpirVModule merges duplicates cascading from a type replacement", "[shaderpipeline]") {
  // A shadow sampler and a plain sampler of the same dimensionality.  The
  // shadow image type is replaced with a fresh non-depth image type,
  // which the deduplication merges with the plain image type; this cascades
  // through the sampled image and pointer types.  The replacement type is
  // declared at the end of the declarations section while its first use
  // precedes it, so this also exercises the emit-time topological sort.
  SpirVModule module = make_module();

  const ShaderType *shadow_type = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float, true));
  const ShaderType *plain_type = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float, false));

  Id id_tex_shadow = module.define_variable(shadow_type, spv::StorageClassUniformConstant);
  Id id_tex_plain = module.define_variable(plain_type, spv::StorageClassUniformConstant);

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    builder.op_load(id_tex_shadow);
    builder.op_load(id_tex_plain);
    builder.op_return();
  }

  // Find the shadow image type declaration (the image operand of the shadow
  // sampled image type).
  Id shadow_image_tid = module.get_type_id(module.find_type(shadow_type));

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());
  REQUIRE(count_op(stream, spv::OpTypeImage) == 2);
  REQUIRE(count_op(stream, spv::OpTypeSampledImage) == 2);
  REQUIRE(count_op(stream, spv::OpTypePointer) == 2);

  // Replace the image type with a fresh declaration having a different depth
  // flag, relying on deduplication to merge identical downstream declarations.
  SpirVModule transformed(stream);
  const SpirVModule::Instruction *decl =
    transformed.find_declaration(shadow_image_tid);
  REQUIRE(decl != nullptr);
  REQUIRE(decl->opcode == spv::OpTypeImage);
  uint32_t sampled = (*decl)[6];
  spv::ImageFormat format = (spv::ImageFormat)(*decl)[7];
  Id new_image_tid = transformed.define_image_type(
    transformed.resolve_type(shadow_image_tid), 0, sampled, format);
  transformed.replace_type_id(shadow_image_tid, new_image_tid);
  transformed.deduplicate_types();
  CHECK(transformed.validate());

  InstructionStream result = transformed.emit();

  // Everything merged down to a single image/sampled image/pointer type.
  CHECK(count_op(result, spv::OpTypeImage) == 1);
  CHECK(count_op(result, spv::OpTypeSampledImage) == 1);
  CHECK(count_op(result, spv::OpTypePointer) == 1);
  CHECK(count_op(result, spv::OpVariable) == 2);
  CHECK(count_op(result, spv::OpLoad) == 2);
}

TEST_CASE("SpirVModule emits appended replacement types before their uses", "[shaderpipeline]") {
  // A private float[2] variable whose array type gets replaced with a
  // float[3] declared at the end of the declarations section.  Arrays are
  // exempt from deduplication, so the forward reference from the pointer type
  // survives until emit(), which must reorder the declarations using the
  // recorded declaration indices.
  SpirVModule module = make_module();

  const ShaderType *array2_type = ShaderType::register_type(
    ShaderType::Array(ShaderType::FLOAT, 2));
  Id id_var = module.define_variable(array2_type, spv::StorageClassPrivate);
  Id array2_tid = module.find_type(array2_type);
  REQUIRE(array2_tid != 0);
  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  // Replace float[2] with a fresh float[3] declaration.  Aggregate types are
  // exempt from deduplication, so emit-time sorting must restore a valid order.
  SpirVModule transformed(stream);
  Id new_array_tid = transformed.define_type(ShaderType::register_type(
    ShaderType::Array(ShaderType::FLOAT, 3)));
  transformed.replace_type_id(array2_tid, new_array_tid);
  transformed.deduplicate_types();
  CHECK(transformed.validate());

  InstructionStream result = transformed.emit();

  // The float[2] declaration is gone, and the replacement precedes the
  // pointer type that references it despite having been declared later.
  int array_pos = -1, pointer_pos = -1, pos = 0;
  for (Instruction op : result) {
    if (op.opcode == spv::OpTypeArray) {
      CHECK(op.args[0] == new_array_tid);
      array_pos = pos;
    }
    else if (op.opcode == spv::OpTypePointer && op.args[2] == new_array_tid) {
      pointer_pos = pos;
    }
    ++pos;
  }
  REQUIRE(array_pos >= 0);
  REQUIRE(pointer_pos >= 0);
  CHECK(array_pos < pointer_pos);

  // The variable resolves to the new array type.
  CHECK(transformed.resolve_type(id_var) ==
        ShaderType::register_type(ShaderType::Array(ShaderType::FLOAT, 3)));
}

TEST_CASE("SpirVModule renumbers member decorations when deleting a member", "[shaderpipeline]") {
  // A uniform block with three members; deleting the middle one must shift
  // the third member's name and offset decoration down.
  SpirVModule module = make_module();

  ShaderType::Struct block;
  block.add_member(ShaderType::FLOAT, "a");
  block.add_member(ShaderType::INT, "b");
  block.add_member(ShaderType::FLOAT, "c");
  const ShaderType *block_type = ShaderType::register_type(std::move(block));

  Id id_block = module.define_variable(block_type, spv::StorageClassUniform);
  Id block_tid = module.find_type(block_type);
  REQUIRE(block_tid != 0);
  module.decorate(block_tid, spv::DecorationBlock);
  module.decorate(id_block, spv::DecorationBinding, 0u);
  module.decorate(id_block, spv::DecorationDescriptorSet, 0u);
  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    builder.op_return();
  }

  int offset_c = module.get_member_offset(block_tid, 2);

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  // Delete the middle struct member by its current index.
  SpirVModule transformed(stream);
  transformed.delete_struct_member(block_tid, 1);
  transformed.deduplicate_types();
  CHECK(transformed.validate());

  InstructionStream result = transformed.emit();

  Instruction op = find_op(result, spv::OpTypeStruct);
  REQUIRE(op.opcode == spv::OpTypeStruct);
  CHECK(op.nargs == 3);  // result id + two members

  // The member names shifted down along with the members.
  const SpirVModule &out_module = transformed;
  REQUIRE(out_module.get_num_members(block_tid) == 2);
  CHECK(out_module.get_member_name(block_tid, 0) == "a");
  CHECK(out_module.get_member_name(block_tid, 1) == "c");

  // Member decorations were renumbered, keeping c's original offset.
  int num_member_decorations = 0;
  for (Instruction ann : result) {
    if (ann.opcode == spv::OpMemberDecorate && ann.args[0] == block_tid) {
      ++num_member_decorations;
      REQUIRE(ann.args[1] <= 1);
      if (ann.args[1] == 1 && ann.args[2] == spv::DecorationOffset) {
        CHECK(ann.args[3] == (uint32_t)offset_c);
      }
    }
  }
  CHECK(num_member_decorations == 2);
}

TEST_CASE("SpirVModule reuses live constants but not deleted ones", "[shaderpipeline]") {
  SpirVModule module = make_module();

  Id id_c7 = module.define_int_constant(7);

  // Reference the constant so that it is not dead weight (not required, but
  // more realistic).
  Id id_var = module.define_variable(ShaderType::INT, spv::StorageClassPrivate);
  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    builder.op_store(id_var, id_c7);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SECTION("an existing constant is reused") {
    // Define an integer constant and reuse the existing declaration.
    SpirVModule transformed(stream);
    Id const_id = transformed.define_int_constant(7);
    transformed.deduplicate_types();
    CHECK(transformed.validate());
    CHECK(const_id == id_c7);

    InstructionStream result = transformed.emit();
    CHECK(count_op(result, spv::OpConstant) == 1);
  }

  SECTION("a deleted constant is never handed back out") {
    // Build a variant module in which the constant is unused, so that the
    // declaration may be deleted before requesting a constant with the same
    // value.
    SpirVModule module2 = make_module();
    Id id_unused = module2.define_int_constant(7);
    {
      SpirVBuilder builder = make_entry_point(module2, spv::ExecutionModelFragment);
      builder.op_return();
    }

    InstructionStream stream2 = module2.emit();
    REQUIRE(stream2.validate());

    // Delete an existing constant before requesting one with the same value.
    SpirVModule transformed(stream2);
    transformed.delete_id(id_unused);
    Id const_id = transformed.define_int_constant(7);
    transformed.deduplicate_types();
    CHECK(transformed.validate());
    CHECK(const_id != id_unused);
    CHECK(const_id != 0);

    InstructionStream result = transformed.emit();
    CHECK(count_op(result, spv::OpConstant) == 1);
  }
}

TEST_CASE("SpirVModule round-trips a module through parse and emit", "[shaderpipeline]") {
  // Parse-emit round trip: the metadata sections are reconstructed from the
  // materialized state, so this checks that nothing is lost along the way.
  SpirVModule module = make_module();

  const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));
  Id id_in = module.define_variable(vec4_type, spv::StorageClassInput);
  Id id_out = module.define_variable(vec4_type, spv::StorageClassOutput);
  module.set_name(id_in, "color_in");
  module.set_name(id_out, "color_out");
  module.decorate(id_in, spv::DecorationLocation, 3u);
  module.decorate(id_out, spv::DecorationLocation, 0u);
  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment, {id_in, id_out});
    builder.op_store(id_out, builder.op_load(id_in));
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVModule reparsed(stream);
  InstructionStream stream2 = reparsed.emit();
  REQUIRE(stream2.validate());

  CHECK(stream.get_data_size() == stream2.get_data_size());
  CHECK(count_op(stream2, spv::OpName) == 2);
  CHECK(count_op(stream2, spv::OpDecorate) == 2);
  CHECK(count_op(stream2, spv::OpVariable) == 2);
  CHECK(count_op(stream2, spv::OpLoad) == 1);
  CHECK(count_op(stream2, spv::OpStore) == 1);

  CHECK(reparsed.get_name(id_in) == "color_in");
  CHECK(reparsed.get_location(id_in) == 3);
  CHECK(reparsed.resolve_type(id_in) == vec4_type);
  CHECK(reparsed.get_storage_class(id_in) == spv::StorageClassInput);
}

TEST_CASE("SpirVModule stores function parameters structurally", "[shaderpipeline]") {
  SpirVModule module = make_module();
  Id float_tid = module.define_type(ShaderType::FLOAT);
  TestFunction fn = make_float_function(module, {float_tid, float_tid});
  {
    SpirVBuilder builder = make_entry_point(module);
    builder.op_return();
  }

  SpirVModule::Function *function = module.find_function(fn.func_id);
  REQUIRE(function != nullptr);
  CHECK(function->parameters == fn.param_ids);
  REQUIRE(function->instructions.size() == 2);
  CHECK(function->instructions[0].opcode == spv::OpLabel);
  CHECK(function->instructions[1].opcode == spv::OpReturnValue);

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());
  CHECK(count_op(stream, spv::OpFunctionParameter) == 2);

  SpirVModule reparsed(stream);
  SpirVModule::Function *reparsed_function =
    reparsed.find_function(fn.func_id);
  REQUIRE(reparsed_function != nullptr);
  CHECK(reparsed_function->parameters == fn.param_ids);
  for (const SpirVModule::Instruction &op : reparsed_function->instructions) {
    CHECK(op.opcode != spv::OpFunctionParameter);
  }
  CHECK(reparsed.validate());
}

TEST_CASE("SpirVModule canonicalizes debug instructions after parameters", "[shaderpipeline]") {
  SpirVModule module = make_module();
  Id float_tid = module.define_type(ShaderType::FLOAT);
  TestFunction fn = make_float_function(module, {float_tid, float_tid});
  {
    SpirVBuilder builder = make_entry_point(module);
    builder.op_return();
  }
  InstructionStream stream = module.emit();

  // Put a debug line instruction between the parameters to verify that
  // parsing extracts only the parameters and emission places the retained
  // debug instruction after the synthesized parameter list.
  InstructionStream::iterator it = stream.begin_functions();
  ++it;
  REQUIRE((*it).opcode == spv::OpFunctionParameter);
  ++it;
  REQUIRE((*it).opcode == spv::OpFunctionParameter);
  stream.insert(it, spv::OpNoLine, {});

  SpirVModule reparsed(stream);
  SpirVModule::Function *function = reparsed.find_function(fn.func_id);
  REQUIRE(function != nullptr);
  CHECK(function->parameters == fn.param_ids);
  REQUIRE(function->instructions.size() == 3);
  CHECK(function->instructions[0].opcode == spv::OpNoLine);
  CHECK(function->instructions[1].opcode == spv::OpLabel);

  InstructionStream canonical = reparsed.emit();
  REQUIRE(canonical.validate());
  it = canonical.begin_functions();
  ++it;
  CHECK((*it).opcode == spv::OpFunctionParameter);
  ++it;
  CHECK((*it).opcode == spv::OpFunctionParameter);
  ++it;
  CHECK((*it).opcode == spv::OpNoLine);
  ++it;
  CHECK((*it).opcode == spv::OpLabel);
}

TEST_CASE("SpirVModule classifies OpSpecConstantOp as a spec constant", "[shaderpipeline]") {
  // An array sized by a spec constant expression (2 * spec constant), which
  // is the shape glslang emits for spec-constant-sized arrays.  Parsing and
  // resolving such a module must not trip any assertion: the expression is a
  // SPEC_CONSTANT whose value resolve_constant reports as 0.
  SpirVModule module = make_module();

  Id id_spec = module.define_spec_constant(ShaderType::INT, 4);
  module.decorate(id_spec, spv::DecorationSpecId, 12u);

  Id int_tid = module.define_type(ShaderType::INT);
  Id id_c2 = module.define_int_constant(2);
  Id id_size = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpSpecConstantOp,
    {int_tid, id_size, (uint32_t)spv::OpIMul, id_spec, id_c2}));

  const ShaderType *float_type = ShaderType::FLOAT;
  Id float_tid = module.define_type(float_type);
  Id array_tid = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpTypeArray,
    {array_tid, float_tid, id_size}));

  Id ptr_tid = module.define_pointer_type(array_tid, spv::StorageClassPrivate);
  Id var_id = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpVariable,
    {ptr_tid, var_id, (uint32_t)spv::StorageClassPrivate}));

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelFragment);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVModule reparsed(stream);
  CHECK(reparsed.get_definition_type(id_size) == SpirVModule::DT_spec_constant);
  CHECK(reparsed.resolve_constant(id_size) == 0);

  // The array resolves (with unknowable size 0) rather than raising.
  const ShaderType *type = reparsed.resolve_type(array_tid);
  REQUIRE(type != nullptr);
  CHECK(type->as_array() != nullptr);

  // The reparsed module is still valid and the index is consistent.
  CHECK(reparsed.validate());
}

TEST_CASE("SpirVModule records functions created by make_function immediately", "[shaderpipeline]") {
  // This must be true before emit/parse, because passes create wrapper
  // functions and may call them again in the same in-memory module.
  SpirVModule module = make_module();

  SpirVBuilder builder = module.make_function(nullptr);
  Id function_id = builder.get_current_function_id();

  CHECK(module.get_definition_type(function_id) == SpirVModule::DT_function);
  CHECK(module.get_function_id(function_id) == function_id);

  Id function_type_id = module.get_type_id(function_id);
  CHECK(function_type_id != 0);
  CHECK(module.get_definition_type(function_type_id) == SpirVModule::DT_function_type);

  builder.op_return();
  module.add_entry_point(spv::ExecutionModelFragment, function_id, "main");
  module.add_execution_mode(function_id, spv::ExecutionModeOriginUpperLeft);

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SpirVModule reparsed(stream);
  CHECK(reparsed.validate());
}

TEST_CASE("SpirVModule constructs non-void functions", "[shaderpipeline]") {
  SpirVModule module = make_module();
  Id value = module.define_float_constant(1.0f);

  SpirVBuilder function_builder = module.make_function(ShaderType::FLOAT);
  Id function_id = function_builder.get_current_function_id();
  Id function_type_id = module.get_type_id(function_id);
  function_builder.op_return_value(value);

  // An identical signature should reuse the existing OpTypeFunction.
  SpirVBuilder second_builder = module.make_function(ShaderType::FLOAT);
  CHECK(module.get_type_id(second_builder.get_current_function_id()) ==
        function_type_id);
  second_builder.op_return_value(value);

  SpirVBuilder main_builder = make_entry_point(module);
  Id call_id = main_builder.op_function_call(function_id);
  CHECK(module.get_type_id(call_id) == module.define_type(ShaderType::FLOAT));
  main_builder.op_return();

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());
  CHECK(count_op(stream, spv::OpTypeFunction) == 2);  // float(), void()
  CHECK(count_op(stream, spv::OpReturnValue) == 2);

  SpirVModule reparsed(stream);
  CHECK(reparsed.validate());
}

TEST_CASE("SpirVBuilder branch_endif terminates a fall-through block", "[shaderpipeline]") {
  // main() { if (true) { b = a; } }
  SpirVModule module = make_module();
  Id var_a = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id var_b = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id merge;
  {
    SpirVBuilder builder = make_entry_point(module);
    Id cond = module.define_constant(ShaderType::BOOL, 1);
    merge = builder.branch_if(cond);
    builder.op_store(var_b, builder.op_load(var_a));
    builder.branch_endif(merge);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  CHECK(stream.validate());

  // The store does not terminate the true block, so branch_endif must have
  // inserted the branch to the merge block itself.
  REQUIRE(count_op(stream, spv::OpBranch) == 1);
  Instruction branch = find_op(stream, spv::OpBranch);
  CHECK(branch.args[0] == merge);
}

TEST_CASE("SpirVBuilder positions at the start of a function body", "[shaderpipeline]") {
  // A function whose entry block declares a local variable; code inserted at
  // the body start must land after the OpVariable, not before it.
  SpirVModule module = make_module();
  Id var_in = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  SpirVBuilder main_builder = make_entry_point(module);
  Id main_id = main_builder.get_current_function_id();
  main_builder.op_return();
  {
    // Add a function-local variable right after the entry label.
    SpirVModule::Function *function = module.find_function(main_id);
    REQUIRE(function != nullptr);
    size_t label_index = 0;
    while (function->instructions[label_index].opcode != spv::OpLabel) {
      ++label_index;
    }
    SpirVBuilder builder(module, main_id, label_index + 1);
    Id local_ptr_tid = module.define_pointer_type(ShaderType::FLOAT, spv::StorageClassFunction);
    Id local_id = module.allocate_id();
    builder.insert(spv::OpVariable, {local_ptr_tid, local_id, (uint32_t)spv::StorageClassFunction});

    // Now insert a store "at the body start"; it must go after the variable.
    SpirVBuilder body_builder(module);
    body_builder.set_insertion_point_to_body_start(main_id);
    body_builder.op_store(local_id, body_builder.op_load(var_in));
  }

  InstructionStream stream = module.emit();
  CHECK(stream.validate());
}

TEST_CASE("SpirVModule tracks type canonicality", "[shaderpipeline]") {
  // A type declaration is "canonical" if it is declared exactly the way
  // define_type() would declare it for its ShaderType; only canonical
  // declarations may be reused via find_type, since ShaderType does not
  // represent all SPIR-V type operands (bit widths, image depth/sampled/
  // format/multisample operands).  This module declares a zoo of canonical
  // and non-canonical variants; the canonical ones via define_type(), the
  // non-canonical ones as raw declaration instructions.  Note that it is
  // deliberately not validated, since the 16/64-bit types would require
  // extra capabilities that are irrelevant to what is being tested.
  SpirVModule module = make_module();

  // The 16-bit float is declared *before* the 32-bit one, to check that
  // canonicality does not depend on the declaration order for scalars.
  Id id_half = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpTypeFloat, {id_half, 16}));
  Id id_float = module.define_type(ShaderType::FLOAT);
  Id id_int = module.define_type(ShaderType::INT);
  Id id_long = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpTypeInt, {id_long, 64, 1}));

  const ShaderType *v4float_type =
    ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));
  Id id_v4float = module.define_type(v4float_type);
  Id id_v4half = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpTypeVector, {id_v4half, id_half, 4}));

  const ShaderType *array_float_type =
    ShaderType::register_type(ShaderType::Array(ShaderType::FLOAT, 2));
  Id id_array_float = module.define_type(array_float_type);
  Id id_const_2 = module.define_int_constant(2);
  Id id_array_half = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpTypeArray, {id_array_half, id_half, id_const_2}));

  // A shadow sampler: the image declaration differs from the canonical
  // (storage image) declaration for its ShaderType, but the sampled image
  // type over it is exactly what define_type() would produce, since the
  // shadow flag is represented on ShaderType::SampledImage.
  const ShaderType *si_shadow_type = ShaderType::register_type(
    ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float, true));
  Id id_si_shadow = module.define_type(si_shadow_type);
  Id id_image_shadow = module.get_type_id(id_si_shadow);

  // Multisampledness is not represented on ShaderType at all, so neither the
  // image nor the sampled image over it may be considered canonical.
  Id id_image_ms = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpTypeImage,
    {id_image_ms, id_float, (uint32_t)spv::Dim2D, 0, 0, 1, 1,
     (uint32_t)spv::ImageFormatUnknown}));
  Id id_si_ms = module.allocate_id();
  module.add_declaration(SpirVModule::Instruction(spv::OpTypeSampledImage, {id_si_ms, id_image_ms}));

  // This is exactly how define_type() declares an image type...
  const ShaderType *image_2d_type = ShaderType::register_type(ShaderType::Image(
    Texture::TT_2d_texture, ShaderType::ST_float, ShaderType::Access::NONE));
  Id id_image_storage = module.define_type(image_2d_type);
  // ...and this differs only in the format operand, which ShaderType does not
  // represent.  Note that all three 2D images map to the same ShaderType.
  Id id_image_rgba8 =
    module.define_image_type(image_2d_type, 2, 2, spv::ImageFormatRgba8);

  // Emit the module and parse it back, so that we are checking the state as
  // freshly derived by the parser, not as maintained by define_type().
  InstructionStream stream = module.emit();
  SpirVModule parsed(stream);

  // Scalars: only the widths that define_type() emits are canonical.
  CHECK(!parsed.is_canonical_type(id_half));
  CHECK(parsed.is_canonical_type(id_float));
  CHECK(parsed.is_canonical_type(id_int));
  CHECK(!parsed.is_canonical_type(id_long));

  // Composites: canonicality must cascade from the component type, or a
  // vec4 of 16-bit floats would be handed out for a request for a regular
  // vec4 (both map to the same ShaderType).
  CHECK(parsed.is_canonical_type(id_v4float));
  CHECK(!parsed.is_canonical_type(id_v4half));
  CHECK(parsed.is_canonical_type(id_array_float));
  CHECK(!parsed.is_canonical_type(id_array_half));

  // Images and sampled images.
  CHECK(!parsed.is_canonical_type(id_image_shadow));
  CHECK(parsed.is_canonical_type(id_si_shadow));
  CHECK(!parsed.is_canonical_type(id_image_ms));
  CHECK(!parsed.is_canonical_type(id_si_ms));
  CHECK(parsed.is_canonical_type(id_image_storage));
  CHECK(!parsed.is_canonical_type(id_image_rgba8));
}
