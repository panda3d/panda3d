// Filename: config_gobj.cxx
// Created by:  drose (01Oct99)
// 

#include <config_util.h>
#include "config_gobj.h"
#include "LOD.h"
#include "drawable.h"
#include "fog.h"
#include "geom.h"
#include "geomprimitives.h"
#include "imageBuffer.h"
#include "material.h"
#include "orthoProjection.h"
#include "perspectiveProjection.h"
#include "pixelBuffer.h"
#include "projection.h"
#include "texture.h"

#include <dconfig.h>

Configure(config_gobj);
NotifyCategoryDef(gobj, "");

// Set this to the maximum size a texture is allowed to be in either
// dimension.  This is generally intended as a simple way to restrict
// texture sizes for limited graphics cards.  When this is greater
// than zero, each texture image loaded from a file (but only those
// loaded from a file) will be automatically scaled down, if
// necessary, so that neither dimension is larger than this value.
const int max_texture_dimension = config_gobj.GetInt("max-texture-dimension", -1);

// Set this to specify how textures should be written into Bam files.
// Currently, the options are:

//    fullpath - write the full pathname loaded.
//    relative - search for the texture as a filename relative to the
//               model-path or texture-path and write the relative pathname.
//    basename - write only the basename of the file, no directory portion
//               at all.

BamTextureMode bam_texture_mode;


static BamTextureMode
parse_texture_mode(const string &mode) {
  if (mode == "fullpath") {
    return BTM_fullpath;
  } else if (mode == "relative") {
    return BTM_relative;
  } else if (mode == "basename") {
    return BTM_basename;
  }

  gobj_cat.error() << "Invalid bam-texture-mode: " << mode << "\n";
  return BTM_relative;
}

ConfigureFn(config_gobj) {
  string texture_mode = config_util.GetString("bam-texture-mode", "relative");
  bam_texture_mode = parse_texture_mode(texture_mode);
  
  Fog::init_type();
  Geom::init_type();
  GeomLine::init_type();
  GeomLinestrip::init_type();
  GeomPoint::init_type();
  GeomSprite::init_type();
  GeomPolygon::init_type();
  GeomQuad::init_type();
  GeomSphere::init_type();
  GeomTri::init_type();
  GeomTrifan::init_type();
  GeomTristrip::init_type();
  ImageBuffer::init_type();
  LOD::init_type();
  Material::init_type();
  OrthoProjection::init_type();
  PerspectiveProjection::init_type();
  PixelBuffer::init_type();
  Projection::init_type();
  Texture::init_type();
  dDrawable::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  GeomPoint::register_with_read_factory();
  GeomLine::register_with_read_factory();
  GeomLinestrip::register_with_read_factory();
  GeomSprite::register_with_read_factory();
  GeomPolygon::register_with_read_factory();
  GeomQuad::register_with_read_factory();
  GeomTri::register_with_read_factory();
  GeomTristrip::register_with_read_factory();
  GeomTrifan::register_with_read_factory();
  GeomSphere::register_with_read_factory();
  Texture::register_with_read_factory();
}
