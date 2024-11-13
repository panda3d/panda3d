/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModuleSpirV.h
 * @author rdb
 * @date 2019-07-15
 */

#ifndef SHADERMODULESPIRV_H
#define SHADERMODULESPIRV_H

#include "shader.h"
#include "bitArray.h"

#ifdef BUILDING_PANDA_SHADERPIPELINE
#define SPV_ENABLE_UTILITY_CODE
#endif
#include "spirv.hpp"

#include <spirv-tools/libspirv.h>

class ShaderType;

/**
 * ShaderModule that contains compiled SPIR-V bytecode.  This class can extract
 * the parameter definitions from the bytecode, assign appropriate locations,
 * link the module to a previous stage, and strip debug information as needed.
 */
class EXPCL_PANDA_SHADERPIPELINE ShaderModuleSpirV final : public ShaderModule {
private:
  ShaderModuleSpirV(Stage stage);

public:
  ShaderModuleSpirV(Stage stage, std::vector<uint32_t> words, BamCacheRecord *record = nullptr);
  virtual ~ShaderModuleSpirV();

  virtual PT(CopyOnWriteObject) make_cow_copy() override;

  INLINE const uint32_t *get_data() const;
  INLINE size_t get_data_size() const;

  virtual bool link_inputs(const ShaderModule *previous, pmap<int, int> &remap) const override;
  virtual void remap_input_locations(const pmap<int, int> &remap) override;

  virtual std::string get_ir() const override;

  class InstructionStream;

  /**
   * A single instruction as returned by the InstructionIterator.
   */
  struct Instruction {
    const spv::Op opcode;
    const uint32_t nargs;
    uint32_t *args;

    INLINE bool is_debug() const;
    INLINE bool is_annotation() const;
  };

  /**
   * Provided by InstructionStream to iterate over the instructions.
   */
  class InstructionIterator {
  public:
    constexpr InstructionIterator() = default;
    INLINE InstructionIterator(uint32_t *words);

    INLINE Instruction operator *();
    INLINE InstructionIterator &operator ++();
    INLINE bool operator ==(const InstructionIterator &other) const;
    INLINE bool operator !=(const InstructionIterator &other) const;
    INLINE InstructionIterator next() const;

    uint32_t *_words = nullptr;
  };

  /**
   * A container that allows conveniently iterating over the instructions.
   */
  class EXPCL_PANDA_SHADERPIPELINE InstructionStream {
  public:
    typedef InstructionIterator iterator;

    InstructionStream() = default;
    INLINE InstructionStream(const uint32_t *words, size_t num_words);
    INLINE InstructionStream(std::vector<uint32_t> words);

    bool validate_header() const;
    bool validate(spv_target_env env = SPV_ENV_UNIVERSAL_1_0) const;
    bool disassemble(std::ostream &out) const;

    INLINE operator std::vector<uint32_t> & ();

    INLINE iterator begin();
    INLINE iterator begin_annotations();
    INLINE iterator end_annotations();
    INLINE iterator begin_functions();
    INLINE iterator end();
    INLINE iterator insert(const iterator &it, spv::Op opcode, std::initializer_list<uint32_t > args);
    INLINE iterator insert(const iterator &it, spv::Op opcode, const uint32_t *args, uint16_t nargs);
    INLINE iterator insert(const iterator &it, const Instruction &op);
    INLINE iterator insert_arg(const iterator &it, uint16_t arg_index, uint32_t arg);
    INLINE iterator erase(const iterator &it);
    INLINE iterator erase_arg(const iterator &it, uint16_t arg);

    INLINE const uint32_t *get_data() const;
    INLINE size_t get_data_size() const;

    INLINE uint32_t get_id_bound() const;
    INLINE uint32_t allocate_id();

  public:
    // We're not using a pvector since glslang/spirv-opt are working with
    // std::vector<uint32_t> and so we can avoid some unnecessary copies.
    std::vector<uint32_t> _words;
  };

  InstructionStream _instructions;

  pmap<uint32_t, const ShaderType::Struct *> _uniform_struct_types;

private:
  void remap_locations(spv::StorageClass storage_class, const pmap<int, int> &locations);
  void strip();

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager) override;

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager) override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderModule::init_type();
    register_type(_type_handle, "ShaderModuleSpirV",
                  ShaderModule::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "shaderModuleSpirV.I"

#endif
