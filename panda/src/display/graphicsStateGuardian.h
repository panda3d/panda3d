// Filename: graphicsStateGuardian.h
// Created by:  drose (02Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef GRAPHICSSTATEGUARDIAN_H
#define GRAPHICSSTATEGUARDIAN_H

#include <pandabase.h>

#include "savedFrameBuffer.h"
#include "frameBufferStack.h"
#include "displayRegionStack.h"

#include <graphicsStateGuardianBase.h>
#include <nodeTransition.h>
#include <luse.h>
#include <coordinateSystem.h>
#include <factory.h>
#include <renderTraverser.h>

#include <list>

class AllAttributesWrapper;
class AllTransitionsWrapper;

////////////////////////////////////////////////////////////////////
// 	 Class : GraphicsStateGuardian
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
  GraphicsStateGuardian(GraphicsWindow *win);

  INLINE void set_render_traverser(RenderTraverser *rt);
  INLINE RenderTraverser *get_render_traverser() const;

  void release_all_textures();

  virtual void set_color_clear_value(const Colorf& value);
  virtual void set_depth_clear_value(const float value);
  virtual void set_stencil_clear_value(const bool value);
  virtual void set_accum_clear_value(const Colorf& value);
  INLINE Colorf get_color_clear_value(void) const {
    return _color_clear_value;
  }
  INLINE float get_depth_clear_value(void) const {
    return _depth_clear_value;
  }
  INLINE bool get_stencil_clear_value(void) const {
    return _stencil_clear_value;
  }
  INLINE Colorf get_accum_clear_value(void) const {
    return _accum_clear_value;
  }
  virtual void clear(const RenderBuffer &buffer)=0;
  virtual void clear(const RenderBuffer &buffer, const DisplayRegion* region)=0;

  virtual void prepare_display_region()=0;

  virtual void render_frame(const AllAttributesWrapper &initial_state)=0;
  virtual void render_scene(Node *root, const ProjectionNode *projnode,
			    const AllAttributesWrapper &initial_state)=0;
  virtual void render_subgraph(RenderTraverser *traverser, 
			       Node *subgraph, const ProjectionNode *projnode,
			       const AllAttributesWrapper &initial_state,
			       const AllTransitionsWrapper &net_trans)=0;
  virtual void render_subgraph(RenderTraverser *traverser,
			       Node *subgraph,
			       const AllAttributesWrapper &initial_state,
			       const AllTransitionsWrapper &net_trans)=0;

  INLINE void enable_normals(bool val) { _normals_enabled = val; }


  // These functions will be queried by the GeomIssuer to determine if
  // it should issue normals, texcoords, and/or colors, based on the
  // GSG's current state.
  virtual bool wants_normals(void) const;
  virtual bool wants_texcoords(void) const;
  virtual bool wants_colors(void) const;

  virtual void begin_decal(GeomNode *base_geom);
  virtual void end_decal(GeomNode *base_geom);

  virtual void reset();

  void set_state(const NodeAttributes &new_state, bool complete);
  INLINE const NodeAttributes &get_state() const;

  RenderBuffer get_render_buffer(int buffer_type);

  INLINE const ProjectionNode* get_current_projection_node(void) const ;
  INLINE const Node* get_current_root_node(void) const;

  INLINE CPT(DisplayRegion) get_current_display_region(void) const;

  INLINE DisplayRegionStack push_display_region(CPT(DisplayRegion) dr);
  INLINE void pop_display_region(DisplayRegionStack &node);
  INLINE FrameBufferStack push_frame_buffer(const RenderBuffer &buffer,
					    CPT(DisplayRegion) dr);
  INLINE void pop_frame_buffer(FrameBufferStack &node);

  INLINE void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;

  // This function may only be called during a render traversal; it
  // will compute the distance to the indicated point, assumed to be
  // in modelview coordinates, from the camera plane.  This is a
  // virtual function because different GSG's may define the modelview
  // coordinate space differently.
  virtual float compute_distance_to(const LPoint3f &point) const=0;
  
protected:
  virtual PT(SavedFrameBuffer) save_frame_buffer(const RenderBuffer &buffer,
						 CPT(DisplayRegion) dr)=0;
  virtual void restore_frame_buffer(SavedFrameBuffer *frame_buffer)=0;

  bool mark_prepared_texture(TextureContext *tc);
  bool unmark_prepared_texture(TextureContext *tc);

protected:
  int _buffer_mask;
  NodeAttributes _state;
  Colorf _color_clear_value;
  float _depth_clear_value;
  bool _stencil_clear_value;
  Colorf _accum_clear_value;

  int _display_region_stack_level;
  int _frame_buffer_stack_level;

  GraphicsWindow *_win;
  PT(RenderTraverser) _render_traverser;

  // These must be set by render_scene().
  Node *_current_root_node;
  const ProjectionNode *_current_projection_node;
  CPT(DisplayRegion) _current_display_region;

  // This is used by wants_normals()
  bool _normals_enabled;

  CoordinateSystem _coordinate_system;

private:
  typedef set<TextureContext *> Textures;
  Textures _prepared_textures;

// factory stuff
public:
  typedef Factory<GraphicsStateGuardian> GsgFactory;
  typedef FactoryParam GsgParam;

  // Make a factory parameter type for the window pointer
  class EXPCL_PANDA GsgWindow : public GsgParam {
  public:
    INLINE GsgWindow(GraphicsWindow* w) : _w(w), GsgParam() {}
    virtual ~GsgWindow(void);
    INLINE GraphicsWindow* get_window(void) { return _w; }
  public:
    static TypeHandle get_class_type(void);
    static void init_type(void);
    virtual TypeHandle get_type(void) const;
    virtual TypeHandle force_init_type(void);
  private:
    GraphicsWindow* _w;
    static TypeHandle _type_handle;

    INLINE GsgWindow(void) : GsgParam() {}
  };

  static GsgFactory _factory;

private:
  static void read_priorities(void);

public:
  INLINE GraphicsWindow* get_window(void) const { return _win; }


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
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
};

#include "graphicsStateGuardian.I"

#endif
