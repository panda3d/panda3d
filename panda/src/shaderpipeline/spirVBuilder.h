/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVBuilder.h
 * @author rdb
 * @date 2026-07-07
 */

#ifndef SPIRVBUILDER_H
#define SPIRVBUILDER_H

#include "spirVModule.h"
#include "spirVValue.h"
#include "spirVPointer.h"

/**
 * Builds SPIR-V function bodies, inserting instructions into a SpirVModule.
 * Operations are inserted at the current cursor, which advances past them.
 *
 * Also offers convenient forwarders for defining variables and constants,
 * which return SpirVPointer or SpirVValue wrappers.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVBuilder {
public:
  using Instruction = SpirVModule::Instruction;
  using Function = SpirVModule::Function;
  using Pointer = SpirVPointer;
  using Value = SpirVValue;
  using Id = SpirVId;

  explicit SpirVBuilder(SpirVModule &module);
  SpirVBuilder(SpirVModule &module, Id function_id, size_t index);

  INLINE SpirVModule &get_module();

  void set_insertion_point(Id function_id, size_t index);
  void set_insertion_point_to_body_start(Id function_id);
  INLINE size_t get_insertion_index() const;
  Function *get_current_function();
  INLINE Id get_current_function_id() const;

  INLINE Id allocate_id();

  void insert(Instruction op);
  INLINE void insert(spv::Op opcode, std::initializer_list<uint32_t> args);
  void insert(spv::Op opcode, const uint32_t *args, size_t nargs);

  // Convenience forwarders returning a Pointer.
  Pointer define_uniform(const ShaderType *type, std::string name);
  Pointer define_input(const ShaderType *type, std::string name, int location);
  Pointer define_output(const ShaderType *type, std::string name, int location);
  Pointer define_private(const ShaderType *type, std::string name = "");
  INLINE Pointer ensure_builtin_input(spv::BuiltIn builtin);
  INLINE Pointer ensure_builtin_output(spv::BuiltIn builtin);

  // Const value constructors are defined here since vector constructors in
  // spirVValue.h cannot take literals (Value requires a builder).
  INLINE Value const_bool(bool value);
  INLINE Value const_int(int32_t value);
  INLINE Value const_uint(uint32_t value);
  INLINE Value const_float(float value);
  Value const_vec2(float x, float y);
  Value const_vec3(float x, float y, float z);
  Value const_vec4(float x, float y, float z, float w);

  // Generic constructor for any (non-const) type.
  INLINE Value construct(const ShaderType *type, std::initializer_list<glsl::Argument> constituents);
  Value construct(const ShaderType *type, const pvector<Value> &constituents);
  Value construct(const ShaderType *type, const glsl::Argument *constituents, size_t num_args);
  Value construct(const ShaderType *type, glsl::Argument value);

  // Low-level functions inserting instructions that directly take Ids;
  // generally, use the higher-level Pointer/Value API instead.
  Id op_load(Id var_id, spv::MemoryAccessMask access = spv::MemoryAccessMaskNone);
  void op_store(Id var_id, Id value, spv::MemoryAccessMask access = spv::MemoryAccessMaskNone);
  INLINE Id op_access_chain(Id var_id, std::initializer_list<Id> chain);
  INLINE Id op_access_chain(Id var_id, const pvector<Id> &chain);
  Id op_access_chain(Id var_id, const Id *chain, size_t count);
  Id op_function_call(Id func_id);
  Id op_image_sample(Id sampled_image, Id coord, uint32_t operands,
                     const uint32_t *operand_ids);

  // Block instructions.
  Id op_label();
  void op_branch(Id target);
  void op_return();
  void op_return_value(Id value);
  void op_kill();

  Id branch_if(Id cond);
  void branch_endif(Id label);
  void kill_if(Id cond);

private:
  SpirVModule &_module;

  size_t _function_index = (size_t)-1;
  size_t _index = 0;
  SpirVId _current_function_id;
};

#include "spirVBuilder.I"

#endif
