/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVTransformPass.h
 * @author rdb
 * @date 2024-10-08
 */

#ifndef SPIRVTRANSFORMPASS_H
#define SPIRVTRANSFORMPASS_H

#include "shaderModuleSpirV.h"
#include "spirVResultDatabase.h"

/**
 * Subclassed in order to provide a specific transformation that can be run
 * through the SpirVTransformer.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVTransformPass {
public:
  friend class SpirVTransformer;

  using Definition = SpirVResultDatabase::Definition;
  using MemberDefinition = SpirVResultDatabase::MemberDefinition;
  using Instruction = ShaderModuleSpirV::Instruction;
  using InstructionStream = ShaderModuleSpirV::InstructionStream;
  using InstructionIterator = ShaderModuleSpirV::InstructionIterator;

  SpirVTransformPass();

  void process_preamble(std::vector<uint32_t> &instructions);
  void process_annotations(std::vector<uint32_t> &instructions);
  void process_definitions(std::vector<uint32_t> &instructions);
  void process_functions(std::vector<uint32_t> &instructions);
  InstructionStream get_result() const;

  virtual void preprocess();
  virtual bool transform_debug_op(Instruction op);
  virtual bool transform_annotation_op(Instruction op);
  virtual bool transform_definition_op(Instruction op);
  virtual bool begin_function(Instruction op);
  virtual bool transform_function_op(Instruction op, uint32_t function_id);
  virtual void end_function(uint32_t function_id);
  virtual void postprocess();

  INLINE uint32_t get_type_id(uint32_t id) const;
  INLINE uint32_t unwrap_pointer_type(uint32_t id) const;

  INLINE uint32_t resolve_constant(uint32_t id) const;
  INLINE const ShaderType *resolve_type(uint32_t id) const;
  INLINE const ShaderType *resolve_pointer_type(uint32_t id) const;

  INLINE uint32_t get_id_bound() const;
  INLINE uint32_t allocate_id();

  void add_name(uint32_t id, const std::string &name);

  void delete_id(uint32_t id);
  void delete_struct_member(uint32_t id, uint32_t member_index);
  void delete_function_parameter(uint32_t type_id, uint32_t param_index);

  INLINE bool is_deleted(uint32_t id) const;
  INLINE bool is_member_deleted(uint32_t id, uint32_t member) const;

  uint32_t define_variable(const ShaderType *type, spv::StorageClass storage_class);
  uint32_t define_pointer_type(const ShaderType *type, spv::StorageClass storage_class);
  uint32_t define_type(const ShaderType *type);
  uint32_t define_int_constant(int32_t constant);
  uint32_t define_constant(const ShaderType *type, uint32_t constant);

  /**
   * Helper class for storing a chain of member or array accesses.
   */
  class AccessChain {
  public:
    AccessChain(uint32_t var_id) : _var_id(var_id) {}
    AccessChain(uint32_t var_id, std::initializer_list<uint32_t> chain) : _var_id(var_id), _chain(std::move(chain)) {}

    INLINE void prepend(uint32_t id);
    INLINE void append(uint32_t id);
    INLINE void extend(const AccessChain &other);

    INLINE bool startswith(const AccessChain &other) const;
    INLINE bool operator < (const AccessChain &other) const;

    uint32_t operator [] (size_t i) const { return _chain[i]; }
    size_t size() const { return _chain.size(); }

    INLINE void output(std::ostream &out) const;

  public:
    uint32_t _var_id;
    pvector<uint32_t> _chain;
  };

protected:
  void r_annotate_struct_layout(uint32_t type_id);

  INLINE bool is_defined(uint32_t id) const;
  INLINE void mark_defined(uint32_t id);
  INLINE void mark_used(uint32_t id);

  INLINE void add_debug(spv::Op opcode, std::initializer_list<uint32_t> args);
  INLINE void add_debug(spv::Op opcode, const uint32_t *args, uint16_t nargs);

  INLINE void add_annotation(spv::Op opcode, std::initializer_list<uint32_t> args);
  INLINE void add_annotation(spv::Op opcode, const uint32_t *args, uint16_t nargs);

  INLINE void add_definition(spv::Op opcode, std::initializer_list<uint32_t> args);
  void add_definition(spv::Op opcode, const uint32_t *args, uint16_t nargs);

  INLINE void add_instruction(spv::Op opcode, std::initializer_list<uint32_t> args);
  void add_instruction(spv::Op opcode, const uint32_t *args, uint16_t nargs);

  // The module is split into sections to make it easier to add instructions
  // to other sections while we are iterating.
  std::vector<uint32_t> _new_preamble;
  std::vector<uint32_t> _new_annotations;
  std::vector<uint32_t> _new_definitions;
  std::vector<uint32_t> _new_functions;

  // Keeps track of what has been defined and deleted during this pass.
  BitArray _defined;
  pset<uint32_t> _deleted_ids;
  pmap<uint32_t, pset<uint32_t> > _deleted_members;
  pmap<uint32_t, pset<uint32_t> > _deleted_function_parameters;

  SpirVResultDatabase _db;
};

INLINE std::ostream &operator << (std::ostream &out, const SpirVTransformPass::AccessChain &obj);

#include "spirVTransformPass.I"

#endif
