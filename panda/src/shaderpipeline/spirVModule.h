/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVModule.h
 * @author rdb
 * @date 2026-07-07
 */

#ifndef SPIRVMODULE_H
#define SPIRVMODULE_H

#include "shaderModuleSpirV.h"
#include "spirVId.h"
#include "spirVUsageAnalysis.h"
#include "small_vector.h"

class SpirVBuilder;

/**
 * A SPIR-V module, materialized into a mutable in-memory representation that
 * transformation passes operate on.
 *
 * Instructions are parsed into module sections, where module-scope
 * declarations (types, constants and variables) go into a declarations
 * section and function instructions go into instruction vectors on separate
 * Function objects.  Usage information is computed on demand by
 * SpirVUsageAnalysis.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVModule {
public:
  using InstructionStream = ShaderModuleSpirV::InstructionStream;
  using Id = SpirVId;

  typedef small_vector<uint32_t, 4> Args;

  /**
   * A single instruction.
   */
  class EXPCL_PANDA_SHADERPIPELINE Instruction {
  public:
    Instruction() = default;
    INLINE explicit Instruction(spv::Op opcode);
    INLINE Instruction(spv::Op opcode, std::initializer_list<uint32_t> args);
    INLINE Instruction(spv::Op opcode, const uint32_t *args, size_t nargs);

    bool has_result() const;
    bool has_result_type() const;
    Id get_result() const;
    Id get_result_type() const;

    INLINE size_t size() const;
    INLINE uint32_t operator [] (size_t i) const;
    INLINE uint32_t &operator [] (size_t i);

    INLINE bool is_nop() const;

    std::string get_string(size_t arg_index, size_t *end_index = nullptr) const;

    spv::Op opcode = spv::OpNop;
    Args args;
  };

  /**
   * An annotation instruction; the target id is not stored in the args but
   * is implicit as the key in the _annotations map below.
   */
  struct Annotation {
    spv::Op opcode;
    Args args;

    INLINE bool is_member_annotation() const;
    INLINE spv::Decoration get_decoration() const;
  };
  typedef pvector<Annotation> Annotations;

  /**
   * An entry point record.  The interface list is not stored: emit() derives
   * it from the instructions (see collect_interface_vars).
   */
  struct EntryPoint {
    spv::ExecutionModel model;
    Id function_id;
    std::string name;
  };
  typedef pvector<EntryPoint> EntryPoints;

  /**
   * A function, its parameter result ids, and its body instructions.
   *
   * Parameter types are stored positionally on the OpTypeFunction declaration
   * identified by type_id.  OpFunction, OpFunctionParameter and OpFunctionEnd
   * are synthesized by emit() and are not present in instructions.
   */
  struct Function {
    Id id;
    Id type_id; // Function type, not return type!
    spv::FunctionControlMask control;
    pvector<Id> parameters;
    pvector<Instruction> instructions;

    INLINE const pvector<Id> &get_parameters() const;
  };
  typedef pvector<Function> Functions;

  /**
   * Classifies what an id refers to.  Note that DT_temporary covers every
   * typed result that isn't classified more specifically, DT_typeless every
   * typeless result.
   */
  enum DefinitionType {
    DT_none,
    DT_type,
    DT_pointer_type,
    DT_function_type,
    DT_variable,
    DT_constant,
    DT_ext_inst,
    DT_function_parameter,
    DT_function,
    DT_temporary,
    DT_typeless,
    DT_spec_constant,
    DT_string,
  };

  SpirVModule() = default;
  explicit SpirVModule(const InstructionStream &stream);

  // Module I/O and inspection.
  bool parse(const uint32_t *words, size_t num_words);
  InstructionStream emit() const;
  bool validate() const;
  SpirVUsageAnalysis analyze_usage() const;

  // Module sections.
  void add_capability(spv::Capability capability);
  void add_extension(std::string_view name);

  INLINE size_t get_num_ext_inst_imports() const;
  INLINE const Instruction &get_ext_inst_import(size_t i) const;
  Id add_ext_inst_import(std::string_view name);

  void set_memory_model(spv::AddressingModel addressing, spv::MemoryModel memory);

  INLINE size_t get_num_entry_points() const;
  INLINE const EntryPoint &get_entry_point(size_t i) const;
  INLINE EntryPoints &modify_entry_points();
  void add_entry_point(spv::ExecutionModel model, Id function_id,
                       std::string name);
  pvector<Id> collect_interface_vars(Id function_id) const;

  INLINE size_t get_num_execution_modes() const;
  INLINE const Instruction &get_execution_mode(size_t i) const;
  void add_execution_mode(Id function_id, spv::ExecutionMode mode,
                          std::initializer_list<uint32_t> args = {});

  const std::string &get_name(Id id) const;
  void set_name(Id id, std::string name);
  const std::string &get_member_name(Id type_id, uint32_t member_index) const;
  void set_member_name(Id type_id, uint32_t member_index, std::string name);
  Id find_named(std::string_view name) const;
  INLINE std::string resolve_string(Id id) const;
  Id add_string(std::string_view str);
  bool is_string_referenced(Id id) const;

  INLINE const Annotations *get_annotations(Id id) const;
  bool has_decoration(Id id, spv::Decoration decoration) const;
  uint32_t get_decoration(Id id, spv::Decoration decoration, uint32_t default_value = 0) const;
  bool has_member_decoration(Id type_id, uint32_t member_index, spv::Decoration decoration) const;
  uint32_t get_member_decoration(Id type_id, uint32_t member_index, spv::Decoration decoration, uint32_t default_value = 0) const;
  INLINE int get_location(Id id) const;
  void set_location(Id id, int location);
  INLINE spv::BuiltIn get_builtin(Id id) const;
  INLINE uint32_t get_spec_id(Id id) const;
  INLINE uint32_t get_array_stride(Id type_id) const;
  INLINE int get_member_offset(Id type_id, uint32_t member_index) const;
  INLINE spv::BuiltIn get_member_builtin(Id type_id, uint32_t member_index) const;

  INLINE void decorate(Id id, spv::Decoration decoration);
  INLINE void decorate(Id id, spv::Decoration decoration, uint32_t value);
  void decorate(Id id, const Annotation &annotation);
  void decorate_member(Id type_id, uint32_t member_index, spv::Decoration decoration);
  void decorate_member(Id type_id, uint32_t member_index, spv::Decoration decoration, uint32_t value);
  bool remove_decoration(Id id, spv::Decoration decoration);
  void set_decoration(Id id, spv::Decoration decoration, uint32_t value);
  void strip_decoration(spv::Decoration decoration);
  void transfer_annotations(Id from, Id to);

  INLINE size_t get_num_declarations() const;
  INLINE Instruction get_declaration(size_t i) const;
  const Instruction *find_declaration(Id id) const;
  Instruction *find_declaration(Id id);
  void add_declaration(Instruction op);

  INLINE size_t get_num_functions() const;
  INLINE const Function &get_function(size_t i) const;
  INLINE Functions &modify_functions();
  Function *find_function(Id function_id);
  SpirVBuilder make_function(const ShaderType *return_type,
                             const pvector<Id> &param_type_ids = pvector<Id>(),
                             spv::FunctionControlMask control = spv::FunctionControlMaskNone);
  SpirVBuilder make_entry_point(spv::ExecutionModel model, std::string name);

  // ID and definition inspection.
  INLINE uint32_t get_id_bound() const;
  INLINE Id allocate_id();
  INLINE DefinitionType get_definition_type(Id id) const;
  INLINE bool is_type(Id id) const;
  INLINE Id get_function_id(Id id) const;
  Id get_type_id(Id id) const;
  spv::StorageClass get_storage_class(Id id) const;
  uint32_t resolve_constant(Id id) const;
  Id unwrap_pointer_type(Id id) const;
  uint32_t get_num_members(Id type_id) const;
  Id get_member_type_id(Id type_id, uint32_t member_index) const;
  Id get_composite_member_type_id(Id type_id, uint32_t member_index) const;
  void record_result(const Instruction &op, Id function_id = {});
  bool get_instruction_id_operands(const Instruction &op,
                                   small_vector<uint16_t, 8> &indices) const;

  // ShaderType resolution.
  const ShaderType *resolve_type(Id id) const;
  Id find_type(const ShaderType *type) const;
  INLINE bool is_canonical_type(Id type_id) const;
  void collect_nested_structs(pmap<Id, const ShaderType::Struct *> &result,
                              Id type_id) const;
  void invalidate_types();

  // Definition helpers.
  Id define_type(const ShaderType *type);
  Id define_image_type(const ShaderType *type, uint32_t depth, uint32_t sampled, spv::ImageFormat format);
  Id define_pointer_type(Id type_id, spv::StorageClass storage_class);
  Id define_pointer_type(const ShaderType *type, spv::StorageClass storage_class);
  Id define_function_type(const ShaderType *return_type,
                          const pvector<Id> &param_type_ids = pvector<Id>());
  Id define_variable(const ShaderType *type, spv::StorageClass storage_class);
  Id define_constant(const ShaderType *type, uint32_t constant);
  Id define_int_constant(int32_t constant);
  INLINE Id define_float_constant(float constant);
  Id define_double_constant(double constant);
  Id define_null_constant(const ShaderType *type);
  Id define_spec_constant(const ShaderType *type, uint32_t def_value);
  INLINE Id ensure_builtin_input(spv::BuiltIn builtin);
  INLINE Id ensure_builtin_output(spv::BuiltIn builtin);

  // Structural transformations.
  void delete_id(Id id);
  void replace_type_id(Id before, Id after);
  void delete_struct_member(Id type_id, uint32_t member_index);
  void remove_function_parameters(Id type_id, const pset<uint32_t> &param_indices);
  void delete_dead_code(pset<Id> &dead_ids);
  void deduplicate_types();
  void reassign_declaration_id(Id old_id, Id new_id);

private:
  /**
   * Per-id record describing a particular result id.  This is the only per-id
   * state the module keeps; it holds what cannot be cheaply re-derived from
   * the instructions.
   */
  struct Definition {
    DefinitionType _dtype = DT_none;
    Id _type_id;

    // If it was declared inside a function, the id of the enclosing function.
    Id _function_id;

    // Index of the declaring instruction in the declarations section, or -1.
    // Every declarations-section mutation keeps this current (the section is
    // append-only, so indices never shift); find_declaration,
    // replace_type_id and the emit-time topological sort all rely on that.
    int32_t _declaration_index = -1;

    // Cached result of resolve_type, thrown away by invalidate_types().
    mutable const ShaderType *_type = nullptr;
  };

  INLINE const Definition &get_definition(Id id) const;
  INLINE Definition &modify_definition(Id id);
  void record_result_at(const Instruction &op, Id function_id, int32_t declaration_index);
  void clear_id_state(Id id);

  const ShaderType *r_resolve_type(Id id) const;
  const ShaderType *apply_member_decorations(Id type_id, uint32_t member_index, const ShaderType *type) const;
  void get_aggregate_access_flags(Id struct_id, bool &non_writable, bool &non_readable) const;
  spv::StorageClass canonicalize_storage_class(spv::StorageClass storage_class, Id pointee_type_id) const;

  void ensure_type_map() const;
  bool is_canonical_type_decl(const Instruction &op) const;
  bool has_builtin_members(Id type_id) const;

  void r_annotate_struct_layout(Id type_id);
  pvector<size_t> sort_declarations() const;
  void r_sort_declaration(size_t index, pvector<int> &state,
                          pvector<size_t> &order) const;
  void rewrite_type_references(Id before, Id after);

  Id ensure_builtin(spv::StorageClass storage_class, spv::BuiltIn builtin);

  void output_id(std::ostream &out, Id id) const;
  Id error_expected(Id id, const char *msg) const;

private:
  // Module header state.
  uint32_t _version = 0x10000;
  uint32_t _generator = 0;
  uint32_t _id_bound = 1;
  uint32_t _schema = 0;

  // The sections, in module order.  Capabilities, extensions, imports, the
  // memory model, execution modes and non-name debug instructions are kept
  // as opaque instruction lists; they are rare and no pass interprets them
  // structurally (execution modes reference the entry function id, which is
  // checked when a function is deleted).
  pvector<Instruction> _capabilities;
  pvector<Instruction> _extensions;
  pvector<Instruction> _ext_inst_imports;
  Instruction _memory_model;
  EntryPoints _entry_points;
  pvector<Instruction> _execution_modes;
  pvector<Instruction> _debug;  // OpString, OpSource*
  pmap<uint32_t, std::string> _names;
  pmap<uint32_t, pvector<std::string> > _member_names;
  pvector<Instruction> _module_processed;
  pmap<uint32_t, Annotations> _annotations;
  pdeque<Instruction> _declarations;
  Functions _functions;

  // The id index.
  typedef pdeque<Definition> Definitions;
  Definitions _defs;

  // Reverse mapping from ShaderType to the id of a canonical declaration.
  // This is a lookup structure for reusing existing declarations; it is not
  // what SPIR-V's type uniqueness rules rest on (deduplicate_types() will
  // handle any duplicate types that remain).  Rebuilt lazily whenever the type
  // resolutions are invalidated.
  typedef pmap<const ShaderType *, Id> TypeMap;
  mutable TypeMap _type_map;
  mutable bool _type_map_valid = false;

  friend class SpirVInstructionCursor;
};

#include "spirVModule.I"

#endif
