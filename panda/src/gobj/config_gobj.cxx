// Filename: config_gobj.cxx
// Created by:  drose (01Oct99)
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

#include "config_util.h"
#include "boundedObject.h"
#include "config_gobj.h"
#include "drawable.h"
#include "geom.h"
#include "geomprimitives.h"
#include "imageBuffer.h"
#include "material.h"
#include "orthographicLens.h"
#include "matrixLens.h"
#include "perspectiveLens.h"
#include "pixelBuffer.h"
#include "lens.h"
#include "texture.h"

#include "dconfig.h"
#include "string_utils.h"

Configure(config_gobj);
NotifyCategoryDef(gobj, "");

// Set this to the maximum size a texture is allowed to be in either
// dimension.  This is generally intended as a simple way to restrict
// texture sizes for limited graphics cards.  When this is greater
// than zero, each texture image loaded from a file (but only those
// loaded from a file) will be automatically scaled down, if
// necessary, so that neither dimension is larger than this value.
const int max_texture_dimension = config_gobj.GetInt("max-texture-dimension", -1);

// Set textures-power-2 to force texture dimensions to a power of two.
// If this is "up" or "down", the textures will be scaled up or down
// to the next power of two, as indicated; otherwise, if this is #t,
// the textures will be scaled down.  If this is #f or unspecified,
// the textures will be left at whatever size they are.

// These are filled in by the ConfigureFn block, below.
bool textures_up_power_2 = false;
bool textures_down_power_2 = false;

// Set textures-square to force texture dimensions to a square aspect
// ratio.  This works similarly to textures-power-2, above.  If this
// is "up" or "down", the textures will be scaled up or down to the
// containing square or the inscribed square, respectively; otherwise,
// if this is #t, the textures will be scaled down.  If this is #f or
// unspecified, the textures will be left at whatever size they are.

// These are filled in by the ConfigureFn block, below.
bool textures_up_square = false;
bool textures_down_square = false;


// Set this to true to retain the ram image for each texture after it
// has been prepared with the GSG.  This will allow the texture to be
// prepared with multiple GSG, or to be re-prepared later after it is
// explicitly released from the GSG, without having to reread the
// texture image from disk; but it will consume memory somewhat
// wastefully.
bool keep_texture_ram = config_gobj.GetBool("keep-texture-ram", false);

// Ditto for Geom's.  This is a little more dangerous, because if
// anyone calls release_all_geoms() on the GSG, we won't be able to
// restore them automatically.
bool keep_geom_ram = config_gobj.GetBool("keep-geom-ram", true);

// Set this true to allow the use of retained mode rendering, which
// creates specific cache information (like display lists or vertex
// buffers) with the GSG for static geometry, when supported by the
// GSG.  Set it false to use only immediate mode, which sends the
// vertices to the GSG every frame.
bool retained_mode = config_gobj.GetBool("retained-mode", false);


// Set this to specify how textures should be written into Bam files.
// Currently, the options are:

//    fullpath - write the full pathname loaded.
//    relative - search for the texture as a filename relative to the
//               model-path or texture-path and write the relative pathname.
//    basename - write only the basename of the file, no directory portion
//               at all.

BamTextureMode bam_texture_mode;

// Set this to enable a speedy-load mode where you don't care what the
// world looks like, you just want it to load in minimal time.  This
// causes all texture loads via the TexturePool to load the same
// texture file, which will presumably only be loaded once.
const string fake_texture_image = config_gobj.GetString("fake-texture-image", "");

// must be set to true for LOD_number debugging items to work
const bool debug_LOD_mode = config_gobj.GetBool("debug-LOD-mode", false);

// if this is >=0, select_child always returns this LOD number
const int select_LOD_number = config_gobj.GetInt("select-LOD-number", -1);

// this controls the LOD child number returned by compute_child
// use it to force the LOD alg to not select the highest level(s) of LOD
// minimum_LOD_number=0 will screen out no LODs, and increasing it
// will screen out successively higher levels
const int minimum_LOD_number = config_gobj.GetInt("minimum-LOD-number", 0);

const float lod_stress_factor = config_gobj.GetFloat("lod-stress-factor", 1.0f);

// The default near and far plane distances.
const float default_near = config_gobj.GetFloat("default-near", 1.0f);
const float default_far = config_gobj.GetFloat("default-far", 1000.0f);

// The default camera FOV.
const float default_fov = config_gobj.GetFloat("default-fov", 40.0f);

static BamTextureMode
parse_texture_mode(const string &mode) {
  if (cmp_nocase(mode, "unchanged") == 0) {
    return BTM_unchanged;
  } else if (cmp_nocase(mode, "fullpath") == 0) {
    return BTM_fullpath;
  } else if (cmp_nocase(mode, "relative") == 0) {
    return BTM_relative;
  } else if (cmp_nocase(mode, "basename") == 0) {
    return BTM_basename;
  } else if (cmp_nocase(mode, "rawdata") == 0) {
    return BTM_rawdata;
  }

  gobj_cat->error() << "Invalid bam-texture-mode: " << mode << "\n";
  return BTM_relative;
}

ConfigureFn(config_gobj) {
  string texture_mode = config_util.GetString("bam-texture-mode", "relative");
  bam_texture_mode = parse_texture_mode(texture_mode);

  string textures_power_2 = config_gobj.GetString("textures-power-2", "");
  if (cmp_nocase(textures_power_2, "up") == 0) {
    textures_up_power_2 = true;
    textures_down_power_2 = false;

  } else if (cmp_nocase(textures_power_2, "down") == 0) {
    textures_up_power_2 = false;
    textures_down_power_2 = true;

  } else {
    textures_up_power_2 = false;
    textures_down_power_2 = config_gobj.GetBool("textures-power-2", false);
  }

  string textures_square = config_gobj.GetString("textures-square", "");
  if (cmp_nocase(textures_square, "up") == 0) {
    textures_up_square = true;
    textures_down_square = false;

  } else if (cmp_nocase(textures_square, "down") == 0) {
    textures_up_square = false;
    textures_down_square = true;

  } else {
    textures_up_square = false;
    textures_down_square = config_gobj.GetBool("textures-square", false);
  }

  BoundedObject::init_type();
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
  Material::init_type();
  OrthographicLens::init_type();
  MatrixLens::init_type();
  PerspectiveLens::init_type();
  PixelBuffer::init_type();
  Lens::init_type();
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
  Material::register_with_read_factory();
  OrthographicLens::register_with_read_factory();
  MatrixLens::register_with_read_factory();
  PerspectiveLens::register_with_read_factory();
  Texture::register_with_read_factory();
}
