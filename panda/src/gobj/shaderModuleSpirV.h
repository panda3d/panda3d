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
#include "spirv.hpp"

class ShaderType;

/**
 * ShaderModule that contains compiled SPIR-V bytecode.  This class can extract
 * the parameter definitions from the bytecode, assign appropriate locations,
 * link the module to a previous stage, and strip debug information as needed.
 */
class EXPCL_PANDA_GOBJ ShaderModuleSpirV final : public ShaderModule {
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
  virtual void remap_parameter_locations(const pmap<int, int> &remap) override;

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

    INLINE Instruction operator *();
    INLINE InstructionIterator &operator ++();
    INLINE bool operator ==(const InstructionIterator &other) const;
    INLINE bool operator !=(const InstructionIterator &other) const;

  private:
    INLINE InstructionIterator(uint32_t *words);

    uint32_t *_words = nullptr;

    friend class InstructionStream;
  };

  /**
   * A container that allows conveniently iterating over the instructions.
   */
  class InstructionStream {
  public:
    typedef InstructionIterator iterator;

    InstructionStream() = default;
    INLINE InstructionStream(const uint32_t *words, size_t num_words);
    INLINE InstructionStream(std::vector<uint32_t> words);

    bool validate_header() const;

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

  private:
    // We're not using a pvector since glslang/spirv-opt are working with
    // std::vector<uint32_t> and so we can avoid some unnecessary copies.
    std::vector<uint32_t> _words;
  };

  InstructionStream _instructions;

  enum DefinitionType {
    DT_none,
    DT_type,
    DT_type_pointer,
    DT_variable,
    DT_constant,
    DT_ext_inst,
    DT_function_parameter,
    DT_function,
    DT_temporary,
    DT_spec_constant,
  };

  enum DefinitionFlags {
    DF_used = 1,

    // Set for image types that have the "depth" flag set
    DF_depth_image = 2,

    // Set on variables to indicate that they were used by a texture sample op,
    // respectively one with and without depth comparison
    DF_dref_sampled = 4,
    DF_non_dref_sampled = 8,

    // Set if we know for sure that this can be const-evaluated.
    DF_constant_expression = 16,

    // Set for arrays that are indexed with a non-const index.
    DF_dynamically_indexed = 32,

    // Has the "buffer block" decoration (older versions of SPIR-V).
    DF_buffer_block = 64,

    // If both of these are set, no access is permitted (size queries only)
    DF_non_writable = 128, // readonly
    DF_non_readable = 256, // writeonly
  };

  /**
   * Used by below Definition struct to hold member info.
   */
  struct MemberDefinition {
    std::string _name;
    uint32_t _type_id = 0;
    int _location = -1;
    int _offset = -1;
    spv::BuiltIn _builtin = spv::BuiltInMax;
    int _flags = 0; // Only readonly/writeonly
  };
  typedef pvector<MemberDefinition> MemberDefinitions;

  /**
   * Temporary structure to hold a single definition, which could be a variable,
   * type or type pointer in the SPIR-V file.
   */
  struct Definition {
    DefinitionType _dtype = DT_none;
    std::string _name;
    const ShaderType *_type = nullptr;
    int _location = -1;
    spv::BuiltIn _builtin = spv::BuiltInMax;
    uint32_t _constant = 0;
    uint32_t _type_id = 0;
    uint32_t _array_stride = 0;
    uint32_t _origin_id = 0; // set for loads, tracks original variable ID
    uint32_t _function_id = 0;
    uint32_t _spec_id = 0;
    MemberDefinitions _members;
    int _flags = 0;

    // Only defined for DT_variable and DT_type_pointer.
    spv::StorageClass _storage_class;

    INLINE bool is_used() const;
    INLINE bool is_builtin() const;
    INLINE bool has_location() const;
    bool has_builtin() const;
    const MemberDefinition &get_member(uint32_t i) const;
    MemberDefinition &modify_member(uint32_t i);
    void clear();
  };
  typedef pvector<Definition> Definitions;

  /**
   * An InstructionWriter can be used for more advanced transformations on a
   * SPIR-V instruction stream.  It sets up temporary support structures that
   * help make changes more efficiently.  Only one writer to a given stream may
   * exist at any given time, and the stream may not be modified by other means
   * in the meantime.
   */
  class InstructionWriter {
  public:
    InstructionWriter(InstructionStream &stream);

    uint32_t find_definition(const std::string &name) const;
    const Definition &get_definition(uint32_t id) const;
    Definition &modify_definition(uint32_t id);

    void assign_locations(Stage stage);
    void bind_descriptor_set(uint32_t set, const vector_int &locations);
    void remove_unused_variables();

    void flatten_struct(uint32_t type_id);
    uint32_t make_block(const ShaderType::Struct *block_type, const pvector<int> &locations,
                        spv::StorageClass storage_class, uint32_t binding=0, uint32_t set=0);

    void set_variable_type(uint32_t id, const ShaderType *type);

    uint32_t find_type_pointer(const ShaderType *type, spv::StorageClass storage_class);
    uint32_t define_variable(const ShaderType *type, spv::StorageClass storage_class);
    uint32_t define_type_pointer(const ShaderType *type, spv::StorageClass storage_class);
    uint32_t define_type(const ShaderType *type);
    uint32_t define_constant(const ShaderType *type, uint32_t constant);

  private:
    uint32_t r_define_variable(InstructionIterator &it, const ShaderType *type, spv::StorageClass storage_class);
    uint32_t r_define_type_pointer(InstructionIterator &it, const ShaderType *type, spv::StorageClass storage_class);
    uint32_t r_define_type(InstructionIterator &it, const ShaderType *type);
    uint32_t r_define_constant(InstructionIterator &it, const ShaderType *type, uint32_t constant);
    void r_annotate_struct_layout(InstructionIterator &it, uint32_t type_id);

    void parse_instruction(const Instruction &op, uint32_t &current_function_id);
    void record_type(uint32_t id, const ShaderType *type);
    void record_type_pointer(uint32_t id, spv::StorageClass storage_class, uint32_t type_id);
    void record_variable(uint32_t id, uint32_t type_pointer_id, spv::StorageClass storage_class, uint32_t function_id=0);
    void record_function_parameter(uint32_t id, uint32_t type_id, uint32_t function_id);
    void record_constant(uint32_t id, uint32_t type_id, const uint32_t *words, uint32_t nwords);
    void record_ext_inst_import(uint32_t id, const char *import);
    void record_function(uint32_t id, uint32_t type_id);
    void record_temporary(uint32_t id, uint32_t type_id, uint32_t from_id, uint32_t function_id);
    void record_spec_constant(uint32_t id, uint32_t type_id);

    void mark_used(uint32_t id);

    InstructionStream &_instructions;
    Definitions _defs;

    // Reverse mapping from type to ID.  Excludes types with BuiltIn decoration.
    typedef pmap<const ShaderType *, uint32_t> TypeMap;
    TypeMap _type_map;
  };

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
