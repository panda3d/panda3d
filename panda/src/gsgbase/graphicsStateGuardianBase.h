// Filename: graphicsStateGuardianBase.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef GRAPHICSSTATEGUARDIANBASE_H
#define GRAPHICSSTATEGUARDIANBASE_H

#include <pandabase.h>

#include <typedReferenceCount.h>
#include <nodeTransition.h>
#include <nodeAttributes.h>

// A handful of forward references.

class RenderBuffer;
class GraphicsWindow;

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

class TextureContext;
class Texture;
class PixelBuffer;

class Material;
class Fog;

class TransformAttribute;
class ColorMatrixAttribute;
class AlphaTransformAttribute;
class TexMatrixAttribute;
class ColorAttribute;
class TextureAttribute;
class LightAttribute;
class MaterialAttribute;
class RenderModeAttribute;
class ColorBlendAttribute;
class TextureApplyAttribute;
class ColorMaskAttribute;
class DepthTestAttribute;
class DepthWriteAttribute;
class TexGenAttribute;
class CullFaceAttribute;
class StencilAttribute;
class ClipPlaneAttribute;
class TransparencyAttribute;
class FogAttribute;
class LinesmoothAttribute;
class PointShapeAttribute;
class PolygonOffsetAttribute;

class Node;
class GeomNode;
class PointLight;
class DirectionalLight;
class Spotlight;
class AmbientLight;

class DisplayRegion;
class Projection;
class ProjectionNode;

////////////////////////////////////////////////////////////////////
// 	 Class : GraphicsStateGuardianBase
// Description : This is a base class for the GraphicsStateGuardian
//               class, which is itself a base class for the various
//               GSG's for different platforms.  This class contains
//               all the function prototypes to support the
//               double-dispatch of GSG to geoms, attributes, etc.  It
//               lives in a separate class in its own package so we
//               can avoid circular build dependency problems.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsStateGuardianBase : public TypedReferenceCount {
public:
  // These functions will be queried by the GeomIssuer to determine if
  // it should issue normals, texcoords, and/or colors, based on the
  // GSG's current state.
  virtual bool wants_normals(void) const=0;
  virtual bool wants_texcoords(void) const=0;
  virtual bool wants_colors(void) const=0;


  // Defined here are some internal interface functions for the
  // GraphicsStateGuardian.  These are here to support
  // double-dispatching from Geoms and NodeAttributes, and are
  // intended to be invoked only directly by the appropriate Geom and
  // NodeAttribute types.  They're public only because it would be too
  // inconvenient to declare each of those types to be friends of this
  // class.

  virtual void draw_point(const GeomPoint *geom)=0;
  virtual void draw_line(const GeomLine *geom)=0;
  virtual void draw_linestrip(const GeomLinestrip *geom)=0;
  virtual void draw_sprite(const GeomSprite *geom)=0;
  virtual void draw_polygon(const GeomPolygon *geom)=0;
  virtual void draw_quad(const GeomQuad *geom)=0;
  virtual void draw_tri(const GeomTri *geom)=0;
  virtual void draw_tristrip(const GeomTristrip *geom)=0;
  virtual void draw_trifan(const GeomTrifan *geom)=0;
  virtual void draw_sphere(const GeomSphere *geom)=0;

  virtual TextureContext *prepare_texture(Texture *tex)=0;
  virtual void apply_texture(TextureContext *tc)=0;
  virtual void release_texture(TextureContext *tc)=0;

  virtual void copy_texture(TextureContext *tc, const DisplayRegion *dr)=0;
  virtual void copy_texture(TextureContext *tc, const DisplayRegion *dr,
			    const RenderBuffer &rb)=0;
  virtual void draw_texture(TextureContext *tc, const DisplayRegion *dr)=0;
  virtual void draw_texture(TextureContext *tc, const DisplayRegion *dr,
			    const RenderBuffer &rb)=0;

  virtual void texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb)=0;
  virtual void texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb,
                                const DisplayRegion *dr)=0;

  virtual void copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr)=0;
  virtual void copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
				 const RenderBuffer &rb)=0;
  virtual void draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
				 const NodeAttributes& na=NodeAttributes())=0;
  virtual void draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
				 const RenderBuffer &rb,
				 const NodeAttributes& na=NodeAttributes())=0;

  virtual void apply_material(const Material *material)=0;
  virtual void apply_fog(Fog *fog)=0;

  virtual void apply_light(PointLight *light)=0;
  virtual void apply_light(DirectionalLight *light)=0;
  virtual void apply_light(Spotlight *light)=0;
  virtual void apply_light(AmbientLight *light)=0;

  virtual void issue_transform(const TransformAttribute *) { }
  virtual void issue_color_transform(const ColorMatrixAttribute *) { }
  virtual void issue_alpha_transform(const AlphaTransformAttribute *) { }
  virtual void issue_tex_matrix(const TexMatrixAttribute *) { }
  virtual void issue_color(const ColorAttribute *) { }
  virtual void issue_texture(const TextureAttribute *) { }
  virtual void issue_light(const LightAttribute *) { }
  virtual void issue_material(const MaterialAttribute *) { }
  virtual void issue_render_mode(const RenderModeAttribute *) { }
  virtual void issue_color_blend(const ColorBlendAttribute *) { }
  virtual void issue_texture_apply(const TextureApplyAttribute *) { }
  virtual void issue_color_mask(const ColorMaskAttribute *) { }
  virtual void issue_depth_test(const DepthTestAttribute *) { }
  virtual void issue_depth_write(const DepthWriteAttribute *) { }
  virtual void issue_tex_gen(const TexGenAttribute *) { }
  virtual void issue_cull_face(const CullFaceAttribute *) { }
  virtual void issue_stencil(const StencilAttribute *) { }
  virtual void issue_clip_plane(const ClipPlaneAttribute *) { }
  virtual void issue_transparency(const TransparencyAttribute *) { }
  virtual void issue_fog(const FogAttribute *) { }
  virtual void issue_linesmooth(const LinesmoothAttribute *) { }
  virtual void issue_point_shape(const PointShapeAttribute *) { }
  virtual void issue_polygon_offset(const PolygonOffsetAttribute *) { }

  virtual void begin_decal(GeomNode *) { }
  virtual void end_decal(GeomNode *) { }

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "GraphicsStateGuardianBase",
 		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#endif
