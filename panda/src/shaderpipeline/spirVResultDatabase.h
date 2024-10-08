#ifndef SPIRVRESULTDATABASE_H
#define SPIRVRESULTDATABASE_H

#include "shaderModuleSpirV.h"

/**
 * Stores a list of definitions making up this module.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVResultDatabase {
private:
  enum DefinitionType {
    DT_none,
    DT_type,
    DT_pointer_type,
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

    DF_relaxed_precision = 512,
  };

public:
  /**
   * Used by below Definition struct to hold member info.
   */
  struct MemberDefinition {
    std::string _name;
    uint32_t _type_id = 0;
    int _location = -1;
    int _offset = -1;
    spv::BuiltIn _builtin = spv::BuiltInMax;
    int _flags = 0; // Only readonly/writeonly/deleted
    int _new_index = -1;
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
    pvector<uint32_t> _parameters;
    int _flags = 0;

    // Only defined for DT_variable and DT_pointer_type.
    spv::StorageClass _storage_class;

    INLINE bool is_type() const;
    INLINE bool is_pointer_type() const;
    INLINE bool is_variable() const;
    INLINE bool is_function_parameter() const;
    INLINE bool is_constant() const;
    INLINE bool is_spec_constant() const;
    INLINE bool is_function() const;
    INLINE bool is_ext_inst() const;

    INLINE bool is_used() const;
    INLINE bool is_dref_sampled() const;
    INLINE bool is_dynamically_indexed() const;
    INLINE bool is_builtin() const;
    INLINE bool has_location() const;
    bool has_builtin() const;
    const MemberDefinition &get_member(uint32_t i) const;
    MemberDefinition &modify_member(uint32_t i);
    void clear();
  };
  typedef pvector<Definition> Definitions;

  uint32_t find_definition(const std::string &name) const;
  const Definition &get_definition(uint32_t id) const;
  Definition &modify_definition(uint32_t id);

  void parse_instruction(spv::Op opcode, uint32_t *args, uint32_t nargs, uint32_t &current_function_id);

  uint32_t find_type(const ShaderType *type);
  uint32_t find_pointer_type(const ShaderType *type, spv::StorageClass storage_class);

  void record_type(uint32_t id, const ShaderType *type);
  void record_pointer_type(uint32_t id, spv::StorageClass storage_class, uint32_t type_id);
  void record_variable(uint32_t id, uint32_t pointer_type_id, spv::StorageClass storage_class, uint32_t function_id=0);
  void record_function_parameter(uint32_t id, uint32_t type_id, uint32_t function_id);
  void record_constant(uint32_t id, uint32_t type_id, const uint32_t *words, uint32_t nwords);
  void record_ext_inst_import(uint32_t id, const char *import);
  void record_function(uint32_t id, uint32_t type_id);
  void record_temporary(uint32_t id, uint32_t type_id, uint32_t from_id, uint32_t function_id);
  void record_spec_constant(uint32_t id, uint32_t type_id);

  void mark_used(uint32_t id);

private:
  Definitions _defs;

  // Reverse mapping from type to ID.  Excludes types with BuiltIn decoration.
  typedef pmap<const ShaderType *, uint32_t> TypeMap;
  TypeMap _type_map;
};

#include "spirVResultDatabase.I"

#endif
