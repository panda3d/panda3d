// Filename: graphicsStateGuardian.h
// Created by:  drose (02Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef GRAPHICSSTATEGUARDIAN_H
#define GRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include "savedFrameBuffer.h"
#include "frameBufferStack.h"
#include "frameBufferProperties.h"
#include "displayRegionStack.h"
#include "lensStack.h"

#include "graphicsStateGuardianBase.h"
#include "graphicsThreadingModel.h"
#include "graphicsPipe.h"
#include "sceneSetup.h"
#include "luse.h"
#include "coordinateSystem.h"
#include "factory.h"
#include "pStatCollector.h"
#include "transformState.h"
#include "renderState.h"
#include "light.h"
#include "planeNode.h"
#include "colorWriteAttrib.h"
#include "colorBlendAttrib.h"
#include "transparencyAttrib.h"
#include "config_display.h"

#include "notify.h"
#include "pvector.h"

class ClearableRegion;
class GraphicsEngine;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsStateGuardian
// Description : Encapsulates all the communication with a particular
//               instance of a given rendering backend.  Tries to
//               guarantee that redundant state-change requests are
//               not issued (hence "state guardian").
//
//               There will be one of these objects for each different
//               graphics context active in the system.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsStateGuardian : public GraphicsStateGuardianBase {
  //
  // Interfaces all GSGs should have
  //
public:
  GraphicsStateGuardian(const FrameBufferProperties &properties);
  virtual ~GraphicsStateGuardian();

PUBLISHED:
  void release_all_textures();
  void release_all_geoms();

public:
  INLINE const FrameBufferProperties &get_properties() const;
  INLINE GraphicsPipe *get_pipe() const;
  INLINE GraphicsEngine *get_engine() const;
  INLINE const GraphicsThreadingModel &get_threading_model() const;

  INLINE void set_active(bool active);
  INLINE bool is_active() const;

  INLINE void set_scene(SceneSetup *scene_setup);
  INLINE SceneSetup *get_scene() const;

  virtual TextureContext *prepare_texture(Texture *tex);
  virtual void apply_texture(TextureContext *tc);
  virtual void release_texture(TextureContext *tc);

  virtual GeomNodeContext *prepare_geom_node(GeomNode *node);
  virtual void draw_geom_node(GeomNode *node, const RenderState *state,
                              GeomNodeContext *gnc);
  virtual void release_geom_node(GeomNodeContext *gnc);

  virtual GeomContext *prepare_geom(Geom *geom);
  virtual void release_geom(GeomContext *gc);

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform);

  virtual void set_color_clear_value(const Colorf &value);
  virtual void set_depth_clear_value(const float value);
  virtual void do_clear(const RenderBuffer &buffer)=0;

  void clear(ClearableRegion *clearable);
  INLINE void clear(DisplayRegion *dr);

  virtual void prepare_display_region()=0;
  virtual bool prepare_lens();

  INLINE void enable_normals(bool val) { _normals_enabled = val; }

  virtual bool begin_frame();
  virtual bool begin_scene();
  virtual void end_scene();
  virtual void end_frame();

  // These functions will be queried by the GeomIssuer to determine if
  // it should issue normals, texcoords, and/or colors, based on the
  // GSG's current state.
  virtual bool wants_normals() const;
  virtual bool wants_texcoords() const;
  virtual bool wants_colors() const;

  virtual bool depth_offset_decals();
  virtual CPT(RenderState) begin_decal_base_first();
  virtual CPT(RenderState) begin_decal_nested();
  virtual CPT(RenderState) begin_decal_base_second();
  virtual void finish_decal();

  INLINE bool reset_if_new();
  virtual void reset();

  INLINE void modify_state(const RenderState *state);
  INLINE void set_state(const RenderState *state);
  INLINE void set_transform(const TransformState *transform);

  RenderBuffer get_render_buffer(int buffer_type);

  INLINE const DisplayRegion *get_current_display_region(void) const;
  INLINE const Lens *get_current_lens() const;

  INLINE DisplayRegionStack push_display_region(const DisplayRegion *dr);
  INLINE void pop_display_region(DisplayRegionStack &node);
  INLINE FrameBufferStack push_frame_buffer(const RenderBuffer &buffer,
                                            const DisplayRegion *dr);
  INLINE void pop_frame_buffer(FrameBufferStack &node);

  INLINE LensStack push_lens(const Lens *lens);
  INLINE void pop_lens(LensStack &stack);
  INLINE bool set_lens(const Lens *lens);

  INLINE void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;
  virtual CoordinateSystem get_internal_coordinate_system() const;

  virtual void issue_transform(const TransformState *transform);
  virtual void issue_color_scale(const ColorScaleAttrib *attrib);
  virtual void issue_color(const ColorAttrib *attrib);
  virtual void issue_light(const LightAttrib *attrib);
  virtual void issue_color_write(const ColorWriteAttrib *attrib);
  virtual void issue_transparency(const TransparencyAttrib *attrib);
  virtual void issue_color_blend(const ColorBlendAttrib *attrib);
  virtual void issue_clip_plane(const ClipPlaneAttrib *attrib);

  virtual void bind_light(PointLight *light, int light_id);
  virtual void bind_light(DirectionalLight *light, int light_id);
  virtual void bind_light(Spotlight *light, int light_id);

protected:
  INLINE Light *get_light(int light_id) const;
  virtual bool slot_new_light(int light_id);
  virtual void enable_lighting(bool enable);
  virtual void set_ambient_light(const Colorf &color);
  virtual void enable_light(int light_id, bool enable);
  virtual void begin_bind_lights();
  virtual void end_bind_lights();

  INLINE PlaneNode *get_clip_plane(int plane_id) const;
  virtual bool slot_new_clip_plane(int plane_id);
  virtual void enable_clip_planes(bool enable);
  virtual void enable_clip_plane(int plane_id, bool enable);
  virtual void begin_bind_clip_planes();
  virtual void bind_clip_plane(PlaneNode *plane, int pane_id);
  virtual void end_bind_clip_planes();

  virtual void set_blend_mode(ColorWriteAttrib::Mode color_write_mode,
                              ColorBlendAttrib::Mode color_blend_mode,
                              TransparencyAttrib::Mode transparency_mode);

  virtual PT(SavedFrameBuffer) save_frame_buffer(const RenderBuffer &buffer,
                                                 CPT(DisplayRegion) dr)=0;
  virtual void restore_frame_buffer(SavedFrameBuffer *frame_buffer)=0;

  bool mark_prepared_texture(TextureContext *tc);
  bool unmark_prepared_texture(TextureContext *tc);
  bool mark_prepared_geom(GeomContext *gc);
  bool unmark_prepared_geom(GeomContext *gc);
  bool mark_prepared_geom_node(GeomNodeContext *gnc);
  bool unmark_prepared_geom_node(GeomNodeContext *gnc);

  virtual void free_pointers();
  virtual void close_gsg();
  void panic_deactivate();

#ifdef DO_PSTATS
  // These functions are used to update the active texture memory
  // usage record (and other frame-based measurements) in Pstats.
  void init_frame_pstats();
  void add_to_texture_record(TextureContext *tc);
  void add_to_geom_record(GeomContext *gc);
  void add_to_geom_node_record(GeomNodeContext *gnc);

  pset<TextureContext *> _current_textures;
  pset<GeomContext *> _current_geoms;
  pset<GeomNodeContext *> _current_geom_nodes;
#else
  INLINE void init_frame_pstats() { }
  INLINE void add_to_texture_record(TextureContext *) { }
  INLINE void add_to_geom_record(GeomContext *) { }
  INLINE void add_to_geom_node_record(GeomNodeContext *) { }
  INLINE void record_state_change(TypeHandle) { }
#endif

  static CPT(RenderState) get_unlit_state();
  static CPT(RenderState) get_unclipped_state();
  static CPT(RenderState) get_untextured_state();

protected:
  PT(SceneSetup) _scene_setup;

  CPT(RenderState) _state;
  CPT(TransformState) _transform;

  int _buffer_mask;
  Colorf _color_clear_value;
  float _depth_clear_value;
  bool _stencil_clear_value;
  Colorf _accum_clear_value;
  int _clear_buffer_type;

  int _display_region_stack_level;
  int _frame_buffer_stack_level;
  int _lens_stack_level;

  CPT(DisplayRegion) _current_display_region;
  CPT(Lens) _current_lens;

  // This is used by wants_normals()
  bool _normals_enabled;

  CoordinateSystem _coordinate_system;

  Colorf _scene_graph_color;
  bool _has_scene_graph_color;
  bool _scene_graph_color_stale;
  bool _vertex_colors_enabled;
  bool _lighting_enabled;
  bool _clip_planes_enabled;

  enum ColorTransform {
    CT_offset  = 0x01,
    CT_scale   = 0x02,
  };
  int _color_transform_enabled;  // Zero or more of ColorTransform bits, above.
  LVecBase4f _current_color_offset;
  LVecBase4f _current_color_scale;

  ColorWriteAttrib::Mode _color_write_mode;
  ColorBlendAttrib::Mode _color_blend_mode;
  TransparencyAttrib::Mode _transparency_mode;

  bool _needs_reset;
  bool _closing_gsg;
  bool _active;

public:
  // Statistics
  static PStatCollector _total_texusage_pcollector;
  static PStatCollector _active_texusage_pcollector;
  static PStatCollector _total_geom_pcollector;
  static PStatCollector _active_geom_pcollector;
  static PStatCollector _total_geom_node_pcollector;
  static PStatCollector _active_geom_node_pcollector;
  static PStatCollector _total_texmem_pcollector;
  static PStatCollector _used_texmem_pcollector;
  static PStatCollector _texmgrmem_total_pcollector;
  static PStatCollector _texmgrmem_resident_pcollector;
  static PStatCollector _vertices_tristrip_pcollector;
  static PStatCollector _vertices_trifan_pcollector;
  static PStatCollector _vertices_tri_pcollector;
  static PStatCollector _vertices_other_pcollector;
  static PStatCollector _transform_state_pcollector;
  static PStatCollector _texture_state_pcollector;
  static PStatCollector _other_state_pcollector;
  static PStatCollector _draw_primitive_pcollector;

private:
  class LightInfo {
  public:
    INLINE LightInfo();
    PT(Light) _light;
    bool _enabled;
    bool _next_enabled;
  };

  pvector<LightInfo> _light_info;
  bool _lighting_enabled_this_frame;

  class ClipPlaneInfo {
  public:
    INLINE ClipPlaneInfo();
    PT(PlaneNode) _plane;
    bool _enabled;
    bool _next_enabled;
  };

  pvector<ClipPlaneInfo> _clip_plane_info;
  bool _clip_planes_enabled_this_frame;

  // NOTE: on win32 another DLL (e.g. libpandadx.dll) cannot access
  // these sets directly due to exported template issue
  typedef pset<TextureContext *> Textures;
  Textures _prepared_textures;  
  typedef pset<GeomContext *> Geoms;
  Geoms _prepared_geoms;  
  typedef pset<GeomNodeContext *> GeomNodes;
  GeomNodes _prepared_geom_nodes;  

  FrameBufferProperties _properties;
  PT(GraphicsPipe) _pipe;
  GraphicsEngine *_engine;
  GraphicsThreadingModel _threading_model;

public:
  void traverse_prepared_textures(bool (*pertex_callbackfn)(TextureContext *,void *),void *callback_arg);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    GraphicsStateGuardianBase::init_type();
    register_type(_type_handle, "GraphicsStateGuardian",
                  GraphicsStateGuardianBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsPipe;
  friend class GraphicsWindow;
  friend class GraphicsEngine;
};

#include "graphicsStateGuardian.I"

#endif
