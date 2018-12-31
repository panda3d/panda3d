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
#include "epvector.h"
#include "asyncFuture.h"
#include "bamCacheRecord.h"

#ifdef HAVE_CG
// I don't want to include the Cg header file into panda as a whole.  Instead,
// I'll just excerpt some opaque declarations.
typedef struct _CGcontext   *CGcontext;
typedef struct _CGprogram   *CGprogram;
typedef struct _CGparameter *CGparameter;
#endif

/**

 */
class EXPCL_PANDA_GOBJ Shader : public TypedWritableReferenceCount {
PUBLISHED:
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

  INLINE bool has_fullpath() const;
  INLINE const Filename &get_fullpath() const;

  INLINE bool get_cache_compiled_shader() const;
  INLINE void set_cache_compiled_shader(bool flag);

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

  enum ShaderArgClass {
    SAC_scalar,
    SAC_vector,
    SAC_matrix,
    SAC_sampler,
    SAC_array,
    SAC_unknown,
  };

  enum ShaderArgType {
    SAT_scalar,
    SAT_vec1,
    SAT_vec2,
    SAT_vec3,
    SAT_vec4,
    SAT_mat1x1,
    SAT_mat1x2,
    SAT_mat1x3,
    SAT_mat1x4,
    SAT_mat2x1,
    SAT_mat2x2,
    SAT_mat2x3,
    SAT_mat2x4,
    SAT_mat3x1,
    SAT_mat3x2,
    SAT_mat3x3,
    SAT_mat3x4,
    SAT_mat4x1,
    SAT_mat4x2,
    SAT_mat4x3,
    SAT_mat4x4,
    SAT_sampler1d,
    SAT_sampler2d,
    SAT_sampler3d,
    SAT_sampler2d_array,
    SAT_sampler_cube,
    SAT_sampler_buffer,
    SAT_sampler_cube_array,
    SAT_unknown
};

  enum ShaderArgDir {
    SAD_in,
    SAD_out,
    SAD_inout,
    SAD_unknown,
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

  struct ShaderArgId {
    std::string     _name;
    ShaderType _type;
    int        _seqno;
  };

  enum ShaderPtrType {
    SPT_float,
    SPT_double,
    SPT_int,
    SPT_uint,
    SPT_unknown
  };

  struct ShaderArgInfo {
    ShaderArgId       _id;
    ShaderArgClass    _class;
    ShaderArgClass    _subclass;
    ShaderArgType     _type;
    ShaderArgDir      _direction;
    bool              _varying;
    ShaderPtrType     _numeric_type;
    NotifyCategory   *_cat;
  };

  // Container structure for data of parameters ShaderPtrSpec.
  struct ShaderPtrData {
  private:
    PT(ReferenceCount) _pta;

  public:
    void *_ptr;
    ShaderPtrType _type;
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

  struct ShaderMatSpec {
    LMatrix4          _cache[2];
    LMatrix4          _value;
    ShaderArgId       _id;
    ShaderMatFunc     _func;
    ShaderMatInput    _part[2];
    PT(InternalName)  _arg[2];
    int               _dep[2];
    int               _index;
    ShaderMatPiece    _piece;
  };

  struct ShaderTexSpec {
    ShaderArgId       _id;
    PT(InternalName)  _name;
    ShaderTexInput    _part;
    int               _stage;
    int               _desired_type;
    PT(InternalName)  _suffix;
  };

  struct ShaderVarSpec {
    ShaderArgId       _id;
    PT(InternalName)  _name;
    int               _append_uv;
    int               _elements;
    ShaderPtrType     _numeric_type;
  };

  struct ShaderPtrSpec {
    ShaderArgId       _id;
    int               _dim[3]; //n_elements,rows,cols
    int               _dep[2];
    PT(InternalName)  _arg;
    ShaderArgInfo     _info;
    ShaderPtrType     _type;
  };

  class EXPCL_PANDA_GOBJ ShaderCaps {
  public:
    void clear();
    INLINE bool operator == (const ShaderCaps &other) const;
    INLINE ShaderCaps();

  public:
    bool _supports_glsl;

#ifdef HAVE_CG
    int _active_vprofile;
    int _active_fprofile;
    int _active_gprofile;
    int _active_tprofile;

    int _ultimate_vprofile;
    int _ultimate_fprofile;
    int _ultimate_gprofile;
    int _ultimate_tprofile;

    pset <ShaderBug> _bug_list;
#endif
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

public:
  // These routines help split the shader into sections, for those shader
  // implementations that need to do so.  Don't use them when you use separate
  // shader programs.
  void parse_init();
  void parse_line(std::string &result, bool rt, bool lt);
  void parse_upto(std::string &result, std::string pattern, bool include);
  void parse_rest(std::string &result);
  bool parse_eof();

  void cp_report_error(ShaderArgInfo &arg, const std::string &msg);
  bool cp_errchk_parameter_words(ShaderArgInfo &arg, int len);
  bool cp_errchk_parameter_in(ShaderArgInfo &arg);
  bool cp_errchk_parameter_ptr(ShaderArgInfo &p);
  bool cp_errchk_parameter_varying(ShaderArgInfo &arg);
  bool cp_errchk_parameter_uniform(ShaderArgInfo &arg);
  bool cp_errchk_parameter_float(ShaderArgInfo &arg, int lo, int hi);
  bool cp_errchk_parameter_sampler(ShaderArgInfo &arg);
  bool cp_parse_eol(ShaderArgInfo &arg,
                    vector_string &pieces, int &next);
  bool cp_parse_delimiter(ShaderArgInfo &arg,
                          vector_string &pieces, int &next);
  std::string cp_parse_non_delimiter(vector_string &pieces, int &next);
  bool cp_parse_coord_sys(ShaderArgInfo &arg,
                          vector_string &pieces, int &next,
                          ShaderMatSpec &spec, bool fromflag);
  int cp_dependency(ShaderMatInput inp);
  void cp_optimize_mat_spec(ShaderMatSpec &spec);

#ifdef HAVE_CG
  void cg_recurse_parameters(CGparameter parameter,
                          const ShaderType &type,
                          bool &success);
#endif

  bool compile_parameter(ShaderArgInfo &p, int *arg_dim);

  void clear_parameters();

  void set_compiled(unsigned int format, const char *data, size_t length);
  bool get_compiled(unsigned int &format, std::string &binary) const;

  static void set_default_caps(const ShaderCaps &caps);

private:
#ifdef HAVE_CG
  ShaderArgClass cg_parameter_class(CGparameter p);
  ShaderArgType cg_parameter_type(CGparameter p);
  ShaderArgDir cg_parameter_dir(CGparameter p);

  CGprogram cg_compile_entry_point(const char *entry, const ShaderCaps &caps,
                                   CGcontext context, ShaderType type);

  bool cg_analyze_entry_point(CGprogram prog, ShaderType type);

  bool cg_analyze_shader(const ShaderCaps &caps);
  bool cg_compile_shader(const ShaderCaps &caps, CGcontext context);
  void cg_release_resources();
  void cg_report_errors();

  // Determines the appropriate cg profile settings and stores them in the
  // active shader caps based on any profile settings stored in the shader's
  // header
  void cg_get_profile_from_header(ShaderCaps &caps);

  ShaderCaps _cg_last_caps;
  static CGcontext  _cg_context;
  CGprogram  _cg_vprogram;
  CGprogram  _cg_fprogram;
  CGprogram  _cg_gprogram;

  int _cg_vprofile;
  int _cg_fprofile;
  int _cg_gprofile;

  CGprogram cg_program_from_shadertype(ShaderType type);

public:
  bool cg_compile_for(const ShaderCaps &caps, CGcontext context,
                      CGprogram &combined_program, pvector<CGparameter> &map);

#endif

public:
  pvector<ShaderPtrSpec> _ptr_spec;
  epvector<ShaderMatSpec> _mat_spec;
  pvector<ShaderTexSpec> _tex_spec;
  pvector<ShaderVarSpec> _var_spec;
  int _mat_deps;

  bool _error_flag;
  ShaderFile _text;

protected:
  ShaderFile _filename;
  Filename _fullpath;
  int _parse;
  bool _loaded;
  ShaderLanguage _language;

  typedef pvector<Filename> Filenames;
  Filenames _included_files;

  // Stores full paths, and includes the fullpaths of the shaders themselves
  // as well as the includes.
  Filenames _source_files;
  time_t _last_modified;

  PT(BamCacheRecord) _record;
  bool _cache_compiled_shader;
  unsigned int _compiled_format;
  std::string _compiled_binary;

  static ShaderCaps _default_caps;
  static int _shaders_generated;

  typedef pmap<ShaderFile, PT(Shader)> ShaderTable;

  static ShaderTable _load_table;
  static ShaderTable _make_table;

  friend class ShaderContext;
  friend class PreparedGraphicsObjects;

  typedef pmap <PreparedGraphicsObjects *, ShaderContext *> Contexts;
  Contexts _contexts;

private:
  void clear_prepared(PreparedGraphicsObjects *prepared_objects);

  Shader(ShaderLanguage lang);

  bool read(const ShaderFile &sfile, BamCacheRecord *record = nullptr);
  bool load(const ShaderFile &sbody, BamCacheRecord *record = nullptr);
  bool do_read_source(std::string &into, const Filename &fn, BamCacheRecord *record);
  bool do_load_source(std::string &into, const std::string &source, BamCacheRecord *record);
  bool r_preprocess_include(std::ostream &out, const Filename &fn,
                            const Filename &source_dir,
                            std::set<Filename> &open_files,
                            BamCacheRecord *record, int depth);
  bool r_preprocess_source(std::ostream &out, std::istream &in,
                           const Filename &fn, const Filename &full_fn,
                           std::set<Filename> &open_files,
                           BamCacheRecord *record,
                           int fileno = 0, int depth = 0);

  bool check_modified() const;

public:
  ~Shader();

  Filename get_filename_from_index(int index, ShaderType type) const;

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
