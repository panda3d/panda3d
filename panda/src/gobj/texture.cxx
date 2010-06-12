// Filename: texture.cxx
// Created by:  mike (09Jan97)
// Updated by: fperazzi, PandaSE(29Apr10) (added TT_2d_texture_array)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "texture.h"
#include "config_gobj.h"
#include "config_util.h"
#include "texturePool.h"
#include "textureContext.h"
#include "bamCache.h"
#include "bamCacheRecord.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "string_utils.h"
#include "preparedGraphicsObjects.h"
#include "pnmImage.h"
#include "virtualFileSystem.h"
#include "datagramInputFile.h"
#include "datagramOutputFile.h"
#include "bam.h"
#include "zStream.h"
#include "indent.h"
#include "cmath.h"
#include "pStatTimer.h"
#include "pbitops.h"
#include "streamReader.h"
#include "texturePeeker.h"

#ifdef HAVE_SQUISH
#include <squish.h>
#endif  // HAVE_SQUISH

#include <stddef.h>

ConfigVariableEnum<Texture::QualityLevel> texture_quality_level
("texture-quality-level", Texture::QL_normal,
 PRC_DESC("This specifies a global quality level for all textures.  You "
          "may specify either fastest, normal, or best.  This actually "
          "affects the meaning of Texture::set_quality_level(QL_default), "
          "so it may be overridden on a per-texture basis.  This generally "
          "only has an effect when using the tinydisplay software renderer; "
          "it has little or no effect on normal, hardware-accelerated "
          "renderers.  See Texture::set_quality_level()."));

ConfigVariableEnum<Texture::FilterType> texture_minfilter
("texture-minfilter", Texture::FT_linear,
 PRC_DESC("This specifies the default minfilter that is applied to a texture "
          "in the absence of a specific minfilter setting.  Normally this "
          "is either 'linear' to disable mipmapping by default, or "
          "'mipmap', to enable trilinear mipmapping by default.  This "
          "does not apply to depth textures.  Note if this variable is "
          "changed at runtime, you may need to reload textures explicitly "
          "in order to change their visible properties."));

ConfigVariableEnum<Texture::FilterType> texture_magfilter
("texture-magfilter", Texture::FT_linear,
 PRC_DESC("This specifies the default magfilter that is applied to a texture "
          "in the absence of a specific magfilter setting.  Normally this "
          "is 'linear' (since mipmapping does not apply to magfilters).  This "
          "does not apply to depth textures.  Note if this variable is "
          "changed at runtime, you may need to reload textures explicitly "
          "in order to change their visible properties."));

ConfigVariableInt texture_anisotropic_degree
("texture-anisotropic-degree", 1,
 PRC_DESC("This specifies the default anisotropic degree that is applied "
          "to a texture in the absence of a particular anisotropic degree "
          "setting (that is, a texture for which the anisotropic degree "
          "is 0, meaning the default setting).  It should be 1 to disable "
          "anisotropic filtering, or a higher number to enable it.  "
          "Note if this variable is "
          "changed at runtime, you may need to reload textures explicitly "
          "in order to change their visible properties."));

PStatCollector Texture::_texture_read_pcollector("*:Texture:Read");
TypeHandle Texture::_type_handle;
AutoTextureScale Texture::_textures_power_2 = ATS_UNSPECIFIED;

// Stuff to read and write DDS files.

//  little-endian, of course
#define DDS_MAGIC 0x20534444


//  DDS_header.dwFlags
#define DDSD_CAPS                   0x00000001
#define DDSD_HEIGHT                 0x00000002
#define DDSD_WIDTH                  0x00000004
#define DDSD_PITCH                  0x00000008
#define DDSD_PIXELFORMAT            0x00001000
#define DDSD_MIPMAPCOUNT            0x00020000
#define DDSD_LINEARSIZE             0x00080000
#define DDSD_DEPTH                  0x00800000

//  DDS_header.sPixelFormat.dwFlags
#define DDPF_ALPHAPIXELS            0x00000001
#define DDPF_FOURCC                 0x00000004
#define DDPF_INDEXED                0x00000020
#define DDPF_RGB                    0x00000040

//  DDS_header.sCaps.dwCaps1
#define DDSCAPS_COMPLEX             0x00000008
#define DDSCAPS_TEXTURE             0x00001000
#define DDSCAPS_MIPMAP              0x00400000

//  DDS_header.sCaps.dwCaps2
#define DDSCAPS2_CUBEMAP            0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX  0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX  0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY  0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY  0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ  0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  0x00008000
#define DDSCAPS2_VOLUME             0x00200000

struct DDSPixelFormat {
  unsigned int pf_size;
  unsigned int pf_flags;
  unsigned int four_cc;
  unsigned int rgb_bitcount;
  unsigned int r_mask;
  unsigned int g_mask;
  unsigned int b_mask;
  unsigned int a_mask;
};

struct DDSCaps2 {
  unsigned int caps1;
  unsigned int caps2;
  unsigned int ddsx;
};

struct DDSHeader {
  unsigned int dds_magic;
  unsigned int dds_size;
  unsigned int dds_flags;
  unsigned int height;
  unsigned int width;
  unsigned int pitch;
  unsigned int depth;
  unsigned int num_levels;

  DDSPixelFormat pf;
  DDSCaps2 caps;
};

////////////////////////////////////////////////////////////////////
//     Function: Texture::Constructor
//       Access: Published
//  Description: Constructs an empty texture.  The default is to set
//               up the texture as an empty 2-d texture; follow up
//               with one of the variants of setup_texture() if this
//               is not what you want.
////////////////////////////////////////////////////////////////////
Texture::
Texture(const string &name) :
  Namable(name),
  _lock(name),
  _cvar(_lock)
{
  _reloading = false;
  _primary_file_num_channels = 0;
  _alpha_file_channel = 0;
  _magfilter = FT_default;
  _minfilter = FT_default;
  _wrap_u = WM_repeat;
  _wrap_v = WM_repeat;
  _wrap_w = WM_repeat;
  _anisotropic_degree = 0;
  _keep_ram_image = true;
  _border_color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _compression = CM_default;
  _ram_image_compression = CM_off;
  _render_to_texture = false;
  _match_framebuffer_format = false;
  _post_load_store_cache = false;
  _quality_level = QL_default;

  _texture_type = TT_2d_texture;
  _x_size = 0;
  _y_size = 1;
  _z_size = 1;
  // Set it to something else first to
  // avoid the check in do_set_format
  // depending on an uninitialised value
  _format = F_rgba;
  do_set_format(F_rgb);
  do_set_component_type(T_unsigned_byte);

  _pad_x_size = 0;
  _pad_y_size = 0;
  _pad_z_size = 0;

  _orig_file_x_size = 0;
  _orig_file_y_size = 0;

  _loaded_from_image = false;
  _loaded_from_txo = false;
  _has_read_pages = false;
  _has_read_mipmaps = false;
  _num_mipmap_levels_read = 0;

  _simple_x_size = 0;
  _simple_y_size = 0;
  _simple_ram_image._page_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::Copy Constructor
//       Access: Protected
//  Description: Use Texture::make_copy() to make a duplicate copy of
//               an existing Texture.
////////////////////////////////////////////////////////////////////
Texture::
Texture(const Texture &copy) :
  Namable(copy),
  _lock(copy.get_name()),
  _cvar(_lock)
{
  _reloading = false;
  _num_mipmap_levels_read = 0;

  operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::Copy Assignment Operator
//       Access: Protected
//  Description: Use Texture::make_copy() to make a duplicate copy of
//               an existing Texture.
////////////////////////////////////////////////////////////////////
void Texture::
operator = (const Texture &copy) {
  Namable::operator = (copy);

  MutexHolder holder(_lock);
  {
    MutexHolder holder2(copy._lock);
    do_assign(copy);
  }

  ++_properties_modified;
  ++_image_modified;
  ++_simple_image_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Texture::
~Texture() {
  release_all();
  nassertv(!_reloading);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::generate_normalization_cube_map
//       Access: Published
//  Description: Generates a special cube map image in the texture
//               that can be used to apply bump mapping effects: for
//               each texel in the cube map that is indexed by the 3-d
//               texture coordinates (x, y, z), the resulting value is
//               the normalized vector (x, y, z) (compressed from
//               -1..1 into 0..1).
////////////////////////////////////////////////////////////////////
void Texture::
generate_normalization_cube_map(int size) {
  MutexHolder holder(_lock);
  do_setup_texture(TT_cube_map, size, size, 6, T_unsigned_byte, F_rgb);
  PTA_uchar image = do_make_ram_image();
  _keep_ram_image = true;

  ++_image_modified;
  ++_properties_modified;

  float half_size = (float)size * 0.5f;
  float center = half_size - 0.5f;

  LMatrix4f scale
    (127.5f, 0.0f, 0.0f, 0.0f,
     0.0f, 127.5f, 0.0f, 0.0f,
     0.0f, 0.0f, 127.5f, 0.0f,
     127.5f, 127.5f, 127.5f, 1.0f);

  unsigned char *p = image;
  int xi, yi;

  // Page 0: positive X.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3f vec(half_size, center - yi, center - xi);
      vec.normalize();
      vec = scale.xform_point(vec);

      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }

  // Page 1: negative X.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3f vec(-half_size, center - yi, xi - center);
      vec.normalize();
      vec = scale.xform_point(vec);
      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }

  // Page 2: positive Y.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3f vec(xi - center, half_size, yi - center);
      vec.normalize();
      vec = scale.xform_point(vec);
      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }

  // Page 3: negative Y.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3f vec(xi - center, -half_size, center - yi);
      vec.normalize();
      vec = scale.xform_point(vec);
      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }

  // Page 4: positive Z.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3f vec(xi - center, center - yi, half_size);
      vec.normalize();
      vec = scale.xform_point(vec);
      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }

  // Page 5: negative Z.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3f vec(center - xi, center - yi, -half_size);
      vec.normalize();
      vec = scale.xform_point(vec);
      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::generate_alpha_scale_map
//       Access: Published
//  Description: Generates a special 256x1 1-d texture that can be
//               used to apply an arbitrary alpha scale to objects by
//               judicious use of texture matrix.  The texture is a
//               gradient, with an alpha of 0 on the left (U = 0), and
//               255 on the right (U = 1).
////////////////////////////////////////////////////////////////////
void Texture::
generate_alpha_scale_map() {
  MutexHolder holder(_lock);
  do_setup_texture(TT_1d_texture, 256, 1, 1, T_unsigned_byte, F_alpha);
  _wrap_u = WM_clamp;
  _minfilter = FT_nearest;
  _magfilter = FT_nearest;
  _compression = CM_off;

  ++_image_modified;
  ++_properties_modified;

  PTA_uchar image = do_make_ram_image();
  _keep_ram_image = true;

  unsigned char *p = image;
  for (int xi = 0; xi < 256; ++xi) {
    *p++ = xi;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read
//       Access: Published
//  Description: Reads the named filename into the texture.
////////////////////////////////////////////////////////////////////
bool Texture::
read(const Filename &fullpath, const LoaderOptions &options) {
  MutexHolder holder(_lock);
  do_clear();
  ++_properties_modified;
  ++_image_modified;
  return do_read(fullpath, Filename(), 0, 0, 0, 0, false, false,
                 options, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read
//       Access: Published
//  Description: Combine a 3-component image with a grayscale image
//               to get a 4-component image.
//
//               See the description of the full-parameter read()
//               method for the meaning of the
//               primary_file_num_channels and alpha_file_channel
//               parameters.
////////////////////////////////////////////////////////////////////
bool Texture::
read(const Filename &fullpath, const Filename &alpha_fullpath,
     int primary_file_num_channels, int alpha_file_channel,
     const LoaderOptions &options) {
  MutexHolder holder(_lock);
  do_clear();
  ++_properties_modified;
  ++_image_modified;
  return do_read(fullpath, alpha_fullpath, primary_file_num_channels,
                 alpha_file_channel, 0, 0, false, false,
                 options, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read
//       Access: Published
//  Description: Reads a single file into a single page or mipmap
//               level, or automatically reads a series of files into
//               a series of pages and/or mipmap levels.
//
//               See the description of the full-parameter read()
//               method for the meaning of the various parameters.
////////////////////////////////////////////////////////////////////
bool Texture::
read(const Filename &fullpath, int z, int n,
     bool read_pages, bool read_mipmaps,
     const LoaderOptions &options) {
  MutexHolder holder(_lock);
  ++_properties_modified;
  ++_image_modified;
  return do_read(fullpath, Filename(), 0, 0, z, n, read_pages, read_mipmaps,
                 options, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read
//       Access: Published
//  Description: Reads the texture from the indicated filename.  If
//               primary_file_num_channels is not 0, it specifies the
//               number of components to downgrade the image to if it
//               is greater than this number.
//
//               If the filename has the extension .txo, this
//               implicitly reads a texture object instead of a
//               filename (which replaces all of the texture
//               properties).  In this case, all the rest of the
//               parameters are ignored, and the filename should not
//               contain any hash marks; just the one named file will
//               be read, since a single .txo file can contain all
//               pages and mipmaps necessary to define a texture.
//
//               If alpha_fullpath is not empty, it specifies the name
//               of a file from which to retrieve the alpha.  In this
//               case, alpha_file_channel represents the numeric
//               channel of this image file to use as the resulting
//               texture's alpha channel; usually, this is 0 to
//               indicate the grayscale combination of r, g, b; or it
//               may be a one-based channel number, e.g. 1 for the red
//               channel, 2 for the green channel, and so on.
//
//               If read pages is false, then z indicates the page
//               number into which this image will be assigned.
//               Normally this is 0 for the first (or only) page of
//               the texture.  3-D textures have one page for each
//               level of depth, and cube map textures always have six
//               pages.
//
//               If read_pages is true, multiple images will be read
//               at once, one for each page of a cube map or a 3-D
//               texture.  In this case, the filename should contain a
//               sequence of one or more hash marks ("#") which will
//               be filled in with the z value of each page,
//               zero-based.  In this case, the z parameter indicates
//               the maximum z value that will be loaded, or 0 to load
//               all filenames that exist.
//
//               If read_mipmaps is false, then n indicates the mipmap
//               level to which this image will be assigned.  Normally
//               this is 0 for the base texture image, but it is
//               possible to load custom mipmap levels into the later
//               images.  After the base texture image is loaded (thus
//               defining the size of the texture), you can call
//               get_expected_num_mipmap_levels() to determine the
//               maximum sensible value for n.
//
//               If read_mipmaps is true, multiple images will be read
//               as above, but this time the images represent the
//               different mipmap levels of the texture image.  In
//               this case, the n parameter indicates the maximum n
//               value that will be loaded, or 0 to load all filenames
//               that exist (up to the expected number of mipmap
//               levels).
//
//               If both read_pages and read_mipmaps is true, then
//               both sequences will be read; the filename should
//               contain two sequences of hash marks, separated by
//               some character such as a hyphen, underscore, or dot.
//               The first hash mark sequence will be filled in with
//               the mipmap level, while the second hash mark sequence
//               will be the page index.
//
//               This method implicitly sets keep_ram_image to false.
////////////////////////////////////////////////////////////////////
bool Texture::
read(const Filename &fullpath, const Filename &alpha_fullpath,
     int primary_file_num_channels, int alpha_file_channel,
     int z, int n, bool read_pages, bool read_mipmaps,
     BamCacheRecord *record,
     const LoaderOptions &options) {
  MutexHolder holder(_lock);
  ++_properties_modified;
  ++_image_modified;
  return do_read(fullpath, alpha_fullpath, primary_file_num_channels,
                 alpha_file_channel, z, n, read_pages, read_mipmaps,
                 options, record);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::estimate_texture_memory
//       Access: Published
//  Description: Estimates the amount of texture memory that will be
//               consumed by loading this texture.  This returns a
//               value that is not specific to any particular graphics
//               card or driver; it tries to make a reasonable
//               assumption about how a driver will load the texture.
//               It does not account for texture compression or
//               anything fancy.  This is mainly useful for debugging
//               and reporting purposes.
//
//               Returns a value in bytes.
////////////////////////////////////////////////////////////////////
size_t Texture::
estimate_texture_memory() const {
  MutexHolder holder(_lock);
  size_t pixels = _x_size * _y_size;

  size_t bpp = 4;
  switch (_format) {
  case Texture::F_rgb332:
    bpp = 1;
    break;

  case Texture::F_alpha:
  case Texture::F_red:
  case Texture::F_green:
  case Texture::F_blue:
  case Texture::F_luminance:
  case Texture::F_luminance_alpha:
  case Texture::F_luminance_alphamask:
    bpp = 4;
    break;

  case Texture::F_rgba:
  case Texture::F_rgba4:
  case Texture::F_rgbm:
  case Texture::F_rgb:
  case Texture::F_rgb5:
  case Texture::F_rgba5:
    bpp = 4;
    break;

  case Texture::F_color_index:
  case Texture::F_rgb8:
  case Texture::F_rgba8:
    bpp = 4;
    break;

  case Texture::F_depth_stencil:
  case Texture::F_depth_component:
    bpp = 32;
    break;

  case Texture::F_rgba12:
  case Texture::F_rgb12:
    bpp = 6;
    break;

  case Texture::F_rgba16:
    bpp = 8;
    break;
  case Texture::F_rgba32:
    bpp = 16;
    break;

  default:
    break;
  }

  size_t bytes = pixels * bpp;
  if (uses_mipmaps()) {
    bytes = (bytes * 4) / 3;
  }

  return bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_aux_data
//       Access: Published
//  Description: Records an arbitrary object in the Texture,
//               associated with a specified key.  The object may
//               later be retrieved by calling get_aux_data() with the
//               same key.
//
//               These data objects are not recorded to a bam or txo
//               file.
////////////////////////////////////////////////////////////////////
void Texture::
set_aux_data(const string &key, TypedReferenceCount *aux_data) {
  MutexHolder holder(_lock);
  _aux_data[key] = aux_data;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_aux_data
//       Access: Published
//  Description: Removes a record previously recorded via
//               set_aux_data().
////////////////////////////////////////////////////////////////////
void Texture::
clear_aux_data(const string &key) {
  MutexHolder holder(_lock);
  _aux_data.erase(key);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_aux_data
//       Access: Published
//  Description: Returns a record previously recorded via
//               set_aux_data().  Returns NULL if there was no record
//               associated with the indicated key.
////////////////////////////////////////////////////////////////////
TypedReferenceCount *Texture::
get_aux_data(const string &key) const {
  MutexHolder holder(_lock);
  AuxData::const_iterator di;
  di = _aux_data.find(key);
  if (di != _aux_data.end()) {
    return (*di).second;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_txo
//       Access: Published
//  Description: Reads the texture from a Panda texture object.  This
//               defines the complete Texture specification, including
//               the image data as well as all texture properties.
//
//               The filename is just for reference.
////////////////////////////////////////////////////////////////////
bool Texture::
read_txo(istream &in, const string &filename) {
  MutexHolder holder(_lock);
  ++_properties_modified;
  ++_image_modified;
  return do_read_txo(in, filename);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write_txo
//       Access: Published
//  Description: Writes the texture to a Panda texture object.  This
//               defines the complete Texture specification, including
//               the image data as well as all texture properties.
//
//               The filename is just for reference.
////////////////////////////////////////////////////////////////////
bool Texture::
write_txo(ostream &out, const string &filename) const {
  MutexHolder holder(_lock);
  return do_write_txo(out, filename);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_dds
//       Access: Published
//  Description: Reads the texture from a DDS file object.  This is a
//               Microsoft-defined file format; it is similar in
//               principle to a txo object, in that it is designed to
//               contain the texture image in a form as similar as
//               possible to its runtime image, and it can contain
//               mipmaps, pre-compressed textures, and so on.
//
//               As with read_txo, the filename is just for reference.
////////////////////////////////////////////////////////////////////
bool Texture::
read_dds(istream &in, const string &filename, bool header_only) {
  MutexHolder holder(_lock);
  ++_properties_modified;
  ++_image_modified;
  return do_read_dds(in, filename, header_only);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::load_related
//       Access: Published
//  Description: Loads a texture whose filename is derived by
//               concatenating a suffix to the filename of this
//               texture.  May return NULL, for example, if this
//               texture doesn't have a filename.
////////////////////////////////////////////////////////////////////
Texture *Texture::
load_related(const InternalName *suffix) const {
  MutexHolder holder(_lock);
  RelatedTextures::const_iterator ti;
  ti = _related_textures.find(suffix);
  if (ti != _related_textures.end()) {
    return (*ti).second;
  }
  if (_fullpath.empty()) {
    return (Texture*)NULL;
  }
  Filename main = _fullpath;
  main.set_basename_wo_extension(main.get_basename_wo_extension() +
                                 suffix->get_name());
  PT(Texture) res;
  if (!_alpha_fullpath.empty()) {
    Filename alph = _alpha_fullpath;
    alph.set_basename_wo_extension(alph.get_basename_wo_extension() +
                                   suffix->get_name());
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    if (vfs->exists(alph)) {
      // The alpha variant of the filename, with the suffix, exists.
      // Use it to load the texture.
      res = TexturePool::load_texture(main, alph,
                                      _primary_file_num_channels,
                                      _alpha_file_channel, false);
    } else {
      // If the alpha variant of the filename doesn't exist, just go
      // ahead and load the related texture without alpha.
      res = TexturePool::load_texture(main);
    }

  } else {
    // No alpha filename--just load the single file.  It doesn't
    // necessarily have the same number of channels as this one.
    res = TexturePool::load_texture(main);
  }

  // I'm casting away the const-ness of 'this' because this
  // field is only a cache.
  ((Texture *)this)->_related_textures.insert(RelatedTextures::value_type(suffix, res));
  return res;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_effective_minfilter
//       Access: Published
//  Description: Returns the filter mode of the texture for
//               minification, with special treatment for FT_default.
//               This will normally not return FT_default, unless
//               there is an error in the config file.
////////////////////////////////////////////////////////////////////
Texture::FilterType Texture::
get_effective_minfilter() const {
  if (_minfilter != FT_default) {
    return _minfilter;
  }
  if (_format == Texture::F_depth_stencil ||
      _format == Texture::F_depth_component) {
    return FT_nearest;
  }
  return texture_minfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_effective_magfilter
//       Access: Published
//  Description: Returns the filter mode of the texture for
//               magnification, with special treatment for FT_default.
//               This will normally not return FT_default, unless
//               there is an error in the config file.
////////////////////////////////////////////////////////////////////
Texture::FilterType Texture::
get_effective_magfilter() const {
  if (_magfilter != FT_default) {
    return _magfilter;
  }
  if (_format == Texture::F_depth_stencil ||
      _format == Texture::F_depth_component) {
    return FT_nearest;
  }
  return texture_magfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_ram_image
//       Access: Published
//  Description: Replaces the current system-RAM image with the new
//               data.  If compression is not CM_off, it indicates
//               that the new data is already pre-compressed in the
//               indicated format.
//
//               This does *not* affect keep_ram_image.
////////////////////////////////////////////////////////////////////
void Texture::
set_ram_image(CPTA_uchar image, Texture::CompressionMode compression,
              size_t page_size) {
  MutexHolder holder(_lock);
  nassertv(compression != CM_default);
  nassertv(compression != CM_off || image.size() == do_get_expected_ram_image_size());
  if (_ram_images.empty()) {
    _ram_images.push_back(RamImage());
  } else {
    do_clear_ram_mipmap_images();
  }
  if (page_size == 0) {
    page_size = image.size();
  }
  if (_ram_images[0]._image != image ||
      _ram_images[0]._page_size != page_size ||
      _ram_image_compression != compression) {
    _ram_images[0]._image = image.cast_non_const();
    _ram_images[0]._page_size = page_size;
    _ram_images[0]._pointer_image = NULL;
    _ram_image_compression = compression;
    ++_image_modified;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_keep_ram_image
//       Access: Published, Virtual
//  Description: Returns the flag that indicates whether this Texture
//               is eligible to have its main RAM copy of the texture
//               memory dumped when the texture is prepared for
//               rendering.  See set_keep_ram_image().
////////////////////////////////////////////////////////////////////
bool Texture::
get_keep_ram_image() const {
  return _keep_ram_image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_num_loadable_ram_mipmap_images
//       Access: Published
//  Description: Returns the number of contiguous mipmap levels that
//               exist in RAM, up until the first gap in the sequence.
//               It is guaranteed that at least mipmap levels [0,
//               get_num_ram_mipmap_images()) exist.
//
//               The number returned will never exceed the number of
//               required mipmap images based on the size of the
//               texture and its filter mode.
//
//               This method is different from
//               get_num_ram_mipmap_images() in that it returns only
//               the number of mipmap levels that can actually be
//               usefully loaded, regardless of the actual number that
//               may be stored.
////////////////////////////////////////////////////////////////////
int Texture::
get_num_loadable_ram_mipmap_images() const {
  MutexHolder holder(_lock);
  if (_ram_images.empty() || _ram_images[0]._image.empty()) {
    // If we don't even have a base image, the answer is none.
    return 0;
  }
  if (!uses_mipmaps()) {
    // If we have a base image and don't require mipmapping, the
    // answer is 1.
    return 1;
  }

  // Check that we have enough mipmap levels to meet the size
  // requirements.
  int size = max(_x_size, max(_y_size, _z_size));
  int n = 0;
  int x = 1;
  while (x < size) {
    x = (x << 1);
    ++n;
    if (n >= (int)_ram_images.size() || _ram_images[n]._image.empty()) {
      return n;
    }
  }

  ++n;
  return n;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_mipmap_image
//       Access: Published
//  Description: Returns the system-RAM image data associated with the
//               nth mipmap level, if present.  Returns NULL if the
//               nth mipmap level is not present.
////////////////////////////////////////////////////////////////////
CPTA_uchar Texture::
get_ram_mipmap_image(int n) {
  MutexHolder holder(_lock);
  if (n < (int)_ram_images.size() && !_ram_images[n]._image.empty()) {
    return _ram_images[n]._image;
  }
  return CPTA_uchar(get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_mipmap_pointer
//       Access: Published
//  Description: Similiar to get_ram_mipmap_image(), however, in this
//               case the void pointer for the given ram image is
//               returned.  This will be NULL unless it has been
//               explicitly set.
////////////////////////////////////////////////////////////////////
void *Texture::
get_ram_mipmap_pointer(int n) {
  MutexHolder holder(_lock);
  if (n < (int)_ram_images.size()) {
    return _ram_images[n]._pointer_image;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_ram_mipmap_pointer
//       Access: Published
//  Description: Sets an explicit void pointer as the texture's mipmap
//               image for the indicated level.  This is a special
//               call to direct a texture to reference some external
//               image location, for instance from a webcam input.
//
//               The texture will henceforth reference this pointer
//               directly, instead of its own internal storage; the
//               user is responsible for ensuring the data at this
//               address remains allocated and valid, and in the
//               correct format, during the lifetime of the texture.
////////////////////////////////////////////////////////////////////
void Texture::
set_ram_mipmap_pointer(int n, void *image, size_t page_size) {
  MutexHolder holder(_lock);
  nassertv(_ram_image_compression != CM_off || do_get_expected_ram_mipmap_image_size(n));

  while (n >= (int)_ram_images.size()) {
    _ram_images.push_back(RamImage());
  }

  _ram_images[n]._page_size = page_size; 
  //_ram_images[n]._image.clear(); wtf is going on?!
  _ram_images[n]._pointer_image = image;
  ++_image_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_ram_mipmap_pointer_from_int
//       Access: Published
//  Description: Accepts a raw pointer cast as an int, which is then
//               passed to set_ram_mipmap_pointer(); see the
//               documentation for that method.
//
//               This variant is particularly useful to set an
//               external pointer from a language like Python, which
//               doesn't support void pointers directly.
////////////////////////////////////////////////////////////////////
void Texture::
set_ram_mipmap_pointer_from_int(long long pointer, int n, int page_size) {
  set_ram_mipmap_pointer(n, (void*)pointer, (size_t)page_size);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_ram_mipmap_image
//       Access: Published
//  Description: Discards the current system-RAM image for the nth
//               mipmap level.
////////////////////////////////////////////////////////////////////
void Texture::
clear_ram_mipmap_image(int n) {
  MutexHolder holder(_lock);
  if (n >= (int)_ram_images.size()) {
    return;
  }
  _ram_images[n]._page_size = 0;
  _ram_images[n]._image.clear();
  _ram_images[n]._pointer_image = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::modify_simple_ram_image
//       Access: Published
//  Description: Returns a modifiable pointer to the internal "simple"
//               texture image.  See set_simple_ram_image().
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
modify_simple_ram_image() {
  MutexHolder holder(_lock);
  _simple_image_date_generated = (PN_int32)time(NULL);
  return _simple_ram_image._image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::new_simple_ram_image
//       Access: Published
//  Description: Creates an empty array for the simple ram image of
//               the indicated size, and returns a modifiable pointer
//               to the new array.  See set_simple_ram_image().
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
new_simple_ram_image(int x_size, int y_size) {
  MutexHolder holder(_lock);
  nassertr(_texture_type == TT_2d_texture, PTA_uchar());
  size_t expected_page_size = (size_t)(x_size * y_size * 4);

  _simple_x_size = x_size;
  _simple_y_size = y_size;
  _simple_ram_image._image = PTA_uchar::empty_array(expected_page_size);
  _simple_ram_image._page_size = expected_page_size;
  _simple_image_date_generated = (PN_int32)time(NULL);
  ++_simple_image_modified;

  return _simple_ram_image._image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::generate_simple_ram_image
//       Access: Published
//  Description: Computes the "simple" ram image by loading the main
//               RAM image, if it is not already available, and
//               reducing it to 16x16 or smaller.  This may be an
//               expensive operation.
////////////////////////////////////////////////////////////////////
void Texture::
generate_simple_ram_image() {
  MutexHolder holder(_lock);

  if (_texture_type != TT_2d_texture ||
      _ram_image_compression != CM_off) {
    return;
  }

  PNMImage pnmimage;
  if (!do_store_one(pnmimage, 0, 0)) {
    return;
  }

  // Start at the suggested size from the config file.
  int x_size = simple_image_size.get_word(0);
  int y_size = simple_image_size.get_word(1);

  // Limit it to no larger than the source image, and also make it a
  // power of two.
  x_size = down_to_power_2(min(x_size, _x_size));
  y_size = down_to_power_2(min(y_size, _y_size));

  // Generate a reduced image of that size.
  PNMImage scaled(x_size, y_size, pnmimage.get_num_channels());
  scaled.quick_filter_from(pnmimage);

  // Make sure the reduced image has 4 components, by convention.
  if (!scaled.has_alpha()) {
    scaled.add_alpha();
    scaled.alpha_fill(1.0);
  }
  scaled.set_num_channels(4);

  // Now see if we can go even smaller.
  bool did_anything;
  do {
    did_anything = false;

    // Try to reduce X.
    if (x_size > 1) {
      int new_x_size = (x_size >> 1);
      PNMImage smaller(new_x_size, y_size, 4);
      smaller.quick_filter_from(scaled);
      PNMImage bigger(x_size, y_size, 4);
      bigger.quick_filter_from(smaller);

      if (compare_images(scaled, bigger)) {
        scaled.take_from(smaller);
        x_size = new_x_size;
        did_anything = true;
      }
    }

    // Try to reduce Y.
    if (y_size > 1) {
      int new_y_size = (y_size >> 1);
      PNMImage smaller(x_size, new_y_size, 4);
      smaller.quick_filter_from(scaled);
      PNMImage bigger(x_size, y_size, 4);
      bigger.quick_filter_from(smaller);

      if (compare_images(scaled, bigger)) {
        scaled.take_from(smaller);
        y_size = new_y_size;
        did_anything = true;
      }
    }
  } while (did_anything);

  size_t expected_page_size = (size_t)(x_size * y_size * 4);
  PTA_uchar image = PTA_uchar::empty_array(expected_page_size, get_class_type());
  convert_from_pnmimage(image, expected_page_size, 0, scaled, 4, 1);

  do_set_simple_ram_image(image, x_size, y_size);
  _simple_image_date_generated = (PN_int32)time(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::peek
//       Access: Published
//  Description: Returns a TexturePeeker object that can be used to
//               examine the individual texels stored within this
//               Texture by (u, v) coordinate.
//
//               If the texture has a ram image resident, that image
//               is used.  If it does not have a full ram image but
//               does have a simple_ram_image resident, that image is
//               used instead.  If neither image is resident the full
//               image is reloaded.
//
//               Returns NULL if the texture cannot find an image to
//               load, or the texture format is incompatible.
////////////////////////////////////////////////////////////////////
PT(TexturePeeker) Texture::
peek() {
  MutexHolder holder(_lock);

  PT(TexturePeeker) peeker = new TexturePeeker(this);
  if (peeker->is_valid()) {
    return peeker;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::prepare
//       Access: Published
//  Description: Indicates that the texture should be enqueued to be
//               prepared in the indicated prepared_objects at the
//               beginning of the next frame.  This will ensure the
//               texture is already loaded into texture memory if it
//               is expected to be rendered soon.
//
//               Use this function instead of prepare_now() to preload
//               textures from a user interface standpoint.
////////////////////////////////////////////////////////////////////
void Texture::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_texture(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::is_prepared
//       Access: Published
//  Description: Returns true if the texture has already been prepared
//               or enqueued for preparation on the indicated GSG,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool Texture::
is_prepared(PreparedGraphicsObjects *prepared_objects) const {
  MutexHolder holder(_lock);
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return true;
  }
  return prepared_objects->is_texture_queued(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::was_image_modified
//       Access: Published
//  Description: Returns true if the texture needs to be re-loaded
//               onto the indicated GSG, either because its image data
//               is out-of-date, or because it's not fully prepared
//               now.
////////////////////////////////////////////////////////////////////
bool Texture::
was_image_modified(PreparedGraphicsObjects *prepared_objects) const {
  MutexHolder holder(_lock);
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    TextureContext *tc = (*ci).second;
    return tc->was_image_modified();
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_data_size_bytes
//       Access: Public
//  Description: Returns the number of bytes which the texture is
//               reported to consume within graphics memory, for the
//               indicated GSG.  This may return a nonzero value even
//               if the texture is not currently resident; you should
//               also check get_resident() if you want to know how
//               much space the texture is actually consuming right
//               now.
////////////////////////////////////////////////////////////////////
size_t Texture::
get_data_size_bytes(PreparedGraphicsObjects *prepared_objects) const {
  MutexHolder holder(_lock);
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    TextureContext *tc = (*ci).second;
    return tc->get_data_size_bytes();
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_active
//       Access: Public
//  Description: Returns true if this Texture was rendered in the most
//               recent frame within the indicated GSG.
////////////////////////////////////////////////////////////////////
bool Texture::
get_active(PreparedGraphicsObjects *prepared_objects) const {
  MutexHolder holder(_lock);
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    TextureContext *tc = (*ci).second;
    return tc->get_active();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_resident
//       Access: Public
//  Description: Returns true if this Texture is reported to be
//               resident within graphics memory for the indicated
//               GSG.
////////////////////////////////////////////////////////////////////
bool Texture::
get_resident(PreparedGraphicsObjects *prepared_objects) const {
  MutexHolder holder(_lock);
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    TextureContext *tc = (*ci).second;
    return tc->get_resident();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::release
//       Access: Published
//  Description: Frees the texture context only on the indicated object,
//               if it exists there.  Returns true if it was released,
//               false if it had not been prepared.
////////////////////////////////////////////////////////////////////
bool Texture::
release(PreparedGraphicsObjects *prepared_objects) {
  MutexHolder holder(_lock);
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    TextureContext *tc = (*ci).second;
    if (tc != (TextureContext *)NULL) {
      prepared_objects->release_texture(tc);
    } else {
      _contexts.erase(ci);
    }
    return true;
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_texture(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::release_all
//       Access: Published
//  Description: Frees the context allocated on all objects for which
//               the texture has been declared.  Returns the number of
//               contexts which have been freed.
////////////////////////////////////////////////////////////////////
int Texture::
release_all() {
  MutexHolder holder(_lock);
  // We have to traverse a copy of the _contexts list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response
  // to each release_texture(), and we don't want to be modifying the
  // _contexts list while we're traversing it.
  Contexts temp = _contexts;
  int num_freed = (int)_contexts.size();

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    PreparedGraphicsObjects *prepared_objects = (*ci).first;
    TextureContext *tc = (*ci).second;
    if (tc != (TextureContext *)NULL) {
      prepared_objects->release_texture(tc);
    }
  }

  // There might still be some outstanding contexts in the map, if
  // there were any NULL pointers there.  Eliminate them.
  _contexts.clear();

  return num_freed;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write
//       Access: Published
//  Description: Not to be confused with write(Filename), this method
//               simply describes the texture properties.
////////////////////////////////////////////////////////////////////
void Texture::
write(ostream &out, int indent_level) const {
  MutexHolder holder(_lock);
  indent(out, indent_level)
    << _texture_type << " " << get_name();
  if (!_filename.empty()) {
    out << " (from " << _filename << ")";
  }
  out << "\n";

  indent(out, indent_level + 2);

  switch (_texture_type) {
  case TT_1d_texture:
    out << "1-d, " << _x_size;
    break;

  case TT_2d_texture:
    out << "2-d, " << _x_size << " x " << _y_size;
    break;

  case TT_3d_texture:
    out << "3-d, " << _x_size << " x " << _y_size << " x " << _z_size;
    break;
  
  case TT_2d_texture_array:
    out << "2-d array, " << _x_size << " x " << _y_size << " x " << _z_size;
    break;
  
  case TT_cube_map:
    out << "cube map, " << _x_size << " x " << _y_size;
    break;
  }

  out << " pixels, each " << _num_components;

  switch (_component_type) {
  case T_unsigned_byte:
    out << " bytes";
    break;

  case T_unsigned_short:
    out << " shorts";
    break;

  case T_float:
    out << " floats";
    break;

  default:
    break;
  }

  out << ", ";
  switch (_format) {
  case F_color_index:
    out << "color_index";
    break;
  case F_depth_stencil:
    out << "depth_stencil";
    break;
  case F_depth_component:
    out << "depth_component";
    break;
  case F_depth_component16:
    out << "depth_component16";
    break;
  case F_depth_component24:
    out << "depth_component24";
    break;
  case F_depth_component32:
    out << "depth_component32";
    break;

  case F_rgba:
    out << "rgba";
    break;
  case F_rgbm:
    out << "rgbm";
    break;
  case F_rgba32:
    out << "rgba32";
    break;
  case F_rgba16:
    out << "rgba16";
    break;
  case F_rgba12:
    out << "rgba12";
    break;
  case F_rgba8:
    out << "rgba8";
    break;
  case F_rgba4:
    out << "rgba4";
    break;

  case F_rgb:
    out << "rgb";
    break;
  case F_rgb12:
    out << "rgb12";
    break;
  case F_rgb8:
    out << "rgb8";
    break;
  case F_rgb5:
    out << "rgb5";
    break;
  case F_rgba5:
    out << "rgba5";
    break;
  case F_rgb332:
    out << "rgb332";
    break;

  case F_red:
    out << "red";
    break;
  case F_green:
    out << "green";
    break;
  case F_blue:
    out << "blue";
    break;
  case F_alpha:
    out << "alpha";
    break;
  case F_luminance:
    out << "luminance";
    break;
  case F_luminance_alpha:
    out << "luminance_alpha";
    break;
  case F_luminance_alphamask:
    out << "luminance_alphamask";
    break;
  }

  if (_compression != CM_default) {
    out << ", compression " << _compression;
  }
  out << "\n";

  indent(out, indent_level + 2);

  switch (_texture_type) {
  case TT_1d_texture:
    out << _wrap_u << ", ";
    break;

  case TT_2d_texture:
    out << _wrap_u << " x " << _wrap_v << ", ";
    break;

  case TT_3d_texture:
    out << _wrap_u << " x " << _wrap_v << " x " << _wrap_w << ", ";
    break;

  case TT_2d_texture_array:
    out << _wrap_u << " x " << _wrap_v << " x " << _wrap_w << ", ";
    break;
  
  case TT_cube_map:
    break;
  }

  out << "min " << _minfilter
      << ", mag " << _magfilter
      << ", aniso " << _anisotropic_degree
      << ", border " << _border_color
      << "\n";

  if (do_has_ram_image()) {
    indent(out, indent_level + 2)
      << do_get_ram_image_size() << " bytes in ram, compression "
      << _ram_image_compression << "\n";

    if (_ram_images.size() > 1) {
      int count = 0;
      size_t total_size = 0;
      for (size_t n = 1; n < _ram_images.size(); ++n) {
        if (!_ram_images[n]._image.empty()) {
          ++count;
          total_size += _ram_images[n]._image.size();
        } else {
          // Stop at the first gap.
          break;
        }
      }
      indent(out, indent_level + 2)
        << count
        << " mipmap levels also present in ram (" << total_size
        << " bytes).\n";
    }

  } else {
    indent(out, indent_level + 2)
      << "no ram image\n";
  }

  if (!_simple_ram_image._image.empty()) {
    indent(out, indent_level + 2)
      << "simple image: " << _simple_x_size << " x "
      << _simple_y_size << ", "
      << _simple_ram_image._image.size() << " bytes\n";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Texture::set_size_padded
//       Access: Published
//  Description: Changes the size of the texture, padding
//               if necessary, and setting the pad region
//               as well.
////////////////////////////////////////////////////////////////////
void Texture::
set_size_padded(int x, int y, int z) {
  MutexHolder holder(_lock);
  if (get_textures_power_2() != ATS_none) {
    do_set_x_size(up_to_power_2(x));
    do_set_y_size(up_to_power_2(y));
    do_set_z_size(up_to_power_2(z));
  } else {
    do_set_x_size(x);
    do_set_y_size(y);
    do_set_z_size(z);
  }
  do_set_pad_size(_x_size - x,
                  _y_size - y,
                  _z_size - z);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_orig_file_size
//       Access: Published
//  Description: Specifies the size of the texture as it exists in its
//               original disk file, before any Panda scaling.
////////////////////////////////////////////////////////////////////
void Texture::
set_orig_file_size(int x, int y, int z) {
  MutexHolder holder(_lock);
  _orig_file_x_size = x;
  _orig_file_y_size = y;

  nassertv(z == _z_size);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::is_mipmap
//       Access: Published, Static
//  Description: Returns true if the indicated filter type requires
//               the use of mipmaps, or false if it does not.
////////////////////////////////////////////////////////////////////
bool Texture::
is_mipmap(FilterType filter_type) {
  switch (filter_type) {
  case FT_nearest_mipmap_nearest:
  case FT_linear_mipmap_nearest:
  case FT_nearest_mipmap_linear:
  case FT_linear_mipmap_linear:
    return true;

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::prepare_now
//       Access: Published
//  Description: Creates a context for the texture on the particular
//               GSG, if it does not already exist.  Returns the new
//               (or old) TextureContext.  This assumes that the
//               GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               textures.  If this is not necessarily the case, you
//               should use prepare() instead.
//
//               Normally, this is not called directly except by the
//               GraphicsStateGuardian; a texture does not need to be
//               explicitly prepared by the user before it may be
//               rendered.
////////////////////////////////////////////////////////////////////
TextureContext *Texture::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  TextureContext *tc = prepared_objects->prepare_texture_now(this, gsg);
  _contexts[prepared_objects] = tc;

  return tc;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::up_to_power_2
//       Access: Published, Static
//  Description: Returns the smallest power of 2 greater than or equal
//               to value.
////////////////////////////////////////////////////////////////////
int Texture::
up_to_power_2(int value) {
  if (value <= 1) {
    return 1;
  }
  int bit = get_next_higher_bit(((unsigned int)value) - 1);
  return (1 << bit);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::down_to_power_2
//       Access: Published, Static
//  Description: Returns the largest power of 2 less than or equal
//               to value.
////////////////////////////////////////////////////////////////////
int Texture::
down_to_power_2(int value) {
  if (value <= 1) {
    return 1;
  }
  int bit = get_next_higher_bit(((unsigned int)value) >> 1);
  return (1 << bit);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::consider_rescale
//       Access: Published
//  Description: Asks the PNMImage to change its scale when it reads
//               the image, according to the whims of the Config.prc
//               file.
//
//               This method should be called after
//               pnmimage.read_header() has been called, but before
//               pnmimage.read().
////////////////////////////////////////////////////////////////////
void Texture::
consider_rescale(PNMImage &pnmimage) {
  consider_rescale(pnmimage, get_name());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::consider_rescale
//       Access: Published, Static
//  Description: Asks the PNMImage to change its scale when it reads
//               the image, according to the whims of the Config.prc
//               file.
//
//               This method should be called after
//               pnmimage.read_header() has been called, but before
//               pnmimage.read().
////////////////////////////////////////////////////////////////////
void Texture::
consider_rescale(PNMImage &pnmimage, const string &name) {
  int new_x_size = pnmimage.get_x_size();
  int new_y_size = pnmimage.get_y_size();
  if (adjust_size(new_x_size, new_y_size, name)) {
    pnmimage.set_read_size(new_x_size, new_y_size);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Texture::texture_uploaded
//       Access: Public
//  Description: This method is called by the GraphicsEngine at the
//               beginning of the frame *after* a texture has been
//               successfully uploaded to graphics memory.  It is
//               intended as a callback so the texture can release its
//               RAM image, if _keep_ram_image is false.
//
//               This is called indirectly when the GSG calls
//               GraphicsEngine::texture_uploaded().
////////////////////////////////////////////////////////////////////
void Texture::
texture_uploaded() {
  MutexHolder holder(_lock);

  if (!keep_texture_ram && !_keep_ram_image) {
    // Once we have prepared the texture, we can generally safely
    // remove the pixels from main RAM.  The GSG is now responsible
    // for remembering what it looks like.

    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Dumping RAM for texture " << get_name() << "\n";
    }
    do_clear_ram_image();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool Texture::
has_cull_callback() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.
//
//               This is called each time the Texture is discovered
//               applied to a Geom in the traversal.  It should return
//               true if the Geom is visible, false if it should be
//               omitted.
////////////////////////////////////////////////////////////////////
bool Texture::
cull_callback(CullTraverser *, const CullTraverserData &) const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::string_wrap_mode
//       Access: Public
//  Description: Returns the WrapMode value associated with the given
//               string representation, or WM_invalid if the string
//               does not match any known WrapMode value.
////////////////////////////////////////////////////////////////////
Texture::WrapMode Texture::
string_wrap_mode(const string &string) {
  if (cmp_nocase_uh(string, "repeat") == 0) {
    return WM_repeat;
  } else if (cmp_nocase_uh(string, "clamp") == 0) {
    return WM_clamp;
  } else if (cmp_nocase_uh(string, "mirror") == 0) {
    return WM_clamp;
  } else if (cmp_nocase_uh(string, "mirror_once") == 0) {
    return WM_clamp;
  } else if (cmp_nocase_uh(string, "border_color") == 0) {
    return WM_border_color;
  } else {
    return WM_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::string_filter_type
//       Access: Public
//  Description: Returns the FilterType value associated with the given
//               string representation, or FT_invalid if the string
//               does not match any known FilterType value.
////////////////////////////////////////////////////////////////////
Texture::FilterType Texture::
string_filter_type(const string &string) {
  if (cmp_nocase_uh(string, "nearest") == 0) {
    return FT_nearest;
  } else if (cmp_nocase_uh(string, "linear") == 0) {
    return FT_linear;
  } else if (cmp_nocase_uh(string, "nearest_mipmap_nearest") == 0) {
    return FT_nearest_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "linear_mipmap_nearest") == 0) {
    return FT_linear_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "nearest_mipmap_linear") == 0) {
    return FT_nearest_mipmap_linear;
  } else if (cmp_nocase_uh(string, "linear_mipmap_linear") == 0) {
    return FT_linear_mipmap_linear;
  } else if (cmp_nocase_uh(string, "mipmap") == 0) {
    return FT_linear_mipmap_linear;
  } else if (cmp_nocase_uh(string, "shadow") == 0) {
    return FT_shadow;
  } else if (cmp_nocase_uh(string, "default") == 0) {
    return FT_default;
  } else {
    return FT_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::make_texture
//       Access: Public, Static
//  Description: A factory function to make a new Texture, used to
//               pass to the TexturePool.
////////////////////////////////////////////////////////////////////
PT(Texture) Texture::
make_texture() {
  return new Texture;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::is_specific
//       Access: Public, Static
//  Description: Returns true if the indicated compression mode is one
//               of the specific compression types, false otherwise.
////////////////////////////////////////////////////////////////////
bool Texture::
is_specific(Texture::CompressionMode compression) {
  switch (compression) {
  case CM_default:
  case CM_off:
  case CM_on:
    return false;

  default:
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_alpha
//       Access: Public, Static
//  Description: Returns true if the indicated format includes alpha,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool Texture::
has_alpha(Format format) {
  switch (format) {
  case F_alpha:
  case F_rgba:
  case F_rgbm:
  case F_rgba4:
  case F_rgba5:
  case F_rgba8:
  case F_rgba12:
  case F_rgba16:
  case F_rgba32:
  case F_luminance_alpha:
  case F_luminance_alphamask:
    return true;

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_binary_alpha
//       Access: Public, Static
//  Description: Returns true if the indicated format includes a
//               binary alpha only, false otherwise.
////////////////////////////////////////////////////////////////////
bool Texture::
has_binary_alpha(Format format) {
  switch (format) {
  case F_rgbm:
    return true;

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::adjust_size
//       Access: Public, Static
//  Description: Computes the proper size of the texture, based on the
//               original size, the filename, and the resizing whims
//               of the config file.
//
//               x_size and y_size should be loaded with the texture
//               image's original size on disk.  On return, they will
//               be loaded with the texture's in-memory target size.
//               The return value is true if the size has been
//               adjusted, or false if it is the same.
////////////////////////////////////////////////////////////////////
bool Texture::
adjust_size(int &x_size, int &y_size, const string &name) {
  bool exclude = false;
  int num_excludes = exclude_texture_scale.get_num_unique_values();
  for (int i = 0; i < num_excludes && !exclude; ++i) {
    GlobPattern pat(exclude_texture_scale.get_unique_value(i));
    if (pat.matches(name)) {
      exclude = true;
    }
  }

  int new_x_size = x_size;
  int new_y_size = y_size;

  if (!exclude) {
    new_x_size = (int)cfloor(new_x_size * texture_scale + 0.5);
    new_y_size = (int)cfloor(new_y_size * texture_scale + 0.5);

    // Don't auto-scale below 4 in either dimension.  This causes
    // problems for DirectX and texture compression.
    new_x_size = min(max(new_x_size, (int)texture_scale_limit), x_size);
    new_y_size = min(max(new_y_size, (int)texture_scale_limit), y_size);
  }

  switch (get_textures_power_2()) {
  case ATS_down:
    new_x_size = down_to_power_2(new_x_size);
    new_y_size = down_to_power_2(new_y_size);
    break;

  case ATS_up:
    new_x_size = up_to_power_2(new_x_size);
    new_y_size = up_to_power_2(new_y_size);
    break;

  case ATS_none:
  case ATS_UNSPECIFIED:
    break;
  }

  switch (textures_square.get_value()) {
  case ATS_down:
    new_x_size = new_y_size = min(new_x_size, new_y_size);
    break;

  case ATS_up:
    new_x_size = new_y_size = max(new_x_size, new_y_size);
    break;

  case ATS_none:
  case ATS_UNSPECIFIED:
    break;
  }

  if (!exclude) {
    int max_dimension = max_texture_dimension;

    if (max_dimension < 0) {
      GraphicsStateGuardianBase *gsg = GraphicsStateGuardianBase::get_default_gsg();
      if (gsg != (GraphicsStateGuardianBase *)NULL) {
        max_dimension = gsg->get_max_texture_dimension();
      }
    }

    if (max_dimension > 0) {
      new_x_size = min(new_x_size, (int)max_dimension);
      new_y_size = min(new_y_size, (int)max_dimension);
    }
  }

  if (x_size != new_x_size || y_size != new_y_size) {
    x_size = new_x_size;
    y_size = new_y_size;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::reconsider_dirty
//       Access: Protected, Virtual
//  Description: Called by TextureContext to give the Texture a chance
//               to mark itself dirty before rendering, if necessary.
////////////////////////////////////////////////////////////////////
void Texture::
reconsider_dirty() {
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_read
//       Access: Protected, Virtual
//  Description: The internal implementation of the various read()
//               methods.
////////////////////////////////////////////////////////////////////
bool Texture::
do_read(const Filename &fullpath, const Filename &alpha_fullpath,
        int primary_file_num_channels, int alpha_file_channel,
        int z, int n, bool read_pages, bool read_mipmaps,
        const LoaderOptions &options, BamCacheRecord *record) {
  PStatTimer timer(_texture_read_pcollector);

  bool header_only = ((options.get_texture_flags() & (LoaderOptions::TF_preload | LoaderOptions::TF_preload_simple)) == 0);
  if (record != (BamCacheRecord *)NULL) {
    header_only = false;
  }

  if ((z == 0 || read_pages) && (n == 0 || read_mipmaps)) {
    // When we re-read the page 0 of the base image, we clear
    // everything and start over.
    do_clear_ram_image();
  }

  if (is_txo_filename(fullpath)) {
    if (record != (BamCacheRecord *)NULL) {
      record->add_dependent_file(fullpath);
    }
    return do_read_txo_file(fullpath);
  }

  if (is_dds_filename(fullpath)) {
    if (record != (BamCacheRecord *)NULL) {
      record->add_dependent_file(fullpath);
    }
    return do_read_dds_file(fullpath, header_only);
  }

  // If read_pages or read_mipmaps is specified, then z and n actually
  // indicate z_size and n_size, respectively--the numerical limits on
  // which to search for filenames.
  int z_size = z;
  int n_size = n;

  // Certain texture types have an implicit z_size.  If z_size is
  // omitted, choose an appropriate default based on the texture
  // type.
  if (z_size == 0) {
    switch (_texture_type) {
    case TT_1d_texture:
    case TT_2d_texture:
      z_size = 1;
      break;

    case TT_cube_map:
      z_size = 6;
      break;

    default:
      break;
    }
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  if (read_pages && read_mipmaps) {
    // Read a sequence of pages * mipmap levels.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    Filename alpha_fullpath_pattern = Filename::pattern_filename(alpha_fullpath);
    do_set_z_size(z_size);

    n = 0;
    while (true) {
      // For mipmap level 0, the total number of pages might be
      // determined by the number of files we find.  After mipmap
      // level 0, though, the number of pages is predetermined.
      if (n != 0) {
        z_size = do_get_expected_mipmap_z_size(n);
      }

      z = 0;

      Filename n_pattern = Filename::pattern_filename(fullpath_pattern.get_filename_index(z));
      Filename alpha_n_pattern = Filename::pattern_filename(alpha_fullpath_pattern.get_filename_index(z));

      if (!n_pattern.has_hash()) {
        gobj_cat.error()
          << "Filename requires two different hash sequences: " << fullpath
          << "\n";
        return false;
      }

      Filename file = n_pattern.get_filename_index(n);
      Filename alpha_file = alpha_n_pattern.get_filename_index(n);

      if ((n_size == 0 && (vfs->exists(file) || n == 0)) ||
          (n_size != 0 && n < n_size)) {
        // Continue through the loop.
      } else {
        // We've reached the end of the mipmap sequence.
        break;
      }

      while ((z_size == 0 && (vfs->exists(file) || z == 0)) ||
             (z_size != 0 && z < z_size)) {
        if (!do_read_one(file, alpha_file, z, n, primary_file_num_channels,
                         alpha_file_channel, options, header_only, record)) {
          return false;
        }
        ++z;

        n_pattern = Filename::pattern_filename(fullpath_pattern.get_filename_index(z));
        file = n_pattern.get_filename_index(n);
        alpha_file = alpha_n_pattern.get_filename_index(n);
      }

      if (n == 0 && n_size == 0) {
        // If n_size is not specified, it gets implicitly set after we
        // read the base texture image (which determines the size of
        // the texture).
        n_size = do_get_expected_num_mipmap_levels();
      }
      ++n;
    }

  } else if (read_pages) {
    // Read a sequence of cube map or 3-D texture pages.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    Filename alpha_fullpath_pattern = Filename::pattern_filename(alpha_fullpath);
    if (!fullpath_pattern.has_hash()) {
      gobj_cat.error()
        << "Filename requires a hash mark: " << fullpath
        << "\n";
      return false;
    }

    do_set_z_size(z_size);
    z = 0;
    Filename file = fullpath_pattern.get_filename_index(z);
    Filename alpha_file = alpha_fullpath_pattern.get_filename_index(z);
    while ((z_size == 0 && (vfs->exists(file) || z == 0)) ||
           (z_size != 0 && z < z_size)) {
      if (!do_read_one(file, alpha_file, z, 0, primary_file_num_channels,
                       alpha_file_channel, options, header_only, record)) {
        return false;
      }
      ++z;

      file = fullpath_pattern.get_filename_index(z);
      alpha_file = alpha_fullpath_pattern.get_filename_index(z);
    }

  } else if (read_mipmaps) {
    // Read a sequence of mipmap levels.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    Filename alpha_fullpath_pattern = Filename::pattern_filename(alpha_fullpath);
    if (!fullpath_pattern.has_hash()) {
      gobj_cat.error()
        << "Filename requires a hash mark: " << fullpath
        << "\n";
      return false;
    }

    n = 0;
    Filename file = fullpath_pattern.get_filename_index(n);
    Filename alpha_file = alpha_fullpath_pattern.get_filename_index(n);

    while ((n_size == 0 && (vfs->exists(file) || n == 0)) ||
           (n_size != 0 && n < n_size)) {
      if (!do_read_one(file, alpha_file, z, n,
                       primary_file_num_channels, alpha_file_channel,
                       options, header_only, record)) {
        return false;
      }
      ++n;

      if (n_size == 0 && n >= do_get_expected_num_mipmap_levels()) {
        // Don't try to read more than the requisite number of mipmap
        // levels (unless the user insisted on it for some reason).
        break;
      }

      file = fullpath_pattern.get_filename_index(n);
      alpha_file = alpha_fullpath_pattern.get_filename_index(n);
    }

  } else {
    // Just an ordinary read of one file.
    if (!do_read_one(fullpath, alpha_fullpath, z, n,
                     primary_file_num_channels, alpha_file_channel,
                     options, header_only, record)) {
      return false;
    }
  }

  _has_read_pages = read_pages;
  _has_read_mipmaps = read_mipmaps;
  _num_mipmap_levels_read = _ram_images.size();

  if (header_only) {
    // If we were only supposed to be checking the image header
    // information, don't let the Texture think that it's got the
    // image now.
    do_clear_ram_image();
  } else {
    if ((options.get_texture_flags() & LoaderOptions::TF_preload) != 0) {
      // If we intend to keep the ram image around, consider
      // compressing it etc.
      bool generate_mipmaps = ((options.get_texture_flags() & LoaderOptions::TF_generate_mipmaps) != 0);
      consider_auto_process_ram_image(generate_mipmaps || uses_mipmaps(), true);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_read_one
//       Access: Protected, Virtual
//  Description: Called only from do_read(), this method reads a
//               single image file, either one page or one mipmap
//               level.
////////////////////////////////////////////////////////////////////
bool Texture::
do_read_one(const Filename &fullpath, const Filename &alpha_fullpath,
            int z, int n, int primary_file_num_channels, int alpha_file_channel,
            const LoaderOptions &options, bool header_only, BamCacheRecord *record) {
  if (record != (BamCacheRecord *)NULL) {
    nassertr(!header_only, false);
    record->add_dependent_file(fullpath);
  }

  PNMImage image;
  if (header_only || textures_header_only) {
    if (!image.read_header(fullpath)) {
      gobj_cat.error()
        << "Texture::read() - couldn't read: " << fullpath << endl;
      return false;
    }
    int x_size = image.get_x_size();
    int y_size = image.get_y_size();
    if (z == 0 && n == 0) {
      _orig_file_x_size = x_size;
      _orig_file_y_size = y_size;
    }

    if (textures_header_only) {
      // In this mode, we never intend to load the actual texture
      // image anyway, so we don't even need to make the size right.
      x_size = 1;
      y_size = 1;

    } else {
      consider_rescale(image, fullpath.get_basename());
      x_size = image.get_read_x_size();
      y_size = image.get_read_y_size();
    }

    image = PNMImage(x_size, y_size, image.get_num_channels(),
                     image.get_maxval(), image.get_type());
    image.fill(0.2, 0.3, 1.0);
    if (image.has_alpha()) {
      image.alpha_fill(1.0);
    }

  } else {
    if (!image.read_header(fullpath, NULL, false)) {
      gobj_cat.error()
        << "Texture::read() - couldn't read: " << fullpath << endl;
      return false;
    }

    if (z == 0 && n == 0) {
      _orig_file_x_size = image.get_x_size();
      _orig_file_y_size = image.get_y_size();
      consider_rescale(image, fullpath.get_basename());
    } else {
      image.set_read_size(do_get_expected_mipmap_x_size(n),
                          do_get_expected_mipmap_y_size(n));
    }

    if (image.get_x_size() != image.get_read_x_size() ||
        image.get_y_size() != image.get_read_y_size()) {
      gobj_cat.info()
        << "Implicitly rescaling " << fullpath.get_basename() << " from "
        << image.get_x_size() << " by " << image.get_y_size() << " to "
        << image.get_read_x_size() << " by " << image.get_read_y_size()
        << "\n";
    }

    if (!image.read(fullpath, NULL, false)) {
      gobj_cat.error()
        << "Texture::read() - couldn't read: " << fullpath << endl;
      return false;
    }
    Thread::consider_yield();
  }

  PNMImage alpha_image;
  if (!alpha_fullpath.empty()) {
    if (record != (BamCacheRecord *)NULL) {
      record->add_dependent_file(alpha_fullpath);
    }

    if (header_only || textures_header_only) {
      if (!alpha_image.read_header(alpha_fullpath)) {
        gobj_cat.error()
          << "Texture::read() - couldn't read: " << alpha_fullpath << endl;
        return false;
      }
      int x_size = image.get_x_size();
      int y_size = image.get_y_size();
      alpha_image = PNMImage(x_size, y_size, alpha_image.get_num_channels(),
                             alpha_image.get_maxval(), alpha_image.get_type());
      alpha_image.fill(1.0);
      if (alpha_image.has_alpha()) {
        alpha_image.alpha_fill(1.0);
      }

    } else {
      if (!alpha_image.read_header(alpha_fullpath, NULL, true)) {
        gobj_cat.error()
          << "Texture::read() - couldn't read (alpha): " << alpha_fullpath << endl;
        return false;
      }

      if (image.get_x_size() != alpha_image.get_x_size() ||
          image.get_y_size() != alpha_image.get_y_size()) {
        gobj_cat.info()
          << "Implicitly rescaling " << alpha_fullpath.get_basename()
          << " from " << alpha_image.get_x_size() << " by "
          << alpha_image.get_y_size() << " to " << image.get_x_size()
          << " by " << image.get_y_size() << "\n";
        alpha_image.set_read_size(image.get_x_size(), image.get_y_size());
      }

      if (!alpha_image.read(alpha_fullpath, NULL, true)) {
        gobj_cat.error()
          << "Texture::read() - couldn't read (alpha): " << alpha_fullpath << endl;
        return false;
      }
      Thread::consider_yield();
    }
  }

  if (z == 0 && n == 0) {
    if (!has_name()) {
      set_name(fullpath.get_basename_wo_extension());
    }
    if (_filename.empty()) {
      _filename = fullpath;
      _alpha_filename = alpha_fullpath;

      // The first time we set the filename via a read() operation, we
      // clear keep_ram_image.  The user can always set it again later
      // if he needs to.
      _keep_ram_image = false;
    }

    _fullpath = fullpath;
    _alpha_fullpath = alpha_fullpath;
  }

  if (!alpha_fullpath.empty()) {
    // The grayscale (alpha channel) image must be the same size as
    // the main image.  This should really have been already
    // guaranteed by the above.
    if (image.get_x_size() != alpha_image.get_x_size() ||
        image.get_y_size() != alpha_image.get_y_size()) {
      gobj_cat.info()
        << "Automatically rescaling " << alpha_fullpath.get_basename()
        << " from " << alpha_image.get_x_size() << " by "
        << alpha_image.get_y_size() << " to " << image.get_x_size()
        << " by " << image.get_y_size() << "\n";

      PNMImage scaled(image.get_x_size(), image.get_y_size(),
                      alpha_image.get_num_channels(),
                      alpha_image.get_maxval(), alpha_image.get_type());
      scaled.quick_filter_from(alpha_image);
      Thread::consider_yield();
      alpha_image = scaled;
    }
  }

  if (n == 0) {
    consider_downgrade(image, primary_file_num_channels, get_name());
    _primary_file_num_channels = image.get_num_channels();
    _alpha_file_channel = 0;
  }

  if (!alpha_fullpath.empty()) {
    // Make the original image a 4-component image by taking the
    // grayscale value from the second image.
    image.add_alpha();

    if (alpha_file_channel == 4 ||
        (alpha_file_channel == 2 && alpha_image.get_num_channels() == 2)) {
      // Use the alpha channel.
      for (int x = 0; x < image.get_x_size(); x++) {
        for (int y = 0; y < image.get_y_size(); y++) {
          image.set_alpha(x, y, alpha_image.get_alpha(x, y));
        }
      }
      _alpha_file_channel = alpha_image.get_num_channels();

    } else if (alpha_file_channel >= 1 && alpha_file_channel <= 3 &&
               alpha_image.get_num_channels() >= 3) {
      // Use the appropriate red, green, or blue channel.
      for (int x = 0; x < image.get_x_size(); x++) {
        for (int y = 0; y < image.get_y_size(); y++) {
          image.set_alpha(x, y, alpha_image.get_channel_val(x, y, alpha_file_channel - 1));
        }
      }
      _alpha_file_channel = alpha_file_channel;

    } else {
      // Use the grayscale channel.
      for (int x = 0; x < image.get_x_size(); x++) {
        for (int y = 0; y < image.get_y_size(); y++) {
          image.set_alpha(x, y, alpha_image.get_gray(x, y));
        }
      }
      _alpha_file_channel = 0;
    }
  }

  return do_load_one(image, fullpath.get_basename(), z, n, options);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_load_one
//       Access: Protected, Virtual
//  Description: Internal method to load a single page or mipmap
//               level.
////////////////////////////////////////////////////////////////////
bool Texture::
do_load_one(const PNMImage &pnmimage, const string &name, int z, int n,
            const LoaderOptions &options) {
  if (_ram_images.size() <= 1 && n == 0) {
    // A special case for mipmap level 0.  When we load mipmap level
    // 0, unless we already have mipmap levels, it determines the
    // image properties like size and number of components.
    if (!do_reconsider_z_size(z)) {
      return false;
    }
    nassertr(z >= 0 && z < _z_size, false);

    if (z == 0) {
      ComponentType component_type = T_unsigned_byte;
      xelval maxval = pnmimage.get_maxval();
      if (maxval > 255) {
        component_type = T_unsigned_short;
      }

      if (!do_reconsider_image_properties(pnmimage.get_x_size(), pnmimage.get_y_size(),
                                          pnmimage.get_num_channels(), component_type,
                                          z, options)) {
        return false;
      }
    }

    do_modify_ram_image();
    _loaded_from_image = true;
  }

  do_modify_ram_mipmap_image(n);

  // Ensure the PNMImage is an appropriate size.
  int x_size = do_get_expected_mipmap_x_size(n);
  int y_size = do_get_expected_mipmap_y_size(n);
  if (pnmimage.get_x_size() != x_size ||
      pnmimage.get_y_size() != y_size) {
    gobj_cat.info()
      << "Automatically rescaling " << name;
    if (n != 0) {
      gobj_cat.info(false)
        << " mipmap level " << n;
    }
    gobj_cat.info(false)
      << " from " << pnmimage.get_x_size() << " by "
      << pnmimage.get_y_size() << " to " << x_size << " by "
      << y_size << "\n";

    PNMImage scaled(x_size, y_size, pnmimage.get_num_channels(),
                    pnmimage.get_maxval(), pnmimage.get_type());
    scaled.quick_filter_from(pnmimage);
    Thread::consider_yield();

    convert_from_pnmimage(_ram_images[n]._image,
                          do_get_expected_ram_mipmap_page_size(n), z,
                          scaled, _num_components, _component_width);
  } else {
    // Now copy the pixel data from the PNMImage into our internal
    // _image component.
    convert_from_pnmimage(_ram_images[n]._image,
                          do_get_expected_ram_mipmap_page_size(n), z,
                          pnmimage, _num_components, _component_width);
  }
  Thread::consider_yield();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_read_txo_file
//       Access: Protected
//  Description: Called internally when read() detects a txo file.
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
bool Texture::
do_read_txo_file(const Filename &fullpath) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename filename = Filename::binary_filename(fullpath);
  PT(VirtualFile) file = vfs->get_file(filename);
  if (file == (VirtualFile *)NULL) {
    // No such file.
    gobj_cat.error()
      << "Could not find " << fullpath << "\n";
    return false;
  }

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Reading texture object " << filename << "\n";
  }

  istream *in = file->open_read_file(true);
  bool success = do_read_txo(*in, fullpath);
  vfs->close_read_file(in);

  _fullpath = fullpath;
  _alpha_fullpath = Filename();
  _keep_ram_image = false;

  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_read_txo
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool Texture::
do_read_txo(istream &in, const string &filename) {
  DatagramInputFile din;

  if (!din.open(in)) {
    gobj_cat.error()
      << "Could not read texture object: " << filename << "\n";
    return false;
  }

  string head;
  if (!din.read_header(head, _bam_header.size())) {
    gobj_cat.error()
      << filename << " is not a texture object file.\n";
    return false;
  }

  if (head != _bam_header) {
    gobj_cat.error()
      << filename << " is not a texture object file.\n";
    return false;
  }

  BamReader reader(&din, filename);
  if (!reader.init()) {
    return false;
  }

  TypedWritable *object = reader.read_object();

  if (object != (TypedWritable *)NULL &&
      object->is_exact_type(BamCacheRecord::get_class_type())) {
    // Here's a special case: if the first object in the file is a
    // BamCacheRecord, it's really a cache data file and not a true
    // txo file; but skip over the cache data record and let the user
    // treat it like an ordinary txo file.
    object = reader.read_object();
  }

  if (object == (TypedWritable *)NULL) {
    gobj_cat.error()
      << "Texture object " << filename << " is empty.\n";
    return false;

  } else if (!object->is_of_type(Texture::get_class_type())) {
    gobj_cat.error()
      << "Texture object " << filename << " contains a "
      << object->get_type() << ", not a Texture.\n";
    return false;
  }

  PT(Texture) other = DCAST(Texture, object);
  if (!reader.resolve()) {
    gobj_cat.error()
      << "Unable to fully resolve texture object file.\n";
    return false;
  }

  Namable::operator = (*other);
  do_assign(*other);
  _loaded_from_image = true;
  _loaded_from_txo = true;
  _has_read_pages = false;
  _has_read_mipmaps = false;
  _num_mipmap_levels_read = 0;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_read_dds_file
//       Access: Private
//  Description: Called internally when read() detects a DDS file.
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
bool Texture::
do_read_dds_file(const Filename &fullpath, bool header_only) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename filename = Filename::binary_filename(fullpath);
  PT(VirtualFile) file = vfs->get_file(filename);
  if (file == (VirtualFile *)NULL) {
    // No such file.
    gobj_cat.error()
      << "Could not find " << fullpath << "\n";
    return false;
  }

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Reading DDS file " << filename << "\n";
  }

  istream *in = file->open_read_file(true);
  bool success = do_read_dds(*in, fullpath, header_only);
  vfs->close_read_file(in);

  if (!has_name()) {
    set_name(fullpath.get_basename_wo_extension());
  }

  _fullpath = fullpath;
  _alpha_fullpath = Filename();
  _keep_ram_image = false;

  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_read_dds
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool Texture::
do_read_dds(istream &in, const string &filename, bool header_only) {
  StreamReader dds(in);

  // DDS header (19 words)
  DDSHeader header;
  header.dds_magic = dds.get_uint32();
  header.dds_size = dds.get_uint32();
  header.dds_flags = dds.get_uint32();
  header.height = dds.get_uint32();
  header.width = dds.get_uint32();
  header.pitch = dds.get_uint32();
  header.depth = dds.get_uint32();
  header.num_levels = dds.get_uint32();
  dds.skip_bytes(44);

  // Pixelformat (8 words)
  header.pf.pf_size = dds.get_uint32();
  header.pf.pf_flags = dds.get_uint32();
  header.pf.four_cc = dds.get_uint32();
  header.pf.rgb_bitcount = dds.get_uint32();
  header.pf.r_mask = dds.get_uint32();
  header.pf.g_mask = dds.get_uint32();
  header.pf.b_mask = dds.get_uint32();
  header.pf.a_mask = dds.get_uint32();

  // Caps (4 words)
  header.caps.caps1 = dds.get_uint32();
  header.caps.caps2 = dds.get_uint32();
  header.caps.ddsx = dds.get_uint32();
  dds.skip_bytes(4);

  // Pad out to 32 words
  dds.skip_bytes(4);

  if (header.dds_magic != DDS_MAGIC || (in.fail() || in.eof())) {
    gobj_cat.error()
      << filename << " is not a DDS file.\n";
    return false;
  }

  if ((header.dds_flags & DDSD_MIPMAPCOUNT) == 0) {
    // No bit set means only the base mipmap level.
    header.num_levels = 1;
  }

  TextureType texture_type;
  if (header.caps.caps2 & DDSCAPS2_CUBEMAP) {
    static const unsigned int all_faces =
      (DDSCAPS2_CUBEMAP_POSITIVEX |
       DDSCAPS2_CUBEMAP_POSITIVEY |
       DDSCAPS2_CUBEMAP_POSITIVEZ |
       DDSCAPS2_CUBEMAP_NEGATIVEX |
       DDSCAPS2_CUBEMAP_NEGATIVEY |
       DDSCAPS2_CUBEMAP_NEGATIVEZ);
    if ((header.caps.caps2 & all_faces) != all_faces) {
      gobj_cat.error()
        << filename << " is missing some cube map faces; cannot load.\n";
      return false;
    }
    header.depth = 6;
    texture_type = TT_cube_map;

  } else if (header.caps.caps2 & DDSCAPS2_VOLUME) {
    texture_type = TT_3d_texture;

  } else {
    texture_type = TT_2d_texture;
    header.depth = 1;
  }

  // Determine the function to use to read the DDS image.
  typedef PTA_uchar (*ReadDDSLevelFunc)(Texture *tex, const DDSHeader &header,
                                        int n, istream &in);
  ReadDDSLevelFunc func = NULL;

  Format format = F_rgb;

  do_clear_ram_image();
  CompressionMode compression = CM_off;

  if (header.pf.pf_flags & DDPF_FOURCC) {
    // Some compressed texture format.
    if (texture_type == TT_3d_texture) {
      gobj_cat.error()
        << filename << ": unsupported compression on 3-d texture.\n";
      return false;
    }

    if (header.pf.four_cc == 0x31545844) {   // 'DXT1', little-endian.
      compression = CM_dxt1;
      func = read_dds_level_dxt1;
    } else if (header.pf.four_cc == 0x32545844) {   // 'DXT2'
      compression = CM_dxt2;
      func = read_dds_level_dxt23;
    } else if (header.pf.four_cc == 0x33545844) {   // 'DXT3'
      compression = CM_dxt3;
      func = read_dds_level_dxt23;
    } else if (header.pf.four_cc == 0x34545844) {   // 'DXT4'
      compression = CM_dxt4;
      func = read_dds_level_dxt45;
    } else if (header.pf.four_cc == 0x35545844) {   // 'DXT5'
      compression = CM_dxt5;
      func = read_dds_level_dxt45;
    } else {
      gobj_cat.error()
        << filename << ": unsupported texture compression.\n";
      return false;
    }

    // All of the compressed formats support alpha, even DXT1 (to some
    // extent, at least).
    format = F_rgba;

  } else {
    // An uncompressed texture format.
    func = read_dds_level_generic_uncompressed;

    if (header.pf.pf_flags & DDPF_ALPHAPIXELS) {
      // An uncompressed format that involves alpha.
      format = F_rgba;
      if (header.pf.rgb_bitcount == 32 &&
          header.pf.r_mask == 0x000000ff &&
          header.pf.g_mask == 0x0000ff00 &&
          header.pf.b_mask == 0x00ff0000 &&
          header.pf.a_mask == 0xff000000U) {
        func = read_dds_level_abgr8;
      } else if (header.pf.rgb_bitcount == 32 &&
          header.pf.r_mask == 0x00ff0000 &&
          header.pf.g_mask == 0x0000ff00 &&
          header.pf.b_mask == 0x000000ff &&
          header.pf.a_mask == 0xff000000U) {
        func = read_dds_level_rgba8;

      } else if (header.pf.r_mask != 0 && 
                 header.pf.g_mask == 0 && 
                 header.pf.b_mask == 0) {
        func = read_dds_level_luminance_uncompressed;
        format = F_luminance_alpha;
      }
    } else {
      // An uncompressed format that doesn't involve alpha.
      if (header.pf.rgb_bitcount == 24 &&
          header.pf.r_mask == 0x00ff0000 &&
          header.pf.g_mask == 0x0000ff00 &&
          header.pf.b_mask == 0x000000ff) {
        func = read_dds_level_bgr8;
      } else if (header.pf.rgb_bitcount == 24 &&
                 header.pf.r_mask == 0x000000ff &&
                 header.pf.g_mask == 0x0000ff00 &&
                 header.pf.b_mask == 0x00ff0000) {
        func = read_dds_level_rgb8;

      } else if (header.pf.r_mask != 0 && 
                 header.pf.g_mask == 0 && 
                 header.pf.b_mask == 0) {
        func = read_dds_level_luminance_uncompressed;
        format = F_luminance;
      }
    }
      
  }

  do_setup_texture(texture_type, header.width, header.height, header.depth,
                   T_unsigned_byte, format);

  _orig_file_x_size = _x_size;
  _orig_file_y_size = _y_size;
  _compression = compression;
  _ram_image_compression = compression;

  if (!header_only) {
    switch (texture_type) {
    case TT_3d_texture:
      {
        // 3-d textures store all the depth slices for mipmap level 0,
        // then all the depth slices for mipmap level 1, and so on.
        for (int n = 0; n < (int)header.num_levels; ++n) {
          int z_size = do_get_expected_mipmap_z_size(n);
          pvector<PTA_uchar> pages;
          size_t page_size = 0;
          int z;
          for (z = 0; z < z_size; ++z) {
            PTA_uchar page = func(this, header, n, in);
            if (page.is_null()) {
              return false;
            }
            nassertr(page_size == 0 || page_size == page.size(), false);
            page_size = page.size();
            pages.push_back(page);
          }
          // Now reassemble the pages into one big image.  Because
          // this is a Microsoft format, the images are stacked in
          // reverse order; re-reverse them.
          PTA_uchar image = PTA_uchar::empty_array(page_size * z_size);
          unsigned char *imagep = (unsigned char *)image.p();
          for (z = 0; z < z_size; ++z) {
            int fz = z_size - 1 - z;
            memcpy(imagep + z * page_size, pages[fz].p(), page_size);
          }

          do_set_ram_mipmap_image(n, image, page_size);
        }
      }
      break;

    case TT_cube_map:
      {
        // Cube maps store all the mipmap levels for face 0, then all
        // the mipmap levels for face 1, and so on.
        pvector<pvector<PTA_uchar> > pages;
        pages.reserve(6);
        int z, n;
        for (z = 0; z < 6; ++z) {
          pages.push_back(pvector<PTA_uchar>());
          pvector<PTA_uchar> &levels = pages.back();
          levels.reserve(header.num_levels);

          for (n = 0; n < (int)header.num_levels; ++n) {
            PTA_uchar image = func(this, header, n, in);
            if (image.is_null()) {
              return false;
            }
            levels.push_back(image);
          }
        }

        // Now, for each level, reassemble the pages into one big
        // image.  Because this is a Microsoft format, the levels are
        // arranged in a rotated order.
        static const int level_remap[6] = {
          0, 1, 5, 4, 2, 3
        };
        for (n = 0; n < (int)header.num_levels; ++n) {
          size_t page_size = pages[0][n].size();
          PTA_uchar image = PTA_uchar::empty_array(page_size * 6);
          unsigned char *imagep = (unsigned char *)image.p();
          for (z = 0; z < 6; ++z) {
            int fz = level_remap[z];
            nassertr(pages[fz][n].size() == page_size, false);
            memcpy(imagep + z * page_size, pages[fz][n].p(), page_size);
          }

          do_set_ram_mipmap_image(n, image, page_size);
        }
      }
      break;

    default:
      // Normal 2-d textures simply store the mipmap levels.
      {
        for (int n = 0; n < (int)header.num_levels; ++n) {
          PTA_uchar image = func(this, header, n, in);
          if (image.is_null()) {
            return false;
          }
          do_set_ram_mipmap_image(n, image, 0);
        }
      }
    }
    _has_read_pages = true;
    _has_read_mipmaps = true;
    _num_mipmap_levels_read = _ram_images.size();
  }

  if (in.fail() || in.eof()) {
    gobj_cat.error()
      << filename << ": truncated DDS file.\n";
    return false;
  }

  _loaded_from_image = true;
  _loaded_from_txo = true;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_write
//       Access: Protected
//  Description: Internal method to write a series of pages and/or
//               mipmap levels to disk files.
////////////////////////////////////////////////////////////////////
bool Texture::
do_write(const Filename &fullpath, int z, int n, bool write_pages, bool write_mipmaps) const {
  if (is_txo_filename(fullpath)) {
    if (!do_has_ram_image()) {
      ((Texture *)this)->do_get_ram_image();
    }
    nassertr(do_has_ram_image(), false);
    return do_write_txo_file(fullpath);
  }

  if (!do_has_uncompressed_ram_image()) {
    ((Texture *)this)->do_get_uncompressed_ram_image();
  }

  nassertr(do_has_ram_mipmap_image(n), false);
  nassertr(_ram_image_compression == CM_off, false);

  if (write_pages && write_mipmaps) {
    // Write a sequence of pages * mipmap levels.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    int num_levels = _ram_images.size();

    for (int n = 0; n < num_levels; ++n) {
      int z_size = do_get_expected_mipmap_z_size(n);

      for (z = 0; z < z_size; ++z) {
        Filename n_pattern = Filename::pattern_filename(fullpath_pattern.get_filename_index(z));

        if (!n_pattern.has_hash()) {
          gobj_cat.error()
            << "Filename requires two different hash sequences: " << fullpath
            << "\n";
          return false;
        }

        if (!do_write_one(n_pattern.get_filename_index(n), z, n)) {
          return false;
        }
      }
    }

  } else if (write_pages) {
    // Write a sequence of pages.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    if (!fullpath_pattern.has_hash()) {
      gobj_cat.error()
        << "Filename requires a hash mark: " << fullpath
        << "\n";
      return false;
    }

    for (z = 0; z < _z_size; ++z) {
      if (!do_write_one(fullpath_pattern.get_filename_index(z), z, n)) {
        return false;
      }
    }

  } else if (write_mipmaps) {
    // Write a sequence of mipmap images.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    if (!fullpath_pattern.has_hash()) {
      gobj_cat.error()
        << "Filename requires a hash mark: " << fullpath
        << "\n";
      return false;
    }

    int num_levels = _ram_images.size();
    for (int n = 0; n < num_levels; ++n) {
      if (!do_write_one(fullpath_pattern.get_filename_index(n), z, n)) {
        return false;
      }
    }

  } else {
    // Write a single file.
    if (!do_write_one(fullpath, z, n)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_write_one
//       Access: Protected
//  Description: Internal method to write the indicated page and
//               mipmap level to a disk image file.
////////////////////////////////////////////////////////////////////
bool Texture::
do_write_one(const Filename &fullpath, int z, int n) const {
  if (!do_has_ram_mipmap_image(n)) {
    return false;
  }

  nassertr(_ram_image_compression == CM_off, false);

  PNMImage pnmimage;
  if (!do_store_one(pnmimage, z, n)) {
    return false;
  }

  if (!pnmimage.write(fullpath)) {
    gobj_cat.error()
      << "Texture::write() - couldn't write: " << fullpath << endl;
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_store_one
//       Access: Protected
//  Description: Internal method to copy a page and/or mipmap level to
//               a PNMImage.
////////////////////////////////////////////////////////////////////
bool Texture::
do_store_one(PNMImage &pnmimage, int z, int n) const {
  // First, reload the ram image if necessary.
  ((Texture *)this)->do_get_uncompressed_ram_image();

  nassertr(do_has_ram_mipmap_image(n), false);
  nassertr(z >= 0 && z < do_get_expected_mipmap_z_size(n), false);
  nassertr(_ram_image_compression == CM_off, false);

  return convert_to_pnmimage(pnmimage,
                             do_get_expected_mipmap_x_size(n),
                             do_get_expected_mipmap_y_size(n),
                             _num_components, _component_width,
                             _ram_images[n]._image,
                             do_get_ram_mipmap_page_size(n), z);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_write_txo_file
//       Access: Private
//  Description: Called internally when write() detects a txo
//               filename.
////////////////////////////////////////////////////////////////////
bool Texture::
do_write_txo_file(const Filename &fullpath) const {
  Filename filename = Filename::binary_filename(fullpath);
  pofstream out;
  if (!filename.open_write(out)) {
    gobj_cat.error()
      << "Unable to open " << filename << "\n";
    return false;
  }

#ifdef HAVE_ZLIB
  if (fullpath.get_extension() == "pz") {
    OCompressStream compressor(&out, false);
    return do_write_txo(compressor, "stream");
  }
#endif  // HAVE_ZLIB
  return do_write_txo(out, fullpath);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_write_txo
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool Texture::
do_write_txo(ostream &out, const string &filename) const {
  DatagramOutputFile dout;

  if (!dout.open(out)) {
    gobj_cat.error()
      << "Could not write texture object: " << filename << "\n";
    return false;
  }

  if (!dout.write_header(_bam_header)) {
    gobj_cat.error()
      << "Unable to write to " << filename << "\n";
    return false;
  }

  BamWriter writer(&dout, filename);
  if (!writer.init()) {
    return false;
  }

  writer.set_file_texture_mode(BamWriter::BTM_rawdata);

  // We have to temporarily release the lock to allow it to write
  // (since the BamWriter will call write_datagram, which in turn
  // will need to grab the lock).
  _lock.release();
  if (!writer.write_object(this)) {
    _lock.acquire();
    return false;
  }
  _lock.acquire();

  if (!do_has_ram_image()) {
    gobj_cat.error()
      << get_name() << " does not have ram image\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_unlock_and_reload_ram_image
//       Access: Protected
//  Description: This is similar to do_reload_ram_image(), except that
//               the lock is released during the actual operation, to
//               allow normal queries into the Texture object to
//               continue during what might be a slow operation.
//
//               The lock is re-acquired after the operation is
//               complete, and only then does all of the new data
//               appear.
//
//               Assumes the lock is held on entry.  It will be held
//               again on return.
////////////////////////////////////////////////////////////////////
void Texture::
do_unlock_and_reload_ram_image(bool allow_compression) {
  // First, wait for any other threads that might be simultaneously
  // performing the same operation.
  while (_reloading) {
    _cvar.wait();
  }

  // Then make sure we still need to reload before continuing.
  bool has_ram_image = do_has_ram_image();
  if (has_ram_image && !allow_compression && get_ram_image_compression() != Texture::CM_off) {
    // If we don't want compression, but the ram image we have is
    // pre-compressed, we don't consider it.
    has_ram_image = false;
  }
  if (_loaded_from_image && !has_ram_image && !_fullpath.empty()) {
    nassertv(!_reloading);
    _reloading = true;

    PT(Texture) tex = do_make_copy();
    _lock.release();

    // Perform the actual reload in a copy of the texture, while our
    // own mutex is left unlocked.
    tex->do_reload_ram_image(allow_compression);

    _lock.acquire();

    // Rather than calling do_assign(), which would copy *all* of the
    // reloaded texture's properties over, we only copy in the ones
    // which are relevant to the ram image.  This way, if the
    // properties have changed during the reload (for instance,
    // because we reloaded a txo), it won't contaminate the original
    // texture.
    _orig_file_x_size = tex->_orig_file_x_size;
    _orig_file_y_size = tex->_orig_file_y_size;

    // If any of *these* properties have changed, the texture has
    // changed in some fundamental way.  Update it appropriately.
    if (tex->_x_size != _x_size ||
        tex->_y_size != _y_size ||
        tex->_z_size != _z_size ||
        tex->_num_components != _num_components ||
        tex->_component_width != _component_width ||
        tex->_texture_type != _texture_type ||
        tex->_component_type != _component_type) {

      _x_size = tex->_x_size;
      _y_size = tex->_y_size;
      _z_size = tex->_z_size;

      _num_components = tex->_num_components;
      _component_width = tex->_component_width;
      _texture_type = tex->_texture_type;
      _format = tex->_format;
      _component_type = tex->_component_type;

      // Normally, we don't update the _modified semaphores in a
      // do_blah method, but we'll make an exception in this case,
      // because it's easiest to modify this here, and only when we
      // know it's needed.
      ++_properties_modified;
      ++_image_modified;
    }

    _keep_ram_image = tex->_keep_ram_image;
    _ram_image_compression = tex->_ram_image_compression;
    _ram_images = tex->_ram_images;

    nassertv(_reloading);
    _reloading = false;

    // We don't generally increment the _image_modified semaphore,
    // because this is just a reload, and presumably the image hasn't
    // changed (unless we hit the if condition above).

    _cvar.notify_all();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_reload_ram_image
//       Access: Protected, Virtual
//  Description: Called when the Texture image is required but the ram
//               image is not available, this will reload it from disk
//               or otherwise do whatever is required to make it
//               available, if possible.
//
//               Assumes the lock is already held.  The lock will be
//               held during the duration of this operation.
////////////////////////////////////////////////////////////////////
void Texture::
do_reload_ram_image(bool allow_compression) {
  BamCache *cache = BamCache::get_global_ptr();
  PT(BamCacheRecord) record;

  if (!do_has_compression()) {
    allow_compression = false;
  }

  if ((cache->get_cache_textures() || (allow_compression && cache->get_cache_compressed_textures())) && !textures_header_only) {
    // See if the texture can be found in the on-disk cache, if it is
    // active.

    record = cache->lookup(_fullpath, "txo");
    if (record != (BamCacheRecord *)NULL &&
        record->has_data()) {
      PT(Texture) tex = DCAST(Texture, record->get_data());
      
      // But don't use the cache record if the config parameters have
      // changed, and we want a different-sized texture now.
      int x_size = _orig_file_x_size;
      int y_size = _orig_file_y_size;
      Texture::adjust_size(x_size, y_size, _filename.get_basename());
      if (x_size != tex->get_x_size() || y_size != tex->get_y_size()) {
        if (gobj_cat.is_debug()) {
          gobj_cat.debug()
            << "Cached texture " << *this << " has size "
            << tex->get_x_size() << " x " << tex->get_y_size()
            << " instead of " << x_size << " x " << y_size
            << "; ignoring cache.\n";
        }
      } else {
        // Also don't keep the cached version if it's compressed but
        // we want uncompressed.
        if (!allow_compression && tex->get_ram_image_compression() != Texture::CM_off) {
          if (gobj_cat.is_debug()) {
            gobj_cat.debug()
              << "Cached texture " << *this
              << " is compressed in cache; ignoring cache.\n";
          }
        } else {
          gobj_cat.info()
            << "Texture " << get_name() << " reloaded from disk cache\n";
          // We don't want to replace all the texture parameters--for
          // instance, we don't want to change the filter type or the
          // border color or anything--we just want to get the image and
          // necessary associated parameters.
          _x_size = tex->get_x_size();
          _y_size = tex->get_y_size();
          _num_components = tex->get_num_components();
          _format = tex->get_format();
          _component_type = tex->get_component_type();
          _compression = tex->get_compression();
          _ram_image_compression = tex->_ram_image_compression;
          _ram_images = tex->_ram_images;
          _loaded_from_image = true;

          bool was_compressed = (_ram_image_compression != CM_off);
          if (consider_auto_process_ram_image(uses_mipmaps(), allow_compression)) {
            bool is_compressed = (_ram_image_compression != CM_off);
            if (!was_compressed && is_compressed &&
                cache->get_cache_compressed_textures()) {
              // We've re-compressed the image after loading it from the
              // cache.  To keep the cache current, rewrite it to the
              // cache now, in its newly compressed form.
              record->set_data(this, this);
              cache->store(record);
            }
          }

          return;
        }
      }
    }
  }

  gobj_cat.info()
    << "Reloading texture " << get_name() << "\n";

  int z = 0;
  int n = 0;

  if (_has_read_pages) {
    z = _z_size;
  }
  if (_has_read_mipmaps) {
    n = _num_mipmap_levels_read;
  }

  _loaded_from_image = false;
  Format orig_format = _format;
  int orig_num_components = _num_components;

  LoaderOptions options;
  options.set_texture_flags(LoaderOptions::TF_preload);
  do_read(_fullpath, _alpha_fullpath,
          _primary_file_num_channels, _alpha_file_channel,
          z, n, _has_read_pages, _has_read_mipmaps, options, NULL);

  if (orig_num_components == _num_components) {
    // Restore the original format, in case it was needlessly changed
    // during the reload operation.
    _format = orig_format;
  }

  if (do_has_ram_image() && record != (BamCacheRecord *)NULL) {
    if (cache->get_cache_textures() || (_ram_image_compression != CM_off && cache->get_cache_compressed_textures())) {
      // Update the cache.
      if (record != (BamCacheRecord *)NULL) {
        record->add_dependent_file(_fullpath);
      }
      record->set_data(this, this);
      cache->store(record);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_modify_ram_image
//       Access: Protected
//  Description: This is called internally to uniquify the ram image
//               pointer without updating _image_modified.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
do_modify_ram_image() {
  if (_ram_images.empty() || _ram_images[0]._image.empty() ||
      _ram_image_compression != CM_off) {
    do_make_ram_image();
  } else {
    do_clear_ram_mipmap_images();
  }
  return _ram_images[0]._image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_make_ram_image
//       Access: Protected
//  Description: This is called internally to make a new ram image
//               without updating _image_modified.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
do_make_ram_image() {
  _ram_images.clear();
  _ram_images.push_back(RamImage());
  _ram_images[0]._page_size = do_get_expected_ram_page_size();
  _ram_images[0]._image = PTA_uchar::empty_array(do_get_expected_ram_image_size(), get_class_type());
  _ram_images[0]._pointer_image = NULL;
  _ram_image_compression = CM_off;
  return _ram_images[0]._image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_modify_ram_mipmap_image
//       Access: Protected
//  Description: This is called internally to uniquify the nth mipmap
//               image pointer without updating _image_modified.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
do_modify_ram_mipmap_image(int n) {
  nassertr(_ram_image_compression == CM_off, PTA_uchar());

  if (n >= (int)_ram_images.size() ||
      _ram_images[n]._image.empty()) {
    do_make_ram_mipmap_image(n);
  }
  return _ram_images[n]._image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_make_ram_mipmap_image
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
do_make_ram_mipmap_image(int n) {
  nassertr(_ram_image_compression == CM_off, PTA_uchar(get_class_type()));

  while (n >= (int)_ram_images.size()) {
    _ram_images.push_back(RamImage());
  }

  _ram_images[n]._image = PTA_uchar::empty_array(do_get_expected_ram_mipmap_image_size(n), get_class_type());
  _ram_images[n]._pointer_image = NULL;
  _ram_images[n]._page_size = do_get_expected_ram_mipmap_page_size(n);
  return _ram_images[n]._image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_ram_mipmap_image
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_ram_mipmap_image(int n, CPTA_uchar image, size_t page_size) {
  nassertv(_ram_image_compression != CM_off || image.size() == do_get_expected_ram_mipmap_image_size(n));

  while (n >= (int)_ram_images.size()) {
    _ram_images.push_back(RamImage());
  }
  if (page_size == 0) {
    page_size = image.size();
  }

  if (_ram_images[n]._image != image ||
      _ram_images[n]._page_size != page_size) {
    _ram_images[n]._image = image.cast_non_const();
    _ram_images[n]._pointer_image = NULL;
    _ram_images[n]._page_size = page_size;
    ++_image_modified;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::consider_auto_process_ram_image
//       Access: Protected
//  Description: Should be called after a texture has been loaded into
//               RAM, this considers generating mipmaps and/or
//               compressing the RAM image.
//
//               Returns true if the image was modified by this
//               operation, false if it wasn't.
////////////////////////////////////////////////////////////////////
bool Texture::
consider_auto_process_ram_image(bool generate_mipmaps, bool allow_compression) {
  bool modified = false;

  if (generate_mipmaps && !driver_generate_mipmaps &&
      _ram_images.size() == 1) {
    do_generate_ram_mipmap_images();
    modified = true;
  }

  if (allow_compression && !driver_compress_textures) {
    CompressionMode compression = _compression;
    if (compression == CM_default && compressed_textures) {
      compression = CM_on;
    }
    if (compression != CM_off && _ram_image_compression == CM_off) {
      GraphicsStateGuardianBase *gsg = GraphicsStateGuardianBase::get_default_gsg();
      if (do_compress_ram_image(compression, QL_default, gsg)) {
        if (gobj_cat.is_debug()) {
          gobj_cat.debug()
            << "Compressed " << get_name() << " with "
            << _ram_image_compression << "\n";
        }
        modified = true;
      }
    }
  }

  return modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_compress_ram_image
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool Texture::
do_compress_ram_image(Texture::CompressionMode compression,
                      Texture::QualityLevel quality_level,
                      GraphicsStateGuardianBase *gsg) {
  nassertr(compression != CM_off, false);

  if (compression == CM_on) {
    // Select an appropriate compression mode automatically.
    switch (_format) {
    case Texture::F_rgbm:
    case Texture::F_rgb:
    case Texture::F_rgb5:
    case Texture::F_rgba5:
    case Texture::F_rgb8:
    case Texture::F_rgb12:
    case Texture::F_rgb332:
      if (gsg == NULL || gsg->get_supports_compressed_texture_format(CM_dxt1)) {
        compression = CM_dxt1;
      } else if (gsg == NULL || gsg->get_supports_compressed_texture_format(CM_dxt3)) {
        compression = CM_dxt3;
      } else if (gsg == NULL || gsg->get_supports_compressed_texture_format(CM_dxt5)) {
        compression = CM_dxt5;
      }
      break;

    case Texture::F_rgba4:
      if (gsg == NULL || gsg->get_supports_compressed_texture_format(CM_dxt3)) {
        compression = CM_dxt3;
      } else if (gsg == NULL || gsg->get_supports_compressed_texture_format(CM_dxt5)) {
        compression = CM_dxt5;
      }
      break;

    case Texture::F_rgba:
    case Texture::F_rgba8:
    case Texture::F_rgba12:
    case Texture::F_rgba16:
    case Texture::F_rgba32:
      if (gsg == NULL || gsg->get_supports_compressed_texture_format(CM_dxt5)) {
        compression = CM_dxt5;
      }
      break;

    default:
      break;
    }
  }

  // Choose an appropriate quality level.
  if (quality_level == Texture::QL_default) {
    quality_level = _quality_level;
  }
  if (quality_level == Texture::QL_default) {
    quality_level = texture_quality_level;
  }

#ifdef HAVE_SQUISH
  if (_texture_type != TT_3d_texture && 
      _texture_type != TT_2d_texture_array && 
      _component_type == T_unsigned_byte) {
    int squish_flags = 0;
    switch (compression) {
    case CM_dxt1:
      squish_flags |= squish::kDxt1;
      break;

    case CM_dxt3:
      squish_flags |= squish::kDxt3;
      break;

    case CM_dxt5:
      squish_flags |= squish::kDxt5;
      break;

    default:
      break;
    }

    if (squish_flags != 0) {
      // This compression mode is supported by squish; use it.
      switch (quality_level) {
      case QL_fastest:
        squish_flags |= squish::kColourRangeFit;
        break;

      case QL_normal:
        // ColourClusterFit is just too slow for everyday use.
        squish_flags |= squish::kColourRangeFit;
        // squish_flags |= squish::kColourClusterFit;
        break;

      case QL_best:
        squish_flags |= squish::kColourIterativeClusterFit;
        break;

      default:
        break;
      }

      if (do_squish(compression, squish_flags)) {
        return true;
      }
    }
  }
#endif  // HAVE_SQUISH

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_uncompress_ram_image
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool Texture::
do_uncompress_ram_image() {

#ifdef HAVE_SQUISH
  if (_texture_type != TT_3d_texture && 
      _texture_type != TT_2d_texture_array && 
      _component_type == T_unsigned_byte) {
    int squish_flags = 0;
    switch (_ram_image_compression) {
    case CM_dxt1:
      squish_flags |= squish::kDxt1;
      break;

    case CM_dxt3:
      squish_flags |= squish::kDxt3;
      break;

    case CM_dxt5:
      squish_flags |= squish::kDxt5;
      break;

    default:
      break;
    }

    if (squish_flags != 0) {
      // This compression mode is supported by squish; use it.
      if (do_unsquish(squish_flags)) {
        return true;
      }
    }
  }
#endif  // HAVE_SQUISH
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_has_all_ram_mipmap_images
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool Texture::
do_has_all_ram_mipmap_images() const {
  if (_ram_images.empty() || _ram_images[0]._image.empty()) {
    // If we don't even have a base image, the answer is no.
    return false;
  }
  if (!uses_mipmaps()) {
    // If we have a base image and don't require mipmapping, the
    // answer is yes.
    return true;
  }

  // Check that we have enough mipmap levels to meet the size
  // requirements.
  int size = max(_x_size, max(_y_size, _z_size));
  int n = 0;
  int x = 1;
  while (x < size) {
    x = (x << 1);
    ++n;
    if (n >= (int)_ram_images.size() || _ram_images[n]._image.empty()) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_reconsider_z_size
//       Access: Protected
//  Description: Considers whether the z_size should automatically be
//               adjusted when the user loads a new page.  Returns
//               true if the z size is valid, false otherwise.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
bool Texture::
do_reconsider_z_size(int z) {
  if (z >= _z_size) {
    // If we're loading a page past _z_size, treat it as an implicit
    // request to enlarge _z_size.  However, this is only legal if
    // this is, in fact, a 3-d texture or a 2d texture array (cube maps
    // always have z_size 6, and other types have z_size 1).
    nassertr(_texture_type == Texture::TT_3d_texture ||
             _texture_type == Texture::TT_2d_texture_array, false);

    _z_size = z + 1;
    // Increase the size of the data buffer to make room for the new
    // texture level.
    size_t new_size = do_get_expected_ram_image_size();
    if (!_ram_images.empty() &&
        !_ram_images[0]._image.empty() &&
        new_size > _ram_images[0]._image.size()) {
      _ram_images[0]._image.insert(_ram_images[0]._image.end(), new_size - _ram_images[0]._image.size(), 0);
      nassertr(_ram_images[0]._image.size() == new_size, false);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_reconsider_image_properties
//       Access: Protected
//  Description: Resets the internal Texture properties when a new
//               image file is loaded.  Returns true if the new image
//               is valid, false otherwise.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
bool Texture::
do_reconsider_image_properties(int x_size, int y_size, int num_components,
                               Texture::ComponentType component_type, int z,
                               const LoaderOptions &options) {
  if (!_loaded_from_image || num_components != _num_components) {
    // Come up with a default format based on the number of channels.
    // But only do this the first time the file is loaded, or if the
    // number of channels in the image changes on subsequent loads.

    switch (num_components) {
    case 1:
      _format = F_luminance;
      break;

    case 2:
      _format = F_luminance_alpha;
      break;

    case 3:
      _format = F_rgb;
      break;

    case 4:
      _format = F_rgba;
      break;

    default:
      // Eh?
      nassertr(false, false);
      _format = F_rgb;
    }
  }

  if (!_loaded_from_image) {
    if ((options.get_texture_flags() & LoaderOptions::TF_allow_1d) &&
        _texture_type == TT_2d_texture && x_size != 1 && y_size == 1) {
      // If we're loading an Nx1 size texture, infer a 1-d texture type.
      _texture_type = TT_1d_texture;
    }

#ifndef NDEBUG
    if (_texture_type == TT_1d_texture) {
      nassertr(y_size == 1, false);
    } else if (_texture_type == TT_cube_map) {
      nassertr(x_size == y_size, false);
    }
#endif
    if ((_x_size != x_size)||(_y_size != y_size)) {
      do_set_pad_size(0, 0, 0);
    }
    _x_size = x_size;
    _y_size = y_size;
    _num_components = num_components;
    do_set_component_type(component_type);

  } else {
    if (_x_size != x_size ||
        _y_size != y_size ||
        _num_components != num_components ||
        _component_type != component_type) {
      gobj_cat.error()
        << "Texture properties have changed for texture " << get_name()
        << " page " << z << ".\n";
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_make_copy
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(Texture) Texture::
do_make_copy() {
  PT(Texture) tex = new Texture(get_name());
  tex->do_assign(*this);
  return tex;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_assign
//       Access: Protected
//  Description: The internal implementation of operator =().  Assumes
//               the lock is already held on both Textures.
////////////////////////////////////////////////////////////////////
void Texture::
do_assign(const Texture &copy) {
  _filename = copy._filename;
  _alpha_filename = copy._alpha_filename;
  if (!copy._fullpath.empty()) {
    // Since the fullpath is often empty on a file loaded directly
    // from a txo, we only assign the fullpath if it is not empty.
    _fullpath = copy._fullpath;
    _alpha_fullpath = copy._alpha_fullpath;
  }
  _primary_file_num_channels = copy._primary_file_num_channels;
  _alpha_file_channel = copy._alpha_file_channel;
  _x_size = copy._x_size;
  _y_size = copy._y_size;
  _z_size = copy._z_size;
  _pad_x_size = copy._pad_x_size;
  _pad_y_size = copy._pad_y_size;
  _pad_z_size = copy._pad_z_size;
  _orig_file_x_size = copy._orig_file_x_size;
  _orig_file_y_size = copy._orig_file_y_size;
  _num_components = copy._num_components;
  _component_width = copy._component_width;
  _texture_type = copy._texture_type;
  _format = copy._format;
  _component_type = copy._component_type;
  _loaded_from_image = copy._loaded_from_image;
  _loaded_from_txo = copy._loaded_from_txo;
  _has_read_pages = copy._has_read_pages;
  _has_read_mipmaps = copy._has_read_mipmaps;
  _num_mipmap_levels_read = copy._num_mipmap_levels_read;
  _wrap_u = copy._wrap_u;
  _wrap_v = copy._wrap_v;
  _wrap_w = copy._wrap_w;
  _minfilter = copy._minfilter;
  _magfilter = copy._magfilter;
  _anisotropic_degree = copy._anisotropic_degree;
  _keep_ram_image = copy._keep_ram_image;
  _border_color = copy._border_color;
  _compression = copy._compression;
  _match_framebuffer_format = copy._match_framebuffer_format;
  _quality_level = copy._quality_level;
  _ram_image_compression = copy._ram_image_compression;
  _ram_images = copy._ram_images;
  _simple_x_size = copy._simple_x_size;
  _simple_y_size = copy._simple_y_size;
  _simple_ram_image = copy._simple_ram_image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_clear
//       Access: Protected, Virtual
//  Description: The protected implementation of clear().  Assumes the
//               lock is already held.
////////////////////////////////////////////////////////////////////
void Texture::
do_clear() {
  do_assign(Texture());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_setup_texture
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_setup_texture(Texture::TextureType texture_type, int x_size, int y_size,
                 int z_size, Texture::ComponentType component_type,
                 Texture::Format format) {
  switch (texture_type) {
  case TT_1d_texture:
    nassertv(y_size == 1 && z_size == 1);
    break;

  case TT_2d_texture:
    nassertv(z_size == 1);
    break;

  case TT_3d_texture:
    break;

  case TT_2d_texture_array:
    break;
  
  case TT_cube_map:
    // Cube maps must always consist of six square images.
    nassertv(x_size == y_size && z_size == 6);

    // In principle the wrap mode shouldn't mean anything to a cube
    // map, but some drivers seem to misbehave if it's other than
    // WM_clamp.
    _wrap_u = WM_clamp;
    _wrap_v = WM_clamp;
    _wrap_w = WM_clamp;
    break;
  }

  if (texture_type != TT_2d_texture) {
    do_clear_simple_ram_image();
  }

  _texture_type = texture_type;
  _x_size = x_size;
  _y_size = y_size;
  _z_size = z_size;
  do_set_component_type(component_type);
  do_set_format(format);

  do_clear_ram_image();
  do_set_pad_size(0, 0, 0);
  _orig_file_x_size = 0;
  _orig_file_y_size = 0;
  _loaded_from_image = false;
  _loaded_from_txo = false;
  _has_read_pages = false;
  _has_read_mipmaps = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_format
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_format(Texture::Format format) {
  if (format == _format) {
    return;
  }
  _format = format;
  ++_properties_modified;

  switch (_format) {
  case F_color_index:
  case F_depth_stencil:
  case F_depth_component:
  case F_depth_component16:
  case F_depth_component24:
  case F_depth_component32:
  case F_red:
  case F_green:
  case F_blue:
  case F_alpha:
  case F_luminance:
    _num_components = 1;
    break;

  case F_luminance_alpha:
  case F_luminance_alphamask:
    _num_components = 2;
    break;

  case F_rgb:
  case F_rgb5:
  case F_rgb8:
  case F_rgb12:
  case F_rgb332:
    _num_components = 3;
    break;

  case F_rgba:
  case F_rgbm:
  case F_rgba4:
  case F_rgba5:
  case F_rgba8:
  case F_rgba12:
  case F_rgba16:
  case F_rgba32:
    _num_components = 4;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_component_type
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_component_type(Texture::ComponentType component_type) {
  _component_type = component_type;

  switch (component_type) {
  case T_unsigned_byte:
    _component_width = 1;
    break;

  case T_unsigned_short:
    _component_width = 2;
    break;

  case T_float:
    _component_width = 4;
    break;

  case T_unsigned_int_24_8:
    //FIXME: I have no idea...
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_x_size
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_x_size(int x_size) {
  if (_x_size != x_size) {
    _x_size = x_size;
    ++_image_modified;
    do_clear_ram_image();
    do_set_pad_size(0, 0, 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_y_size
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_y_size(int y_size) {
  if (_y_size != y_size) {
    nassertv(_texture_type != Texture::TT_1d_texture || y_size == 1);
    _y_size = y_size;
    ++_image_modified;
    do_clear_ram_image();
    do_set_pad_size(0, 0, 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_z_size
//       Access: Protected
//  Description: Changes the z size indicated for the texture.  This
//               also implicitly unloads the texture if it has already
//               been loaded.
////////////////////////////////////////////////////////////////////
void Texture::
do_set_z_size(int z_size) {
  if (_z_size != z_size) {
    nassertv((_texture_type == Texture::TT_3d_texture) ||
             (_texture_type == Texture::TT_cube_map && z_size == 6) ||
             (_texture_type == Texture::TT_2d_texture_array) || (z_size == 1));
    _z_size = z_size;
    ++_image_modified;
    do_clear_ram_image();
    do_set_pad_size(0, 0, 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_wrap_u
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_wrap_u(Texture::WrapMode wrap) {
  if (_wrap_u != wrap) {
    ++_properties_modified;
    _wrap_u = wrap;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_wrap_v
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_wrap_v(Texture::WrapMode wrap) {
  if (_wrap_v != wrap) {
    ++_properties_modified;
    _wrap_v = wrap;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_wrap_w
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_wrap_w(Texture::WrapMode wrap) {
  if (_wrap_w != wrap) {
    ++_properties_modified;
    _wrap_w = wrap;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_minfilter
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_minfilter(Texture::FilterType filter) {
  if (_minfilter != filter) {
    ++_properties_modified;
    _minfilter = filter;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_magfilter
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_magfilter(Texture::FilterType filter) {
  if (_magfilter != filter) {
    ++_properties_modified;
    _magfilter = filter;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_anisotropic_degree
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_anisotropic_degree(int anisotropic_degree) {
  if (_anisotropic_degree != anisotropic_degree) {
    ++_properties_modified;
    _anisotropic_degree = anisotropic_degree;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_border_color
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_border_color(const Colorf &color) {
  if (_border_color != color) {
    ++_properties_modified;
    _border_color = color;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_compression
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_compression(Texture::CompressionMode compression) {
  if (_compression != compression) {
    ++_properties_modified;
    _compression = compression;
    if (do_has_ram_image()) {
      do_reload();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_quality_level
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_quality_level(Texture::QualityLevel quality_level) {
  if (_quality_level != quality_level) {
    ++_properties_modified;
    _quality_level = quality_level;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_has_compression
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool Texture::
do_has_compression() const {
  if (_compression == CM_default) {
    return compressed_textures;
  } else {
    return (_compression != CM_off);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_has_ram_image
//       Access: Protected, Virtual
//  Description: The protected implementation of has_ram_image().
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
bool Texture::
do_has_ram_image() const {
  return !_ram_images.empty() && !_ram_images[0]._image.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_has_uncompressed_ram_image
//       Access: Protected, Virtual
//  Description: The protected implementation of
//               has_uncompressed_ram_image().  Assumes the lock is
//               already held.
////////////////////////////////////////////////////////////////////
bool Texture::
do_has_uncompressed_ram_image() const {
  return !_ram_images.empty() && !_ram_images[0]._image.empty() && _ram_image_compression == CM_off;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_ram_image
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
CPTA_uchar Texture::
do_get_ram_image() {
  if (_loaded_from_image && !do_has_ram_image() && !_fullpath.empty()) {
    do_unlock_and_reload_ram_image(true);

    // Normally, we don't update the _modified semaphores in a do_blah
    // method, but we'll make an exception in this case, because it's
    // easiest to modify these here, and only when we know it's
    // needed.
    ++_image_modified;
    ++_properties_modified;
  }

  if (_ram_images.empty()) {
    return CPTA_uchar(get_class_type());
  }

  return _ram_images[0]._image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_uncompressed_ram_image
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
CPTA_uchar Texture::
do_get_uncompressed_ram_image() {
  if (!_ram_images.empty() && _ram_image_compression != CM_off) {
    // We have an image in-ram, but it's compressed.  Try to
    // uncompress it first.
    if (do_uncompress_ram_image()) {
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Uncompressed " << get_name() << "\n";
      }
      return _ram_images[0]._image;
    }
  }

  // Couldn't uncompress the existing image.  Try to reload it.
  if (_loaded_from_image && (!do_has_ram_image() || _ram_image_compression != CM_off) && !_fullpath.empty()) {
    do_unlock_and_reload_ram_image(false);
  }

  if (!_ram_images.empty() && _ram_image_compression != CM_off) {
    // Great, now we have an image.
    if (do_uncompress_ram_image()) {
      gobj_cat.info()
        << "Uncompressed " << get_name() << "\n";
      return _ram_images[0]._image;
    }
  }

  if (_ram_images.empty() || _ram_image_compression != CM_off) {
    return CPTA_uchar(get_class_type());
  }

  return _ram_images[0]._image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_image_as
//       Access: Published
//  Description: Returns the uncompressed system-RAM image data
//               associated with the texture. Rather than
//               just returning a pointer to the data, like
//               get_uncompressed_ram_image, this function first
//               processes the data and reorders the components
//               using the specified format string, and places these
//               into a new char array. The 'format' argument should
//               specify in which order the components of the texture
//               must be. For example, valid format strings are
//               "RGBA", "GA", "ABRG" or "AAA". A component can
//               also be written as "0" or "1", which means an
//               empty/black or a full/white channel, respectively.
//               This function is particularly useful to
//               copy an image in-memory to a different library
//               (for example, PIL or wxWidgets) that require
//               a different component order than Panda's internal
//               format, BGRA. Note, however, that this conversion
//               can still be too slow if you want to do it every
//               frame, and should thus be avoided for that purpose.
//               The only requirement for the reordering is that
//               an uncompressed image must be available. If the
//               RAM image is compressed, it will attempt to re-load
//               the texture from disk, if it doesn't find an
//               uncompressed image there, it will return NULL.
////////////////////////////////////////////////////////////////////
CPTA_uchar Texture::
get_ram_image_as(const string &requested_format) {
  MutexHolder holder(_lock);
  string format = upcase(requested_format);

  // Make sure we can grab something that's uncompressed.
  CPTA_uchar data = do_get_uncompressed_ram_image();
  if (data == NULL) {
    gobj_cat.error() << "Couldn't find an uncompressed RAM image!\n";
    return CPTA_uchar(get_class_type());
  }
  int imgsize = _x_size * _y_size;
  nassertr(_num_components > 0 && _num_components <= 4, CPTA_uchar(get_class_type()));
  nassertr(data.size() == (size_t)(_component_width * _num_components * imgsize), CPTA_uchar(get_class_type()));

  // Check if the format is already what we have internally.
  if ((_num_components == 1 && format.size() == 1) ||
      (_num_components == 2 && format.size() == 2 && format.at(1) == 'A' && format.at(0) != 'A') ||
      (_num_components == 3 && format == "BGR") ||
      (_num_components == 4 && format == "BGRA")) {
    // The format string is already our format, so we just need to copy it.
    return CPTA_uchar(data);
  }

  // Create a new empty array that can hold our image.
  PTA_uchar newdata = PTA_uchar::empty_array(imgsize * format.size() * _component_width, get_class_type());

  // These ifs are for optimization of commonly used image types.
  if (format == "RGBA" && _num_components == 4 && _component_width == 1) {
    imgsize *= 4;
    for (int p = 0; p < imgsize; p += 4) {
      newdata[p    ] = data[p + 2];
      newdata[p + 1] = data[p + 1];
      newdata[p + 2] = data[p    ];
      newdata[p + 3] = data[p + 3];
    }
    return newdata;
  }
  if (format == "RGB" && _num_components == 3 && _component_width == 1) {
    imgsize *= 3;
    for (int p = 0; p < imgsize; p += 3) {
      newdata[p    ] = data[p + 2];
      newdata[p + 1] = data[p + 1];
      newdata[p + 2] = data[p    ];
    }
    return newdata;
  }
  if (format == "A" && _component_width == 1 && _num_components != 3) {
    // We can generally rely on alpha to be the last component.
    int component = _num_components - 1;
    for (int p = 0; p < imgsize; ++p) {
      newdata[p] = data[component];
    }
    return newdata;
  }
  if (_component_width == 1) {
    for (int p = 0; p < imgsize; ++p) {
      for (uchar s = 0; s < format.size(); ++s) {
        signed char component = -1;
        if (format.at(s) == 'B' || (_num_components <= 2 && format.at(s) != 'A')) {
          component = 0;
        } else if (format.at(s) == 'G') {
          component = 1;
        } else if (format.at(s) == 'R') {
          component = 2;
        } else if (format.at(s) == 'A') {
          nassertr(_num_components != 3, CPTA_uchar(get_class_type()));
          component = _num_components - 1;
        } else if (format.at(s) == '0') {
          newdata[p * format.size() + s] = 0x00;
        } else if (format.at(s) == '1') {
          newdata[p * format.size() + s] = 0xff;
        } else {
          gobj_cat.error() << "Unexpected component character '"
            << format.at(s) << "', expected one of RGBA!\n";
          return CPTA_uchar(get_class_type());
        }
        if (component >= 0) {
          newdata[p * format.size() + s] = data[p * _num_components + component];
        }
      }
    }
    return newdata;
  }
  for (int p = 0; p < imgsize; ++p) {
    for (uchar s = 0; s < format.size(); ++s) {
      signed char component = -1;
      if (format.at(s) == 'B' || (_num_components <= 2 && format.at(s) != 'A')) {
        component = 0;
      } else if (format.at(s) == 'G') {
        component = 1;
      } else if (format.at(s) == 'R') {
        component = 2;
      } else if (format.at(s) == 'A') {
        nassertr(_num_components != 3, CPTA_uchar(get_class_type()));
        component = _num_components - 1;
      } else if (format.at(s) == '0') {
        memset((void*)(newdata + (p * format.size() + s) * _component_width),  0, _component_width);
      } else if (format.at(s) == '1') {
        memset((void*)(newdata + (p * format.size() + s) * _component_width), -1, _component_width);
      } else {
        gobj_cat.error() << "Unexpected component character '"
          << format.at(s) << "', expected one of RGBA!\n";
        return CPTA_uchar(get_class_type());
      }
      if (component >= 0) {
        memcpy((void*)(newdata + (p * format.size() + s) * _component_width),
               (void*)(data + (p * _num_components + component) * _component_width),
               _component_width);
      }
    }
  }
  return newdata;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_simple_ram_image
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_simple_ram_image(CPTA_uchar image, int x_size, int y_size) {
  nassertv(_texture_type == TT_2d_texture);
  size_t expected_page_size = (size_t)(x_size * y_size * 4);
  nassertv(image.size() == expected_page_size);

  _simple_x_size = x_size;
  _simple_y_size = y_size;
  _simple_ram_image._image = image.cast_non_const();
  _simple_ram_image._page_size = image.size();
  _simple_image_date_generated = (PN_int32)time(NULL);
  ++_simple_image_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_expected_num_mipmap_levels
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
int Texture::
do_get_expected_num_mipmap_levels() const {
  int size = max(_x_size, max(_y_size, _z_size));
  int count = 1;
  while (size > 1) {
    size >>= 1;
    ++count;
  }
  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_ram_mipmap_page_size
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
size_t Texture::
do_get_ram_mipmap_page_size(int n) const {
  if (_ram_image_compression != CM_off) {
    if (n >= 0 && n < (int)_ram_images.size()) {
      return _ram_images[n]._page_size;
    }
    return 0;
  } else {
    return do_get_expected_ram_mipmap_page_size(n);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_expected_mipmap_x_size
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
int Texture::
do_get_expected_mipmap_x_size(int n) const {
  int size = max(_x_size, 1);
  while (n > 0 && size > 1) {
    size >>= 1;
    --n;
  }
  return size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_expected_mipmap_y_size
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
int Texture::
do_get_expected_mipmap_y_size(int n) const {
  int size = max(_y_size, 1);
  while (n > 0 && size > 1) {
    size >>= 1;
    --n;
  }
  return size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_get_expected_mipmap_z_size
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
int Texture::
do_get_expected_mipmap_z_size(int n) const {
  // 3-D textures have a different number of pages per each mipmap
  // level.  Other kinds of textures--especially, cube map
  // textures--always have the same.
  if (_texture_type == Texture::TT_3d_texture) {
    int size = max(_z_size, 1);
    while (n > 0 && size > 1) {
      size >>= 1;
      --n;
    }
    return size;

  } else {
    return _z_size;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_clear_simple_ram_image
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_clear_simple_ram_image() {
  _simple_x_size = 0;
  _simple_y_size = 0;
  _simple_ram_image._image.clear();
  _simple_ram_image._page_size = 0;
  _simple_image_date_generated = 0;

  // We allow this exception: we update the _simple_image_modified
  // here, since no one really cares much about that anyway, and it's
  // convenient to do it here.
  ++_simple_image_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_clear_ram_mipmap_images
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_clear_ram_mipmap_images() {
  if (!_ram_images.empty()) {
    _ram_images.erase(_ram_images.begin() + 1, _ram_images.end());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_generate_ram_mipmap_images
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_generate_ram_mipmap_images() {
  nassertv(do_has_ram_image());
  nassertv(_component_type != T_float);
  if (do_get_expected_num_mipmap_levels() == 1) {
    // Don't bother.
    return;
  }

  RamImage orig_compressed_image;
  CompressionMode orig_compression_mode = CM_off;

  if (_ram_image_compression != CM_off) {
    // The RAM image is compressed.  This means we need to uncompress
    // it in order to generate mipmap images.  Save the original
    // first, to avoid lossy recompression.
    orig_compressed_image = _ram_images[0];
    orig_compression_mode = _ram_image_compression;

    // Now try to get the uncompressed source image.
    do_get_uncompressed_ram_image();

    nassertv(_ram_image_compression == CM_off);
  }

  do_clear_ram_mipmap_images();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Generating mipmap levels for " << *this << "\n";
  }

  if (_texture_type == Texture::TT_3d_texture && _z_size != 1) {
    // Eek, a 3-D texture.
    int x_size = _x_size;
    int y_size = _y_size;
    int z_size = _z_size;
    int n = 0;
    while (x_size > 1 || y_size > 1 || z_size > 1) {
      _ram_images.push_back(RamImage());
      filter_3d_mipmap_level(_ram_images[n + 1], _ram_images[n],
                             x_size, y_size, z_size);
      x_size = max(x_size >> 1, 1);
      y_size = max(y_size >> 1, 1);
      z_size = max(z_size >> 1, 1);
      ++n;
    }

  } else {
    // A 1-D, 2-D, or cube map texture.
    int x_size = _x_size;
    int y_size = _y_size;
    int n = 0;
    while (x_size > 1 || y_size > 1) {
      _ram_images.push_back(RamImage());
      filter_2d_mipmap_pages(_ram_images[n + 1], _ram_images[n],
                             x_size, y_size);
      x_size = max(x_size >> 1, 1);
      y_size = max(y_size >> 1, 1);
      ++n;
    }
  }

  if (orig_compression_mode != CM_off) {
    // Now attempt to recompress the mipmap images according to the
    // original compression mode.  We don't need to bother compressing
    // the first image (it was already compressed, after all), so
    // temporarily remove it from the top of the mipmap stack, and
    // compress all of the rest of them instead.
    nassertv(_ram_images.size() > 1);
    int l0_x_size = _x_size;
    int l0_y_size = _y_size;
    int l0_z_size = _z_size;
    _x_size = do_get_expected_mipmap_x_size(1);
    _y_size = do_get_expected_mipmap_y_size(1);
    _z_size = do_get_expected_mipmap_z_size(1);
    RamImage uncompressed_image = _ram_images[0];
    _ram_images.erase(_ram_images.begin());

    bool success = do_compress_ram_image(orig_compression_mode, QL_default, NULL);
    // Now restore the toplevel image.
    if (success) {
      _ram_images.insert(_ram_images.begin(), orig_compressed_image);
    } else {
      _ram_images.insert(_ram_images.begin(), uncompressed_image);
    }
    _x_size = l0_x_size;
    _y_size = l0_y_size;
    _z_size = l0_z_size;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_set_pad_size
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
do_set_pad_size(int x, int y, int z) {
  if (x > _x_size) {
    x = _x_size;
  }
  if (y > _y_size) {
    y = _y_size;
  }
  if (z > _z_size) {
    z = _z_size;
  }

  _pad_x_size = x;
  _pad_y_size = y;
  _pad_z_size = z;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_reload
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
bool Texture::
do_reload() {
  if (_loaded_from_image && !_fullpath.empty()) {
    do_clear_ram_image();
    do_unlock_and_reload_ram_image(true);
    if (do_has_ram_image()) {
      // An explicit call to reload() should increment image_modified.
      ++_image_modified;
      return true;
    }
    return false;
  }

  // We don't have a filename to load from.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::convert_from_pnmimage
//       Access: Private, Static
//  Description: Internal method to convert pixel data from the
//               indicated PNMImage into the given ram_image.
////////////////////////////////////////////////////////////////////
void Texture::
convert_from_pnmimage(PTA_uchar &image, size_t page_size, int z,
                      const PNMImage &pnmimage,
                      int num_components, int component_width) {
  int x_size = pnmimage.get_x_size();
  int y_size = pnmimage.get_y_size();
  xelval maxval = pnmimage.get_maxval();

  bool is_grayscale = (num_components == 1 || num_components == 2);
  bool has_alpha = (num_components == 2 || num_components == 4);
  bool img_has_alpha = pnmimage.has_alpha();

  int idx = page_size * z;
  nassertv(idx + page_size <= image.size());
  unsigned char *p = &image[idx];

  if (maxval == 255 && component_width == 1) {
    // Most common case: one byte per pixel, and the source image
    // shows a maxval of 255.  No scaling is necessary.
    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        if (is_grayscale) {
          store_unscaled_byte(p, pnmimage.get_gray_val(i, j));
        } else {
          store_unscaled_byte(p, pnmimage.get_blue_val(i, j));
          store_unscaled_byte(p, pnmimage.get_green_val(i, j));
          store_unscaled_byte(p, pnmimage.get_red_val(i, j));
        }
        if (has_alpha) {
          if (img_has_alpha) {
            store_unscaled_byte(p, pnmimage.get_alpha_val(i, j));
          } else {
            store_unscaled_byte(p, 255);
          }
        }
      }
    }

  } else if (maxval == 65535 && component_width == 2) {
    // Another possible case: two bytes per pixel, and the source
    // image shows a maxval of 65535.  Again, no scaling is necessary.
    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        if (is_grayscale) {
          store_unscaled_short(p, pnmimage.get_gray_val(i, j));
        } else {
          store_unscaled_short(p, pnmimage.get_blue_val(i, j));
          store_unscaled_short(p, pnmimage.get_green_val(i, j));
          store_unscaled_short(p, pnmimage.get_red_val(i, j));
        }
        if (has_alpha) {
          if (img_has_alpha) {
            store_unscaled_short(p, pnmimage.get_alpha_val(i, j));
          } else {
            store_unscaled_short(p, 65535);
          }
        }
      }
    }

  } else if (component_width == 1) {
    // A less common case: one byte per pixel, but the maxval is
    // something other than 255.  In this case, we should scale the
    // pixel values up to the appropriate amount.
    double scale = 255.0 / (double)maxval;

    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        if (is_grayscale) {
          store_scaled_byte(p, pnmimage.get_gray_val(i, j), scale);
        } else {
          store_scaled_byte(p, pnmimage.get_blue_val(i, j), scale);
          store_scaled_byte(p, pnmimage.get_green_val(i, j), scale);
          store_scaled_byte(p, pnmimage.get_red_val(i, j), scale);
        }
        if (has_alpha) {
          if (img_has_alpha) {
            store_scaled_byte(p, pnmimage.get_alpha_val(i, j), scale);
          } else {
            store_unscaled_byte(p, 255);
          }
        }
      }
    }

  } else {  // component_width == 2
    // Another uncommon case: two bytes per pixel, and the maxval is
    // something other than 65535.  Again, we must scale the pixel
    // values.
    double scale = 65535.0 / (double)maxval;

    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        if (is_grayscale) {
          store_scaled_short(p, pnmimage.get_gray_val(i, j), scale);
        } else {
          store_scaled_short(p, pnmimage.get_blue_val(i, j), scale);
          store_scaled_short(p, pnmimage.get_green_val(i, j), scale);
          store_scaled_short(p, pnmimage.get_red_val(i, j), scale);
        }
        if (has_alpha) {
          if (img_has_alpha) {
            store_scaled_short(p, pnmimage.get_alpha_val(i, j), 1.0);
          } else {
            store_unscaled_short(p, 65535);
          }
        }
      }
    }
  }

  nassertv(p == &image[idx] + page_size);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::convert_to_pnmimage
//       Access: Private, Static
//  Description: Internal method to convert pixel data to the
//               indicated PNMImage from the given ram_image.
////////////////////////////////////////////////////////////////////
bool Texture::
convert_to_pnmimage(PNMImage &pnmimage, int x_size, int y_size,
                    int num_components, int component_width,
                    CPTA_uchar image, size_t page_size, int z) {
  pnmimage.clear(x_size, y_size, num_components);
  bool has_alpha = pnmimage.has_alpha();
  bool is_grayscale = pnmimage.is_grayscale();

  int idx = page_size * z;
  nassertr(idx + page_size <= image.size(), false);
  const unsigned char *p = &image[idx];

  if (component_width == 1) {
    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        if (is_grayscale) {
          pnmimage.set_gray(i, j, get_unsigned_byte(p));
        } else {
          pnmimage.set_blue(i, j, get_unsigned_byte(p));
          pnmimage.set_green(i, j, get_unsigned_byte(p));
          pnmimage.set_red(i, j, get_unsigned_byte(p));
        }
        if (has_alpha) {
          pnmimage.set_alpha(i, j, get_unsigned_byte(p));
        }
      }
    }

  } else if (component_width == 2) {
    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        if (is_grayscale) {
          pnmimage.set_gray(i, j, get_unsigned_short(p));
        } else {
          pnmimage.set_blue(i, j, get_unsigned_short(p));
          pnmimage.set_green(i, j, get_unsigned_short(p));
          pnmimage.set_red(i, j, get_unsigned_short(p));
        }
        if (has_alpha) {
          pnmimage.set_alpha(i, j, get_unsigned_short(p));
        }
      }
    }

  } else {
    return false;
  }

  nassertr(p == &image[idx] + page_size, false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_dds_level_bgr8
//       Access: Private, Static
//  Description: Called by read_dds for a DDS file in BGR8 format.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
read_dds_level_bgr8(Texture *tex, const DDSHeader &header, int n, istream &in) {
  // This is in order B, G, R.
  int x_size = tex->do_get_expected_mipmap_x_size(n);
  int y_size = tex->do_get_expected_mipmap_y_size(n);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(n);
  size_t row_bytes = x_size * 3;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    nassertr(p + row_bytes <= image.p() + size, PTA_uchar());
    in.read((char *)p, row_bytes);
  }

  return image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_dds_level_rgb8
//       Access: Private, Static
//  Description: Called by read_dds for a DDS file in RGB8 format.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
read_dds_level_rgb8(Texture *tex, const DDSHeader &header, int n, istream &in) {
  // This is in order R, G, B.
  int x_size = tex->do_get_expected_mipmap_x_size(n);
  int y_size = tex->do_get_expected_mipmap_y_size(n);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(n);
  size_t row_bytes = x_size * 3;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    nassertr(p + row_bytes <= image.p() + size, PTA_uchar());
    in.read((char *)p, row_bytes);

    // Now reverse the r, g, b triples.
    for (int x = 0; x < x_size; ++x) {
      unsigned char r = p[0];
      p[0] = p[2];
      p[2] = r;
      p += 3;
    }
    nassertr(p <= image.p() + size, PTA_uchar());
  }

  return image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_dds_level_abgr8
//       Access: Private, Static
//  Description: Called by read_dds for a DDS file in ABGR8 format.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
read_dds_level_abgr8(Texture *tex, const DDSHeader &header, int n, istream &in) {
  // This is laid out in order R, G, B, A.
  int x_size = tex->do_get_expected_mipmap_x_size(n);
  int y_size = tex->do_get_expected_mipmap_y_size(n);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(n);
  size_t row_bytes = x_size * 4;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    in.read((char *)p, row_bytes);

    PN_uint32 *pw = (PN_uint32 *)p;
    for (int x = 0; x < x_size; ++x) {
      PN_uint32 w = *pw;
#ifdef WORDS_BIGENDIAN
      // bigendian: convert R, G, B, A to B, G, R, A.
      w = ((w & 0xff00) << 16) | ((w & 0xff000000U) >> 16) | (w & 0xff00ff);
#else
      // littendian: convert A, B, G, R to to A, R, G, B.
      w = ((w & 0xff) << 16) | ((w & 0xff0000) >> 16) | (w & 0xff00ff00U);
#endif
      *pw = w;
      ++pw;
    }
    nassertr((unsigned char *)pw <= image.p() + size, PTA_uchar());
  }

  return image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_dds_level_rgba8
//       Access: Private, Static
//  Description: Called by read_dds for a DDS file in RGBA8 format.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
read_dds_level_rgba8(Texture *tex, const DDSHeader &header, int n, istream &in) {
  // This is actually laid out in order B, G, R, A.
  int x_size = tex->do_get_expected_mipmap_x_size(n);
  int y_size = tex->do_get_expected_mipmap_y_size(n);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(n);
  size_t row_bytes = x_size * 4;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    nassertr(p + row_bytes <= image.p() + size, PTA_uchar());
    in.read((char *)p, row_bytes);
  }

  return image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_dds_level_generic_uncompressed
//       Access: Private, Static
//  Description: Called by read_dds for a DDS file whose format isn't
//               one we've specifically optimized.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
read_dds_level_generic_uncompressed(Texture *tex, const DDSHeader &header,
                                    int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(n);
  int y_size = tex->do_get_expected_mipmap_y_size(n);

  int pitch = (x_size * header.pf.rgb_bitcount) / 8;

  // MS says the pitch can be supplied in the header file and must be
  // DWORD aligned, but this appears to apply to level 0 mipmaps only
  // (where it almost always will be anyway).  Other mipmap levels
  // seem to be tightly packed, but there isn't a separate pitch for
  // each mipmap level.  Weird.
  if (n == 0) {
    pitch = ((pitch + 3) / 4) * 4;
    if (header.dds_flags & DDSD_PITCH) {
      pitch = header.pitch;
    }
  }

  int bpp = header.pf.rgb_bitcount / 8;
  int skip_bytes = pitch - (bpp * x_size);
  nassertr(skip_bytes >= 0, PTA_uchar());

  unsigned int r_mask = header.pf.r_mask;
  unsigned int g_mask = header.pf.g_mask;
  unsigned int b_mask = header.pf.b_mask;
  unsigned int a_mask = header.pf.a_mask;

  // Determine the number of bits to shift each mask to the right so
  // that the lowest on bit is at bit 0.
  int r_shift = get_lowest_on_bit(r_mask);
  int g_shift = get_lowest_on_bit(g_mask);
  int b_shift = get_lowest_on_bit(b_mask);
  int a_shift = get_lowest_on_bit(a_mask);

  // Then determine the scale factor required to raise the highest
  // color value to 0xff000000.
  unsigned int r_scale = 0;
  if (r_mask != 0) {
    r_scale = 0xff000000 / (r_mask >> r_shift);
  }
  unsigned int g_scale = 0;
  if (g_mask != 0) {
    g_scale = 0xff000000 / (g_mask >> g_shift);
  }
  unsigned int b_scale = 0;
  if (b_mask != 0) {
    b_scale = 0xff000000 / (b_mask >> b_shift);
  }
  unsigned int a_scale = 0;
  if (a_mask != 0) {
    a_scale = 0xff000000 / (a_mask >> a_shift);
  }

  bool add_alpha = has_alpha(tex->_format);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(n);
  size_t row_bytes = x_size * tex->_num_components;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    for (int x = 0; x < x_size; ++x) {

      // Read a little-endian numeric value of bpp bytes.
      unsigned int pixel = 0;
      int shift = 0;
      for (int bi = 0; bi < bpp; ++bi) {
        unsigned int ch = (unsigned char)in.get();
        pixel |= (ch << shift);
        shift += 8;
      }

      // Then break apart that value into its R, G, B, and maybe A
      // components.
      unsigned int r = (((pixel & r_mask) >> r_shift) * r_scale) >> 24;
      unsigned int g = (((pixel & g_mask) >> g_shift) * g_scale) >> 24;
      unsigned int b = (((pixel & b_mask) >> b_shift) * b_scale) >> 24;

      // Store the components in the Texture's image data.
      store_unscaled_byte(p, b);
      store_unscaled_byte(p, g);
      store_unscaled_byte(p, r);
      if (add_alpha) {
        unsigned int a = (((pixel & a_mask) >> a_shift) * a_scale) >> 24;
        store_unscaled_byte(p, a);
      }
    }
    nassertr(p <= image.p() + size, PTA_uchar());
    for (int bi = 0; bi < skip_bytes; ++bi) {
      in.get();
    }
  }

  return image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_dds_level_luminance_uncompressed
//       Access: Private, Static
//  Description: Called by read_dds for a DDS file in uncompressed
//               luminance or luminance-alpha format.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
read_dds_level_luminance_uncompressed(Texture *tex, const DDSHeader &header,
                                      int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(n);
  int y_size = tex->do_get_expected_mipmap_y_size(n);

  int pitch = (x_size * header.pf.rgb_bitcount) / 8;

  // MS says the pitch can be supplied in the header file and must be
  // DWORD aligned, but this appears to apply to level 0 mipmaps only
  // (where it almost always will be anyway).  Other mipmap levels
  // seem to be tightly packed, but there isn't a separate pitch for
  // each mipmap level.  Weird.
  if (n == 0) {
    pitch = ((pitch + 3) / 4) * 4;
    if (header.dds_flags & DDSD_PITCH) {
      pitch = header.pitch;
    }
  }

  int bpp = header.pf.rgb_bitcount / 8;
  int skip_bytes = pitch - (bpp * x_size);
  nassertr(skip_bytes >= 0, PTA_uchar());

  unsigned int r_mask = header.pf.r_mask;
  unsigned int a_mask = header.pf.a_mask;

  // Determine the number of bits to shift each mask to the right so
  // that the lowest on bit is at bit 0.
  int r_shift = get_lowest_on_bit(r_mask);
  int a_shift = get_lowest_on_bit(a_mask);

  // Then determine the scale factor required to raise the highest
  // color value to 0xff000000.
  unsigned int r_scale = 0;
  if (r_mask != 0) {
    r_scale = 0xff000000 / (r_mask >> r_shift);
  }
  unsigned int a_scale = 0;
  if (a_mask != 0) {
    a_scale = 0xff000000 / (a_mask >> a_shift);
  }

  bool add_alpha = has_alpha(tex->_format);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(n);
  size_t row_bytes = x_size * tex->_num_components;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    for (int x = 0; x < x_size; ++x) {

      // Read a little-endian numeric value of bpp bytes.
      unsigned int pixel = 0;
      int shift = 0;
      for (int bi = 0; bi < bpp; ++bi) {
        unsigned int ch = (unsigned char)in.get();
        pixel |= (ch << shift);
        shift += 8;
      }

      unsigned int r = (((pixel & r_mask) >> r_shift) * r_scale) >> 24;

      // Store the components in the Texture's image data.
      store_unscaled_byte(p, r);
      if (add_alpha) {
        unsigned int a = (((pixel & a_mask) >> a_shift) * a_scale) >> 24;
        store_unscaled_byte(p, a);
      }
    }
    nassertr(p <= image.p() + size, PTA_uchar());
    for (int bi = 0; bi < skip_bytes; ++bi) {
      in.get();
    }
  }

  return image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_dds_level_dxt1
//       Access: Private, Static
//  Description: Called by read_dds for DXT1 file format.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
read_dds_level_dxt1(Texture *tex, const DDSHeader &header, int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(n);
  int y_size = tex->do_get_expected_mipmap_y_size(n);

  static const int div = 4;
  static const int block_bytes = 8;

  // The DXT1 image is divided into num_rows x num_cols blocks, where
  // each block represents 4x4 pixels.
  int num_cols = max(div, x_size) / div;
  int num_rows = max(div, y_size) / div;
  int row_length = num_cols * block_bytes;
  int linear_size = row_length * num_rows;

  if (n == 0) {
    if (header.dds_flags & DDSD_LINEARSIZE) {
      nassertr(linear_size == (int)header.pitch, PTA_uchar());
    }
  }

  PTA_uchar image = PTA_uchar::empty_array(linear_size);

  if (y_size >= 4) {
    // We have to flip the image as we read it, because of DirectX's
    // inverted sense of up.  That means we (a) reverse the order of the
    // rows of blocks . . .
    for (int ri = num_rows - 1; ri >= 0; --ri) {
      unsigned char *p = image.p() + row_length * ri;
      in.read((char *)p, row_length);

      for (int ci = 0; ci < num_cols; ++ci) {
        // . . . and (b) within each block, we reverse the 4 individual
        // rows of 4 pixels.
        PN_uint32 *cells = (PN_uint32 *)p;
        PN_uint32 w = cells[1];
        w = ((w & 0xff) << 24) | ((w & 0xff00) << 8) | ((w & 0xff0000) >> 8) | ((w & 0xff000000U) >> 24);
        cells[1] = w;

        p += block_bytes;
      }
    }

  } else if (y_size >= 2) {
    // To invert a two-pixel high image, we just flip two rows within a cell.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);

    for (int ci = 0; ci < num_cols; ++ci) {
      PN_uint32 *cells = (PN_uint32 *)p;
      PN_uint32 w = cells[1];
      w = ((w & 0xff) << 8) | ((w & 0xff00) >> 8);
      cells[1] = w;

      p += block_bytes;
    }

  } else if (y_size >= 1) {
    // No need to invert a one-pixel-high image.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);
  }

  return image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_dds_level_dxt23
//       Access: Private, Static
//  Description: Called by read_dds for DXT2 or DXT3 file format.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
read_dds_level_dxt23(Texture *tex, const DDSHeader &header, int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(n);
  int y_size = tex->do_get_expected_mipmap_y_size(n);

  static const int div = 4;
  static const int block_bytes = 16;

  // The DXT3 image is divided into num_rows x num_cols blocks, where
  // each block represents 4x4 pixels.  Unlike DXT1, each block
  // consists of two 8-byte chunks, representing the alpha and color
  // separately.
  int num_cols = max(div, x_size) / div;
  int num_rows = max(div, y_size) / div;
  int row_length = num_cols * block_bytes;
  int linear_size = row_length * num_rows;

  if (n == 0) {
    if (header.dds_flags & DDSD_LINEARSIZE) {
      nassertr(linear_size == (int)header.pitch, PTA_uchar());
    }
  }

  PTA_uchar image = PTA_uchar::empty_array(linear_size);

  if (y_size >= 4) {
    // We have to flip the image as we read it, because of DirectX's
    // inverted sense of up.  That means we (a) reverse the order of the
    // rows of blocks . . .
    for (int ri = num_rows - 1; ri >= 0; --ri) {
      unsigned char *p = image.p() + row_length * ri;
      in.read((char *)p, row_length);

      for (int ci = 0; ci < num_cols; ++ci) {
        // . . . and (b) within each block, we reverse the 4 individual
        // rows of 4 pixels.
        PN_uint32 *cells = (PN_uint32 *)p;

        // Alpha.  The block is four 16-bit words of pixel data.
        PN_uint32 w0 = cells[0];
        PN_uint32 w1 = cells[1];
        w0 = ((w0 & 0xffff) << 16) | ((w0 & 0xffff0000U) >> 16);
        w1 = ((w1 & 0xffff) << 16) | ((w1 & 0xffff0000U) >> 16);
        cells[0] = w1;
        cells[1] = w0;

        // Color.  Only the second 32-bit dword of the color block
        // represents the pixel data.
        PN_uint32 w = cells[3];
        w = ((w & 0xff) << 24) | ((w & 0xff00) << 8) | ((w & 0xff0000) >> 8) | ((w & 0xff000000U) >> 24);
        cells[3] = w;

        p += block_bytes;
      }
    }

  } else if (y_size >= 2) {
    // To invert a two-pixel high image, we just flip two rows within a cell.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);

    for (int ci = 0; ci < num_cols; ++ci) {
      PN_uint32 *cells = (PN_uint32 *)p;

      PN_uint32 w0 = cells[0];
      w0 = ((w0 & 0xffff) << 16) | ((w0 & 0xffff0000U) >> 16);
      cells[0] = w0;

      PN_uint32 w = cells[3];
      w = ((w & 0xff) << 8) | ((w & 0xff00) >> 8);
      cells[3] = w;

      p += block_bytes;
    }

  } else if (y_size >= 1) {
    // No need to invert a one-pixel-high image.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);
  }

  return image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_dds_level_dxt45
//       Access: Private, Static
//  Description: Called by read_dds for DXT4 or DXT5 file format.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
read_dds_level_dxt45(Texture *tex, const DDSHeader &header, int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(n);
  int y_size = tex->do_get_expected_mipmap_y_size(n);

  static const int div = 4;
  static const int block_bytes = 16;

  // The DXT5 image is similar to DXT3, in that there each 4x4 block
  // of pixels consists of an alpha block and a color block, but the
  // layout of the alpha block is different.
  int num_cols = max(div, x_size) / div;
  int num_rows = max(div, y_size) / div;
  int row_length = num_cols * block_bytes;
  int linear_size = row_length * num_rows;

  if (n == 0) {
    if (header.dds_flags & DDSD_LINEARSIZE) {
      nassertr(linear_size == (int)header.pitch, PTA_uchar());
    }
  }

  PTA_uchar image = PTA_uchar::empty_array(linear_size);

  if (y_size >= 4) {
    // We have to flip the image as we read it, because of DirectX's
    // inverted sense of up.  That means we (a) reverse the order of the
    // rows of blocks . . .
    for (int ri = num_rows - 1; ri >= 0; --ri) {
      unsigned char *p = image.p() + row_length * ri;
      in.read((char *)p, row_length);

      for (int ci = 0; ci < num_cols; ++ci) {
        // . . . and (b) within each block, we reverse the 4 individual
        // rows of 4 pixels.
        PN_uint32 *cells = (PN_uint32 *)p;

        // Alpha.  The block is one 16-bit word of reference values,
        // followed by six words of pixel values, in 12-bit rows.
        // Tricky to invert.
        unsigned char p2 = p[2];
        unsigned char p3 = p[3];
        unsigned char p4 = p[4];
        unsigned char p5 = p[5];
        unsigned char p6 = p[6];
        unsigned char p7 = p[7];

        p[2] = ((p7 & 0xf) << 4) | ((p6 & 0xf0) >> 4);
        p[3] = ((p5 & 0xf) << 4) | ((p7 & 0xf0) >> 4);
        p[4] = ((p6 & 0xf) << 4) | ((p5 & 0xf0) >> 4);
        p[5] = ((p4 & 0xf) << 4) | ((p3 & 0xf0) >> 4);
        p[6] = ((p2 & 0xf) << 4) | ((p4 & 0xf0) >> 4);
        p[7] = ((p3 & 0xf) << 4) | ((p2 & 0xf0) >> 4);

        // Color.  Only the second 32-bit dword of the color block
        // represents the pixel data.
        PN_uint32 w = cells[3];
        w = ((w & 0xff) << 24) | ((w & 0xff00) << 8) | ((w & 0xff0000) >> 8) | ((w & 0xff000000U) >> 24);
        cells[3] = w;

        p += block_bytes;
      }
    }

  } else if (y_size >= 2) {
    // To invert a two-pixel high image, we just flip two rows within a cell.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);

    for (int ci = 0; ci < num_cols; ++ci) {
      PN_uint32 *cells = (PN_uint32 *)p;

      unsigned char p2 = p[2];
      unsigned char p3 = p[3];
      unsigned char p4 = p[4];

      p[2] = ((p4 & 0xf) << 4) | ((p3 & 0xf0) >> 4);
      p[3] = ((p2 & 0xf) << 4) | ((p4 & 0xf0) >> 4);
      p[4] = ((p3 & 0xf) << 4) | ((p2 & 0xf0) >> 4);

      PN_uint32 w0 = cells[0];
      w0 = ((w0 & 0xffff) << 16) | ((w0 & 0xffff0000U) >> 16);
      cells[0] = w0;

      PN_uint32 w = cells[3];
      w = ((w & 0xff) << 8) | ((w & 0xff00) >> 8);
      cells[3] = w;

      p += block_bytes;
    }

  } else if (y_size >= 1) {
    // No need to invert a one-pixel-high image.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);
  }

  return image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_prepared
//       Access: Private
//  Description: Removes the indicated PreparedGraphicsObjects table
//               from the Texture's table, without actually releasing
//               the texture.  This is intended to be called only from
//               PreparedGraphicsObjects::release_texture(); it should
//               never be called by user code.
////////////////////////////////////////////////////////////////////
void Texture::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_prepared() was given a
    // prepared_objects which the texture didn't know about.
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::consider_downgrade
//       Access: Private, Static
//  Description: Reduces the number of channels in the texture, if
//               necessary, according to num_channels.
////////////////////////////////////////////////////////////////////
void Texture::
consider_downgrade(PNMImage &pnmimage, int num_channels, const string &name) {
  if (num_channels != 0 && num_channels < pnmimage.get_num_channels()) {
    // One special case: we can't reduce from 3 to 2 components, since
    // that would require adding an alpha channel.
    if (pnmimage.get_num_channels() == 3 && num_channels == 2) {
      return;
    }

    gobj_cat.info()
      << "Downgrading " << name << " from "
      << pnmimage.get_num_channels() << " components to "
      << num_channels << ".\n";
    pnmimage.set_num_channels(num_channels);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::compare_images
//       Access: Private, Static
//  Description: Called by generate_simple_ram_image(), this compares
//               the two PNMImages pixel-by-pixel.  If they're similar
//               enough (within a given threshold), returns true.
////////////////////////////////////////////////////////////////////
bool Texture::
compare_images(const PNMImage &a, const PNMImage &b) {
  nassertr(a.get_maxval() == 255 && b.get_maxval() == 255, false);
  nassertr(a.get_num_channels() == 4 && b.get_num_channels() == 4, false);
  nassertr(a.get_x_size() == b.get_x_size() &&
           a.get_y_size() == b.get_y_size(), false);

  int delta = 0;
  for (int yi = 0; yi < a.get_y_size(); ++yi) {
    for (int xi = 0; xi < a.get_x_size(); ++xi) {
      delta += abs(a.get_red_val(xi, yi) - b.get_red_val(xi, yi));
      delta += abs(a.get_green_val(xi, yi) - b.get_green_val(xi, yi));
      delta += abs(a.get_blue_val(xi, yi) - b.get_blue_val(xi, yi));
      delta += abs(a.get_alpha_val(xi, yi) - b.get_alpha_val(xi, yi));
    }
  }

  double average_delta = (double)delta / ((double)a.get_x_size() * (double)b.get_y_size() * (double)a.get_maxval());
  return (average_delta <= simple_image_threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::filter_2d_mipmap_pages
//       Access: Private
//  Description: Generates the next mipmap level from the previous
//               one.  If there are multiple pages (e.g. a cube map),
//               generates each page independently.
//
//               x_size and y_size are the size of the previous level.
//               They need not be a power of 2, or even a multiple of
//               2.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void Texture::
filter_2d_mipmap_pages(Texture::RamImage &to, const Texture::RamImage &from,
                       int x_size, int y_size) {
  size_t pixel_size = _num_components * _component_width;
  size_t row_size = (size_t)x_size * pixel_size;

  int to_x_size = max(x_size >> 1, 1);
  int to_y_size = max(y_size >> 1, 1);

  size_t to_row_size = (size_t)to_x_size * pixel_size;
  to._page_size = (size_t)to_y_size * to_row_size;
  to._image = PTA_uchar::empty_array(to._page_size * _z_size, get_class_type());

  Filter2DComponent *filter_component = (_component_type == T_unsigned_byte ? &filter_2d_unsigned_byte : filter_2d_unsigned_short);

  for (int z = 0; z < _z_size; ++z) {
    // For each level.
    unsigned char *p = to._image.p() + z * to._page_size;
    const unsigned char *q = from._image.p() + z * from._page_size;
    if (y_size != 1) {
      int y;
      for (y = 0; y < y_size - 1; y += 2) {
        // For each row.
        nassertv(p == to._image.p() + z * to._page_size + (y / 2) * to_row_size);
        nassertv(q == from._image.p() + z * from._page_size + y * row_size);
        if (x_size != 1) {
          int x;
          for (x = 0; x < x_size - 1; x += 2) {
            // For each pixel.
            for (int c = 0; c < _num_components; ++c) {
              // For each component.
              filter_component(p, q, pixel_size, row_size);
            }
            q += pixel_size;
          }
          if (x < x_size) {
            // Skip the last odd pixel.
            q += pixel_size;
          }
        } else {
          // Just one pixel.
          for (int c = 0; c < _num_components; ++c) {
            // For each component.
            filter_component(p, q, 0, row_size);
          }
        }
        q += row_size;
        Thread::consider_yield();
      }
      if (y < y_size) {
        // Skip the last odd row.
        q += row_size;
      }
    } else {
      // Just one row.
      if (x_size != 1) {
        int x;
        for (x = 0; x < x_size - 1; x += 2) {
          // For each pixel.
          for (int c = 0; c < _num_components; ++c) {
            // For each component.
            filter_component(p, q, pixel_size, 0);
          }
          q += pixel_size;
        }
        if (x < x_size) {
          // Skip the last odd pixel.
          q += pixel_size;
        }
      } else {
        // Just one pixel.
        for (int c = 0; c < _num_components; ++c) {
          // For each component.
          filter_component(p, q, 0, 0);
        }
      }
    }

    nassertv(p == to._image.p() + (z + 1) * to._page_size);
    nassertv(q == from._image.p() + (z + 1) * from._page_size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::filter_3d_mipmap_level
//       Access: Private
//  Description: Generates the next mipmap level from the previous
//               one, treating all the pages of the level as a single
//               3-d block of pixels.
//
//               x_size, y_size, and z_size are the size of the
//               previous level.  They need not be a power of 2, or
//               even a multiple of 2.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void Texture::
filter_3d_mipmap_level(Texture::RamImage &to, const Texture::RamImage &from,
                       int x_size, int y_size, int z_size) {
  size_t pixel_size = _num_components * _component_width;
  size_t row_size = (size_t)x_size * pixel_size;
  size_t page_size = (size_t)y_size * row_size;

  int to_x_size = max(x_size >> 1, 1);
  int to_y_size = max(y_size >> 1, 1);
  int to_z_size = max(z_size >> 1, 1);

  size_t to_row_size = (size_t)to_x_size * pixel_size;
  size_t to_page_size = (size_t)to_y_size * to_row_size;
  to._page_size = to_page_size;
  to._image = PTA_uchar::empty_array(to_page_size * to_z_size, get_class_type());

  Filter3DComponent *filter_component = (_component_type == T_unsigned_byte ? &filter_3d_unsigned_byte : filter_3d_unsigned_short);

  unsigned char *p = to._image.p();
  const unsigned char *q = from._image.p();
  if (z_size != 1) {
    int z;
    for (z = 0; z < z_size - 1; z += 2) {
      // For each level.
      nassertv(p == to._image.p() + (z / 2) * to_page_size);
      nassertv(q == from._image.p() + z * page_size);
      if (y_size != 1) {
        int y;
        for (y = 0; y < y_size - 1; y += 2) {
          // For each row.
          nassertv(p == to._image.p() + (z / 2) * to_page_size + (y / 2) * to_row_size);
          nassertv(q == from._image.p() + z * page_size + y * row_size);
          if (x_size != 1) {
            int x;
            for (x = 0; x < x_size - 1; x += 2) {
              // For each pixel.
              for (int c = 0; c < _num_components; ++c) {
                // For each component.
                filter_component(p, q, pixel_size, row_size, page_size);
              }
              q += pixel_size;
            }
            if (x < x_size) {
              // Skip the last odd pixel.
              q += pixel_size;
            }
          } else {
            // Just one pixel.
            for (int c = 0; c < _num_components; ++c) {
              // For each component.
              filter_component(p, q, 0, row_size, page_size);
            }
          }
          q += row_size;
          Thread::consider_yield();
        }
        if (y < y_size) {
          // Skip the last odd row.
          q += row_size;
        }
      } else {
        // Just one row.
        if (x_size != 1) {
          int x;
          for (x = 0; x < x_size - 1; x += 2) {
            // For each pixel.
            for (int c = 0; c < _num_components; ++c) {
              // For each component.
              filter_component(p, q, pixel_size, 0, page_size);
            }
            q += pixel_size;
          }
          if (x < x_size) {
            // Skip the last odd pixel.
            q += pixel_size;
          }
        } else {
          // Just one pixel.
          for (int c = 0; c < _num_components; ++c) {
            // For each component.
            filter_component(p, q, 0, 0, page_size);
          }
        }
      }
      q += page_size;
    }
    if (z < z_size) {
      // Skip the last odd page.
      q += page_size;
    }
  } else {
    // Just one page.
    if (y_size != 1) {
      int y;
      for (y = 0; y < y_size - 1; y += 2) {
        // For each row.
        nassertv(p == to._image.p() + (y / 2) * to_row_size);
        nassertv(q == from._image.p() + y * row_size);
        if (x_size != 1) {
          int x;
          for (x = 0; x < x_size - 1; x += 2) {
            // For each pixel.
            for (int c = 0; c < _num_components; ++c) {
              // For each component.
              filter_component(p, q, pixel_size, row_size, 0);
            }
            q += pixel_size;
          }
          if (x < x_size) {
            // Skip the last odd pixel.
            q += pixel_size;
          }
        } else {
          // Just one pixel.
          for (int c = 0; c < _num_components; ++c) {
            // For each component.
            filter_component(p, q, 0, row_size, 0);
          }
        }
        q += row_size;
        Thread::consider_yield();
      }
      if (y < y_size) {
        // Skip the last odd row.
        q += row_size;
      }
    } else {
      // Just one row.
      if (x_size != 1) {
        int x;
        for (x = 0; x < x_size - 1; x += 2) {
          // For each pixel.
          for (int c = 0; c < _num_components; ++c) {
            // For each component.
            filter_component(p, q, pixel_size, 0, 0);
          }
          q += pixel_size;
        }
        if (x < x_size) {
          // Skip the last odd pixel.
          q += pixel_size;
        }
      } else {
        // Just one pixel.
        for (int c = 0; c < _num_components; ++c) {
          // For each component.
          filter_component(p, q, 0, 0, 0);
        }
      }
    }
  }

  nassertv(p == to._image.p() + to_z_size * to_page_size);
  nassertv(q == from._image.p() + z_size * page_size);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::filter_2d_unsigned_byte
//       Access: Public, Static
//  Description: Averages a 2x2 block of pixel components into a
//               single pixel component, for producing the next mipmap
//               level.  Increments p and q to the next component.
////////////////////////////////////////////////////////////////////
void Texture::
filter_2d_unsigned_byte(unsigned char *&p, const unsigned char *&q,
                        size_t pixel_size, size_t row_size) {
  unsigned int result = ((unsigned int)q[0] +
                         (unsigned int)q[pixel_size] +
                         (unsigned int)q[row_size] +
                         (unsigned int)q[pixel_size + row_size]) >> 2;
  *p = (unsigned char)result;
  ++p;
  ++q;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::filter_2d_unsigned_short
//       Access: Public, Static
//  Description: Averages a 2x2 block of pixel components into a
//               single pixel component, for producing the next mipmap
//               level.  Increments p and q to the next component.
////////////////////////////////////////////////////////////////////
void Texture::
filter_2d_unsigned_short(unsigned char *&p, const unsigned char *&q,
                         size_t pixel_size, size_t row_size) {
  unsigned int result = ((unsigned int)*(unsigned short *)&q[0] +
                         (unsigned int)*(unsigned short *)&q[pixel_size] +
                         (unsigned int)*(unsigned short *)&q[row_size] +
                         (unsigned int)*(unsigned short *)&q[pixel_size + row_size]) >> 2;
  store_unscaled_short(p, result);
  q += 2;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::filter_3d_unsigned_byte
//       Access: Public, Static
//  Description: Averages a 2x2x2 block of pixel components into a
//               single pixel component, for producing the next mipmap
//               level.  Increments p and q to the next component.
////////////////////////////////////////////////////////////////////
void Texture::
filter_3d_unsigned_byte(unsigned char *&p, const unsigned char *&q,
                        size_t pixel_size, size_t row_size, size_t page_size) {
  unsigned int result = ((unsigned int)q[0] +
                         (unsigned int)q[pixel_size] +
                         (unsigned int)q[row_size] +
                         (unsigned int)q[pixel_size + row_size] +
                         (unsigned int)q[page_size] +
                         (unsigned int)q[pixel_size + page_size] +
                         (unsigned int)q[row_size + page_size] +
                         (unsigned int)q[pixel_size + row_size + page_size]) >> 3;
  *p = (unsigned char)result;
  ++p;
  ++q;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::filter_3d_unsigned_short
//       Access: Public, Static
//  Description: Averages a 2x2x2 block of pixel components into a
//               single pixel component, for producing the next mipmap
//               level.  Increments p and q to the next component.
////////////////////////////////////////////////////////////////////
void Texture::
filter_3d_unsigned_short(unsigned char *&p, const unsigned char *&q,
                         size_t pixel_size, size_t row_size,
                         size_t page_size) {
  unsigned int result = ((unsigned int)*(unsigned short *)&q[0] +
                         (unsigned int)*(unsigned short *)&q[pixel_size] +
                         (unsigned int)*(unsigned short *)&q[row_size] +
                         (unsigned int)*(unsigned short *)&q[pixel_size + row_size] +
                         (unsigned int)*(unsigned short *)&q[page_size] +
                         (unsigned int)*(unsigned short *)&q[pixel_size + page_size] +
                         (unsigned int)*(unsigned short *)&q[row_size + page_size] +
                         (unsigned int)*(unsigned short *)&q[pixel_size + row_size + page_size]) >> 3;
  store_unscaled_short(p, result);
  q += 2;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_squish
//       Access: Private
//  Description: Invokes the squish library to compress the RAM
//               image(s).
////////////////////////////////////////////////////////////////////
bool Texture::
do_squish(Texture::CompressionMode compression, int squish_flags) {
#ifdef HAVE_SQUISH
  if (_ram_images.empty() || _ram_image_compression != CM_off) {
    return false;
  }

  if (!do_has_all_ram_mipmap_images()) {
    // If we're about to compress the RAM image, we should ensure that
    // we have all of the mipmap levels first.
    do_generate_ram_mipmap_images();
  }

  RamImages compressed_ram_images;
  compressed_ram_images.reserve(_ram_images.size());
  for (size_t n = 0; n < _ram_images.size(); ++n) {
    RamImage compressed_image;
    int x_size = do_get_expected_mipmap_x_size(n);
    int y_size = do_get_expected_mipmap_y_size(n);
    int z_size = do_get_expected_mipmap_z_size(n);
    int page_size = squish::GetStorageRequirements(x_size, y_size, squish_flags);
    int cell_size = squish::GetStorageRequirements(4, 4, squish_flags);

    compressed_image._page_size = page_size;
    compressed_image._image = PTA_uchar::empty_array(page_size * z_size);
    for (int z = 0; z < z_size; ++z) {
      unsigned char *dest_page = compressed_image._image.p() + z * page_size;
      unsigned const char *source_page = _ram_images[n]._image.p() + z * _ram_images[n]._page_size;
      unsigned const char *source_page_end = source_page + _ram_images[n]._page_size;
      // Convert one 4 x 4 cell at a time.
      unsigned char *d = dest_page;
      for (int y = 0; y < y_size; y += 4) {
        for (int x = 0; x < x_size; x += 4) {
          unsigned char tb[16 * 4];
          int mask = 0;
          unsigned char *t = tb;
          for (int i = 0; i < 16; ++i) {
            int xi = x + i % 4;
            int yi = y + i / 4;
            unsigned const char *s = source_page + (yi * x_size + xi) * _num_components;
            if (s < source_page_end) {
              switch (_num_components) {
              case 1:
                t[0] = s[0];   // r
                t[1] = s[0];   // g
                t[2] = s[0];   // b
                t[3] = 255;    // a
                break;

              case 2:
                t[0] = s[0];   // r
                t[1] = s[0];   // g
                t[2] = s[0];   // b
                t[3] = s[1];   // a
                break;

              case 3:
                t[0] = s[2];   // r
                t[1] = s[1];   // g
                t[2] = s[0];   // b
                t[3] = 255;    // a
                break;

              case 4:
                t[0] = s[2];   // r
                t[1] = s[1];   // g
                t[2] = s[0];   // b
                t[3] = s[3];   // a
                break;
              }
              mask |= (1 << i);
            }
            t += 4;
          }
          squish::CompressMasked(tb, mask, d, squish_flags);
          d += cell_size;
          Thread::consider_yield();
        }
      }
    }
    compressed_ram_images.push_back(compressed_image);
  }
  _ram_images.swap(compressed_ram_images);
  _ram_image_compression = compression;
  return true;

#else  // HAVE_SQUISH
  return false;

#endif  // HAVE_SQUISH
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_unsquish
//       Access: Private
//  Description: Invokes the squish library to uncompress the RAM
//               image(s).
////////////////////////////////////////////////////////////////////
bool Texture::
do_unsquish(int squish_flags) {
#ifdef HAVE_SQUISH
  if (_ram_images.empty()) {
    return false;
  }
  RamImages uncompressed_ram_images;
  uncompressed_ram_images.reserve(_ram_images.size());
  for (size_t n = 0; n < _ram_images.size(); ++n) {
    RamImage uncompressed_image;
    int x_size = do_get_expected_mipmap_x_size(n);
    int y_size = do_get_expected_mipmap_y_size(n);
    int z_size = do_get_expected_mipmap_z_size(n);
    int page_size = squish::GetStorageRequirements(x_size, y_size, squish_flags);
    int cell_size = squish::GetStorageRequirements(4, 4, squish_flags);

    uncompressed_image._page_size = do_get_expected_ram_mipmap_page_size(n);
    uncompressed_image._image = PTA_uchar::empty_array(uncompressed_image._page_size * z_size);
    for (int z = 0; z < z_size; ++z) {
      unsigned char *dest_page = uncompressed_image._image.p() + z * uncompressed_image._page_size;
      unsigned char *dest_page_end = dest_page + uncompressed_image._page_size;
      unsigned const char *source_page = _ram_images[n]._image.p() + z * page_size;
      // Unconvert one 4 x 4 cell at a time.
      unsigned const char *s = source_page;
      for (int y = 0; y < y_size; y += 4) {
        for (int x = 0; x < x_size; x += 4) {
          unsigned char tb[16 * 4];
          squish::Decompress(tb, s, squish_flags);
          s += cell_size;

          unsigned char *t = tb;
          for (int i = 0; i < 16; ++i) {
            int xi = x + i % 4;
            int yi = y + i / 4;
            unsigned char *d = dest_page + (yi * x_size + xi) * _num_components;
            if (d < dest_page_end) {
              switch (_num_components) {
              case 1:
                d[0] = t[1];   // g
                break;

              case 2:
                d[0] = t[1];   // g
                d[1] = t[3];   // a
                break;

              case 3:
                d[2] = t[0];   // r
                d[1] = t[1];   // g
                d[0] = t[2];   // b
                break;

              case 4:
                d[2] = t[0];   // r
                d[1] = t[1];   // g
                d[0] = t[2];   // b
                d[3] = t[3];   // a
                break;
              }
            }
            t += 4;
          }
        }
        Thread::consider_yield();
      }
    }
    uncompressed_ram_images.push_back(uncompressed_image);
  }
  _ram_images.swap(uncompressed_ram_images);
  _ram_image_compression = CM_off;
  return true;

#else  // HAVE_SQUISH
  return false;

#endif  // HAVE_SQUISH
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a Texture object
////////////////////////////////////////////////////////////////////
void Texture::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::make_from_bam
//       Access: Protected, Static
//  Description: Factory method to generate a Texture object
////////////////////////////////////////////////////////////////////
TypedWritable *Texture::
make_from_bam(const FactoryParams &params) {
  // The process of making a texture is slightly different than making
  // other TypedWritable objects.  That is because all creation of
  // Textures should be done through calls to TexturePool, which
  // ensures that any loads of the same filename refer to the same
  // memory.
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  // Get the filenames and texture type so we can look up the file on
  // disk first.
  string name = scan.get_string();
  Filename filename = scan.get_string();
  Filename alpha_filename = scan.get_string();

  int primary_file_num_channels = scan.get_uint8();
  int alpha_file_channel = scan.get_uint8();
  bool has_rawdata = scan.get_bool();
  TextureType texture_type = (TextureType)scan.get_uint8();

  Texture *me = NULL;
  if (has_rawdata) {
    // If the raw image data is included, then just create a Texture
    // and don't load from the file.
    me = new Texture(name);
    me->_filename = filename;
    me->_alpha_filename = alpha_filename;
    me->_primary_file_num_channels = primary_file_num_channels;
    me->_alpha_file_channel = alpha_file_channel;
    me->_texture_type = texture_type;

    // Read the texture attributes directly from the bam stream.
    me->fillin(scan, manager, has_rawdata);

  } else {
    // Now create a temporary Texture object to read all the
    // attributes from the bam stream.
    PT(Texture) dummy = new Texture("");
    dummy->fillin(scan, manager, has_rawdata);

    if (filename.empty()) {
      // This texture has no filename; since we don't have an image to
      // load, we can't actually create the texture.
      gobj_cat.info()
        << "Cannot create texture '" << name << "' with no filename.\n";

    } else {
      // This texture does have a filename, so try to load it from disk.
      VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
      if (!manager->get_filename().empty()) {
        // If texture filename was given relative to the bam filename,
        // expand it now.
        Filename bam_dir = manager->get_filename().get_dirname();
        vfs->resolve_filename(filename, bam_dir);
        if (!alpha_filename.empty()) {
          vfs->resolve_filename(alpha_filename, bam_dir);
        }
      }

      LoaderOptions options = manager->get_loader_options();
      if (dummy->uses_mipmaps()) {
        options.set_texture_flags(options.get_texture_flags() | LoaderOptions::TF_generate_mipmaps);
      }

      switch (texture_type) {
      case TT_1d_texture:
      case TT_2d_texture:
        if (alpha_filename.empty()) {
          me = TexturePool::load_texture(filename, primary_file_num_channels,
                                         false, options);
        } else {
          me = TexturePool::load_texture(filename, alpha_filename,
                                         primary_file_num_channels,
                                         alpha_file_channel,
                                         false, options);
        }
        break;

      case TT_3d_texture:
        me = TexturePool::load_3d_texture(filename, false, options);
        break;
      
      case TT_2d_texture_array:
        me = TexturePool::load_2d_texture_array(filename, false, options);
        break;
      
      case TT_cube_map:
        me = TexturePool::load_cube_map(filename, false, options);
        break;
      }
    }

    if (me != (Texture *)NULL) {
      me->fillin_from(dummy);
      me->set_name(name);
    }
  }

  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void Texture::
fillin(DatagramIterator &scan, BamReader *manager, bool has_rawdata) {
  // We have already read in the filenames; don't read them again.

  _wrap_u = (WrapMode)scan.get_uint8();
  _wrap_v = (WrapMode)scan.get_uint8();
  _wrap_w = (WrapMode)scan.get_uint8();
  _minfilter = (FilterType)scan.get_uint8();
  _magfilter = (FilterType)scan.get_uint8();
  _anisotropic_degree = scan.get_int16();
  _border_color.read_datagram(scan);

  if (manager->get_file_minor_ver() >= 1) {
    _compression = (CompressionMode)scan.get_uint8();
  }
  if (manager->get_file_minor_ver() >= 16) {
    _quality_level = (QualityLevel)scan.get_uint8();
  }

  _format = (Format)scan.get_uint8();
  _num_components = scan.get_uint8();
  ++_properties_modified;

  bool has_simple_ram_image = false;
  if (manager->get_file_minor_ver() >= 18) {
    _orig_file_x_size = scan.get_uint32();
    _orig_file_y_size = scan.get_uint32();

    has_simple_ram_image = scan.get_bool();
  }

  if (has_simple_ram_image) {
    _simple_x_size = scan.get_uint32();
    _simple_y_size = scan.get_uint32();
    _simple_image_date_generated = scan.get_int32();

    size_t u_size = scan.get_uint32();
    PTA_uchar image = PTA_uchar::empty_array(u_size, get_class_type());
    for (size_t u_idx = 0; u_idx < u_size; ++u_idx) {
      image[(int)u_idx] = scan.get_uint8();
    }

    _simple_ram_image._image = image;
    _simple_ram_image._page_size = u_size;
    ++_simple_image_modified;
  }

  if (has_rawdata) {
    _x_size = scan.get_uint32();
    _y_size = scan.get_uint32();
    _z_size = scan.get_uint32();
    _component_type = (ComponentType)scan.get_uint8();
    _component_width = scan.get_uint8();
    _ram_image_compression = CM_off;
    if (manager->get_file_minor_ver() >= 1) {
      _ram_image_compression = (CompressionMode)scan.get_uint8();
    }

    int num_ram_images = 1;
    if (manager->get_file_minor_ver() >= 3) {
      num_ram_images = scan.get_uint8();
    }

    _ram_images.clear();
    _ram_images.reserve(num_ram_images);
    for (int n = 0; n < num_ram_images; ++n) {
      _ram_images.push_back(RamImage());
      _ram_images[n]._page_size = get_expected_ram_page_size();
      if (manager->get_file_minor_ver() >= 1) {
        _ram_images[n]._page_size = scan.get_uint32();
      }

      size_t u_size = scan.get_uint32();

      // fill the _image buffer with image data
      PTA_uchar image = PTA_uchar::empty_array(u_size, get_class_type());
      for (size_t u_idx = 0; u_idx < u_size; ++u_idx) {
        image[(int)u_idx] = scan.get_uint8();
      }
      _ram_images[n]._image = image;
    }
    _loaded_from_image = true;
    do_set_pad_size(0, 0, 0);
    ++_image_modified;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::fillin_from
//       Access: Protected
//  Description: Called in make_from_bam(), this method properly
//               copies the attributes from the bam stream (as stored
//               in dummy) into this texture, updating the modified
//               flags appropriately.
////////////////////////////////////////////////////////////////////
void Texture::
fillin_from(Texture *dummy) {
  MutexHolder holder(_lock);

  // Use the setters instead of setting these directly, so we can
  // correctly avoid incrementing _properties_modified if none of
  // these actually change.  (Otherwise, we'd have to reload the
  // texture to the GSG every time we loaded a new bam file that
  // reference the texture, since each bam file reference passes
  // through this function.)

  do_set_wrap_u(dummy->get_wrap_u());
  do_set_wrap_v(dummy->get_wrap_v());
  do_set_wrap_w(dummy->get_wrap_w());
  do_set_border_color(dummy->get_border_color());

  if (dummy->get_minfilter() != FT_default) {
    do_set_minfilter(dummy->get_minfilter());
  }
  if (dummy->get_magfilter() != FT_default) {
    do_set_magfilter(dummy->get_magfilter());
  }
  if (dummy->get_anisotropic_degree() != 0) {
    do_set_anisotropic_degree(dummy->get_anisotropic_degree());
  }
  if (dummy->get_compression() != CM_default) {
    do_set_compression(dummy->get_compression());
  }
  if (dummy->get_quality_level() != QL_default) {
    do_set_quality_level(dummy->get_quality_level());
  }

  Format format = dummy->get_format();
  int num_components = dummy->get_num_components();

  if (num_components == _num_components) {
    // Only reset the format if the number of components hasn't
    // changed, since if the number of components has changed our
    // texture no longer matches what it was when the bam was
    // written.
    do_set_format(format);
  }

  if (dummy->has_simple_ram_image()) {
    // Only replace the simple ram image if it was generated more
    // recently than the one we already have.
    if (_simple_ram_image._image.empty() ||
        dummy->_simple_image_date_generated > _simple_image_date_generated) {
      do_set_simple_ram_image(dummy->get_simple_ram_image(),
                              dummy->get_simple_x_size(),
                              dummy->get_simple_y_size());
      _simple_image_date_generated = dummy->_simple_image_date_generated;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void Texture::
write_datagram(BamWriter *manager, Datagram &me) {
  MutexHolder holder(_lock);

  // Write out the texture's raw pixel data if (a) the current Bam
  // Texture Mode requires that, or (b) there's no filename, so the
  // file can't be loaded up from disk, but the raw pixel data is
  // currently available in RAM.

  // Otherwise, we just write out the filename, and assume whoever
  // loads the bam file later will have access to the image file on
  // disk.
  BamWriter::BamTextureMode file_texture_mode = manager->get_file_texture_mode();
  bool has_rawdata =
    (file_texture_mode == BamWriter::BTM_rawdata || (do_has_ram_image() && _filename.empty()));
  if (has_rawdata && !do_has_ram_image()) {
    do_get_ram_image();
    if (!do_has_ram_image()) {
      // No image data after all.
      has_rawdata = false;
    }
  }

  bool has_bam_dir = !manager->get_filename().empty();
  Filename bam_dir = manager->get_filename().get_dirname();
  Filename filename = _filename;
  Filename alpha_filename = _alpha_filename;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  switch (file_texture_mode) {
  case BamWriter::BTM_unchanged:
  case BamWriter::BTM_rawdata:
    break;

  case BamWriter::BTM_fullpath:
    filename = _fullpath;
    alpha_filename = _alpha_fullpath;
    break;

  case BamWriter::BTM_relative:
    filename = _fullpath;
    alpha_filename = _alpha_fullpath;
    bam_dir.make_absolute(vfs->get_cwd());
    if (!has_bam_dir || !filename.make_relative_to(bam_dir, true)) {
      filename.find_on_searchpath(get_model_path());
    }
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Texture file " << _fullpath
        << " found as " << filename << "\n";
    }
    if (!has_bam_dir || !alpha_filename.make_relative_to(bam_dir, true)) {
      alpha_filename.find_on_searchpath(get_model_path());
    }
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Alpha image " << _alpha_fullpath
        << " found as " << alpha_filename << "\n";
    }
    break;

  case BamWriter::BTM_basename:
    filename = _fullpath.get_basename();
    alpha_filename = _alpha_fullpath.get_basename();
    break;

  default:
    gobj_cat.error()
      << "Unsupported bam-texture-mode: " << (int)file_texture_mode << "\n";
  }

  if (filename.empty() && do_has_ram_image()) {
    // If we don't have a filename, we have to store rawdata anyway.
    has_rawdata = true;
  }

  me.add_string(get_name());
  me.add_string(filename);
  me.add_string(alpha_filename);
  me.add_uint8(_primary_file_num_channels);
  me.add_uint8(_alpha_file_channel);
  me.add_bool(has_rawdata);
  me.add_uint8(_texture_type);

  // The data beginning at this point is handled by fillin().
  me.add_uint8(_wrap_u);
  me.add_uint8(_wrap_v);
  me.add_uint8(_wrap_w);
  me.add_uint8(_minfilter);
  me.add_uint8(_magfilter);
  me.add_int16(_anisotropic_degree);
  _border_color.write_datagram(me);
  me.add_uint8(_compression);
  me.add_uint8(_quality_level);

  me.add_uint8(_format);
  me.add_uint8(_num_components);

  me.add_uint32(_orig_file_x_size);
  me.add_uint32(_orig_file_y_size);

  bool has_simple_ram_image = !_simple_ram_image._image.empty();
  me.add_bool(has_simple_ram_image);

  // Write out the simple image too, so it will be available later.
  if (has_simple_ram_image) {
    me.add_uint32(_simple_x_size);
    me.add_uint32(_simple_y_size);
    me.add_int32(_simple_image_date_generated);
    me.add_uint32(_simple_ram_image._image.size());
    me.append_data(_simple_ram_image._image, _simple_ram_image._image.size());
  }

  // If we are also including the texture's image data, then stuff it
  // in here.
  if (has_rawdata) {
    me.add_uint32(_x_size);
    me.add_uint32(_y_size);
    me.add_uint32(_z_size);
    me.add_uint8(_component_type);
    me.add_uint8(_component_width);
    me.add_uint8(_ram_image_compression);
    me.add_uint8(_ram_images.size());
    for (size_t n = 0; n < _ram_images.size(); ++n) {
      me.add_uint32(_ram_images[n]._page_size);
      me.add_uint32(_ram_images[n]._image.size());
      me.append_data(_ram_images[n]._image, _ram_images[n]._image.size());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::TextureType output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, Texture::TextureType tt) {
  switch (tt) {
  case Texture::TT_1d_texture:
    return out << "1d_texture";
  case Texture::TT_2d_texture:
    return out << "2d_texture";
  case Texture::TT_3d_texture:
    return out << "3d_texture";
  case Texture::TT_2d_texture_array:
    return out << "2d_texture_array";
  case Texture::TT_cube_map:
    return out << "cube_map";
  }

  return out << "(**invalid Texture::TextureType(" << (int)tt << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::ComponentType output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, Texture::ComponentType ct) {
  switch (ct) {
  case Texture::T_unsigned_byte:
    return out << "unsigned_byte";
  case Texture::T_unsigned_short:
    return out << "unsigned_short";
  case Texture::T_float:
    return out << "float";
  case Texture::T_unsigned_int_24_8:
    return out << "unsigned_int_24_8";
  }

  return out << "(**invalid Texture::ComponentType(" << (int)ct << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::Format output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, Texture::Format f) {
  switch (f) {
  case Texture::F_depth_stencil:
    return out << "depth_stencil";
  case Texture::F_depth_component:
    return out << "depth_component";
  case Texture::F_depth_component16:
    return out << "depth_component16";
  case Texture::F_depth_component24:
    return out << "depth_component24";
  case Texture::F_depth_component32:
    return out << "depth_component32";
  case Texture::F_color_index:
    return out << "color_index";
  case Texture::F_red:
    return out << "red";
  case Texture::F_green:
    return out << "green";
  case Texture::F_blue:
    return out << "blue";
  case Texture::F_alpha:
    return out << "alpha";
  case Texture::F_rgb:
    return out << "rgb";
  case Texture::F_rgb5:
    return out << "rgb5";
  case Texture::F_rgb8:
    return out << "rgb8";
  case Texture::F_rgb12:
    return out << "rgb12";
  case Texture::F_rgb332:
    return out << "rgb332";
  case Texture::F_rgba:
    return out << "rgba";
  case Texture::F_rgbm:
    return out << "rgbm";
  case Texture::F_rgba4:
    return out << "rgba4";
  case Texture::F_rgba5:
    return out << "rgba5";
  case Texture::F_rgba8:
    return out << "rgba8";
  case Texture::F_rgba12:
    return out << "rgba12";
  case Texture::F_luminance:
    return out << "luminance";
  case Texture::F_luminance_alpha:
    return out << "luminance_alpha";
  case Texture::F_luminance_alphamask:
    return out << "luminance_alphamask";
  case Texture::F_rgba16:
    return out << "rgba16";
  case Texture::F_rgba32:
    return out << "rgba32";
  }

  return out << "(**invalid Texture::Format(" << (int)f << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::FilterType output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, Texture::FilterType ft) {
  switch (ft) {
  case Texture::FT_nearest:
    return out << "nearest";
  case Texture::FT_linear:
    return out << "linear";

  case Texture::FT_nearest_mipmap_nearest:
    return out << "nearest_mipmap_nearest";
  case Texture::FT_linear_mipmap_nearest:
    return out << "linear_mipmap_nearest";
  case Texture::FT_nearest_mipmap_linear:
    return out << "nearest_mipmap_linear";
  case Texture::FT_linear_mipmap_linear:
    return out << "linear_mipmap_linear";

  case Texture::FT_shadow:
    return out << "shadow";

  case Texture::FT_default:
    return out << "default";

  case Texture::FT_invalid:
    return out << "invalid";
  }

  return out << "(**invalid Texture::FilterType(" << (int)ft << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::FilterType input operator
//  Description:
////////////////////////////////////////////////////////////////////
istream &
operator >> (istream &in, Texture::FilterType &ft) {
  string word;
  in >> word;

  ft = Texture::string_filter_type(word);
  return in;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::WrapMode output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, Texture::WrapMode wm) {
  switch (wm) {
  case Texture::WM_clamp:
    return out << "clamp";
  case Texture::WM_repeat:
    return out << "repeat";
  case Texture::WM_mirror:
    return out << "mirror";
  case Texture::WM_mirror_once:
    return out << "mirror_once";
  case Texture::WM_border_color:
    return out << "border_color";

  case Texture::WM_invalid:
    return out << "invalid";
  }

  return out << "(**invalid Texture::WrapMode(" << (int)wm << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::WrapMode input operator
//  Description:
////////////////////////////////////////////////////////////////////
istream &
operator >> (istream &in, Texture::WrapMode &wm) {
  string word;
  in >> word;

  wm = Texture::string_wrap_mode(word);
  return in;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::CompressionMode output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, Texture::CompressionMode cm) {
  switch (cm) {
  case Texture::CM_default:
    return out << "default";
  case Texture::CM_off:
    return out << "off";
  case Texture::CM_on:
    return out << "on";
  case Texture::CM_fxt1:
    return out << "fxt1";
  case Texture::CM_dxt1:
    return out << "dxt1";
  case Texture::CM_dxt2:
    return out << "dxt2";
  case Texture::CM_dxt3:
    return out << "dxt3";
  case Texture::CM_dxt4:
    return out << "dxt4";
  case Texture::CM_dxt5:
    return out << "dxt5";
  }

  return out << "(**invalid Texture::CompressionMode(" << (int)cm << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::QualityLevel output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, Texture::QualityLevel tql) {
  switch (tql) {
  case Texture::QL_default:
    return out << "default";
  case Texture::QL_fastest:
    return out << "fastest";
  case Texture::QL_normal:
    return out << "normal";
  case Texture::QL_best:
    return out << "best";
  }

  return out << "**invalid Texture::QualityLevel (" << (int)tql << ")**";
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::QualityLevel input operator
//  Description:
////////////////////////////////////////////////////////////////////
istream &
operator >> (istream &in, Texture::QualityLevel &tql) {
  string word;
  in >> word;

  if (cmp_nocase(word, "default") == 0) {
    tql = Texture::QL_default;

  } else if (cmp_nocase(word, "fastest") == 0) {
    tql = Texture::QL_fastest;

  } else if (cmp_nocase(word, "normal") == 0) {
    tql = Texture::QL_normal;

  } else if (cmp_nocase(word, "best") == 0) {
    tql = Texture::QL_best;

  } else {
    gobj_cat->error() << "Invalid Texture::QualityLevel value: " << word << "\n";
    tql = Texture::QL_default;
  }

  return in;
}
