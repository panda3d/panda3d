// Filename: graphicsStateGuardianBase.h
// Created by:  drose (06Oct99)
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

#ifndef GRAPHICSSTATEGUARDIANBASE_H
#define GRAPHICSSTATEGUARDIANBASE_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "luse.h"

// A handful of forward references.

class RenderBuffer;
class GraphicsWindow;
class NodePath;

class VertexBufferContext;
class IndexBufferContext;
class GeomContext;
class GeomNode;
class Geom;
class GeomPoint;
class GeomLine;
class GeomLinestrip;
class GeomSprite;
class GeomPolygon;
class GeomQuad;
class GeomTri;
class GeomTristrip;
class GeomTrifan;
class GeomSphere;
class Geom;
class GeomVertexData;
class GeomVertexArrayData;
class GeomPrimitive;
class GeomTriangles;
class GeomTristrips;
class GeomTrifans;
class GeomLines;
class GeomLinestrips;
class GeomPoints;
class GeomMunger;

class PreparedGraphicsObjects;
class GraphicsOutput;
class Texture;
class TextureContext;
class ShaderContext;
class ShaderExpansion;
class RenderState;
class TransformState;
class Material;

class ColorScaleAttrib;
class TexMatrixAttrib;
class ColorAttrib;
class TextureAttrib;
class LightAttrib;
class MaterialAttrib;
class RenderModeAttrib;
class AntialiasAttrib;
class RescaleNormalAttrib;
class ColorBlendAttrib;
class ColorWriteAttrib;
class AlphaTestAttrib;
class DepthTestAttrib;
class DepthWriteAttrib;
class TexGenAttrib;
class ShaderAttrib;
class CullFaceAttrib;
class ClipPlaneAttrib;
class ShadeModelAttrib;
class TransparencyAttrib;
class FogAttrib;
class DepthOffsetAttrib;

class PointLight;
class DirectionalLight;
class Spotlight;
class AmbientLight;

class DisplayRegion;
class Lens;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsStateGuardianBase
// Description : This is a base class for the GraphicsStateGuardian
//               class, which is itself a base class for the various
//               GSG's for different platforms.  This class contains
//               all the function prototypes to support the
//               double-dispatch of GSG to geoms, transitions, etc.  It
//               lives in a separate class in its own package so we
//               can avoid circular build dependency problems.
//
//               GraphicsStateGuardians are not actually writable to
//               bam files, of course, but they may be passed as event
//               parameters, so they inherit from
//               TypedWritableReferenceCount instead of
//               TypedReferenceCount for that convenience.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsStateGuardianBase : public TypedWritableReferenceCount {
PUBLISHED:
  virtual bool get_supports_multisample() const=0;
  virtual int get_supported_geom_rendering() const=0;

public:
  // These are some general interface functions; they're defined here
  // mainly to make it easy to call these from code in some directory
  // that display depends on.
  virtual PreparedGraphicsObjects *get_prepared_objects()=0;

  virtual TextureContext *prepare_texture(Texture *tex)=0;
  virtual void release_texture(TextureContext *tc)=0;

  virtual GeomContext *prepare_geom(Geom *geom)=0;
  virtual void release_geom(GeomContext *gc)=0;

  virtual ShaderContext *prepare_shader(ShaderExpansion *shader)=0;
  virtual void release_shader(ShaderContext *sc)=0;
  
  virtual VertexBufferContext *prepare_vertex_buffer(GeomVertexArrayData *data)=0;
  virtual void release_vertex_buffer(VertexBufferContext *vbc)=0;

  virtual IndexBufferContext *prepare_index_buffer(GeomPrimitive *data)=0;
  virtual void release_index_buffer(IndexBufferContext *ibc)=0;

  virtual PT(GeomMunger) get_geom_munger(const RenderState *state)=0;

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform)=0;

  // This function may only be called during a render traversal; it
  // will compute the distance to the indicated point, assumed to be
  // in eye coordinates, from the camera plane.  This is a virtual
  // function because different GSG's may define the eye coordinate
  // space differently.
  virtual float compute_distance_to(const LPoint3f &point) const=0;

  // These are used to implement decals.  If depth_offset_decals()
  // returns true, none of the remaining functions will be called,
  // since depth offsets can be used to implement decals fully (and
  // usually faster).
  virtual bool depth_offset_decals()=0;
  virtual CPT(RenderState) begin_decal_base_first()=0;
  virtual CPT(RenderState) begin_decal_nested()=0;
  virtual CPT(RenderState) begin_decal_base_second()=0;
  virtual void finish_decal()=0;

  // Defined here are some internal interface functions for the
  // GraphicsStateGuardian.  These are here to support
  // double-dispatching from Geoms and NodeTransitions, and are
  // intended to be invoked only directly by the appropriate Geom and
  // NodeTransition types.  They're public only because it would be too
  // inconvenient to declare each of those types to be friends of this
  // class.

  virtual void draw_point(GeomPoint *geom, GeomContext *gc) { }
  virtual void draw_line(GeomLine *geom, GeomContext *gc) { }
  virtual void draw_linestrip(GeomLinestrip *geom, GeomContext *gc) { }
  virtual void draw_sprite(GeomSprite *geom, GeomContext *gc) { }
  virtual void draw_polygon(GeomPolygon *geom, GeomContext *gc) { }
  virtual void draw_quad(GeomQuad *geom, GeomContext *gc) { }
  virtual void draw_tri(GeomTri *geom, GeomContext *gc) { }
  virtual void draw_tristrip(GeomTristrip *geom, GeomContext *gc) { }
  virtual void draw_trifan(GeomTrifan *geom, GeomContext *gc) { }
  virtual void draw_sphere(GeomSphere *geom, GeomContext *gc) { }

  virtual bool begin_draw_primitives(const Geom *geom, 
                                     const GeomMunger *munger,
                                     const GeomVertexData *vertex_data)=0;
  virtual void draw_triangles(const GeomTriangles *primitive)=0;
  virtual void draw_tristrips(const GeomTristrips *primitive)=0;
  virtual void draw_trifans(const GeomTrifans *primitive)=0;
  virtual void draw_lines(const GeomLines *primitive)=0;
  virtual void draw_linestrips(const GeomLinestrips *primitive)=0;
  virtual void draw_points(const GeomPoints *primitive)=0;
  virtual void end_draw_primitives()=0;

  virtual void framebuffer_copy_to_texture
  (Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb)=0;
  virtual bool framebuffer_copy_to_ram
  (Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb)=0;
  
  virtual CoordinateSystem get_internal_coordinate_system() const=0;

  virtual void bind_light(PointLight *light_obj, const NodePath &light, 
                          int light_id) { }
  virtual void bind_light(DirectionalLight *light_obj, const NodePath &light,
                          int light_id) { }
  virtual void bind_light(Spotlight *light_obj, const NodePath &light,
                          int light_id) { }

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "GraphicsStateGuardianBase",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
