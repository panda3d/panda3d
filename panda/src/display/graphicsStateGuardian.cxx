// Filename: graphicsStateGuardian.cxx
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


#include "graphicsStateGuardian.h"
#include "config_display.h"
#include "textureContext.h"
#include "vertexBufferContext.h"
#include "indexBufferContext.h"
#include "renderBuffer.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "lightAttrib.h"
#include "textureAttrib.h"
#include "renderState.h"
#include "depthWriteAttrib.h"
#include "colorWriteAttrib.h"
#include "textureAttrib.h"
#include "lightAttrib.h"
#include "clipPlaneAttrib.h"
#include "light.h"
#include "planeNode.h"
#include "ambientLight.h"
#include "throw_event.h"
#include "clockObject.h"
#include "pStatTimer.h"
#include "qpgeomTristrips.h"
#include "qpgeomTrifans.h"
#include "qpgeomLinestrips.h"

#include <algorithm>

PStatCollector GraphicsStateGuardian::_total_texusage_pcollector("Texture usage");
PStatCollector GraphicsStateGuardian::_active_texusage_pcollector("Texture usage:Active");
PStatCollector GraphicsStateGuardian::_texture_count_pcollector("Prepared Textures");
PStatCollector GraphicsStateGuardian::_active_texture_count_pcollector("Prepared Textures:Active");
PStatCollector GraphicsStateGuardian::_vertex_buffer_switch_pcollector("Vertex buffer switch:Vertex");
PStatCollector GraphicsStateGuardian::_index_buffer_switch_pcollector("Vertex buffer switch:Index");
PStatCollector GraphicsStateGuardian::_load_vertex_buffer_pcollector("Draw:Transfer data:Vertex buffer");
PStatCollector GraphicsStateGuardian::_load_index_buffer_pcollector("Draw:Transfer data:Index buffer");
PStatCollector GraphicsStateGuardian::_load_texture_pcollector("Draw:Transfer data:Texture");
PStatCollector GraphicsStateGuardian::_data_transferred_pcollector("Data transferred");
PStatCollector GraphicsStateGuardian::_total_geom_pcollector("Prepared Geoms");
PStatCollector GraphicsStateGuardian::_active_geom_pcollector("Prepared Geoms:Active");
PStatCollector GraphicsStateGuardian::_total_buffers_pcollector("Vertex buffer size");
PStatCollector GraphicsStateGuardian::_active_vertex_buffers_pcollector("Vertex buffer size:Active vertex");
PStatCollector GraphicsStateGuardian::_active_index_buffers_pcollector("Vertex buffer size:Active index");
PStatCollector GraphicsStateGuardian::_total_geom_node_pcollector("Prepared GeomNodes");
PStatCollector GraphicsStateGuardian::_active_geom_node_pcollector("Prepared GeomNodes:Active");
PStatCollector GraphicsStateGuardian::_total_texmem_pcollector("Texture memory");
PStatCollector GraphicsStateGuardian::_used_texmem_pcollector("Texture memory:In use");
PStatCollector GraphicsStateGuardian::_texmgrmem_total_pcollector("Texture manager");
PStatCollector GraphicsStateGuardian::_texmgrmem_resident_pcollector("Texture manager:Resident");
PStatCollector GraphicsStateGuardian::_primitive_batches_pcollector("Primitive batches");
PStatCollector GraphicsStateGuardian::_primitive_batches_tristrip_pcollector("Primitive batches:Triangle strips");
PStatCollector GraphicsStateGuardian::_primitive_batches_trifan_pcollector("Primitive batches:Triangle fans");
PStatCollector GraphicsStateGuardian::_primitive_batches_tri_pcollector("Primitive batches:Triangles");
PStatCollector GraphicsStateGuardian::_primitive_batches_other_pcollector("Primitive batches:Other");
PStatCollector GraphicsStateGuardian::_vertices_tristrip_pcollector("Vertices:Triangle strips");
PStatCollector GraphicsStateGuardian::_vertices_trifan_pcollector("Vertices:Triangle fans");
PStatCollector GraphicsStateGuardian::_vertices_tri_pcollector("Vertices:Triangles");
PStatCollector GraphicsStateGuardian::_vertices_other_pcollector("Vertices:Other");
PStatCollector GraphicsStateGuardian::_vertices_indexed_tristrip_pcollector("Vertices:Indexed triangle strips");
PStatCollector GraphicsStateGuardian::_state_pcollector("State changes");
PStatCollector GraphicsStateGuardian::_transform_state_pcollector("State changes:Transforms");
PStatCollector GraphicsStateGuardian::_texture_state_pcollector("State changes:Textures");
PStatCollector GraphicsStateGuardian::_draw_primitive_pcollector("Draw:Primitive");
PStatCollector GraphicsStateGuardian::_clear_pcollector("Draw:Clear");
PStatCollector GraphicsStateGuardian::_flush_pcollector("Draw:Flush");

TypeHandle GraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsStateGuardian::
GraphicsStateGuardian(const FrameBufferProperties &properties,
                      CoordinateSystem internal_coordinate_system) :
  _internal_coordinate_system(internal_coordinate_system),
  _properties(properties)
{
  _coordinate_system = CS_invalid;
  set_coordinate_system(get_default_coordinate_system());

  _current_display_region = (DisplayRegion*)0L;
  _current_lens = (Lens *)NULL;
  _needs_reset = true;
  _closing_gsg = false;
  _active = true;
  _prepared_objects = new PreparedGraphicsObjects;

  // Initially, we set this to 1 (the default--no multitexturing
  // supported).  A derived GSG may set this differently if it
  // supports multitexturing.
  _max_texture_stages = 1;

  // Also initially, we assume there are no limits on texture sizes,
  // and that 3-d and cube-map textures are not supported.
  _max_texture_dimension = -1;
  _max_3d_texture_dimension = 0;
  _max_cube_map_dimension = 0;

  // Assume no vertex blending capability.
  _max_vertex_transforms = 0;
  _max_vertex_transform_indices = 0;

  // Initially, we set this to false; a GSG that knows it has this
  // property should set it to true.
  _copy_texture_inverted = false;

  // Similarly with these capabilities flags.
  _supports_multisample = false;
  _supports_generate_mipmap = false;
  _supports_render_texture = false;

  _supported_geom_rendering = 0;

  // If this is true, then we can apply a color and/or color scale by
  // twiddling the material and/or ambient light (which could mean
  // enabling lighting even without a LightAttrib).
  _color_scale_via_lighting = color_scale_via_lighting;

  if (!use_qpgeom) {
    // The old Geom interface doesn't really work too well with the
    // color_scale_via_lighting trick.
    _color_scale_via_lighting = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsStateGuardian::
~GraphicsStateGuardian() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_multisample
//       Access: Published, Virtual
//  Description: Returns true if this particular GSG supports using
//               the multisample bits to provide antialiasing, and
//               also supports M_multisample and M_multisample_mask
//               transparency modes.  If this is not true for a
//               particular GSG, Panda will map the M_multisample
//               modes to M_binary.
//
//               This method is declared virtual solely so that it can
//               be queried from cullResult.cxx.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
get_supports_multisample() const {
  return _supports_multisample;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supported_geom_rendering
//       Access: Published, Virtual
//  Description: Returns the union of Geom::GeomRendering values that
//               this particular GSG can support directly.  If a Geom
//               needs to be rendered that requires some additional
//               properties, the StandardMunger and/or the
//               CullableObject will convert it as needed.
//
//               This method is declared virtual solely so that it can
//               be queried from cullableObject.cxx.
////////////////////////////////////////////////////////////////////
int GraphicsStateGuardian::
get_supported_geom_rendering() const {
  return _supported_geom_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
reset() {
  _needs_reset = false;

  _display_region_stack_level = 0;
  _frame_buffer_stack_level = 0;
  _lens_stack_level = 0;

  _state = RenderState::make_empty();
  _transform = TransformState::make_identity();

  _buffer_mask = 0;
  _color_clear_value.set(0.0f, 0.0f, 0.0f, 0.0f);
  _depth_clear_value = 1.0f;
  _stencil_clear_value = 0.0f;
  _accum_clear_value.set(0.0f, 0.0f, 0.0f, 0.0f);
  _force_normals = 0;

  _has_scene_graph_color = false;
  _scene_graph_color_stale = false;
  _color_blend_involves_color_scale = false;
  _texture_involves_color_scale = false;
  _vertex_colors_enabled = true;
  _lighting_enabled = false;
  _lighting_enabled_this_frame = false;

  _clip_planes_enabled = false;
  _clip_planes_enabled_this_frame = false;

  _color_scale_enabled = false;
  _current_color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);

  _color_write_mode = ColorWriteAttrib::M_on;
  _color_blend_mode = ColorBlendAttrib::M_none;
  _transparency_mode = TransparencyAttrib::M_none;

  _color_blend = NULL;
  _blend_mode_stale = false;
  _pending_texture = NULL;
  _texture_stale = false;
  _pending_light = NULL;
  _light_stale = false;
  _pending_material = NULL;
  _material_stale = false;

  _has_material_force_color = false;
  _material_force_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _light_color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_render_buffer
//       Access: Public
//  Description: Returns a RenderBuffer object suitable for operating
//               on the requested set of buffers.  buffer_type is the
//               union of all the desired RenderBuffer::Type values.
////////////////////////////////////////////////////////////////////
RenderBuffer GraphicsStateGuardian::
get_render_buffer(int buffer_type) {
  return RenderBuffer(this, buffer_type & _buffer_mask);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_prepared_objects
//       Access: Public, Virtual
//  Description: Returns the set of texture and geom objects that have
//               been prepared with this GSG (and possibly other GSG's
//               that share objects).
////////////////////////////////////////////////////////////////////
PreparedGraphicsObjects *GraphicsStateGuardian::
get_prepared_objects() {
  return _prepared_objects;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_texture
//       Access: Public, Virtual
//  Description: Creates whatever structures the GSG requires to
//               represent the texture internally, and returns a
//               newly-allocated TextureContext object with this data.
//               It is the responsibility of the calling function to
//               later call release_texture() with this same pointer
//               (which will also delete the pointer).
//
//               This function should not be called directly to
//               prepare a texture.  Instead, call Texture::prepare().
////////////////////////////////////////////////////////////////////
TextureContext *GraphicsStateGuardian::
prepare_texture(Texture *) {
  return (TextureContext *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_texture
//       Access: Public, Virtual
//  Description: Frees the resources previously allocated via a call
//               to prepare_texture(), including deleting the
//               TextureContext itself, if it is non-NULL.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
release_texture(TextureContext *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_geom
//       Access: Public, Virtual
//  Description: Prepares the indicated Geom for retained-mode
//               rendering, by creating whatever structures are
//               necessary in the GSG (for instance, vertex buffers).
//               Returns the newly-allocated GeomContext that can be
//               used to render the geom.
////////////////////////////////////////////////////////////////////
GeomContext *GraphicsStateGuardian::
prepare_geom(Geom *) {
  return (GeomContext *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_geom
//       Access: Public, Virtual
//  Description: Frees the resources previously allocated via a call
//               to prepare_geom(), including deleting the GeomContext
//               itself, if it is non-NULL.
//
//               This function should not be called directly to
//               prepare a Geom.  Instead, call Geom::prepare().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
release_geom(GeomContext *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_vertex_buffer
//       Access: Public, Virtual
//  Description: Prepares the indicated buffer for retained-mode
//               rendering.
////////////////////////////////////////////////////////////////////
VertexBufferContext *GraphicsStateGuardian::
prepare_vertex_buffer(qpGeomVertexArrayData *) {
  return (VertexBufferContext *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_vertex_buffer
//       Access: Public, Virtual
//  Description: Frees the resources previously allocated via a call
//               to prepare_data(), including deleting the
//               VertexBufferContext itself, if necessary.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
release_vertex_buffer(VertexBufferContext *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_index_buffer
//       Access: Public, Virtual
//  Description: Prepares the indicated buffer for retained-mode
//               rendering.
////////////////////////////////////////////////////////////////////
IndexBufferContext *GraphicsStateGuardian::
prepare_index_buffer(qpGeomPrimitive *) {
  return (IndexBufferContext *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_index_buffer
//       Access: Public, Virtual
//  Description: Frees the resources previously allocated via a call
//               to prepare_data(), including deleting the
//               IndexBufferContext itself, if necessary.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
release_index_buffer(IndexBufferContext *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_geom_munger
//       Access: Public
//  Description: Looks up or creates a GeomMunger object to munge
//               vertices appropriate to this GSG for the indicated
//               state.
////////////////////////////////////////////////////////////////////
PT(qpGeomMunger) GraphicsStateGuardian::
get_geom_munger(const RenderState *state) {
  // Before we even look up the map, see if the _last_mi value points
  // to this GSG.  This is likely because we tend to visit the same
  // state multiple times during a frame.  Also, this might well be
  // the only GSG in the world anyway.
  if (!state->_mungers.empty()) {
    RenderState::Mungers::const_iterator mi = state->_last_mi;
    if ((*mi).first == this && !(*mi).first.was_deleted()) {
      return (*mi).second;
    }
  }

  // Nope, we have to loop up in the map.
  RenderState::Mungers::const_iterator mi = state->_mungers.find(this);
  if (mi != state->_mungers.end() && !(*mi).first.was_deleted()) {
    ((RenderState *)state)->_last_mi = mi;
    return (*mi).second;
  }

  // Nothing in the map; create a new entry.
  PT(qpGeomMunger) munger = make_geom_munger(state);

  // Cast the RenderState to a non-const object.  We can do this
  // because we are only updating a cache within the RenderState, not
  // really changing any of its properties.
  RenderState *nc_state = (RenderState *)state;
  mi = nc_state->_mungers.insert(RenderState::Mungers::value_type(this, munger)).first;
  nc_state->_last_mi = mi;

  return munger;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::make_geom_munger
//       Access: Public, Virtual
//  Description: Creates a new GeomMunger object to munge vertices
//               appropriate to this GSG for the indicated state.
////////////////////////////////////////////////////////////////////
PT(qpGeomMunger) GraphicsStateGuardian::
make_geom_munger(const RenderState *state) {
  // The default implementation returns no munger at all, but
  // presumably, every kind of GSG needs some special munging action,
  // so real GSG's will override this to return something more
  // useful.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_state_and_transform
//       Access: Public, Virtual
//  Description: Simultaneously resets the render state and the
//               transform state.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
set_state_and_transform(const RenderState *state,
                        const TransformState *transform) {
  set_transform(transform);
  set_state(state);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_color_clear_value
//       Access: Public
//  Description: Sets the color that the next do_clear() command will set
//               the color buffer to
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
set_color_clear_value(const Colorf& value) {
  _color_clear_value = value;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_depth_clear_value
//       Access: Public
//  Description: Sets the depth that the next do_clear() command will set
//               the depth buffer to
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
set_depth_clear_value(const float value) {
  _depth_clear_value = value;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::clear
//       Access: Public
//  Description: Clears the framebuffer within the current
//               DisplayRegion, according to the flags indicated by
//               the given DrawableRegion object.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
clear(DrawableRegion *clearable) {
  PStatTimer timer(_clear_pcollector);

  int clear_buffer_type = 0;
  if (clearable->get_clear_color_active()) {
    clear_buffer_type |= clearable->get_draw_buffer_type();
    set_color_clear_value(clearable->get_clear_color());
  }
  if (clearable->get_clear_depth_active()) {
    clear_buffer_type |= RenderBuffer::T_depth;
    set_depth_clear_value(clearable->get_clear_depth());
  }

  if (clear_buffer_type != 0) {
    prepare_display_region();
    do_clear(get_render_buffer(clear_buffer_type));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_lens
//       Access: Public, Virtual
//  Description: Makes the current lens (whichever lens was most
//               recently specified with set_scene()) active, so
//               that it will transform future rendered geometry.
//               Normally this is only called from the draw process,
//               and usually it is called by set_scene().
//
//               The return value is true if the lens is acceptable,
//               false if it is not.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
prepare_lens() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_frame
//       Access: Public, Virtual
//  Description: Called before each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup before
//               beginning the frame.
//
//               The return value is true if successful (in which case
//               the frame will be drawn and end_frame() will be
//               called later), or false if unsuccessful (in which
//               case nothing will be drawn and end_frame() will not
//               be called).
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
begin_frame() {
  // Now we know the GSG is the currently active context, so this is a
  // good time to release any textures or geoms that had been queued
  // up to release in the past frame, and load up any newly requested
  // textures.
  _prepared_objects->update(this);

#ifdef DO_PSTATS
  // For Pstats to track our current texture memory usage, we have to
  // reset the set of current textures each frame.
  init_frame_pstats();

  // But since we don't get sent a new issue_texture() unless our
  // texture state has changed, we have to be sure to clear the
  // current texture state now.  A bit unfortunate, but probably not
  // measurably expensive.
  modify_state(get_untextured_state());
#endif
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_scene
//       Access: Public, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the beginning of drawing commands for a "scene"
//               (usually a particular DisplayRegion) within a frame.
//               All 3-D drawing commands, except the clear operation,
//               must be enclosed within begin_scene() .. end_scene().
//
//               The return value is true if successful (in which case
//               the scene will be drawn and end_scene() will be
//               called later), or false if unsuccessful (in which
//               case nothing will be drawn and end_scene() will not
//               be called).
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
begin_scene() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_scene
//       Access: Public, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the end of drawing commands for a "scene" (usually a
//               particular DisplayRegion) within a frame.  All 3-D
//               drawing commands, except the clear operation, must be
//               enclosed within begin_scene() .. end_scene().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
end_scene() {
  // We should clear this pointer now, so that we don't keep unneeded
  // reference counts dangling.
  _scene_setup = NULL;
  
  // Undo any lighting we had enabled last scene, to force the lights
  // to be reissued, in case their parameters or positions have
  // changed between scenes.
  if (_lighting_enabled_this_frame) {
    for (int i = 0; i < (int)_light_info.size(); i++) {
      if (_light_info[i]._enabled) {
        enable_light(i, false);
        _light_info[i]._enabled = false;
      }
      _light_info[i]._light = NodePath();
    }

    // Also force the lighting state to unlit, so that issue_light()
    // will be guaranteed to be called next frame even if we have the
    // same set of light pointers we had this frame.
    //    modify_state(get_unlit_state());

    _lighting_enabled_this_frame = false;
  }

  // Ditto for the clipping planes.
  if (_clip_planes_enabled_this_frame) {
    for (int i = 0; i < (int)_clip_plane_info.size(); i++) {
      if (_clip_plane_info[i]._enabled) {
        enable_clip_plane(i, false);
        _clip_plane_info[i]._enabled = false;
      }
      _clip_plane_info[i]._plane = (PlaneNode *)NULL;
    }

    //    modify_state(get_unclipped_state());

    _clip_planes_enabled_this_frame = false;
  }

  // Actually, just clear all the state between scenes.
  set_state(RenderState::make_empty());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_frame
//       Access: Public, Virtual
//  Description: Called after each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup after
//               rendering the frame, and before the window flips.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
end_frame() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::wants_normals
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
wants_normals() const {
  return (_lighting_enabled || (_force_normals != 0));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::wants_texcoords
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
wants_texcoords() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::wants_colors
//       Access: Public, Virtual
//  Description: Returns true if the GSG should issue geometry color
//               commands, false otherwise.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
wants_colors() const {
  return _vertex_colors_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::depth_offset_decals
//       Access: Public, Virtual
//  Description: Returns true if this GSG can implement decals using a
//               DepthOffsetAttrib, or false if that is unreliable
//               and the three-step rendering process should be used
//               instead.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
depth_offset_decals() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_decal_base_first
//       Access: Public, Virtual
//  Description: Called during draw to begin a three-step rendering
//               phase to draw decals.  The first step,
//               begin_decal_base_first(), is called prior to drawing the
//               base geometry.  It should set up whatever internal
//               state is appropriate, as well as returning a
//               RenderState object that should be applied to the base
//               geometry for rendering.
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
begin_decal_base_first() {
  // Turn off writing the depth buffer to render the base geometry.
  static CPT(RenderState) decal_base_first;
  if (decal_base_first == (const RenderState *)NULL) {
    decal_base_first = RenderState::make
      (DepthWriteAttrib::make(DepthWriteAttrib::M_off),
       RenderState::get_max_priority());
  }
  return decal_base_first;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_decal_nested
//       Access: Public, Virtual
//  Description: Called during draw to begin a three-step rendering
//               phase to draw decals.  The second step,
//               begin_decal_nested(), is called after drawing the
//               base geometry and prior to drawing any of the nested
//               decal geometry that is to be applied to the base
//               geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
begin_decal_nested() {
  // We should keep the depth buffer off during this operation, so
  // that decals on decals will render properly.
  static CPT(RenderState) decal_nested;
  if (decal_nested == (const RenderState *)NULL) {
    decal_nested = RenderState::make
      (DepthWriteAttrib::make(DepthWriteAttrib::M_off),
       RenderState::get_max_priority());
  }
  return decal_nested;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_decal_base_second
//       Access: Public, Virtual
//  Description: Called during draw to begin a three-step rendering
//               phase to draw decals.  The third step,
//               begin_decal_base_second(), is called after drawing the
//               base geometry and the nested decal geometry, and
//               prior to drawing the base geometry one more time (if
//               needed).
//
//               It should return a RenderState object appropriate for
//               rendering the base geometry the second time, or NULL
//               if it is not necessary to re-render the base
//               geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
begin_decal_base_second() {
  // Now let the depth buffer go back on, but turn off writing the
  // color buffer to render the base geometry after the second pass.
  // Also, turn off texturing since there's no need for it now.
  static CPT(RenderState) decal_base_second;
  if (decal_base_second == (const RenderState *)NULL) {
    decal_base_second = RenderState::make
      (ColorWriteAttrib::make(ColorWriteAttrib::M_off),
       // On reflection, we need to leave texturing on so the alpha
       // test mechanism can work (if it is enabled, e.g. we are
       // rendering an object with M_dual transparency).
       //       TextureAttrib::make_off(),
       RenderState::get_max_priority());
  }
  return decal_base_second;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::finish_decal
//       Access: Public, Virtual
//  Description: Called during draw to clean up after decals are
//               finished.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
finish_decal() {
  // No need to do anything special here.
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_draw_primitives()
//       Access: Public, Virtual
//  Description: Called before a sequence of draw_primitive()
//               functions are called, this should prepare the vertex
//               data for rendering.  It returns true if the vertices
//               are ok, false to abort this group of primitives.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
begin_draw_primitives(const qpGeom *, const qpGeomMunger *munger,
                      const qpGeomVertexData *data) {
  _munger = munger;
  _vertex_data = data;

  return _vertex_data->has_vertex();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_triangles
//       Access: Public, Virtual
//  Description: Draws a series of disconnected triangles.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
draw_triangles(const qpGeomTriangles *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_tristrips
//       Access: Public, Virtual
//  Description: Draws a series of triangle strips.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
draw_tristrips(const qpGeomTristrips *primitive) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_trifans
//       Access: Public, Virtual
//  Description: Draws a series of triangle fans.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
draw_trifans(const qpGeomTrifans *primitive) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_lines
//       Access: Public, Virtual
//  Description: Draws a series of disconnected line segments.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
draw_lines(const qpGeomLines *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_linestrips
//       Access: Public, Virtual
//  Description: Draws a series of line strips.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
draw_linestrips(const qpGeomLinestrips *primitive) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_points
//       Access: Public, Virtual
//  Description: Draws a series of disconnected points.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
draw_points(const qpGeomPoints *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_draw_primitives()
//       Access: Public, Virtual
//  Description: Called after a sequence of draw_primitive()
//               functions are called, this should do whatever cleanup
//               is appropriate.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
end_draw_primitives() {
  _munger = NULL;
  _vertex_data = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::framebuffer_bind_to_texture
//       Access: Public, Virtual
//  Description: Works in lieu of copy_texture() to bind the primary
//               render buffer of the framebuffer to the indicated
//               texture (which must have been created via
//               GraphicsOutput::setup_render_texture()).
//
//               If supported by the graphics backend, this will make
//               the framebuffer memory directly accessible within the
//               texture, but the frame cannot be rendered again until
//               framebuffer_release_texture() is called.
//
//               The return value is true if successful, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
framebuffer_bind_to_texture(GraphicsOutput *, Texture *) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::framebuffer_release_texture
//       Access: Public, Virtual
//  Description: Undoes a previous call to
//               framebuffer_bind_to_texture().  The framebuffer may
//               again be rendered into, and the contents of the
//               texture is undefined.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
framebuffer_release_texture(GraphicsOutput *, Texture *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_coordinate_system
//       Access: Public
//  Description: Changes the coordinate system in effect on this
//               particular gsg.  This is also called the "external"
//               coordinate system, since it is the coordinate system
//               used by the scene graph, external to to GSG.
//
//               Normally, this will be the default coordinate system,
//               but it might be set differently at runtime.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
set_coordinate_system(CoordinateSystem cs) {
  _coordinate_system = cs;

  // Changing the external coordinate system changes the cs_transform.
  if (_internal_coordinate_system == CS_default ||
      _internal_coordinate_system == _coordinate_system) {
    _cs_transform = TransformState::make_identity();

  } else {
    _cs_transform = 
      TransformState::make_mat
      (LMatrix4f::convert_mat(_coordinate_system,
                              _internal_coordinate_system));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_internal_coordinate_system
//       Access: Public, Virtual
//  Description: Returns the coordinate system used internally by the
//               GSG.  This may be the same as the external coordinate
//               system reported by get_coordinate_system(), or it may
//               be something different.
//
//               In any case, vertices that have been transformed
//               before being handed to the GSG (that is, vertices
//               with a contents value of C_clip_point) will be
//               expected to be in this coordinate system.
////////////////////////////////////////////////////////////////////
CoordinateSystem GraphicsStateGuardian::
get_internal_coordinate_system() const {
  return _internal_coordinate_system;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::issue_transform
//       Access: Public, Virtual
//  Description: Sends the indicated transform matrix to the graphics
//               API to be applied to future vertices.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
issue_transform(const TransformState *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::issue_color_scale
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
issue_color_scale(const ColorScaleAttrib *attrib) {
  _color_scale_enabled = attrib->has_scale();
  _current_color_scale = attrib->get_scale();

  _scene_graph_color_stale = _has_scene_graph_color;

  if (_color_blend_involves_color_scale) {
    _blend_mode_stale = true;
  }
  if (_texture_involves_color_scale) {
    _texture_stale = true;
  }
  if (_color_scale_via_lighting) {
    _light_stale = true;
    _material_stale = true;

    determine_light_color_scale();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::issue_color
//       Access: Public, Virtual
//  Description: This method is defined in the base class because it
//               is likely that this functionality will be used for
//               all (or at least most) kinds of
//               GraphicsStateGuardians--it's not specific to any one
//               rendering backend.
//
//               The ColorAttribute just changes the interpretation of
//               the color on the vertices, and fiddles with
//               _vertex_colors_enabled, etc.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
issue_color(const ColorAttrib *attrib) {
  switch (attrib->get_color_type()) {
  case ColorAttrib::T_flat:
    // Color attribute flat: it specifies a scene graph color that
    // overrides the vertex color.
    _scene_graph_color = attrib->get_color();
    _has_scene_graph_color = true;
    _vertex_colors_enabled = false;
    _scene_graph_color_stale = true;
    break;

  case ColorAttrib::T_off:
    // Color attribute off: it specifies that no scene graph color is
    // in effect, and vertex color is not important either.
    _has_scene_graph_color = false;
    _scene_graph_color_stale = false;
    _vertex_colors_enabled = false;
    break;

  case ColorAttrib::T_vertex:
    // Color attribute vertex: it specifies that vertex color should
    // be revealed.
    _has_scene_graph_color = false;
    _scene_graph_color_stale = false;
    _vertex_colors_enabled = true;
    break;
  }

  if (_color_scale_via_lighting) {
    _light_stale = true;
    _material_stale = true;

    determine_light_color_scale();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::issue_light
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
issue_light(const LightAttrib *attrib) {
  // By default, we don't apply the light attrib right away, since
  // it might have a dependency on the current ColorScaleAttrib.
  _pending_light = attrib;
  _light_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::issue_material
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
issue_material(const MaterialAttrib *attrib) {
  // By default, we don't apply the material attrib right away, since
  // it might have a dependency on the current ColorScaleAttrib.
  _pending_material = attrib;
  _material_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::issue_color_write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
issue_color_write(const ColorWriteAttrib *attrib) {
  _color_write_mode = attrib->get_mode();
  _blend_mode_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::issue_transparency
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
issue_transparency(const TransparencyAttrib *attrib) {
  _transparency_mode = attrib->get_mode();
  _blend_mode_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::issue_color_blend
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
issue_color_blend(const ColorBlendAttrib *attrib) {
  _color_blend = attrib;
  _color_blend_mode = attrib->get_mode();
  _color_blend_involves_color_scale = attrib->involves_color_scale();
  _blend_mode_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::issue_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
issue_texture(const TextureAttrib *attrib) {
  // By default, we don't apply the texture attrib right away, since
  // it might have a dependency on the current ColorScaleAttrib.
  _pending_texture = attrib;
  _texture_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::issue_clip_plane
//       Access: Public, Virtual
//  Description: This is fundametically similar to issue_light(), with
//               calls to slot_new_clip_plane(), apply_clip_plane(),
//               and enable_clip_planes(), as appropriate.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
issue_clip_plane(const ClipPlaneAttrib *attrib) {
  int i;
  int max_planes = (int)_clip_plane_info.size();
  for (i = 0; i < max_planes; i++) {
    _clip_plane_info[i]._next_enabled = false;
  }

  bool any_bound = false;

  int num_enabled = 0;
  int num_on_planes = attrib->get_num_on_planes();
  for (int li = 0; li < num_on_planes; li++) {
    NodePath plane = attrib->get_on_plane(li);
    nassertv(!plane.is_empty() && plane.node()->is_of_type(PlaneNode::get_class_type()));

    num_enabled++;

    // Clipping should be enabled before we apply any planes.
    enable_clip_planes(true);
    _clip_planes_enabled = true;
    _clip_planes_enabled_this_frame = true;

    // Check to see if this plane has already been bound to an id
    int cur_plane_id = -1;
    for (i = 0; i < max_planes; i++) {
      if (_clip_plane_info[i]._plane == plane) {
        // Plane has already been bound to an id, we only need to
        // enable the plane, not reapply it.
        cur_plane_id = -2;
        enable_clip_plane(i, true);
        _clip_plane_info[i]._enabled = true;
        _clip_plane_info[i]._next_enabled = true;
        break;
      }
    }
        
    // See if there are any unbound plane ids
    if (cur_plane_id == -1) {
      for (i = 0; i < max_planes; i++) {
        if (_clip_plane_info[i]._plane.is_empty()) {
          _clip_plane_info[i]._plane = plane;
          cur_plane_id = i;
          break;
        }
      }
    }
        
    // If there were no unbound plane ids, see if we can replace
    // a currently unused but previously bound id
    if (cur_plane_id == -1) {
      for (i = 0; i < max_planes; i++) {
        if (!attrib->has_on_plane(_clip_plane_info[i]._plane)) {
          _clip_plane_info[i]._plane = plane;
          cur_plane_id = i;
          break;
        }
      }
    }

    // If we *still* don't have a plane id, slot a new one.
    if (cur_plane_id == -1) {
      if (slot_new_clip_plane(max_planes)) {
        cur_plane_id = max_planes;
        _clip_plane_info.push_back(ClipPlaneInfo());
        max_planes++;
        nassertv(max_planes == (int)_clip_plane_info.size());
      }
    }
        
    if (cur_plane_id >= 0) {
      enable_clip_plane(cur_plane_id, true);
      _clip_plane_info[cur_plane_id]._enabled = true;
      _clip_plane_info[cur_plane_id]._next_enabled = true;
      
      if (!any_bound) {
        begin_bind_clip_planes();
        any_bound = true;
      }
      
      // This is the first time this frame that this plane has been
      // bound to this particular id.
      bind_clip_plane(plane, cur_plane_id);
      
    } else if (cur_plane_id == -1) {
      gsg_cat.warning()
        << "Failed to bind " << plane << " to id.\n";
    }
  }

  // Disable all unused planes
  for (i = 0; i < max_planes; i++) {
    if (!_clip_plane_info[i]._next_enabled) {
      enable_clip_plane(i, false);
      _clip_plane_info[i]._enabled = false;
    }
  }

  // If no planes were enabled, disable clip planes in general.
  if (num_enabled == 0) {
    enable_clip_planes(false);
    _clip_planes_enabled = false;
  }

  if (any_bound) {
    end_bind_clip_planes();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
bind_light(PointLight *light_obj, const NodePath &light, int light_id) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
bind_light(DirectionalLight *light_obj, const NodePath &light, int light_id) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
bind_light(Spotlight *light_obj, const NodePath &light, int light_id) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::do_issue_light
//       Access: Protected
//  Description: This implementation of issue_light() assumes
//               we have a limited number of hardware lights
//               available.  This function assigns each light to a
//               different hardware light id, trying to keep each
//               light associated with the same id where possible, but
//               reusing id's when necessary.  When it is no longer
//               possible to reuse existing id's (e.g. all id's are in
//               use), slot_new_light() is called to prepare the next
//               sequential light id.
//
//               It will call apply_light() each time a light is
//               assigned to a particular id for the first time in a
//               given frame, and it will subsequently call
//               enable_light() to enable or disable each light as the
//               frame is rendered, as well as enable_lighting() to
//               enable or disable overall lighting.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
do_issue_light() {
  // Initialize the current ambient light total and newly enabled
  // light list
  Colorf cur_ambient_light(0.0f, 0.0f, 0.0f, 0.0f);
  int i;
  int max_lights = (int)_light_info.size();
  for (i = 0; i < max_lights; i++) {
    _light_info[i]._next_enabled = false;
  }

  bool any_bound = false;

  int num_enabled = 0;
  if (_pending_light != (LightAttrib *)NULL) {
    int num_on_lights = _pending_light->get_num_on_lights();
    for (int li = 0; li < num_on_lights; li++) {
      NodePath light = _pending_light->get_on_light(li);
      nassertv(!light.is_empty() && light.node()->as_light() != (Light *)NULL);
      Light *light_obj = light.node()->as_light();
      
      num_enabled++;
      
      // Lighting should be enabled before we apply any lights.
      enable_lighting(true);
      _lighting_enabled = true;
      _lighting_enabled_this_frame = true;
      
      if (light_obj->get_type() == AmbientLight::get_class_type()) {
        // Ambient lights don't require specific light ids; simply add
        // in the ambient contribution to the current total
        cur_ambient_light += light_obj->get_color();
        
      } else {
        // Check to see if this light has already been bound to an id
        int cur_light_id = -1;
        for (i = 0; i < max_lights; i++) {
          if (_light_info[i]._light == light) {
            // Light has already been bound to an id; reuse the same id.
            cur_light_id = -2;
            enable_light(i, true);
            _light_info[i]._enabled = true;
            _light_info[i]._next_enabled = true;
          
            if (!any_bound) {
              begin_bind_lights();
              any_bound = true;
            }
            light_obj->bind(this, light, i);
            break;
          }
        }
        
        // See if there are any unbound light ids
        if (cur_light_id == -1) {
          for (i = 0; i < max_lights; i++) {
            if (_light_info[i]._light.is_empty()) {
              _light_info[i]._light = light;
              cur_light_id = i;
              break;
            }
          }
        }
        
        // If there were no unbound light ids, see if we can replace
        // a currently unused but previously bound id
        if (cur_light_id == -1) {
          for (i = 0; i < max_lights; i++) {
            if (!_pending_light->has_on_light(_light_info[i]._light)) {
              _light_info[i]._light = light;
              cur_light_id = i;
              break;
            }
          }
        }
        
        // If we *still* don't have a light id, slot a new one.
        if (cur_light_id == -1) {
          if (slot_new_light(max_lights)) {
            cur_light_id = max_lights;
            _light_info.push_back(LightInfo());
            max_lights++;
            nassertv(max_lights == (int)_light_info.size());
          }
        }
        
        if (cur_light_id >= 0) {
          enable_light(cur_light_id, true);
          _light_info[cur_light_id]._enabled = true;
          _light_info[cur_light_id]._next_enabled = true;
          
          if (!any_bound) {
            begin_bind_lights();
            any_bound = true;
          }
          
          // This is the first time this frame that this light has been
          // bound to this particular id.
          light_obj->bind(this, light, cur_light_id);
          
        } else if (cur_light_id == -1) {
          gsg_cat.warning()
            << "Failed to bind " << light << " to id.\n";
        }
      }
    }
  }
    
  // Disable all unused lights
  for (i = 0; i < max_lights; i++) {
    if (!_light_info[i]._next_enabled) {
      enable_light(i, false);
      _light_info[i]._enabled = false;
    }
  }

  // If no lights were enabled, disable lighting
  if (num_enabled == 0) {
    if (_color_scale_via_lighting && (_has_material_force_color || _light_color_scale != LVecBase4f(1.0f, 1.0f, 1.0f, 1.0f))) {
      // Unless we need lighting anyway to apply a color or color
      // scale.
      enable_lighting(true);
      _lighting_enabled = true;
      _lighting_enabled_this_frame = true;
      set_ambient_light(Colorf(1.0f, 1.0f, 1.0f, 1.0f));

    } else {
      enable_lighting(false);
      _lighting_enabled = false;
    }

  } else {
    set_ambient_light(cur_ambient_light);
  }

  if (any_bound) {
    end_bind_lights();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::do_issue_material
//       Access: Protected, Virtual
//  Description: Should be overridden by derived classes to actually
//               apply the material saved in _pending_material.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
do_issue_material() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::do_issue_texture
//       Access: Protected, Virtual
//  Description: Should be overridden by derived classes to actually
//               apply the texture saved in _pending_texture.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
do_issue_texture() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::slot_new_light
//       Access: Protected, Virtual
//  Description: This will be called by the base class before a
//               particular light id will be used for the first time.
//               It is intended to allow the derived class to reserve
//               any additional resources, if required, for the new
//               light; and also to indicate whether the hardware
//               supports this many simultaneous lights.
//
//               The return value should be true if the additional
//               light is supported, or false if it is not.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
slot_new_light(int light_id) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::enable_lighting
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable or disable the use of lighting overall.  This
//               is called by issue_light() according to whether any
//               lights are in use or not.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
enable_lighting(bool enable) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_ambient_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               indicate the color of the ambient light that should
//               be in effect.  This is called by issue_light() after
//               all other lights have been enabled or disabled.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
set_ambient_light(const Colorf &color) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::enable_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated light id.  A specific Light will
//               already have been bound to this id via bind_light().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
enable_light(int light_id, bool enable) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_bind_lights
//       Access: Protected, Virtual
//  Description: Called immediately before bind_light() is called,
//               this is intended to provide the derived class a hook
//               in which to set up some state (like transform) that
//               might apply to several lights.
//
//               The sequence is: begin_bind_lights() will be called,
//               then one or more bind_light() calls, then
//               end_bind_lights().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
begin_bind_lights() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_bind_lights
//       Access: Protected, Virtual
//  Description: Called after before bind_light() has been called one
//               or more times (but before any geometry is issued or
//               additional state is changed), this is intended to
//               clean up any temporary changes to the state that may
//               have been made by begin_bind_lights().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
end_bind_lights() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::slot_new_clip_plane
//       Access: Protected, Virtual
//  Description: This will be called by the base class before a
//               particular clip plane id will be used for the first
//               time.  It is intended to allow the derived class to
//               reserve any additional resources, if required, for
//               the new clip plane; and also to indicate whether the
//               hardware supports this many simultaneous clipping
//               planes.
//
//               The return value should be true if the additional
//               plane is supported, or false if it is not.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
slot_new_clip_plane(int plane_id) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::enable_clip_planes
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable or disable the use of clipping planes overall.
//               This is called by issue_clip_plane() according to
//               whether any planes are in use or not.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
enable_clip_planes(bool enable) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::enable_clip_plane
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated plane id.  A specific PlaneNode
//               will already have been bound to this id via
//               bind_clip_plane().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
enable_clip_plane(int plane_id, bool enable) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_bind_clip_planes
//       Access: Protected, Virtual
//  Description: Called immediately before bind_clip_plane() is called,
//               this is intended to provide the derived class a hook
//               in which to set up some state (like transform) that
//               might apply to several planes.
//
//               The sequence is: begin_bind_clip_planes() will be
//               called, then one or more bind_clip_plane() calls,
//               then end_bind_clip_planes().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
begin_bind_clip_planes() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::bind_clip_plane
//       Access: Public, Virtual
//  Description: Called the first time a particular clipping plane has been
//               bound to a given id within a frame, this should set
//               up the associated hardware (or API) clipping plane
//               with the plane's properties.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
bind_clip_plane(const NodePath &plane, int plane_id) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_bind_clip_planes
//       Access: Protected, Virtual
//  Description: Called after before bind_clip_plane() has been called one
//               or more times (but before any geometry is issued or
//               additional state is changed), this is intended to
//               clean up any temporary changes to the state that may
//               have been made by begin_bind_clip_planes().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
end_bind_clip_planes() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_blend_mode
//       Access: Protected, Virtual
//  Description: Called after any of the things that might change
//               blending state have changed, this function is
//               responsible for setting the appropriate color
//               blending mode based on the current properties.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
set_blend_mode() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::finish_modify_state
//       Access: Protected, Virtual
//  Description: Called after the GSG state has been modified via
//               modify_state() or set_state(), this hook is provided
//               for the derived class to do any further state setup
//               work.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
finish_modify_state() {
  if (_blend_mode_stale) {
    _blend_mode_stale = false;
    set_blend_mode();
  }

  if (_texture_stale) {
    _texture_stale = false;
    do_issue_texture();
  }

  if (_material_stale) {
    _material_stale = false;
    do_issue_material();
  }

  if (_light_stale) {
    _light_stale = false;
    do_issue_light();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::free_pointers
//       Access: Protected, Virtual
//  Description: Frees some memory that was explicitly allocated
//               within the glgsg.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
free_pointers() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::close_gsg
//       Access: Protected, Virtual
//  Description: This is called by the associated GraphicsWindow when
//               close_window() is called.  It should null out the
//               _win pointer and possibly free any open resources
//               associated with the GSG.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
close_gsg() {
  _closing_gsg = true;
  free_pointers();

  // If we're not sharing the prepared objects list with any other
  // GSG, go ahead and release all the textures and geoms now.  This
  // isn't really a reliable test of whether we are sharing this list,
  // but it's not too important if we get this wrong since this ought
  // to be an optional cleanup anyway.  (Presumably, the underlying
  // graphics API will properly clean up outstanding textures and
  // geoms when the last context using them is released.)
  if (_prepared_objects->get_ref_count() == 1) {
    release_all_textures();
    release_all_geoms();
    release_all_vertex_buffers();
    release_all_index_buffers();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::panic_deactivate
//       Access: Protected
//  Description: This is called internally when it is determined that
//               things are just fubar.  It temporarily deactivates
//               the GSG just so things don't get out of hand, and
//               throws an event so the application can deal with this
//               if it needs to.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
panic_deactivate() {
  if (_active) {
    display_cat.error()
      << "Deactivating " << get_type() << ".\n";
    set_active(false);
    throw_event("panic-deactivate-gsg", this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::determine_light_color_scale
//       Access: Protected
//  Description: Called whenever the color or color scale is changed,
//               if _color_scale_via_lighting is true.  This will
//               rederive _material_force_color and _light_color_scale
//               appropriately.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
determine_light_color_scale() {
  if (_has_scene_graph_color) {
    // If we have a scene graph color, it, plus the color scale, goes
    // directly into the material; we don't color scale the
    // lights--this allows an alpha color scale to work properly.
    _has_material_force_color = true;
    _material_force_color = _scene_graph_color;
    _light_color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);
    if (!_color_blend_involves_color_scale && _color_scale_enabled) {
      _material_force_color.set(_scene_graph_color[0] * _current_color_scale[0],
                                _scene_graph_color[1] * _current_color_scale[1],
                                _scene_graph_color[2] * _current_color_scale[2],
                                _scene_graph_color[3] * _current_color_scale[3]);
    }

  } else {
    // Otherise, leave the materials alone, but we might still scale
    // the lights.
    _has_material_force_color = false;
    _light_color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);
    if (!_color_blend_involves_color_scale && _color_scale_enabled) {
      _light_color_scale = _current_color_scale;
    }
  }
}

#ifdef DO_PSTATS

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::init_frame_pstats
//       Access: Protected
//  Description: Initializes the relevant PStats data at the beginning
//               of the frame.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
init_frame_pstats() {
  if (PStatClient::is_connected()) {
    _current_textures.clear();
    _current_geoms.clear();
    _current_vertex_buffers.clear();
    _current_index_buffers.clear();
    _active_texusage_pcollector.clear_level();
    _data_transferred_pcollector.clear_level();
    _active_geom_pcollector.clear_level();
    _active_geom_node_pcollector.clear_level();
    _active_vertex_buffers_pcollector.clear_level();
    _active_index_buffers_pcollector.clear_level();
    _vertex_buffer_switch_pcollector.clear_level();
    _index_buffer_switch_pcollector.clear_level();
    
    _primitive_batches_pcollector.clear_level();
    _primitive_batches_tristrip_pcollector.clear_level();
    _primitive_batches_trifan_pcollector.clear_level();
    _primitive_batches_tri_pcollector.clear_level();
    _primitive_batches_other_pcollector.clear_level();
    _vertices_tristrip_pcollector.clear_level();
    _vertices_trifan_pcollector.clear_level();
    _vertices_tri_pcollector.clear_level();
    _vertices_other_pcollector.clear_level();
    _vertices_indexed_tristrip_pcollector.clear_level();
    
    _state_pcollector.clear_level();
    _transform_state_pcollector.clear_level();
    _texture_state_pcollector.clear_level();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::add_to_texture_record
//       Access: Protected
//  Description: Records that the indicated texture has been applied
//               this frame, and thus must be present in current
//               texture memory.  This function is only used to update
//               the PStats current_texmem collector; it gets compiled
//               out if we aren't using PStats.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
add_to_texture_record(TextureContext *tc) {
  if (PStatClient::is_connected()) {
    if (_current_textures.insert(tc).second) {
      _active_texusage_pcollector.add_level(tc->estimate_texture_memory());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::add_to_geom_record
//       Access: Protected
//  Description: Records that the indicated Geom has been drawn this
//               frame.  This function is only used to update the
//               PStats active_geom collector; it gets compiled out
//               if we aren't using PStats.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
add_to_geom_record(GeomContext *gc) {
  if (PStatClient::is_connected()) {
    if (gc != (GeomContext *)NULL && _current_geoms.insert(gc).second) {
      _active_geom_pcollector.add_level(1);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::add_to_vertex_buffer_record
//       Access: Protected
//  Description: Records that the indicated data array has been drawn
//               this frame.  This function is only used to update the
//               PStats active_vertex_buffers collector; it gets
//               compiled out if we aren't using PStats.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
add_to_vertex_buffer_record(VertexBufferContext *vbc) {
  if (vbc != (VertexBufferContext *)NULL) {
    if (PStatClient::is_connected()) {
      _vertex_buffer_switch_pcollector.add_level(1);
      if (_current_vertex_buffers.insert(vbc).second) {
        _active_vertex_buffers_pcollector.add_level(vbc->get_data()->get_data_size_bytes());
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::add_to_index_buffer_record
//       Access: Protected
//  Description: Records that the indicated data array has been drawn
//               this frame.  This function is only used to update the
//               PStats active_index_buffers collector; it gets compiled out
//               if we aren't using PStats.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
add_to_index_buffer_record(IndexBufferContext *ibc) {
  if (ibc != (IndexBufferContext *)NULL) {
    if (PStatClient::is_connected()) {
      _index_buffer_switch_pcollector.add_level(1);
      if (_current_index_buffers.insert(ibc).second) {
        _active_index_buffers_pcollector.add_level(ibc->get_data()->get_data_size_bytes());
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::add_to_total_buffer_record
//       Access: Protected
//  Description: Records that the indicated data array has been loaded
//               this frame.  This function is only used to update the
//               PStats total_buffers collector; it gets
//               compiled out if we aren't using PStats.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
add_to_total_buffer_record(VertexBufferContext *vbc) {
  if (vbc != (VertexBufferContext *)NULL) {
    int delta = vbc->get_data()->get_data_size_bytes() - vbc->get_data_size_bytes();
    _total_buffers_pcollector.add_level(delta);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::add_to_total_buffer_record
//       Access: Protected
//  Description: Records that the indicated data array has been loaded
//               this frame.  This function is only used to update the
//               PStats total_buffers collector; it gets
//               compiled out if we aren't using PStats.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
add_to_total_buffer_record(IndexBufferContext *ibc) {
  if (ibc != (IndexBufferContext *)NULL) {
    int delta = ibc->get_data()->get_data_size_bytes() - ibc->get_data_size_bytes();
    _total_buffers_pcollector.add_level(delta);
  }
}

#endif  // DO_PSTATS

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_unlit_state
//       Access: Protected, Static
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
get_unlit_state() {
  static CPT(RenderState) state = NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make(LightAttrib::make_all_off());
  }
  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_unclipped_state
//       Access: Protected, Static
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
get_unclipped_state() {
  static CPT(RenderState) state = NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make(ClipPlaneAttrib::make_all_off());
  }
  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_untextured_state
//       Access: Protected, Static
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
get_untextured_state() {
  static CPT(RenderState) state = NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make(TextureAttrib::make_off());
  }
  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::traverse_prepared_textures
//       Access: Public
//  Description: Calls the indicated function on all
//               currently-prepared textures, or until the callback
//               function returns false.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
traverse_prepared_textures(bool (*pertex_callbackfn)(TextureContext *,void *),void *callback_arg) {
  PreparedGraphicsObjects::Textures::const_iterator ti;
  for (ti = _prepared_objects->_prepared_textures.begin(); 
       ti != _prepared_objects->_prepared_textures.end();
       ++ti) {
    bool bResult=(*pertex_callbackfn)(*ti,callback_arg);
    if(!bResult)
      return;
  }
}
