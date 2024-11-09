/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shader.h
 * @author jyelon
 * @date 2005-09-01
 * @author fperazzi, PandaSE
 * @date 2010-04-29
 */

#ifndef SHADER_H
#define SHADER_H

#include "pandabase.h"
#include "config_gobj.h"

#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "graphicsStateGuardianBase.h"
#include "internalName.h"
#include "pta_int.h"
#include "pta_float.h"
#include "pta_double.h"
#include "pta_stdfloat.h"
#include "pta_LMatrix4.h"
#include "pta_LMatrix3.h"
#include "pta_LVecBase4.h"
#include "pta_LVecBase3.h"
#include "pta_LVecBase2.h"
#include "pStatCollector.h"
#include "pvector.h"
#include "asyncFuture.h"
#include "shaderModule.h"
#include "copyOnWritePointer.h"
#include "shaderInputBinding.h"

class BamCacheRecord;
class ShaderCompiler;
class ShaderInputBinding;

/**
 *
 */
class EXPCL_PANDA_GOBJ Shader : public TypedWritableReferenceCount, public ShaderEnums {
PUBLISHED:
  using Stage = ShaderModule::Stage;
  using ScalarType = ShaderType::ScalarType;

  enum DeprecatedShaderType {
    ST_none = 0,
    ST_vertex,
    ST_fragment,
    ST_geometry,
    ST_tess_control,
    ST_tess_evaluation,
    ST_compute,
    ST_COUNT
  };

  enum AutoShaderSwitch {
    AS_normal = 0x01,
    AS_glow   = 0x02,
    AS_gloss  = 0x04,
    AS_ramp   = 0x08,
    AS_shadow = 0x10,
  };

  enum AutoShaderBit {
    bit_AutoShaderNormal = 0, // bit for AS_normal
    bit_AutoShaderGlow   = 1, // bit for AS_glow
    bit_AutoShaderGloss  = 2, // bit for AS_gloss
    bit_AutoShaderRamp   = 3, // bit for AS_ramp
    bit_AutoShaderShadow = 4, // bit for AS_shadow
  };

PUBLISHED:
  Shader(SourceLanguage lang);

  static PT(Shader) load(const Filename &file, SourceLanguage lang = SL_none);
  static PT(Shader) make(std::string body, SourceLanguage lang = SL_none);
  static PT(Shader) load(SourceLanguage lang,
                         const Filename &vertex, const Filename &fragment,
                         const Filename &geometry = "",
                         const Filename &tess_control = "",
                         const Filename &tess_evaluation = "");
  static PT(Shader) load_compute(SourceLanguage lang, const Filename &fn);
  static PT(Shader) make(SourceLanguage lang,
                         std::string vertex, std::string fragment,
                         std::string geometry = "",
                         std::string tess_control = "",
                         std::string tess_evaluation = "");
  static PT(Shader) make_compute(SourceLanguage lang, std::string body);

  INLINE Filename get_filename(DeprecatedShaderType type = ST_none) const;
  INLINE void set_filename(DeprecatedShaderType type, const Filename &filename);
  INLINE const std::string &get_text(DeprecatedShaderType type = ST_none) const;
  INLINE bool get_error_flag() const;
  INLINE SourceLanguage get_language() const;
  INLINE uint64_t get_used_capabilities() const;

  INLINE bool has_fullpath() const;
  INLINE const Filename &get_fullpath() const;

  INLINE bool has_stage(Stage stage) const;
  INLINE CPT(ShaderModule) get_module(Stage stage) const;
  INLINE PT(ShaderModule) modify_module(Stage stage);
  bool add_module(PT(ShaderModule) module);

  INLINE bool get_cache_compiled_shader() const;
  INLINE void set_cache_compiled_shader(bool flag);

  INLINE bool set_constant(CPT_InternalName name, bool value);
  INLINE bool set_constant(CPT_InternalName name, int value);
  INLINE bool set_constant(CPT_InternalName name, float value);
  bool set_constant(CPT_InternalName name, unsigned int value);

  PT(AsyncFuture) prepare(PreparedGraphicsObjects *prepared_objects);
  bool is_prepared(PreparedGraphicsObjects *prepared_objects) const;
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

  ShaderContext *prepare_now(PreparedGraphicsObjects *prepared_objects,
                             GraphicsStateGuardianBase *gsg);

public:
  enum ShaderBug {
    SBUG_ati_draw_buffers,
  };

  struct Parameter {
    CPT_InternalName _name;
    const ShaderType *_type = nullptr;
    PT(ShaderInputBinding) _binding = nullptr;
    int _location = -1;
    int _stage_mask = 0;
  };

  enum ShaderPtrType {
    SPT_float = ScalarType::ST_float,
    SPT_double = ScalarType::ST_double,
    SPT_int = ScalarType::ST_int,
    SPT_uint = ScalarType::ST_uint,
    SPT_bool = ScalarType::ST_bool,
    SPT_unknown = ScalarType::ST_unknown,
  };

  // Container structure for data of parameters ShaderPtrSpec.
  struct ShaderPtrData {
  private:
    PT(ReferenceCount) _pta;

  public:
    void *_ptr;
    ScalarType _type;
    bool _updated;
    size_t _size; //number of elements vec3[4]=12

  public:
    INLINE ShaderPtrData();
    INLINE ShaderPtrData(const PTA_float &ptr);
    INLINE ShaderPtrData(const PTA_LVecBase4f &ptr);
    INLINE ShaderPtrData(const PTA_LVecBase3f &ptr);
    INLINE ShaderPtrData(const PTA_LVecBase2f &ptr);
    INLINE ShaderPtrData(const PTA_LMatrix4f &mat);
    INLINE ShaderPtrData(const PTA_LMatrix3f &mat);
    INLINE ShaderPtrData(const LVecBase4f &vec);
    INLINE ShaderPtrData(const LVecBase3f &vec);
    INLINE ShaderPtrData(const LVecBase2f &vec);
    INLINE ShaderPtrData(const LMatrix4f &mat);
    INLINE ShaderPtrData(const LMatrix3f &mat);

    INLINE ShaderPtrData(const PTA_double &ptr);
    INLINE ShaderPtrData(const PTA_LVecBase4d &ptr);
    INLINE ShaderPtrData(const PTA_LVecBase3d &ptr);
    INLINE ShaderPtrData(const PTA_LVecBase2d &ptr);
    INLINE ShaderPtrData(const PTA_LMatrix4d &mat);
    INLINE ShaderPtrData(const PTA_LMatrix3d &mat);
    INLINE ShaderPtrData(const LVecBase4d &vec);
    INLINE ShaderPtrData(const LVecBase3d &vec);
    INLINE ShaderPtrData(const LVecBase2d &vec);
    INLINE ShaderPtrData(const LMatrix4d &mat);
    INLINE ShaderPtrData(const LMatrix3d &mat);

    INLINE ShaderPtrData(const PTA_int &ptr);
    INLINE ShaderPtrData(const PTA_LVecBase4i &ptr);
    INLINE ShaderPtrData(const PTA_LVecBase3i &ptr);
    INLINE ShaderPtrData(const PTA_LVecBase2i &ptr);
    INLINE ShaderPtrData(const LVecBase4i &vec);
    INLINE ShaderPtrData(const LVecBase3i &vec);
    INLINE ShaderPtrData(const LVecBase2i &vec);

    INLINE void write_datagram(Datagram &dg) const;
    INLINE void read_datagram(DatagramIterator &source);
  };

  /**
   * Describes a matrix making up a single part of the StateMatrix cache.
   * The cache is made up of a continuous array of matrices, as described by
   * a successive list of MatrixCacheItem.
   * The cache itself is stored in the back-end.
   */
  struct MatrixCacheItem {
    StateMatrix _part;
    int _dep = 0;
    CPT(InternalName) _arg;
  };

  typedef pvector<MatrixCacheItem> MatrixCacheDesc;

  struct ShaderVarSpec {
    Parameter         _id;
    PT(InternalName)  _name;
    int               _append_uv;
    int               _elements;
    ScalarType        _scalar_type;
  };

  class ShaderFile : public ReferenceCount {
  public:
    INLINE ShaderFile() {};
    INLINE ShaderFile(std::string shared);
    INLINE ShaderFile(std::string vertex, std::string fragment, std::string geometry,
                      std::string tess_control, std::string tess_evaluation);

    INLINE void write_datagram(Datagram &dg) const;
    INLINE void read_datagram(DatagramIterator &source);

    INLINE bool operator < (const ShaderFile &other) const;

  public:
    bool _separate;
    std::string _shared;
    std::string _vertex;
    std::string _fragment;
    std::string _geometry;
    std::string _tess_control;
    std::string _tess_evaluation;
    std::string _compute;
  };

  /**
   * Contains external values given to the specialization constants of a single
   * ShaderModule.
   */
  class ModuleSpecConstants {
  public:
    INLINE ModuleSpecConstants() {};

    INLINE bool set_constant(uint32_t id, uint32_t value);
  public:
    pvector<uint32_t> _values;
    pvector<uint32_t> _indices;
  };

protected:
  bool report_parameter_error(const InternalName *name, const ShaderType *type, const char *msg);

public:
  size_t add_matrix_cache_item(StateMatrix input, const InternalName *arg, int dep);
  size_t get_matrix_cache_size() const;

  void clear_parameters();

  void set_compiled(unsigned int format, const char *data, size_t length);
  bool get_compiled(unsigned int &format, std::string &binary) const;

  INLINE PStatCollector &get_prepare_shader_pcollector();
  INLINE const std::string &get_debug_name() const;

public:
  pvector<Parameter> _parameters;
  pvector<ShaderVarSpec> _var_spec;

  MatrixCacheDesc _matrix_cache_desc;
  int _matrix_cache_deps = 0;

  bool _error_flag;
  ShaderFile _text;

  struct LinkedModule {
    LinkedModule(COWPT(ShaderModule) module) : _module(std::move(module)) {}

    COWPT(ShaderModule) _module;
    ModuleSpecConstants _consts;
  };

  typedef pvector<LinkedModule> Modules;
  Modules _modules;
  uint32_t _module_mask = 0;
  uint64_t _used_caps = 0;

protected:
  ShaderFile _filename;
  Filename _fullpath;
  SourceLanguage _language;

  typedef pvector<Filename> Filenames;

  bool _cache_compiled_shader;
  unsigned int _compiled_format;
  std::string _compiled_binary;

  static int _shaders_generated;

  typedef pmap<ShaderFile, PT(Shader)> ShaderTable;

  static ShaderTable _load_table;
  static ShaderTable _make_table;

  friend class ShaderContext;
  friend class PreparedGraphicsObjects;

  typedef pmap <PreparedGraphicsObjects *, ShaderContext *> Contexts;
  Contexts _contexts;

  PStatCollector _prepare_shader_pcollector;
  std::string _debug_name;

private:
  void clear_prepared(PreparedGraphicsObjects *prepared_objects);

  bool read(const ShaderFile &sfile, BamCacheRecord *record = nullptr);
  bool load(const ShaderFile &sbody, BamCacheRecord *record = nullptr);
  bool do_read_source(ShaderModule::Stage stage, const Filename &fn, BamCacheRecord *record);
  bool do_read_source(ShaderModule::Stage stage, std::istream &in,
                      const Filename &fullpath, BamCacheRecord *record);
  bool do_load_source(ShaderModule::Stage stage, const std::string &source, BamCacheRecord *record);

public:
  bool link();
  void add_parameter(const InternalName *name, const ShaderType *type, int location = -1);
  bool bind_vertex_input(const InternalName *name, const ShaderType *type, int location);

  bool check_modified() const;

  ~Shader();

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager) override;
  virtual bool require_fully_complete() const override;

  virtual void finalize(BamReader *manager) override;

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "Shader",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shader.I"

#endif
