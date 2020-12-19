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
#include "epvector.h"
#include "asyncFuture.h"
#include "shaderModule.h"
#include "copyOnWritePointer.h"

class BamCacheRecord;
class ShaderModuleGlsl;
class ShaderCompiler;

/**

 */
class EXPCL_PANDA_GOBJ Shader : public TypedWritableReferenceCount {
PUBLISHED:
  using Stage = ShaderModule::Stage;
  using ScalarType = ShaderType::ScalarType;

  enum ShaderLanguage {
    SL_none,
    SL_Cg,
    SL_GLSL,
    SL_HLSL,
    SL_SPIR_V,
  };

  enum ShaderType {
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

private:
  Shader(ShaderLanguage lang);

PUBLISHED:
  static PT(Shader) load(const Filename &file, ShaderLanguage lang = SL_none);
  static PT(Shader) make(std::string body, ShaderLanguage lang = SL_none);
  static PT(Shader) load(ShaderLanguage lang,
                         const Filename &vertex, const Filename &fragment,
                         const Filename &geometry = "",
                         const Filename &tess_control = "",
                         const Filename &tess_evaluation = "");
  static PT(Shader) load_compute(ShaderLanguage lang, const Filename &fn);
  static PT(Shader) make(ShaderLanguage lang,
                         std::string vertex, std::string fragment,
                         std::string geometry = "",
                         std::string tess_control = "",
                         std::string tess_evaluation = "");
  static PT(Shader) make_compute(ShaderLanguage lang, std::string body);

  INLINE Filename get_filename(ShaderType type = ST_none) const;
  INLINE void set_filename(ShaderType type, const Filename &filename);
  INLINE const std::string &get_text(ShaderType type = ST_none) const;
  INLINE bool get_error_flag() const;
  INLINE ShaderLanguage get_language() const;
  INLINE int get_used_capabilities() const;

  INLINE bool has_fullpath() const;
  INLINE const Filename &get_fullpath() const;

  INLINE bool has_stage(Stage stage) const;
  INLINE CPT(ShaderModule) get_module(Stage stage) const;
  INLINE PT(ShaderModule) modify_module(Stage stage);

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
  enum ShaderMatInput {
    SMO_identity,

    SMO_window_size,
    SMO_pixel_size,
    SMO_texpad_x,
    SMO_texpix_x,

    SMO_attr_material,
    SMO_attr_color,
    SMO_attr_colorscale,

    SMO_alight_x,
    SMO_dlight_x,
    SMO_plight_x,
    SMO_slight_x,
    SMO_satten_x,
    SMO_texmat_i,
    SMO_plane_x,
    SMO_clipplane_x,

    SMO_mat_constant_x,
    SMO_vec_constant_x,

    SMO_world_to_view,
    SMO_view_to_world,

    SMO_model_to_view,
    SMO_view_to_model,

    SMO_apiview_to_view,
    SMO_view_to_apiview,

    SMO_clip_to_view,
    SMO_view_to_clip,

    SMO_apiclip_to_view,
    SMO_view_to_apiclip,

    SMO_view_x_to_view,
    SMO_view_to_view_x,

    SMO_apiview_x_to_view,
    SMO_view_to_apiview_x,

    SMO_clip_x_to_view,
    SMO_view_to_clip_x,

    SMO_apiclip_x_to_view,
    SMO_view_to_apiclip_x,

    SMO_attr_fog,
    SMO_attr_fogcolor,

    SMO_frame_number,
    SMO_frame_time,
    SMO_frame_delta,

    SMO_mat_constant_x_attrib,
    SMO_vec_constant_x_attrib,

    SMO_light_ambient,
    SMO_light_source_i_attrib,

    SMO_light_product_i_ambient,
    SMO_light_product_i_diffuse,
    SMO_light_product_i_specular,

    // SMO_clipplane_x is world coords, GLSL needs eye coords
    SMO_apiview_clipplane_i,

    SMO_model_to_apiview,
    SMO_apiview_to_model,
    SMO_apiview_to_apiclip,
    SMO_apiclip_to_apiview,

    SMO_inv_texmat_i,

    // Additional properties for PBR materials
    SMO_attr_material2,

    // Hack for text rendering.  Don't use in user shaders.
    SMO_tex_is_alpha_i,

    SMO_transform_i,
    SMO_slider_i,

    SMO_light_source_i_packed,

    // Texture scale component of texture matrix.
    SMO_texscale_i,

    // Color of an M_blend texture stage.
    SMO_texcolor_i,

    SMO_INVALID
  };

  enum ShaderTexInput {
    STO_INVALID,

    STO_named_input,
    STO_named_stage,

    STO_stage_i,
    STO_light_i_shadow_map,
  };

  enum ShaderMatPiece {
    SMP_whole,
    SMP_transpose,
    SMP_row0,
    SMP_row1,
    SMP_row2,
    SMP_row3,
    SMP_col0,
    SMP_col1,
    SMP_col2,
    SMP_col3,
    SMP_row3x1,
    SMP_row3x2,
    SMP_row3x3,
    SMP_upper3x3,
    SMP_transpose3x3,
    SMP_cell15,
    SMP_cell14,
    SMP_cell13,
    SMP_upper3x4,
    SMP_upper4x3,
    SMP_transpose3x4,
    SMP_transpose4x3,
  };

  enum ShaderStateDep {
    SSD_NONE          = 0x000,
    SSD_general       = 0x001,
    SSD_transform    = 0x2002,
    SSD_color         = 0x004,
    SSD_colorscale    = 0x008,
    SSD_material      = 0x010,
    SSD_shaderinputs  = 0x020,
    SSD_fog           = 0x040,
    SSD_light         = 0x080,
    SSD_clip_planes   = 0x100,
    SSD_tex_matrix    = 0x200,
    SSD_frame         = 0x400,
    SSD_projection    = 0x800,
    SSD_texture      = 0x1000,
    SSD_view_transform= 0x2000,
  };

  enum ShaderBug {
    SBUG_ati_draw_buffers,
  };

  enum ShaderMatFunc {
    SMF_compose,
    SMF_transform_dlight,
    SMF_transform_plight,
    SMF_transform_slight,
    SMF_first,
  };

  struct Parameter {
    CPT_InternalName _name;
    const ::ShaderType *_type = nullptr;
    int _location = -1;
    int _stage_mask = 0;
  };

  enum ShaderPtrType {
    SPT_float = ScalarType::ST_float,
    SPT_double = ScalarType::ST_double,
    SPT_int = ScalarType::ST_int,
    SPT_uint = ScalarType::ST_uint,
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
   * Describes a matrix making up a single part of the ShaderMatInput cache.
   * The cache is made up of a continuous array of matrices, as described by
   * a successive list of ShaderMatPart (each of which takes up _count matrices)
   */
  struct ShaderMatPart {
    ShaderMatInput _part;
    CPT(InternalName) _arg;
    int _count = 1;
    int _dep = SSD_NONE;
  };

  /**
   * Describes a shader input that is sourced from the render state.
   */
  struct ShaderMatSpec {
    size_t _cache_offset[2];
    Parameter         _id;
    ShaderMatFunc     _func;
    ShaderMatInput    _part[2];
    CPT(InternalName) _arg[2];
    LMatrix4          _value;
    int               _dep = SSD_NONE;
    int               _index = 0;
    ShaderMatPiece    _piece;
    ScalarType        _scalar_type = ScalarType::ST_float;
  };

  struct ShaderTexSpec {
    Parameter         _id;
    CPT(InternalName) _name;
    ShaderTexInput    _part;
    int               _stage;
    int               _desired_type;
    PT(InternalName)  _suffix;
  };

  struct ShaderImgSpec {
    Parameter         _id;
    CPT(InternalName) _name;
    int               _desired_type;
    bool              _writable;
  };

  struct ShaderVarSpec {
    Parameter         _id;
    PT(InternalName)  _name;
    int               _append_uv;
    int               _elements;
    ScalarType        _scalar_type;
  };

  struct ShaderPtrSpec {
    Parameter         _id;
    uint32_t          _dim[3]; //n_elements,rows,cols
    CPT(InternalName) _arg;
    ScalarType        _type;
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
  bool report_parameter_error(const InternalName *name, const ::ShaderType *type, const char *msg);
  bool expect_num_words(const InternalName *name, const ::ShaderType *type, size_t len);
  bool expect_float_vector(const InternalName *name, const ::ShaderType *type, int lo, int hi);
  bool expect_float_matrix(const InternalName *name, const ::ShaderType *type, int lo, int hi);
  bool expect_coordinate_system(const InternalName *name, const ::ShaderType *type,
                                vector_string &pieces, int &next,
                                ShaderMatSpec &spec, bool fromflag);
  int cp_dependency(ShaderMatInput inp);

public:
  void cp_add_mat_spec(ShaderMatSpec &spec);
  size_t cp_get_mat_cache_size() const;

  bool compile_parameter(Parameter &p);

  void clear_parameters();

  void set_compiled(unsigned int format, const char *data, size_t length);
  bool get_compiled(unsigned int &format, std::string &binary) const;

  INLINE PStatCollector &get_prepare_shader_pcollector();
  INLINE const std::string &get_debug_name() const;

public:
  pvector<ShaderPtrSpec> _ptr_spec;
  epvector<ShaderMatSpec> _mat_spec;
  pvector<ShaderTexSpec> _tex_spec;
  pvector<ShaderImgSpec> _img_spec;
  pvector<ShaderVarSpec> _var_spec;
  pvector<ShaderMatPart> _mat_parts;
  int _mat_deps = 0;
  int _mat_cache_size = 0;

  // These are here because we don't support passing these via ShaderMatSpec yet
  int _frame_number_loc = -1;
  int _transform_table_loc = -1;
  uint32_t _transform_table_size = 0;
  bool _transform_table_reduced = false;
  int _slider_table_loc = -1;
  uint32_t _slider_table_size = 0;

  bool _error_flag;
  ShaderFile _text;

  typedef pvector<COWPT(ShaderModule)> Modules;
  Modules _modules;
  typedef pmap<const ShaderModule *, ModuleSpecConstants> ModuleSpecConsts;
  ModuleSpecConsts _module_spec_consts;
  uint32_t _module_mask = 0;
  int _used_caps = 0;

protected:
  ShaderFile _filename;
  Filename _fullpath;
  int _parse;
  bool _loaded;
  ShaderLanguage _language;

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
  bool bind_vertex_input(const InternalName *name, const ::ShaderType *type, int location);
  bool bind_parameter(const Parameter &parameter);

  bool check_modified() const;
  ShaderCompiler *get_compiler(ShaderLanguage lang) const;

  ~Shader();

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "Shader",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shader.I"

#endif
