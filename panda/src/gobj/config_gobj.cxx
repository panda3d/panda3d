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

#include "bufferContext.h"
#include "config_util.h"
#include "config_gobj.h"
#include "geom.h"
#include "geomMunger.h"
#include "geomPrimitive.h"
#include "geomTriangles.h"
#include "geomTristrips.h"
#include "geomTrifans.h"
#include "geomLines.h"
#include "geomLinestrips.h"
#include "geomPoints.h"
#include "geomVertexArrayData.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "material.h"
#include "occlusionQueryContext.h"
#include "orthographicLens.h"
#include "matrixLens.h"
#include "perspectiveLens.h"
#include "lens.h"
#include "queryContext.h"
#include "sliderTable.h"
#include "texture.h"
#include "texturePoolFilter.h"
#include "textureStage.h"
#include "textureContext.h"
#include "shaderExpansion.h"
#include "shaderContext.h"
#include "transformBlend.h"
#include "transformBlendTable.h"
#include "transformTable.h"
#include "userVertexSlider.h"
#include "userVertexTransform.h"
#include "vertexDataBook.h"
#include "vertexDataBuffer.h"
#include "vertexTransform.h"
#include "vertexSlider.h"
#include "videoTexture.h"
#include "geomContext.h"
#include "vertexBufferContext.h"
#include "indexBufferContext.h"
#include "internalName.h"

#include "dconfig.h"
#include "string_utils.h"

Configure(config_gobj);
NotifyCategoryDef(gobj, "");

ConfigVariableInt max_texture_dimension
("max-texture-dimension", -1,
 PRC_DESC("Set this to the maximum size a texture is allowed to be in either "
          "dimension.  This is generally intended as a simple way to restrict "
          "texture sizes for limited graphics cards.  When this is greater "
          "than zero, each texture image loaded from a file (but only those "
          "loaded from a file) will be automatically scaled down, if "
          "necessary, so that neither dimension is larger than this value."));

ConfigVariableDouble texture_scale
("texture-scale", 1.0,
 PRC_DESC("This is a global scale factor that is applied to each texture "
          "as it is loaded from disk.  For instance, a value of 0.5 will "
          "reduce each texture to one-half its size in each dimension.  This "
          "scale factor is applied before textures-power-2 or "
          "max-texture-dimension."));

ConfigVariableList exclude_texture_scale
("exclude-texture-scale", 
 PRC_DESC("This is a list of glob patterns for texture filenames "
          "(excluding the directory part of the filename, but including "
          "the extension); for instance, 'digits_*.png'.  Any texture "
          "filenames that match one of these patterns will not be affected "
          "by max-texture-dimension or texture-scale."));

ConfigVariableBool keep_texture_ram
("keep-texture-ram", false,
 PRC_DESC("Set this to true to retain the ram image for each texture after it "
          "has been prepared with the GSG.  This will allow the texture to be "
          "prepared with multiple GSG's, or to be re-prepared later after it is "
          "explicitly released from the GSG, without having to reread the "
          "texture image from disk; but it will consume memory somewhat "
          "wastefully."));

ConfigVariableBool preload_textures
("preload-textures", true,
 PRC_DESC("When this is true, texture images are loaded from disk as soon "
          "as the Texture is created from the TexturePool.  When this is "
          "false, the Texture is created immediately, but the image data "
          "is not loaded from disk until the Texture is actually rendered "
          "(or otherwise prepared) on the GSG.  This can help reduce "
          "wasted memory from Textures that are created but never used "
          "to render."));

ConfigVariableBool compressed_textures
("compressed-textures", false,
 PRC_DESC("Set this to true to compress textures as they are loaded into "
	  "texture memory, if the driver supports this.  Specifically, this "
	  "changes the meaning of set_compression(Texture::CM_default) to "
	  "Texture::CM_on."));

ConfigVariableBool vertex_buffers
("vertex-buffers", true,
 PRC_DESC("Set this true to allow the use of vertex buffers (or buffer "
          "objects, as OpenGL dubs them) for rendering vertex data.  This "
          "can greatly improve rendering performance on "
          "higher-end graphics cards, at the cost of some additional "
          "graphics memory (which might otherwise be used for textures "
          "or offscreen buffers).  On lower-end graphics cards this will "
          "make little or no difference."));

ConfigVariableBool vertex_arrays
("vertex-arrays", true,
 PRC_DESC("Set this true to allow the use of vertex arrays for rendering "
          "OpenGL vertex data.  This, or vertex buffers, is the normal "
          "way of issuing vertices ever since OpenGL 1.1, and you "
          "almost always want to have this set to true.  However, some very "
          "buggy graphics drivers may have problems handling vertex arrays "
          "correctly, so if you are experiencing problems you might try "
          "setting this to false.  If this is false, Panda will fall back "
          "to using immediate-mode commands like glVertex3f(), etc., to "
          "issue the vertices, which is potentially much slower than "
          "vertex arrays.  Setting this false also disables vertex buffers, "
          "effectively ignoring the setting of the vertex-buffers variable "
          "(since vertex buffers are a special case of vertex arrays in "
          "OpenGL).  This variable is normally not enabled in a production "
          "build.  This has no effect on DirectX rendering."));

ConfigVariableBool display_lists
("display-lists", false,
 PRC_DESC("Set this true to allow the use of OpenGL display lists for "
          "rendering static geometry.  On some systems, this can result "
          "in a performance improvement over vertex buffers alone; on "
          "other systems (particularly low-end systems) it makes little to "
          "no difference.  On some systems, using display lists can actually "
          "reduce performance.  This has no effect on DirectX rendering or "
          "on dynamic geometry (e.g. soft-skinned animation)."));

ConfigVariableBool hardware_animated_vertices
("hardware-animated-vertices", false,
 PRC_DESC("Set this true to allow the transforming of soft-skinned "
          "animated vertices via hardware, if supported, or false always "
          "to perform the vertex animation via software within Panda.  "
          "If you have a card that supports this, and your scene does "
          "not contain too many vertices already, this can provide a "
          "performance boost by offloading some work from your CPU onto "
          "your graphics card.  It may also help by reducing the bandwidth "
          "necessary on your computer's bus.  However, in some cases it "
          "may actually reduce performance."));

ConfigVariableBool hardware_point_sprites
("hardware-point-sprites", true,
 PRC_DESC("Set this true to allow the use of hardware extensions when "
          "rendering perspective-scaled points and point sprites.  When "
          "false, these large points are always simulated via quads "
          "computed in software, even if the hardware claims it can "
          "support them directly."));

ConfigVariableBool matrix_palette
("matrix-palette", false,
 PRC_DESC("Set this true to allow the use of the matrix palette when "
          "animating vertices in hardware.  The matrix palette is "
          "not supported by all devices, but if it is, using "
          "it can allow animation of more sophisticated meshes "
          "in hardware, and it can also improve the "
          "performance of animating some simpler meshes.  Without "
          "this option, certain meshes will have to be animated in "
          "software.  However, this option is not enabled by default, "
          "because its support seems to be buggy in certain drivers "
          "(ATI FireGL T2 8.103 in particular.)"));

ConfigVariableBool display_list_animation
("display-list-animation", false,
 PRC_DESC("Set this true to allow the use of OpenGL display lists for "
          "rendering animated geometry (when the geometry is animated "
          "by the hardware).  This is not on by default because there "
          "appear to be some driver issues with this on my FireGL T2, "
          "but it should be perfectly doable in principle, and might get "
          "you a small performance boost."));

ConfigVariableBool connect_triangle_strips
("connect-triangle-strips", true,
 PRC_DESC("Set this true to send a batch of triangle strips to the graphics "
          "card as one long triangle strip, connected by degenerate "
          "triangles, or false to send them as separate triangle strips "
          "with no degenerate triangles.  On PC hardware, using one long "
          "triangle strip may help performance by reducing the number "
          "of separate graphics calls that have to be made."));

ConfigVariableEnum<AutoTextureScale> textures_power_2
("textures-power-2", ATS_down,
 PRC_DESC("Specify whether textures should automatically be constrained to "
          "dimensions which are a power of 2 when they are loaded from "
          "disk.  Set this to 'none' to disable this feature, or to "
          "'down' or 'up' to scale down or up to the nearest power of 2, "
          "respectively.  This only has effect on textures which are not "
          "already a power of 2."));

ConfigVariableEnum<AutoTextureScale> textures_square
("textures-square", ATS_none,
 PRC_DESC("Specify whether textures should automatically be constrained to "
          "a square aspect ratio when they are loaded from disk.  Set this "
          "to 'none', 'down', or 'up'.  See textures-power-2."));

extern EXPCL_PANDA ConfigVariableBool textures_header_only;
ConfigVariableBool textures_header_only
("textures-header-only", false,
 PRC_DESC("If this is true, texture images will not actually be loaded from "
          "disk, but the image header information will be consulted to verify "
          "number of channels and so forth.  The texture images themselves "
          "will be generated in a default blue color."));

ConfigVariableInt geom_cache_size
("geom-cache-size", 5000,
 PRC_DESC("Specifies the maximum number of entries in the cache "
          "for storing pre-processed data for rendering "
          "vertices.  This limit is flexible, and may be "
          "temporarily exceeded if many different Geoms are "
          "pre-processed during the space of a single frame."));

ConfigVariableInt geom_cache_min_frames
("geom-cache-min-frames", 1,
 PRC_DESC("Specifies the minimum number of frames any one particular "
          "object will remain in the geom cache, even if geom-cache-size "
          "is exceeded."));

ConfigVariableDouble default_near
("default-near", 1.0,
 PRC_DESC("The default near clipping distance for all cameras."));

ConfigVariableDouble default_far
("default-far", 1000.0,
 PRC_DESC("The default far clipping distance for all cameras."));

ConfigVariableDouble default_fov
("default-fov", 30.0,
 PRC_DESC("The default field of view in degrees for all cameras.  This is "
          "defined as a min_fov; that is, it is the field-of-view for the "
          "smallest of the X and Y sizes of the window, which is usually "
          "the vertical field of view (windows are usually wider than they "
          "are tall).  For a 4x3 window, 30 degrees vertical is roughly "
          "40 degrees horizontal."));

ConfigVariableDouble default_iod
("default-iod", 0.2,
 PRC_DESC("The default interocular distance for stereo cameras."));

ConfigVariableDouble default_converge
("default-converge", 25.0,
 PRC_DESC("The default convergence distance for stereo cameras."));

ConfigVariableDouble default_keystone
("default-keystone", 0.0f,
 PRC_DESC("The default keystone correction, as an x y pair, for all cameras."));

ConfigVariableFilename vertex_save_file_directory
("vertex-save-file-directory", "",
 PRC_DESC("The directory in which the saved vertex data file is created "
          "for saving vertex buffers that have been evicted from RAM.  If "
          "this is the empty string, or an invalid directory, a system "
          "default directory will be chosen."));

ConfigVariableString vertex_save_file_prefix
("vertex-save-file-prefix", "p3d_vdata_",
 PRC_DESC("A prefix used to generate the filename for the saved vertex "
          "data file which is created for saving vertex buffers that have "
          "been evicted from RAM.  A uniquifying sequence number and "
          "filename extension will be appended to this string."));

ConfigVariableInt vertex_data_small_size
("vertex-data-small-size", 64,
 PRC_DESC("When a GeomVertexArrayData is this number of bytes or smaller, it "
          "is deemed too small to pay the overhead of paging it in and out, "
          "and it is permanently retained resident."));



ConfigureFn(config_gobj) {
  BufferContext::init_type();
  Geom::init_type();
  GeomPipelineReader::init_type();
  GeomContext::init_type();
  GeomLines::init_type();
  GeomLinestrips::init_type();
  GeomMunger::init_type();
  GeomPoints::init_type();
  GeomPrimitive::init_type();
  GeomPrimitivePipelineReader::init_type();
  GeomTriangles::init_type();
  GeomTrifans::init_type();
  GeomTristrips::init_type();
  GeomVertexArrayData::init_type();
  GeomVertexArrayDataHandle::init_type();
  GeomVertexArrayFormat::init_type();
  GeomVertexData::init_type();
  GeomVertexDataPipelineReader::init_type();
  GeomVertexDataPipelineWriter::init_type();
  GeomVertexFormat::init_type();
  IndexBufferContext::init_type();
  InternalName::init_type();
  Lens::init_type();
  Material::init_type();
  MatrixLens::init_type();
  OcclusionQueryContext::init_type();
  OrthographicLens::init_type();
  PerspectiveLens::init_type();
  QueryContext::init_type();
  ShaderContext::init_type();
  ShaderExpansion::init_type();
  SliderTable::init_type();
  Texture::init_type();
  TextureContext::init_type();
  TexturePoolFilter::init_type();
  TextureStage::init_type();
  TransformBlend::init_type();
  TransformBlendTable::init_type();
  TransformTable::init_type();
  UserVertexSlider::init_type();
  UserVertexTransform::init_type();
  VertexBufferContext::init_type();
  VertexSlider::init_type();
  VertexDataBuffer::init_type();
  VertexDataPage::init_type();
  VertexTransform::init_type();
  VideoTexture::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  Geom::register_with_read_factory();
  GeomLines::register_with_read_factory();
  GeomLinestrips::register_with_read_factory();
  GeomPoints::register_with_read_factory();
  GeomTriangles::register_with_read_factory();
  GeomTrifans::register_with_read_factory();
  GeomTristrips::register_with_read_factory();
  GeomVertexArrayData::register_with_read_factory();
  GeomVertexArrayFormat::register_with_read_factory();
  GeomVertexData::register_with_read_factory();
  GeomVertexFormat::register_with_read_factory();
  InternalName::register_with_read_factory();
  Material::register_with_read_factory();
  MatrixLens::register_with_read_factory();
  OrthographicLens::register_with_read_factory();
  PerspectiveLens::register_with_read_factory();
  SliderTable::register_with_read_factory();
  Texture::register_with_read_factory();
  TextureStage::register_with_read_factory();
  TransformBlendTable::register_with_read_factory();
  TransformTable::register_with_read_factory();
  UserVertexSlider::register_with_read_factory();
  UserVertexTransform::register_with_read_factory();
}

ostream &
operator << (ostream &out, AutoTextureScale ats) {
  switch (ats) {
  case ATS_none:
    return out << "none";
   
  case ATS_down:
    return out << "down";
    
  case ATS_up:
    return out << "up";
  }

  return out << "**invalid AutoTextureScale (" << (int)ats << ")**";
}

istream &
operator >> (istream &in, AutoTextureScale &ats) {
  string word;
  in >> word;

  if (cmp_nocase(word, "none") == 0 ||
      cmp_nocase(word, "0") == 0 ||
      cmp_nocase(word, "#f") == 0 ||
      tolower(word[0] == 'f')) {
    ats = ATS_none;

  } else if (cmp_nocase(word, "down") == 0 ||
          cmp_nocase(word, "1") == 0 ||
          cmp_nocase(word, "#t") == 0 ||
          tolower(word[0] == 't')) {
    ats = ATS_down;

  } else if (cmp_nocase(word, "up") == 0) {
    ats = ATS_up;

  } else {
    gobj_cat->error() << "Invalid AutoTextureScale value: " << word << "\n";
    ats = ATS_none;
  }

  return in;
}
