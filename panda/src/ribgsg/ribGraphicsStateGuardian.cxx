// Filename: ribGraphicsStateGuardian.cxx
// Created by:  drose (15Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "ribGraphicsStateGuardian.h"
#include "ribStuffTraverser.h"
#include "config_ribgsg.h"

#include <directRenderTraverser.h>
#include <displayRegion.h>
#include <projectionNode.h>
#include <projection.h>
#include <camera.h>
#include <renderBuffer.h>
#include <transformTransition.h>
#include <colorTransition.h>
#include <textureTransition.h>
#include <lightTransition.h>
#include <geom.h>
#include <geomprimitives.h>
#include <geomIssuer.h>
#include "graphicsWindow.h"
#include <graphicsChannel.h>
#include <indent.h>
#include <dftraverser.h>
#include <node.h>
#include <projectionNode.h>
#include <texture.h>
#include <textureContext.h>
#include <light.h>
#include <get_rel_pos.h>
#include <projection.h>
#include <perspectiveProjection.h>
#include <frustum.h>
#include <ambientLight.h>
#include <directionalLight.h>
#include <pointLight.h>
#include <spotlight.h>
#include "pandabase.h"

#include <assert.h>

TypeHandle RIBGraphicsStateGuardian::_type_handle;

// Here are some global arrays we use in the various draw() routines
// to hold temporary coordinate values for each Geom for formatting.

static pvector<Vertexf> rib_vertices;
static pvector<Normalf> rib_normals;
static pvector<TexCoordf> rib_texcoords;
static pvector<RGBColorf> rib_colors;

static void
issue_vertex_rib(const Geom *geom, Geom::VertexIterator &vi) {
  rib_vertices.push_back(geom->get_next_vertex(vi));
}

static void
issue_normal_rib(const Geom *geom, Geom::NormalIterator &ni) {
  rib_normals.push_back(geom->get_next_normal(ni));
}

static void
issue_texcoord_rib(const Geom *geom, Geom::TexCoordIterator &ti) {
  // We need to reverse the V coordinate for RIB.
  static LMatrix3f
    texmat(1.0, 0.0, 0.0,
           0.0, -1.0, 0.0,
           0.0, 1.0, 1.0);
  rib_texcoords.push_back(geom->get_next_texcoord(ti) * texmat);
}

static void
issue_color_rib(const Geom *geom, Geom::ColorIterator &ci,
                const GraphicsStateGuardianBase *) {
  // RIB only cares about three-component color, so we have to convert
  // the four-component color transition to three-component color here.
  rib_colors.push_back((const RGBColorf &)geom->get_next_color(ci));
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
RIBGraphicsStateGuardian::
RIBGraphicsStateGuardian(GraphicsWindow *win) : GraphicsStateGuardian(win) {
  reset();

  // Create a default RenderTraverser.
  _render_traverser =
    new DirectRenderTraverser(this, RenderRelation::get_class_type());

  _texture_directory = "maps";
  _texture_extension = "tiff";
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Resets all internal state and prepares a new RIB
//               file.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
reset() {
  GraphicsStateGuardian::reset();

  // We only have a color buffer in RenderMan.
  _buffer_mask = RenderBuffer::T_color;

  _output = NULL;
  _indent_level = 0;

  // We clear the texture names only for each file, not for each
  // frame, because texture definitions remain across frames.
  _texture_names.clear();

  reset_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::reset_file
//       Access: Public
//  Description: Resets all internal state and prepares a new RIB
//               file to the indicated output stream.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
reset_file(ostream &out) {
  reset();
  _output = &out;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::reset_frame
//       Access: Public
//  Description: Resets whatever state is appropriate at the end of a
//               frame.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
reset_frame() {
  // We must clear the light definitions for each frame.
  _light_ids.clear();
  _enabled_lights.clear();
  _enabled_lights.push_back(true);

  _current_color.set(1.0, 1.0, 1.0);
  _state.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::clear
//       Access: Public, Virtual
//  Description: Clears all of the indicated buffers to their assigned
//               colors.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
clear(const RenderBuffer &) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::clear
//       Access: Public, Virtual
//  Description: Clears all of the indicated buffers to their assigned
//               colors.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
clear(const RenderBuffer &, const DisplayRegion* ) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::prepare_display_region
//       Access: Public
//  Description: Prepare a display region for rendering (set up
//               scissor region and viewport)
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
prepare_display_region() {
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::render_frame
//       Access: Public, Virtual
//  Description: Renders an entire frame, including all display
//               regions within the frame, and includes any necessary
//               pre- and post-processing.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
render_frame(const AllTransitionsWrapper &initial_state) {
  _win->begin_frame();
  assert(_output != NULL);
  _indent_level += 2;

  int max_channel_index = _win->get_max_channel_index();
  for (int c = 0; c < max_channel_index; c++) {
    if (_win->is_channel_defined(c)) {
      GraphicsChannel *chan = _win->get_channel(c);
      if (chan->is_active()) {
        int num_layers = chan->get_num_layers();
        for (int l = 0; l < num_layers; l++) {
          GraphicsLayer *layer = chan->get_layer(l);
          if (layer->is_active()) {
            int num_drs = layer->get_num_drs();
            for (int d = 0; d < num_drs; d++) {
              DisplayRegion *dr = layer->get_dr(d);
              Camera *cam = dr->get_camera();

              // For each display region, render from the camera's view.
              if (dr->is_active() && cam != (Camera *)NULL &&
                  cam->is_active() && cam->get_scene() != (Node *)NULL) {
                DisplayRegionStack old_dr = push_display_region(dr);
                prepare_display_region();
                render_scene(cam->get_scene(), cam, initial_state);
                pop_display_region(old_dr);
              }
            }
          }
        }
      }
    }
  }

  _indent_level -= 2;
  _win->end_frame();

  reset_frame();
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::render_scene
//       Access: Public, Virtual
//  Description: Renders an entire scene, from the root node of the
//               scene graph, as seen from a particular ProjectionNode
//               and with a given initial state.  This initial state
//               may be modified during rendering.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
render_scene(Node *root, ProjectionNode *projnode,
             const AllTransitionsWrapper &initial_state) {
  _current_root_node = root;

  render_subgraph(_render_traverser, root, projnode,
                  initial_state, AllTransitionsWrapper());
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::render_subgraph
//       Access: Public, Virtual
//  Description: Renders a subgraph of the scene graph as seen from a
//               given projection node, and with a particular initial
//               state.  This state may be modified by the render
//               process.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
render_subgraph(RenderTraverser *traverser,
                Node *subgraph, ProjectionNode *projnode,
                const AllTransitionsWrapper &initial_state,
                const AllTransitionsWrapper &net_trans) {
  ProjectionNode *old_projection_node = _current_projection_node;
  _current_projection_node = projnode;

  (*_output) << "\n";

  int width = _win->get_width();
  int height = _win->get_height();
  float frame_aspect = (float)width / (float)height;

  // BMRT, for one, doesn't seem like general matrices for the
  // projection matrix.  Therefore, we'll examine the projection type
  // and write out the high-level description of the projection if we
  // can, instead of just dumping a matrix.

  const Projection *projection = projnode->get_projection();
  if (projection->is_of_type(PerspectiveProjection::get_class_type())) {
    const PerspectiveProjection &pp =
      *DCAST(PerspectiveProjection, projection);
    const Frustumf &frustum = pp.get_frustum();

    float yfov, fnear, ffar;
    frustum.get_perspective_params(yfov, frame_aspect, fnear, ffar);

    switch (_coordinate_system) {
    case CS_zup_right:
      new_line() << "Scale [ 1 1 -1 ]\n";      // left-handed to right-handed
      new_line() << "Rotate [ -90 1 0 0 ]\n";  // y-up to z-up
      break;

    case CS_zup_left:
      new_line() << "Rotate [ -90 1 0 0 ]\n";  // y-up to z-up
      break;

    case CS_yup_right:
      new_line() << "Scale [ 1 1 -1 ]\n";      // left-handed to right-handed
      break;

    case CS_yup_left:
      break;
    };

    new_line() << "Orientation \"lh\"\n";
    new_line() << "Clipping " << fnear << " " << ffar << "\n";
    new_line() << "Projection \"perspective\" \"fov\" " << yfov << "\n";

  } else {
    // Hmm, some unknown projection type.  We'll have to just write
    // out the projection matrix and hope for the best.
    LMatrix4f proj_mat = projection->get_projection_mat(_coordinate_system);
    concat_transform(proj_mat);

    new_line() << "Orientation \"lh\"\n";
    new_line() << "Projection \"null\"\n";
  }

  new_line() << "Format " << width << " " << height << " "
             << (float)height * frame_aspect / (float)width << "\n";
  new_line() << "FrameAspectRatio " << frame_aspect << "\n";

  new_line() << "Sides 1\n";
  new_line() << "Color [ 1 1 1 ]\n";

  // Get the lights and stuff, while we're here in camera space.
  get_rib_stuff(_current_root_node, initial_state);

  // We infer the modelview matrix by doing a wrt on the projection
  // node.
  LMatrix4f modelview_mat;
  get_rel_mat(subgraph, _current_projection_node, modelview_mat);
  concat_transform(modelview_mat);

  /*
  // And then we must make sure the matrix transform is cleared from
  // the initial state.
  initial_state.clear_attribute(TransformTransition::get_class_type());
  */

  new_line() << "WorldBegin\n";
  _indent_level += 2;

  render_subgraph(traverser, subgraph, initial_state, net_trans);

  _indent_level -= 2;
  new_line() << "WorldEnd\n";
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::render_subgraph
//       Access: Public, Virtual
//  Description: Renders a subgraph of the scene graph as seen from the
//               current projection node, and with a particular
//               initial state.  This state may be modified during the
//               render process.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
render_subgraph(RenderTraverser *traverser,
                Node *subgraph,
                const AllTransitionsWrapper &initial_state,
                const AllTransitionsWrapper &net_trans) {
  nassertv(traverser != (RenderTraverser *)NULL);
  traverser->traverse(subgraph, initial_state, net_trans);
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::wants_normals
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool RIBGraphicsStateGuardian::
wants_normals() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::wants_texcoords
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool RIBGraphicsStateGuardian::
wants_texcoords() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::wants_colors
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool RIBGraphicsStateGuardian::
wants_colors() const {
  // If we have scene graph color enabled, return false to indicate we
  // shouldn't bother issuing geometry color commands.

  const ColorTransition *catt;
  if (!get_transition_into(catt, _state, ColorTransition::get_class_type())) {
    // No scene graph color at all.
    return true;
  }

  // We should issue geometry colors only if the scene graph color is
  // off.
  return catt->is_off();
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::compute_distance_to
//       Access: Public, Virtual
//  Description: This function may only be called during a render
//               traversal; it will compute the distance to the
//               indicated point, assumed to be in modelview
//               coordinates, from the camera plane.
////////////////////////////////////////////////////////////////////
float RIBGraphicsStateGuardian::
compute_distance_to(const LPoint3f &point) const {
  // In the case of a RIBGraphicsStateGuardian, we know the modelview
  // matrix does not include the camera transform, so we should apply
  // that now.  But for now we'll punt, since no one cares anyway.

  return point[1];
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_point
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_point(const GeomPoint *) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_line
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_line(const GeomLine *) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_sprite
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_sprite(const GeomSprite *) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_polygon
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_polygon(const GeomPolygon *geom) {
  draw_simple_poly(geom);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_tri
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_tri(const GeomTri *geom) {
  draw_simple_poly(geom);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_quad
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_quad(const GeomQuad *geom) {
  draw_simple_poly(geom);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_tristrip
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_tristrip(const GeomTristrip *geom) {
  Geom *temp = geom->explode();
  draw_simple_poly(temp);
  delete temp;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_trifan
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_trifan(const GeomTrifan *geom) {
  Geom *temp = geom->explode();
  draw_simple_poly(temp);
  delete temp;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_sphere(const GeomSphere *) {
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::prepare_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TextureContext *RIBGraphicsStateGuardian::
prepare_texture(Texture *tex) {
  TextureContext *tc = new TextureContext(tex);

  bool inserted = mark_prepared_texture(tc);

  // If this assertion fails, the same texture was prepared twice,
  // which shouldn't be possible, since the texture itself should
  // detect this.
  assert(inserted);

  return tc;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::apply_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
apply_texture(TextureContext *) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::release_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
release_texture(TextureContext *tc) {
  Texture *tex = tc->_texture;

  bool erased = unmark_prepared_texture(tc);

  // If this assertion fails, a texture was released that hadn't been
  // prepared (or a texture was released twice).
  assert(erased);

  tex->clear_gsg(this);

  delete tc;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::copy_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
copy_texture(TextureContext *, const DisplayRegion *) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::copy_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
copy_texture(TextureContext *, const DisplayRegion *, const RenderBuffer &) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_texture(TextureContext *, const DisplayRegion *) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_texture(TextureContext *, const DisplayRegion *, const RenderBuffer &) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::copy_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
copy_pixel_buffer(PixelBuffer *, const DisplayRegion *) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::copy_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
copy_pixel_buffer(PixelBuffer *, const DisplayRegion *, const RenderBuffer &) {
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_pixel_buffer(PixelBuffer *, const DisplayRegion *,
        const NodeTransitions &) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_pixel_buffer(PixelBuffer *, const DisplayRegion *, const RenderBuffer &,
        const NodeTransitions &) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::issue_transform
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
issue_transform(const TransformTransition *attrib) {
  reset_transform(attrib->get_matrix());
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::issue_color
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
issue_color(const ColorTransition *attrib) {
  if (attrib->is_on() && attrib->is_real()) {
    const Colorf c = attrib->get_color();
    set_color(RGBColorf(c[0], c[1], c[2]));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::issue_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
issue_texture(const TextureTransition *attrib) {
  if (attrib->is_off()) {
    // If no textures are enabled, we can use the nontextured shader.
    new_line()
      << "Surface \"plastic\"\n";

  } else {
    // If we have a texture enabled, just use the normal
    // paintedplastic shader.

    Texture *tex = attrib->get_texture();
    nassertv(tex != (Texture *)NULL);
    const Filename &rib_name = _texture_names[tex];

    // We should have gotten all the texture names already in the
    // get_rib_stuff() call.  If this name is empty, we somehow
    // missed it!
    nassertv(!rib_name.empty());

    new_line()
      << "Surface \"paintedplastic\" \"texturename\" \""
      << rib_name << "\"\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::issue_light
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
issue_light(const LightTransition *attrib) {
  nassertv(attrib->get_properties_is_on());
  int num_enabled = attrib->size();
  if (num_enabled == 0) {
    // If no lights are enabled, lighting is off.  Turn off all lights
    // except the default one.

    // (if the default light is already on, we'll assume everything
    // else is already off and won't bother to run the list.)
    if (!_enabled_lights[0]) {
      for (int i = 1; i < _enabled_lights.size(); i++) {
        if (_enabled_lights[i]) {
          new_line() << "Illuminate " << i << " 0\n";
          _enabled_lights[i] = false;
        }
      }

      // And turn on the default light, which illuminates the scene in
      // the absence of lighting.
      new_line() << "Illuminate 0 1\n";
      _enabled_lights[0] = true;
    }

  } else {
    // If some lights are enabled, we'll turn off the ones that were
    // enabled from before, and turn on the ones we need.

    LightTransition::const_iterator li;
    for (li = attrib->begin(); li != attrib->end(); ++li) {
      Light *light = (*li);
      LightIDs::const_iterator ii = _light_ids.find(light);
      assert(ii != _light_ids.end());
      int id = (*ii).second;

      if (!_enabled_lights[id]) {
        new_line() << "Illuminate " << id << " 1\n";
      }

      // We'll temporarily set the enabled flag to false, even
      // though we've just activated the light.  This is so we can
      // later identify the lights we need to turn off.
      _enabled_lights[id] = false;
    }

    // Now turn off all the lights that are still marked "on".
    for (int i = 0; i < _enabled_lights.size(); i++) {
      if (_enabled_lights[i]) {
        new_line() << "Illuminate " << i << " 0\n";
        _enabled_lights[i] = false;
      }
    }

    // Finally, mark as "on" all the ones that we just enabled.
    for (li = attrib->begin(); li != attrib->end(); ++li) {
      Light *light = (*li);
      int id = _light_ids[light];
      _enabled_lights[id] = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::set_texture_directory
//       Access: Public
//  Description: Sets the name of the directory into which texture
//               maps are copied to be available to the RIB file.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
set_texture_directory(const string &directory) {
  _texture_directory = directory;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::get_texture_directory
//       Access: Public
//  Description: Returns the name of the directory into which texture
//               maps are copied to be available to the RIB file.
////////////////////////////////////////////////////////////////////
string RIBGraphicsStateGuardian::
get_texture_directory() const {
  return _texture_directory;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::set_texture_extension
//       Access: Public
//  Description: Specifies the filename extension that texture map
//               files are given when they are copied into the
//               directory for RIB files.  This might also imply an
//               image type.  The default is "tiff", which implies
//               TIFF files.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
set_texture_extension(const string &extension) {
  _texture_extension = extension;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::get_texture_extension
//       Access: Public
//  Description: Returns the filename extension that texture map
//               files are given when they are copied into the
//               directory for RIB files.
////////////////////////////////////////////////////////////////////
string RIBGraphicsStateGuardian::
get_texture_extension() const {
  return _texture_extension;
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::save_frame_buffer
//       Access: Public
//  Description: Saves the indicated planes of the frame buffer
//               (within the indicated display region) and returns it
//               in some meaningful form that can be restored later
//               via restore_frame_buffer().  This is a helper
//               function for push_frame_buffer() and
//               pop_frame_buffer().
////////////////////////////////////////////////////////////////////
PT(SavedFrameBuffer) RIBGraphicsStateGuardian::
save_frame_buffer(const RenderBuffer &buffer,
                  CPT(DisplayRegion) dr) {
  return new SavedFrameBuffer(buffer, dr);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::restore_frame_buffer
//       Access: Public
//  Description: Restores the frame buffer that was previously saved.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
restore_frame_buffer(SavedFrameBuffer *) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::set_color
//       Access: Protected
//  Description: Issues the sequence to change the node color to that
//               indicated.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
set_color(const RGBColorf &color) {
  if (_current_color != color) {
    new_line() << "Color [ " << color << " ]\n";
    _current_color = color;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::get_rib_stuff
//       Access: Protected
//  Description: Traverses the scene graph to identify any textures
//               or lights, or anything that we need to define up
//               front in RIB.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
get_rib_stuff(Node *root, const AllTransitionsWrapper &initial_state) {
  RibStuffTraverser trav(this);
  df_traverse(root, trav, initial_state, NullLevelState(),
              RenderRelation::get_class_type());
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::define_texture
//       Access: Protected
//  Description: Called by the RibStuffTraverser (initiated above),
//               this defines a single texture object in the RIB file
//               if it has not already been defined.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
define_texture(const Texture *tex) {
  Filename &rib_name = _texture_names[tex];

  if (rib_name.empty()) {
    Filename image_filename = tex->get_name();
    image_filename.set_dirname(_texture_directory);
    image_filename.set_extension(_texture_extension);
    tex->write(image_filename);

    rib_name = image_filename;
    rib_name.set_extension("tx");

    new_line() << "MakeTexture \"" << image_filename << "\"\n";
    new_line(12) << "\"" << rib_name << "\"";

    if (tex->get_wrapu() == Texture::WM_clamp) {
      (*_output) << " \"clamp\"";
    } else {
      (*_output) << " \"periodic\"";
    }

    if (tex->get_wrapv() == Texture::WM_clamp) {
      (*_output) << " \"clamp\"";
    } else {
      (*_output) << " \"periodic\"";
    }

    (*_output) << " \"box\" 1 1\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::define_light
//       Access: Protected
//  Description: Called by the RibStuffTraverser (initiated above),
//               this defines a single light object in the RIB file
//               if it has not already been defined.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
define_light(const Light *light) {
  LightIDs::const_iterator li = _light_ids.find(light);
  if (li == _light_ids.end()) {
    // This is the first time this light has been encountered; define
    // it.

    // Create a new ID number.
    int id = _light_ids.size() + 1;
    _light_ids[light] = id;
    assert(id == _enabled_lights.size());
    _enabled_lights.push_back(false);

    if (light->get_light_type() == PointLight::get_class_type()) {
      const PointLight *plight = (const PointLight *)light;
      new_line() << "LightSource \"pointlight\" " << id;
      write_light_color(plight->get_color());
      write_light_from(plight);
      (*_output) << "\n";

    } else if (light->get_light_type() == DirectionalLight::get_class_type()) {
      const DirectionalLight *dlight = (const DirectionalLight *)light;
      new_line() << "LightSource \"distantlight\" " << id;
      write_light_color(dlight->get_color());
      write_light_from(dlight);
      write_light_to(dlight);
      (*_output) << "\n";

    } else if (light->get_light_type() == Spotlight::get_class_type()) {
      const Spotlight *slight = (const Spotlight *)light;
      new_line() << "LightSource \"spotlight\" " << id;
      write_light_color(slight->get_color());
      write_light_from(slight);
      write_light_to(slight);
      (*_output)
        << " \"coneangle\" " << deg_2_rad(slight->get_cutoff_angle())
        << "\n";

    } else if (light->get_light_type() == AmbientLight::get_class_type()) {
      const AmbientLight *alight = (const AmbientLight *)light;
      new_line() << "LightSource \"ambientlight\" " << id;
      write_light_color(alight->get_color());
      (*_output) << "\n";

    } else {
      cerr << "Ignoring unknown light type " << light->get_light_type() << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::write_light_color
//       Access: Protected
//  Description: Called by define_light() to write out a single
//               light's color and intensity values.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
write_light_color(const Colorf &color) const {
  RGBColorf output_color;
  float intensity;

  get_color_and_intensity((const RGBColorf &)color, output_color, intensity);
  (*_output) << " \"lightcolor\" [ " << output_color << " ] \"intensity\" "
             << intensity;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::write_light_from
//       Access: Protected
//  Description: Called by define_light() to write out a single
//               light's position.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
write_light_from(const Node *light) const {
  LPoint3f pos = get_rel_pos(light, _current_projection_node);

  (*_output) << " \"from\" [ " << pos << " ]";
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::write_light_to
//       Access: Protected
//  Description: Called by define_light() to write out a single
//               light's direction.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
write_light_to(const Node *light) const {
  LPoint3f pos = get_rel_pos(light, _current_projection_node);
  LVector3f forward = get_rel_forward(light, _current_projection_node);

  (*_output) << " \"to\" [ " << pos + forward << " ]";
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::new_line
//       Access: Protected
//  Description: Beins a new line of output at the current indenting
//               level.  (Does not actually issue the newline
//               character, however).
////////////////////////////////////////////////////////////////////
ostream &RIBGraphicsStateGuardian::
new_line(int extra_indent) const {
  return indent(*_output, _indent_level + extra_indent);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::reset_transform
//       Access: Protected
//  Description: Outputs an RiTransform command with the given
//               transformation matrix, which resets the current
//               transformation to that specified.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
reset_transform(const LMatrix4f &mat) const {
  new_line() << "Transform [ " << mat(0,0) << " " << mat(0,1) << " "
             << mat(0,2) << " " << mat(0,3) << "\n";
  new_line(12) << mat(1,0) << " " << mat(1,1) << " "
               << mat(1,2) << " " << mat(1,3) << "\n";
  new_line(12) << mat(2,0) << " " << mat(2,1) << " "
               << mat(2,2) << " " << mat(2,3) << "\n";
  new_line(12) << mat(3,0) << " " << mat(3,1) << " "
               << mat(3,2) << " " << mat(3,3) << " ]\n";
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::concat_transform
//       Access: Protected
//  Description: Outputs an RiTransform command with the given
//               transformation matrix, which composes the specified
//               matrix with the current transformation.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
concat_transform(const LMatrix4f &mat) const {
  new_line() << "ConcatTransform [ " << mat(0,0) << " " << mat(0,1) << " "
             << mat(0,2) << " " << mat(0,3) << "\n";
  new_line(18) << mat(1,0) << " " << mat(1,1) << " "
               << mat(1,2) << " " << mat(1,3) << "\n";
  new_line(18) << mat(2,0) << " " << mat(2,1) << " "
               << mat(2,2) << " " << mat(2,3) << "\n";
  new_line(18) << mat(3,0) << " " << mat(3,1) << " "
               << mat(3,2) << " " << mat(3,3) << " ]\n";
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::draw_simple_poly
//       Access: Protected
//  Description: Draws a GeomPolygon, GeomTri, or GeomQuad object.
//               This consists of one or more unconnected polygons.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
draw_simple_poly(const Geom *geom) {
  if (geom == NULL) {
    return;
  }

  int nprims = geom->get_num_prims();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer issuer(geom, this,
                    issue_vertex_rib,
                    issue_normal_rib,
                    issue_texcoord_rib,
                    issue_color_rib);

  for (int i = 0; i < nprims; i++) {
    // First, for each primitive, we build up the various polygon
    // attributes in our global arrays.

    // If the colors or normals have overall binding, we'll need to
    // repeat it for each primitive.  Thus, we need to reset the
    // iterator for each primitive.
    if (geom->get_binding(G_COLOR) == G_OVERALL) {
      ci = geom->make_color_iterator();
    }
    if (geom->get_binding(G_NORMAL) == G_OVERALL) {
      ni = geom->make_normal_iterator();
    }

    // Draw overall
    issuer.issue_color(G_OVERALL, ci);
    issuer.issue_normal(G_OVERALL, ni);

    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);
    issuer.issue_normal(G_PER_PRIM, ni);

    for (int j = 0; j < geom->get_length(i); j++) {
      // Draw per vertex
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }

    write_polygon(geom->get_length(i));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::write_polygon
//       Access: Protected
//  Description: Writes out the RIB command to draw the polygon
//               described by the global rib_* arrays.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
write_polygon(int num_verts) {
  if (num_verts < 3) {
    return;
  }

  assert(rib_vertices.size() == num_verts);

  if (rib_colors.size() == 1) {
    // This polygon has a flat color; just issue the color.
    set_color(rib_colors[0]);
    rib_colors.clear();
  }


  // We reverse the order of the vertices when we write then out,
  // because RIB has a clockwise-ordering convention.

  // Vertices are always per-vertex.
  write_long_list(*_output, _indent_level,
                  rib_vertices.rbegin(), rib_vertices.rend(),
                  "Polygon \"P\" ", "            ", 72);

  if (rib_normals.size() == 1) {
    // A single polygon normal.
    write_long_list(*_output, _indent_level,
                    rib_normals.rbegin(), rib_normals.rend(),
                    "       \"Np\" ", "            ", 72);
  } else if (!rib_normals.empty()) {
    // Multiple per-vertex normals.
    assert(rib_normals.size() == num_verts);
    write_long_list(*_output, _indent_level,
                    rib_normals.rbegin(), rib_normals.rend(),
                    "        \"N\" ", "            ", 72);
  }

  if (!rib_texcoords.empty()) {
    // Per-vertex texcoords.
    assert(rib_texcoords.size() == num_verts);
    write_long_list(*_output, _indent_level,
                    rib_texcoords.rbegin(), rib_texcoords.rend(),
                    "       \"st\" ", "            ", 72);
  }

  if (!rib_colors.empty()) {
    assert(rib_colors.size() == num_verts);

    write_long_list(*_output, _indent_level,
                    rib_colors.rbegin(), rib_colors.rend(),
                    "       \"Cs\" ", "            ", 72);
  }

  // Clear the arrays for the next primitive.
  rib_vertices.clear();
  rib_normals.clear();
  rib_texcoords.clear();
  rib_colors.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateGuardian::get_color_and_intensity
//       Access: Protected, Static
//  Description: Given a three-component color value, extracts it into
//               a normalized three-component color with each
//               component in the range [0..1], and a separate
//               intensity value.
////////////////////////////////////////////////////////////////////
void RIBGraphicsStateGuardian::
get_color_and_intensity(const RGBColorf &input,
                        RGBColorf &output,
                        float &intensity) {
  intensity = max(max(input[0], input[1]), input[2]);
  if (intensity == 0.0) {
    output.set(1.0, 1.0, 1.0);
  } else {
    output = input / intensity;
  }
}

// type and factory stuff

GraphicsStateGuardian *RIBGraphicsStateGuardian::
make_RIBGraphicsStateGuardian(const FactoryParams &params) {
  GraphicsStateGuardian::GsgWindow *win_param;
  if (!get_param_into(win_param, params)) {
    ribgsg_cat.error()
      << "No window specified for gsg creation!" << endl;
    return NULL;
  }

  GraphicsWindow *win = win_param->get_window();
  return new RIBGraphicsStateGuardian(win);
}

TypeHandle RIBGraphicsStateGuardian::get_type(void) const {
  return get_class_type();
}

TypeHandle RIBGraphicsStateGuardian::get_class_type(void) {
  return _type_handle;
}

void RIBGraphicsStateGuardian::init_type(void) {
  GraphicsStateGuardian::init_type();
  register_type(_type_handle, "RIBGraphicsStateGuardian",
                GraphicsStateGuardian::get_class_type());
}
