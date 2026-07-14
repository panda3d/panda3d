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

/**
 * Builds SPIR-V function bodies, inserting instructions into a SpirVModule.
 * Operations are inserted at the current cursor, which advances past them.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVBuilder {
public:
  using Instruction = SpirVModule::Instruction;
  using Function = SpirVModule::Function;
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

  Id op_load(Id var_id, spv::MemoryAccessMask access = spv::MemoryAccessMaskNone);
  void op_store(Id var_id, Id value);
  Id op_select(Id cond, Id obj1, Id obj2);
  Id op_access_chain(Id var_id, std::initializer_list<Id> chain);
  Id op_vector_shuffle(Id vec1, Id vec2, const pvector<uint32_t> &components);
  Id op_composite_construct(const ShaderType *type, const pvector<Id> &constituents);
  Id op_composite_extract(Id obj_id, std::initializer_list<uint32_t> chain);
  Id op_composite_insert(Id obj_id, Id composite_id, std::initializer_list<uint32_t> chain);
  Id op_compare(spv::Op opcode, Id obj1, Id obj2);
  Id op_convert(ShaderType::ScalarType to_scalar_type, Id value);
  Id op_bitcast(const ShaderType *type, Id value);
  Id op_transpose(Id obj);
  Id op_add(Id left, Id right);
  Id op_sub(Id left, Id right);
  Id op_div(Id left, Id right);
  Id op_dot(Id left, Id right);
  Id op_negate(Id value);
  Id op_multiply(Id left, Id right);
  Id op_image_sample(Id image, Id coord, uint32_t operands = 0u, const uint32_t *ids = nullptr);
  Id op_function_call(Id func_id);

  Id op_label();
  void op_branch(Id target);
  void op_return();
  void op_return_value(Id value);
  void op_kill();

  Id branch_if(Id cond);
  void branch_endif(Id label);

private:
  SpirVModule &_module;

  size_t _function_index = (size_t)-1;
  size_t _index = 0;
  SpirVId _current_function_id;
};

#include "spirVBuilder.I"

#endif
