// Filename: ribGraphicsStateGuardian.h
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

#ifndef RIBGRAPHICSSTATEGUARDIAN_H
#define RIBGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include <graphicsStateGuardian.h>
#include <filename.h>

class Geom;
class Texture;
class Light;
class Material;
class DisplayRegion;
class RenderBuffer;
class PixelBuffer;
class Fog;

////////////////////////////////////////////////////////////////////
//       Class : RIBGraphicsStateGuardian
// Description : A GraphicsStateGuardian specialized for creating RIB
//               files, suitable for shipping off to a
//               Renderman-friendly non-real-time renderer.
////////////////////////////////////////////////////////////////////
class RIBGraphicsStateGuardian : public GraphicsStateGuardian {
public:
  RIBGraphicsStateGuardian(GraphicsWindow *win);

  virtual void reset();
  void reset_file(ostream &out);
  void reset_frame();

  virtual void clear(const RenderBuffer &buffer);
  virtual void clear(const RenderBuffer &buffer, const DisplayRegion *region);

  virtual void prepare_display_region();

  virtual void render_frame(const AllAttributesWrapper &initial_state);
  virtual void render_scene(Node *root, ProjectionNode *projnode,
                            const AllAttributesWrapper &initial_state);
  virtual void render_subgraph(RenderTraverser *traverser,
                               Node *subgraph, ProjectionNode *projnode,
                               const AllAttributesWrapper &initial_state,
                               const AllTransitionsWrapper &net_trans);
  virtual void render_subgraph(RenderTraverser *traverser,
                               Node *subgraph,
                               const AllAttributesWrapper &initial_state,
                               const AllTransitionsWrapper &net_trans);

  virtual bool wants_normals(void) const;
  virtual bool wants_texcoords(void) const;
  virtual bool wants_colors(void) const;

  virtual float compute_distance_to(const LPoint3f &point) const;

  virtual void draw_point(const GeomPoint *geom);
  virtual void draw_line(const GeomLine *geom);
  virtual void draw_linestrip(const GeomLinestrip *) { }
  virtual void draw_sprite(const GeomSprite *geom);
  virtual void draw_polygon(const GeomPolygon *geom);
  virtual void draw_quad(const GeomQuad *geom);
  virtual void draw_tri(const GeomTri *geom);
  virtual void draw_tristrip(const GeomTristrip *geom);
  virtual void draw_trifan(const GeomTrifan *geom);
  virtual void draw_sphere(const GeomSphere *geom);

  virtual TextureContext *prepare_texture(Texture *tex);
  virtual void apply_texture(TextureContext *tc);
  virtual void release_texture(TextureContext *tc);

  virtual void copy_texture(TextureContext *tc, const DisplayRegion *dr);
  virtual void copy_texture(TextureContext *tc, const DisplayRegion *dr,
                            const RenderBuffer &rb);
  virtual void draw_texture(TextureContext *tc, const DisplayRegion *dr);
  virtual void draw_texture(TextureContext *tc, const DisplayRegion *dr,
                            const RenderBuffer &rb);

  virtual void texture_to_pixel_buffer(TextureContext *, PixelBuffer *) { }
  virtual void texture_to_pixel_buffer(TextureContext *, PixelBuffer *,
                                const DisplayRegion *) { }

  virtual void copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr);
  virtual void copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                                 const RenderBuffer &rb);
  virtual void draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                                 const NodeAttributes& na=NodeAttributes());
  virtual void draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                                 const RenderBuffer &rb,
                                 const NodeAttributes& na=NodeAttributes());

  virtual void apply_material(Material*) { }
  virtual void apply_fog(Fog*) { }

  virtual void apply_light(PointLight*) { }
  virtual void apply_light(DirectionalLight*) { }
  virtual void apply_light(Spotlight*) { }
  virtual void apply_light(AmbientLight*) { }

  virtual void issue_transform(const TransformAttribute *attrib);
  virtual void issue_color(const ColorAttribute *attrib);
  virtual void issue_texture(const TextureAttribute *attrib);
  virtual void issue_light(const LightAttribute *attrib);

  // Normally, these functions are called through the RIBGraphicsWindow.
  void set_texture_directory(const string &directory);
  string get_texture_directory() const;
  void set_texture_extension(const string &extension);
  string get_texture_extension() const;

protected:
  virtual PT(SavedFrameBuffer) save_frame_buffer(const RenderBuffer &buffer,
                                                 CPT(DisplayRegion) dr);
  virtual void restore_frame_buffer(SavedFrameBuffer *frame_buffer);

  void set_color(const RGBColorf &color);

  void get_rib_stuff(Node *root, const AllAttributesWrapper &initial_state);
  void define_texture(const Texture *tex);
  void define_light(const Light *light);
  void write_light_color(const Colorf &color) const;
  void write_light_from(const Node *light) const;
  void write_light_to(const Node *light) const;

  ostream &new_line(int extra_indent = 0) const;
  void reset_transform(const LMatrix4f &mat) const;
  void concat_transform(const LMatrix4f &mat) const;

  void draw_simple_poly(const Geom *geom);
  void write_polygon(int num_verts);

  static void get_color_and_intensity(const RGBColorf &input,
                                      RGBColorf &output,
                                      float &intensity);

  RGBColorf _current_color;

  string _texture_directory;
  string _texture_extension;
  ostream *_output;
  int _indent_level;
  typedef pmap<const Texture *, Filename> TextureNames;
  TextureNames _texture_names;
  typedef pmap<const Light *, int> LightIDs;
  LightIDs _light_ids;
  typedef pvector<bool> EnabledLights;
  EnabledLights _enabled_lights;


public:
  static GraphicsStateGuardian *
  make_RIBGraphicsStateGuardian(const FactoryParams &params);

  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type(void);
  static void init_type(void);

private:
  static TypeHandle _type_handle;

  friend class RibStuffTraverser;
};

#endif

