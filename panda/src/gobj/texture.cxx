// Filename: texture.cxx
// Created by:  mike (09Jan97)
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

#include <stddef.h>

PStatCollector Texture::_texture_read_pcollector("*:Texture:Read");
TypeHandle Texture::_type_handle;
AutoTextureScale Texture::_textures_power_2 = ATS_UNSPECIFIED;

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
  Namable(name)
{
  _primary_file_num_channels = 0;
  _alpha_file_channel = 0;
  _magfilter = FT_default;
  _minfilter = FT_default;
  _wrap_u = WM_repeat;
  _wrap_v = WM_repeat;
  _wrap_w = WM_repeat;
  _anisotropic_degree = 1;
  _keep_ram_image = true;
  _border_color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _compression = CM_default;
  _ram_image_compression = CM_off;
  _render_to_texture = false;
  _match_framebuffer_format = false;

  _texture_type = TT_2d_texture;
  _x_size = 0;
  _y_size = 1;
  _z_size = 1;
  set_format(F_rgb);
  set_component_type(T_unsigned_byte);

  _pad_x_size = 0;
  _pad_y_size = 0;
  _pad_z_size = 0;

  _loaded_from_image = false;
  _loaded_from_txo = false;
  _has_read_pages = false;
  _has_read_mipmaps = false;
  _num_mipmap_levels_read = 0;
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
  _filename(copy._filename),
  _alpha_filename(copy._alpha_filename),
  _fullpath(copy._fullpath),
  _alpha_fullpath(copy._alpha_fullpath),
  _primary_file_num_channels(copy._primary_file_num_channels),
  _alpha_file_channel(copy._alpha_file_channel),
  _x_size(copy._x_size),
  _y_size(copy._y_size),
  _z_size(copy._z_size),
  _num_components(copy._num_components),
  _component_width(copy._component_width),
  _texture_type(copy._texture_type),
  _format(copy._format),
  _component_type(copy._component_type),
  _loaded_from_image(copy._loaded_from_image),
  _loaded_from_txo(copy._loaded_from_txo),
  _has_read_pages(copy._has_read_pages),
  _has_read_mipmaps(copy._has_read_mipmaps),
  _num_mipmap_levels_read(copy._num_mipmap_levels_read),
  _wrap_u(copy._wrap_u),
  _wrap_v(copy._wrap_v),
  _wrap_w(copy._wrap_w),
  _minfilter(copy._minfilter),
  _magfilter(copy._magfilter),
  _anisotropic_degree(copy._anisotropic_degree),
  _keep_ram_image(copy._keep_ram_image),
  _border_color(copy._border_color),
  _compression(copy._compression),
  _match_framebuffer_format(copy._match_framebuffer_format),
  _pad_x_size(copy._pad_x_size),
  _pad_y_size(copy._pad_y_size),
  _pad_z_size(copy._pad_z_size),
  _ram_image_compression(copy._ram_image_compression),
  _ram_images(copy._ram_images)
{
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
  _num_components = copy._num_components;
  _component_width = copy._component_width;
  _texture_type = copy._texture_type;
  _format = copy._format;
  _component_type = copy._component_type;
  _loaded_from_image = copy._loaded_from_image;
  _loaded_from_txo = copy._loaded_from_txo;
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
  _ram_image_compression = copy._ram_image_compression;
  _ram_images = copy._ram_images;
  ++_properties_modified;
  ++_image_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Texture::
~Texture() {
  release_all();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::make_copy
//       Access: Published, Virtual
//  Description: Returns a new copy of the same Texture.  This copy,
//               if applied to geometry, will be copied into texture
//               as a separate texture from the original, so it will
//               be duplicated in texture memory (and may be
//               independently modified if desired).
//
//               If the Texture is a VideoTexture, the resulting
//               duplicate may be animated independently of the
//               original.
////////////////////////////////////////////////////////////////////
PT(Texture) Texture::
make_copy() {
  return new Texture(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear
//       Access: Published, Virtual
//  Description: Reinitializes the texture to its default, empty
//               state.
////////////////////////////////////////////////////////////////////
void Texture::
clear() {
  operator =(Texture(get_name()));
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::setup_texture
//       Access: Published
//  Description: Sets the texture to the indicated type and
//               dimensions, presumably in preparation for calling
//               read() or load(), or set_ram_image() or
//               modify_ram_image().
////////////////////////////////////////////////////////////////////
void Texture::
setup_texture(Texture::TextureType texture_type, int x_size, int y_size,
              int z_size, Texture::ComponentType component_type,
              Texture::Format format) {
  if (texture_type == TT_cube_map) {
    // Cube maps must always consist of six square images.
    nassertv(x_size == y_size && z_size == 6);

    // In principle the wrap mode shouldn't mean anything to a cube
    // map, but some drivers seem to misbehave if it's other than
    // WM_clamp.
    _wrap_u = WM_clamp;
    _wrap_v = WM_clamp;
    _wrap_w = WM_clamp;
  }

  _texture_type = texture_type;
  _x_size = x_size;
  _y_size = y_size;
  _z_size = z_size;
  set_component_type(component_type);
  set_format(format);

  clear_ram_image();
  set_pad_size();
  _loaded_from_image = false;
  _loaded_from_txo = false;
  _has_read_pages = false;
  _has_read_mipmaps = false;
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
  setup_cube_map(size, T_unsigned_byte, F_rgb);
  PTA_uchar image = make_ram_image();
  _keep_ram_image = true;

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
  setup_1d_texture(256, T_unsigned_byte, F_alpha);
  set_wrap_u(WM_clamp);
  set_minfilter(FT_nearest);
  set_magfilter(FT_nearest);
  set_compression(CM_off);

  PTA_uchar image = make_ram_image();
  _keep_ram_image = true;

  unsigned char *p = image;
  for (int xi = 0; xi < 256; ++xi) {
    *p++ = xi;
  }
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
  size_t pixels = get_x_size() * get_y_size();

  size_t bpp = 4;
  switch (get_format()) {
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

  (*this) = (*other);
  _loaded_from_image = true;
  _loaded_from_txo = true;
  _has_read_pages = false;
  _has_read_mipmaps = false;
  _num_mipmap_levels_read = 0;
  return true;
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

  writer.set_file_texture_mode(BTM_rawdata);

  if (!writer.write_object(this)) {
    return false;
  }

  if (!has_ram_image()) {
    gobj_cat.error()
      << get_name() << " does not have ram image\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::reload
//       Access: Published
//  Description: Re-reads the Texture from its disk file.  Useful when
//               you know the image on disk has recently changed, and
//               you want to update the Texture image.
//
//               Returns true on success, false on failure (in which
//               case, the Texture may or may not still be valid).
////////////////////////////////////////////////////////////////////
bool Texture::
reload() {
  if (_loaded_from_image && has_filename()) {
    reload_ram_image();
    ++_image_modified;
    return has_ram_image();
  }

  // We don't have a filename to load from.
  return false;
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
  RelatedTextures::const_iterator ti;
  ti = _related_textures.find(suffix);
  if (ti != _related_textures.end()) {
    return (*ti).second;
  }
  if (!has_fullpath()) {
    return (Texture*)NULL;
  }
  Filename main = get_fullpath();
  main.set_basename_wo_extension(main.get_basename_wo_extension() +
                                 suffix->get_name());
  Texture *res;
  if (has_alpha_fullpath()) {
    Filename alph = get_alpha_fullpath();
    alph.set_basename_wo_extension(alph.get_basename_wo_extension() +
                                   suffix->get_name());
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    if (vfs->exists(alph)) {
      // The alpha variant of the filename, with the suffix, exists.
      // Use it to load the texture.
      res = TexturePool::load_texture(main, alph,
                                      _primary_file_num_channels,
                                      _alpha_file_channel);
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
//     Function: Texture::set_wrap_u
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
set_wrap_u(Texture::WrapMode wrap) {
  if (_wrap_u != wrap) {
    ++_properties_modified;
    _wrap_u = wrap;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_wrap_v
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
set_wrap_v(Texture::WrapMode wrap) {
  if (_wrap_v != wrap) {
    ++_properties_modified;
    _wrap_v = wrap;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_wrap_w
//       Access: Published
//  Description: The W wrap direction is only used for 3-d textures.
////////////////////////////////////////////////////////////////////
void Texture::
set_wrap_w(Texture::WrapMode wrap) {
  if (_wrap_w != wrap) {
    ++_properties_modified;
    _wrap_w = wrap;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_minfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
set_minfilter(Texture::FilterType filter) {
  if (_minfilter != filter) {
    ++_properties_modified;
    _minfilter = filter;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_magfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
set_magfilter(Texture::FilterType filter) {
  if (_magfilter != filter) {
    ++_properties_modified;
    _magfilter = filter;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_anisotropic_degree
//       Access: Published
//  Description: Specifies the level of anisotropic filtering to apply
//               to the texture.  Normally, this is 1, to indicate
//               anisotropic filtering is disabled.  This may be set
//               to a number higher than one to enable anisotropic
//               filtering, if the rendering backend supports this.
////////////////////////////////////////////////////////////////////
void Texture::
set_anisotropic_degree(int anisotropic_degree) {
  if (_anisotropic_degree != anisotropic_degree) {
    ++_properties_modified;
    _anisotropic_degree = anisotropic_degree;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_border_color
//       Access: Published
//  Description: Specifies the solid color of the texture's border.
//               Some OpenGL implementations use a border for tiling
//               textures; in Panda, it is only used for specifying
//               the clamp color.
////////////////////////////////////////////////////////////////////
void Texture::
set_border_color(const Colorf &color) {
  if (_border_color != color) {
    ++_properties_modified;
    _border_color = color;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_compression
//       Access: Published
//  Description: Requests that this particular Texture be compressed
//               when it is loaded into texture memory.  
//
//               This refers to the internal compression of the
//               texture image within texture memory; it is not
//               related to jpeg or png compression, which are disk
//               file compression formats.  The actual disk file that
//               generated this texture may be stored in a compressed
//               or uncompressed format supported by Panda; it will be
//               decompressed on load, and then recompressed by the
//               graphics API if this parameter is not CM_off.
//
//               If the GSG does not support this texture compression
//               mode, the texture will silently be loaded
//               uncompressed.
////////////////////////////////////////////////////////////////////
void Texture::
set_compression(Texture::CompressionMode compression) {
  if (_compression != compression) {
    ++_properties_modified;
    _compression = compression;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_render_to_texture
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void Texture::
set_render_to_texture(bool render_to_texture) {
  _render_to_texture = render_to_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_num_mipmap_levels
//       Access: Published
//  Description: Returns the number of mipmap levels that should be
//               defined for this texture, given the texture's size.
//
//               Note that this returns a number appropriate for
//               mipmapping, even if the texture does not currently
//               have mipmapping enabled.
////////////////////////////////////////////////////////////////////
int Texture::
get_expected_num_mipmap_levels() const {
  int size = max(_x_size, max(_y_size, _z_size));
  int count = 1;
  while (size > 1) {
    size >>= 1;
    ++count;
  }
  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_mipmap_x_size
//       Access: Published
//  Description: Returns the x_size that the nth mipmap level should
//               have, based on the texture's size.
////////////////////////////////////////////////////////////////////
int Texture::
get_expected_mipmap_x_size(int n) const {
  int size = max(_x_size, 1);
  while (n > 0 && size > 1) {
    size >>= 1;
    --n;
  }
  return size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_mipmap_y_size
//       Access: Published
//  Description: Returns the y_size that the nth mipmap level should
//               have, based on the texture's size.
////////////////////////////////////////////////////////////////////
int Texture::
get_expected_mipmap_y_size(int n) const {
  int size = max(_y_size, 1);
  while (n > 0 && size > 1) {
    size >>= 1;
    --n;
  }
  return size;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_expected_mipmap_z_size
//       Access: Published
//  Description: Returns the z_size that the nth mipmap level should
//               have, based on the texture's size.
////////////////////////////////////////////////////////////////////
int Texture::
get_expected_mipmap_z_size(int n) const {
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
//     Function: Texture::has_ram_image
//       Access: Published, Virtual
//  Description: Returns true if the Texture has its image contents
//               available in main RAM, false if it exists only in
//               texture memory or in the prepared GSG context.
//
//               Note that this has nothing to do with whether
//               get_ram_image() will fail or not.  Even if
//               has_ram_image() returns false, get_ram_image() may
//               still return a valid RAM image, because
//               get_ram_image() will automatically load the texture
//               from disk if necessary.  The only thing
//               has_ram_image() tells you is whether the texture is
//               available right now without hitting the disk first.
//
//               Note also that if an application uses only one GSG,
//               it may appear that has_ram_image() returns true if
//               the texture has not yet been loaded by the GSG, but
//               this correlation is not true in general and should
//               not be depended on.  Specifically, if an application
//               ever uses multiple GSG's in its lifetime (for
//               instance, by opening more than one window, or by
//               closing its window and opening another one later),
//               then has_ram_image() may well return false on
//               textures that have never been loaded on the current
//               GSG.
////////////////////////////////////////////////////////////////////
bool Texture::
has_ram_image() const {
  return !_ram_images.empty() && !_ram_images[0]._image.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_image
//       Access: Published
//  Description: Returns the system-RAM image data associated with the
//               texture.  If the texture does not currently have an
//               associated RAM image, and the texture was generated
//               by loading an image from a disk file (the most common
//               case), this forces the reload of the same texture.
//               This can happen if keep_texture_ram is configured to
//               false, and we have previously prepared this texture
//               with a GSG.
//
//               Note that it is not correct to call has_ram_image()
//               first to test whether this function will fail.  A
//               false return value from has_ram_image() indicates
//               only that get_ram_image() may need to reload the
//               texture from disk, which it will do automatically.
//               However, you can call might_have_ram_image(), which
//               will return true if the ram image exists, or there is
//               a reasonable reason to believe it can be loaded.
//
//               On the other hand, it is possible that the texture
//               cannot be found on disk or is otherwise unavailable.
//               If that happens, this function will return NULL.
//               There is no way to predict with 100% accuracy whether
//               get_ram_image() will return NULL without calling it
//               first; might_have_ram_image() is the closest.
////////////////////////////////////////////////////////////////////
CPTA_uchar Texture::
get_ram_image() {
  if (_loaded_from_image && !has_ram_image() && has_filename()) {
    reload_ram_image();
  }

  if (_ram_images.empty()) {
    return CPTA_uchar(get_class_type());
  }

  return _ram_images[0]._image;
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
set_ram_image(PTA_uchar image, Texture::CompressionMode compression,
              size_t page_size) {
  nassertv(compression != CM_default);
  nassertv(compression != CM_off || image.size() == get_expected_ram_image_size());
  if (_ram_images.empty()) {
    _ram_images.push_back(RamImage());
  } else {
    clear_ram_mipmap_images();
  }
  if (page_size == 0) {
    page_size = image.size();
  }
  if (_ram_images[0]._image != image ||
      _ram_images[0]._page_size != page_size ||
      _ram_image_compression != compression) {
    _ram_images[0]._image = image;
    _ram_images[0]._page_size = page_size;
    _ram_image_compression = compression;
    ++_image_modified;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_ram_image
//       Access: Published
//  Description: Discards the current system-RAM image.
////////////////////////////////////////////////////////////////////
void Texture::
clear_ram_image() {
  _ram_image_compression = CM_off;
  _ram_images.clear();
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
//     Function: Texture::has_all_ram_mipmap_images
//       Access: Published
//  Description: Returns true if all expected mipmap levels have been
//               defined and exist in the system RAM, or false if even
//               one mipmap level is missing.
////////////////////////////////////////////////////////////////////
bool Texture::
has_all_ram_mipmap_images() const {
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
//     Function: Texture::get_ram_mipmap_image
//       Access: Published
//  Description: Returns the system-RAM image data associated with the
//               nth mipmap level, if present.  Returns NULL if the
//               nth mipmap level is not present.
////////////////////////////////////////////////////////////////////
CPTA_uchar Texture::
get_ram_mipmap_image(int n) {
  if (n < (int)_ram_images.size()) {
    return _ram_images[n]._image;
  }
  return CPTA_uchar(get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::make_ram_mipmap_image
//       Access: Published
//  Description: Discards the current system-RAM image for the
//               nth mipmap level, if any, and allocates a new buffer
//               of the appropriate size.  Returns the new buffer.
//
//               This does *not* affect keep_ram_image.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
make_ram_mipmap_image(int n) {
  nassertr(_ram_image_compression == CM_off, PTA_uchar(get_class_type()));

  while (n >= (int)_ram_images.size()) {
    _ram_images.push_back(RamImage());
    _ram_images.back()._page_size = 0;
  }

  _ram_images[n]._image = PTA_uchar::empty_array(get_expected_ram_mipmap_image_size(n), get_class_type());
  _ram_images[n]._page_size = get_expected_ram_mipmap_page_size(n);
  ++_image_modified;
  return _ram_images[n]._image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_ram_mipmap_image
//       Access: Published
//  Description: Replaces the current system-RAM image for the
//               indicated mipmap level with the new data.  If
//               compression is not CM_off, it indicates that the new
//               data is already pre-compressed in the indicated
//               format.
//
//               This does *not* affect keep_ram_image.
////////////////////////////////////////////////////////////////////
void Texture::
set_ram_mipmap_image(int n, PTA_uchar image, size_t page_size) {
  nassertv(_ram_image_compression != CM_off || image.size() == get_expected_ram_mipmap_image_size(n));

  while (n >= (int)_ram_images.size()) {
    _ram_images.push_back(RamImage());
    _ram_images.back()._page_size = 0;
  }
  if (page_size == 0) {
    page_size = image.size();
  }

  if (_ram_images[n]._image != image ||
      _ram_images[n]._page_size != page_size) {
    _ram_images[n]._image = image;
    _ram_images[n]._page_size = page_size;
    ++_image_modified;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_ram_mipmap_image
//       Access: Published
//  Description: Discards the current system-RAM image for the nth
//               mipmap level.
////////////////////////////////////////////////////////////////////
void Texture::
clear_ram_mipmap_image(int n) {
  if (n >= (int)_ram_images.size()) {
    return;
  }
  _ram_images[n]._image.clear();
  _ram_images[n]._page_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_ram_mipmap_images
//       Access: Published
//  Description: Discards the current system-RAM image for all
//               mipmap levels, except level 0 (the base image).
////////////////////////////////////////////////////////////////////
void Texture::
clear_ram_mipmap_images() {
  if (!_ram_images.empty()) {
    _ram_images.erase(_ram_images.begin() + 1, _ram_images.end());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::generate_ram_mipmap_images
//       Access: Published
//  Description: Automatically fills in the n mipmap levels of the
//               Texture, based on the texture's source image.  This
//               requires the texture's ram image to be available in
//               system memory.
//
//               This call is not normally necessary, since the mipmap
//               levels will be generated automatically if needed.
//               But there may be certain cases in which you would
//               like to call this explicitly.
////////////////////////////////////////////////////////////////////
void Texture::
generate_ram_mipmap_images() {
  nassertv(has_ram_image());
  nassertv(get_ram_image_compression() == CM_off);
  nassertv(get_component_type() != T_float);
  clear_ram_mipmap_images();

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
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return true;
  }
  return prepared_objects->is_texture_queued(this);
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
  indent(out, indent_level)
    << get_type() << " " << get_name();
  if (!get_filename().empty()) {
    out << " (from " << get_filename() << ")";
  }
  out << "\n";

  indent(out, indent_level + 2);
  
  switch (get_texture_type()) {
  case TT_1d_texture:
    out << "1-d, " << get_x_size();
    break;

  case TT_2d_texture:
    out << "2-d, " << get_x_size() << " x " << get_y_size();
    break;

  case TT_3d_texture:
    out << "3-d, " << get_x_size() << " x " << get_y_size()
        << " x " << get_z_size();
    break;

  case TT_cube_map:
    out << "cube map, " << get_x_size() << " x " << get_y_size();
    break;
  }

  out << " pixels, each " << get_num_components();

  switch (get_component_type()) {
  case T_unsigned_byte:
    out << " bytes";
    break;

  case T_unsigned_short:
    out << " shorts";
    break;

  case T_float:
    out << " floats";
    break;
  }

  out << ", ";
  switch (get_format()) {
  case F_color_index:
    out << "color_index";
    break;
  case F_depth_stencil:
    out << "depth_stencil";
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

  if (get_compression() != CM_default) {
    out << ", compression " << get_compression();
  }
  out << "\n";

  indent(out, indent_level + 2);
  
  switch (get_texture_type()) {
  case TT_1d_texture:
    out << get_wrap_u() << ", ";
    break;

  case TT_2d_texture:
    out << get_wrap_u() << " x " << get_wrap_v() << ", ";
    break;

  case TT_3d_texture:
    out << get_wrap_u() << " x " << get_wrap_v() 
        << " x " << get_wrap_w() << ", ";
    break;

  case TT_cube_map:
    break;
  }

  out << "min " << get_minfilter()
      << ", mag " << get_magfilter()
      << ", aniso " << get_anisotropic_degree()
      << ", border " << get_border_color()
      << "\n";

  if (has_ram_image()) {
    indent(out, indent_level + 2)
      << get_ram_image_size() << " bytes in ram, compression "
      << get_ram_image_compression() << "\n";

    int num_ram_mipmap_images = get_num_ram_mipmap_images();
    if (num_ram_mipmap_images > 1) {
      int count = 0;
      size_t total_size = 0;
      for (int n = 1; n < num_ram_mipmap_images; ++n) {
        if (has_ram_mipmap_image(n)) {
          ++count;
          total_size += get_ram_mipmap_image_size(n);
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
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_format
//       Access: Published
//  Description: Changes the format value for the texture components.
//               This implicitly sets num_components as well.
////////////////////////////////////////////////////////////////////
void Texture::
set_format(Texture::Format format) {
  _format = format;

  switch (_format) {
  case F_color_index:
  case F_depth_stencil:
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
//     Function: Texture::set_component_type
//       Access: Published
//  Description: Changes the data value for the texture components.
//               This implicitly sets component_width as well.
////////////////////////////////////////////////////////////////////
void Texture::
set_component_type(Texture::ComponentType component_type) {
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
  }
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
//     Function: Texture::texture_uploaded
//       Access: Public
//  Description: This method is called by the GraphicsStateGuardian
//               after a texture has been successfully uploaded to
//               graphics memory.  It is intended as a callback so the
//               texture can release its RAM image, if _keep_ram_image
//               is false.
//
//               Normally, this is not called directly except by the
//               GraphicsStateGuardian.
////////////////////////////////////////////////////////////////////
void Texture::
texture_uploaded() {
  if (!keep_texture_ram && !_keep_ram_image) {
    // Once we have prepared the texture, we can generally safely
    // remove the pixels from main RAM.  The GSG is now responsible
    // for remembering what it looks like.
    
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Dumping RAM for texture " << get_name() << "\n";
    }
    clear_ram_image();
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
//     Function: Texture::do_read
//       Access: Protected, Virtual
//  Description: The internal implementation of the various read()
//               methods.
////////////////////////////////////////////////////////////////////
bool Texture::
do_read(const Filename &fullpath, const Filename &alpha_fullpath,
        int primary_file_num_channels, int alpha_file_channel,
        int z, int n, bool read_pages, bool read_mipmaps,
        bool header_only, BamCacheRecord *record) {
  PStatTimer timer(_texture_read_pcollector);

  if (record != (BamCacheRecord *)NULL) {
    header_only = false;
  }

  if ((z == 0 || read_pages) && (n == 0 || read_mipmaps)) {
    // When we re-read the page 0 of the base image, we clear
    // everything and start over.
    clear_ram_image();
  }
  
  if (is_txo_filename(fullpath)) {
    if (record != (BamCacheRecord *)NULL) {
      record->add_dependent_file(fullpath);
    }
    return read_txo_file(fullpath);
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
    set_z_size(z_size);

    n = 0;
    while (true) {
      // For mipmap level 0, the total number of pages might be
      // determined by the number of files we find.  After mipmap
      // level 0, though, the number of pages is predetermined.
      if (n != 0) {
        z_size = get_expected_mipmap_z_size(n);
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
                         alpha_file_channel, header_only, record)) {
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
        n_size = get_expected_num_mipmap_levels();
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

    set_z_size(z_size);
    z = 0;
    Filename file = fullpath_pattern.get_filename_index(z);
    Filename alpha_file = alpha_fullpath_pattern.get_filename_index(z);
    while ((z_size == 0 && (vfs->exists(file) || z == 0)) ||
           (z_size != 0 && z < z_size)) {
      if (!do_read_one(file, alpha_file, z, 0, primary_file_num_channels,
                       alpha_file_channel, header_only, record)) {
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
                       header_only, record)) {
        return false;
      }
      ++n;

      if (n_size == 0 && n >= get_expected_num_mipmap_levels()) {
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
                     header_only, record)) {
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
    clear_ram_image();
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
            bool header_only, BamCacheRecord *record) {
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
      consider_rescale(image, fullpath.get_basename());
    } else {
      image.set_read_size(get_expected_mipmap_x_size(n),
                          get_expected_mipmap_y_size(n));
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
    }
  }

  if (z == 0 && n == 0) {
    if (!has_name()) {
      set_name(fullpath.get_basename_wo_extension());
    }
    if (!has_filename()) {
      set_filename(fullpath);
      set_alpha_filename(alpha_fullpath);
      
      // The first time we set the filename via a read() operation, we
      // clear keep_ram_image.  The user can always set it again later
      // if he needs to.
      _keep_ram_image = false;
    }
    
    set_fullpath(fullpath);
    set_alpha_fullpath(alpha_fullpath);
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
      alpha_image = scaled;
    }
  }

  if (n == 0) {
    consider_downgrade(image, primary_file_num_channels);
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

  return do_load_one(image, fullpath.get_basename(), z, n);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_write
//       Access: Protected
//  Description: Internal method to write a series of pages and/or
//               mipmap levels to disk files.
////////////////////////////////////////////////////////////////////
bool Texture::
do_write(const Filename &fullpath, int z, int n, bool write_pages, bool write_mipmaps) const {
  if (!has_ram_image()) {
    ((Texture *)this)->get_ram_image();
  }
  nassertr(has_ram_image(), false);

  if (is_txo_filename(fullpath)) {
    return write_txo_file(fullpath);
  }

  nassertr(has_ram_mipmap_image(n), false);
  nassertr(get_ram_image_compression() == CM_off, false);

  if (write_pages && write_mipmaps) {
    // Write a sequence of pages * mipmap levels.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    int num_levels = get_num_ram_mipmap_images();

    for (int n = 0; n < num_levels; ++n) {
      int z_size = get_expected_mipmap_z_size(n);

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

    int num_levels = get_num_ram_mipmap_images();
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
  if (!has_ram_mipmap_image(n)) {
    return false;
  }

  nassertr(get_ram_image_compression() == CM_off, false);

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
//     Function: Texture::do_load_one
//       Access: Protected, Virtual
//  Description: Internal method to load a single page or mipmap
//               level.
////////////////////////////////////////////////////////////////////
bool Texture::
do_load_one(const PNMImage &pnmimage, const string &name, int z, int n) {
  if (_ram_images.size() <= 1 && n == 0) {
    // A special case for mipmap level 0.  When we load mipmap level
    // 0, unless we already have mipmap levels, it determines the
    // image properties like size and number of components.
    if (!reconsider_z_size(z)) {
      return false;
    }
    nassertr(z >= 0 && z < _z_size, false);

    if (z == 0) {
      ComponentType component_type = T_unsigned_byte;
      xelval maxval = pnmimage.get_maxval();
      if (maxval > 255) {
        component_type = T_unsigned_short;
      }
      
      if (!reconsider_image_properties(pnmimage.get_x_size(), pnmimage.get_y_size(),
                                       pnmimage.get_num_channels(), component_type,
                                       z)) {
        return false;
      }
    }
      
    do_modify_ram_image();
    _loaded_from_image = true;
  }

  do_modify_ram_mipmap_image(n);

  // Ensure the PNMImage is an appropriate size.
  int x_size = get_expected_mipmap_x_size(n);
  int y_size = get_expected_mipmap_y_size(n);
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

    convert_from_pnmimage(_ram_images[n]._image, 
                          get_expected_ram_mipmap_page_size(n), z,
                          scaled);
  } else {
    // Now copy the pixel data from the PNMImage into our internal
    // _image component.
    convert_from_pnmimage(_ram_images[n]._image, 
                          get_expected_ram_mipmap_page_size(n), z,
                          pnmimage);
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
  ((Texture *)this)->get_ram_image();

  nassertr(has_ram_mipmap_image(n), false);
  nassertr(z >= 0 && z < get_expected_mipmap_z_size(n), false);
  nassertr(_ram_image_compression == CM_off, false);

  return convert_to_pnmimage(pnmimage, 
                             get_expected_mipmap_x_size(n), 
                             get_expected_mipmap_y_size(n), 
                             _ram_images[n]._image, 
                             get_ram_mipmap_page_size(n), z);
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
//     Function: Texture::reload_ram_image
//       Access: Protected, Virtual
//  Description: Called when the Texture image is required but the ram
//               image is not available, this will reload it from disk
//               or otherwise do whatever is required to make it
//               available, if possible.
////////////////////////////////////////////////////////////////////
void Texture::
reload_ram_image() {
  BamCache *cache = BamCache::get_global_ptr();
  if (cache->get_active() && !textures_header_only) {
    // See if the texture can be found in the on-disk cache, if it is
    // active.
    PT(BamCacheRecord) record = cache->lookup(get_fullpath(), "txo");
    if (record != (BamCacheRecord *)NULL && 
        record->has_data()) {
      gobj_cat.info()
        << "Texture " << get_name() << " reloaded from disk cache.\n";
      PT(Texture) tex = DCAST(Texture, record->extract_data());
      // We don't want to replace all the texture parameters--for
      // instance, we don't want to change the filter type or the
      // border color or anything--we just want to get the image and
      // necessary associated parameters.
      reconsider_image_properties(tex->get_x_size(), tex->get_y_size(),
                                  tex->get_num_components(), 
                                  tex->get_component_type(), 0);
      set_compression(tex->get_compression());
      _ram_images = tex->_ram_images;
      return;
    }
  }

  gobj_cat.info()
    << "Reloading texture " << get_name() << "\n";
  do_make_ram_image();

  int z = 0;
  int n = 0;

  if (_has_read_pages) {
    z = _z_size;
  }
  if (_has_read_mipmaps) {
    n = _num_mipmap_levels_read;
  }

  do_read(get_fullpath(), get_alpha_fullpath(),
          _primary_file_num_channels, _alpha_file_channel,
          z, n, _has_read_pages, _has_read_mipmaps, false, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_modify_ram_image
//       Access: Protected
//  Description: This is called internally to uniquify the ram image
//               pointer without updating _image_modified.
////////////////////////////////////////////////////////////////////
void Texture::
do_modify_ram_image() {
  if (_ram_images.empty() || _ram_images[0]._image.empty() || 
      _ram_image_compression != CM_off) {
    do_make_ram_image();
  } else {
    clear_ram_mipmap_images();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_make_ram_image
//       Access: Protected
//  Description: This is called internally to make a new ram image
//               without updating _image_modified.
////////////////////////////////////////////////////////////////////
void Texture::
do_make_ram_image() {
  _ram_images.clear();
  _ram_images.push_back(RamImage());
  _ram_images[0]._page_size = get_expected_ram_page_size();
  _ram_images[0]._image = PTA_uchar::empty_array(get_expected_ram_image_size(), get_class_type());
  _ram_image_compression = CM_off;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::do_modify_ram_mipmap_image
//       Access: Protected
//  Description: This is called internally to uniquify the nth mipmap
//               image pointer without updating _image_modified.
////////////////////////////////////////////////////////////////////
void Texture::
do_modify_ram_mipmap_image(int n) {
  nassertv(_ram_image_compression == CM_off);

  if (n >= (int)_ram_images.size() ||
      _ram_images[n]._image.empty()) {
    make_ram_mipmap_image(n);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Texture::up_to_power_2
//       Access: Protected, Static
//  Description: Returns the smallest power of 2 greater than or equal
//               to value.
////////////////////////////////////////////////////////////////////
int Texture::
up_to_power_2(int value) {
  int x = 1;
  while (x < value) {
    x = (x << 1);
  }
  return x;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::down_to_power_2
//       Access: Protected, Static
//  Description: Returns the largest power of 2 less than or equal
//               to value.
////////////////////////////////////////////////////////////////////
int Texture::
down_to_power_2(int value) {
  int x = 1;
  while ((x << 1) <= value) {
    x = (x << 1);
  }
  return x;
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
//     Function: Texture::reconsider_z_size
//       Access: Protected
//  Description: Considers whether the z_size should automatically be
//               adjusted when the user loads a new page.  Returns
//               true if the z size is valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool Texture::
reconsider_z_size(int z) {
  if (z >= _z_size) {
    // If we're loading a page past _z_size, treat it as an implicit
    // request to enlarge _z_size.  However, this is only legal if
    // this is, in fact, a 3-d texture (cube maps always have z_size
    // 6, and other types have z_size 1).
    nassertr(_texture_type == Texture::TT_3d_texture, false);

    _z_size = z + 1;
    // Increase the size of the data buffer to make room for the new
    // texture level.
    size_t new_size = get_expected_ram_image_size();
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
//     Function: Texture::reconsider_image_properties
//       Access: Protected
//  Description: Resets the internal Texture properties when a new
//               image file is loaded.  Returns true if the new image
//               is valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool Texture::
reconsider_image_properties(int x_size, int y_size, int num_components,
                            Texture::ComponentType component_type, int z) {
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
#ifndef NDEBUG
    if (_texture_type == TT_1d_texture) {
      nassertr(y_size == 1, false);
    } else if (_texture_type == TT_cube_map) {
      nassertr(x_size == y_size, false);
    }
#endif
    if ((_x_size != x_size)||(_y_size != y_size)) {
      set_pad_size();
    }
    _x_size = x_size;
    _y_size = y_size;
    _num_components = num_components;
    set_component_type(component_type);
    
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
//     Function: Texture::convert_from_pnmimage
//       Access: Private
//  Description: Internal method to convert pixel data from the
//               indicated PNMImage into the given ram_image.
////////////////////////////////////////////////////////////////////
void Texture::
convert_from_pnmimage(PTA_uchar &image, size_t page_size, int z,
                      const PNMImage &pnmimage) {
  int x_size = pnmimage.get_x_size();
  int y_size = pnmimage.get_y_size();
  xelval maxval = pnmimage.get_maxval();

  bool is_grayscale = (_num_components == 1 || _num_components == 2);
  bool has_alpha = (_num_components == 2 || _num_components == 4);
  bool img_has_alpha = pnmimage.has_alpha();

  int idx = page_size * z;
  nassertv(idx + page_size <= image.size());
  unsigned char *p = &image[idx];

  if (maxval == 255) {
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

  } else if (maxval == 65535) {
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

  } else if (maxval <= 255) {
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

  } else {
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
//       Access: Private
//  Description: Internal method to convert pixel data to the
//               indicated PNMImage from the given ram_image.
////////////////////////////////////////////////////////////////////
bool Texture::
convert_to_pnmimage(PNMImage &pnmimage, int x_size, int y_size,
                    CPTA_uchar image, size_t page_size, int z) const {
  pnmimage.clear(x_size, y_size, _num_components);
  bool has_alpha = pnmimage.has_alpha();
  bool is_grayscale = pnmimage.is_grayscale();
  
  int idx = page_size * z;
  nassertr(idx + page_size <= image.size(), false);
  const unsigned char *p = &image[idx];

  if (_component_type == T_unsigned_byte) {
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

  } else if (_component_type == T_unsigned_short) {
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
    gobj_cat.error()
      << "Couldn't write image for " << get_name()
      << "; inappropriate data type " << (int)_component_type << ".\n";
    return false;
  }

  nassertr(p == &image[idx] + page_size, false);
  return true;
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
//     Function: Texture::set_size_padded
//  Description: Changes the size of the texture, padding
//               if necessary, and setting the pad region
//               as well.
////////////////////////////////////////////////////////////////////
void Texture::
set_size_padded(int x, int y, int z) {
  if (get_textures_power_2() != ATS_none) {
    set_x_size(up_to_power_2(x));
    set_y_size(up_to_power_2(y));
    set_z_size(up_to_power_2(z));
  } else {
    set_x_size(x);
    set_y_size(y);
    set_z_size(z);
  }
  set_pad_size(get_x_size() - x,
               get_y_size() - y,
               get_z_size() - z);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::consider_rescale
//       Access: Private
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
  bool exclude = false;
  int num_excludes = exclude_texture_scale.get_num_unique_values();
  for (int i = 0; i < num_excludes && !exclude; ++i) {
    GlobPattern pat(exclude_texture_scale.get_unique_value(i));
    if (pat.matches(name)) {
      exclude = true;
    }
  }

  int new_x_size = pnmimage.get_x_size();
  int new_y_size = pnmimage.get_y_size();

  if (!exclude) {
    new_x_size = (int)cfloor(new_x_size * texture_scale + 0.5);
    new_y_size = (int)cfloor(new_y_size * texture_scale + 0.5);
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

  if (pnmimage.get_x_size() != new_x_size ||
      pnmimage.get_y_size() != new_y_size) {
    pnmimage.set_read_size(new_x_size, new_y_size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::consider_downgrade
//       Access: Private
//  Description: Reduces the number of channels in the texture, if
//               necessary, according to num_channels.
////////////////////////////////////////////////////////////////////
void Texture::
consider_downgrade(PNMImage &pnmimage, int num_channels) {
  if (num_channels != 0 && num_channels < pnmimage.get_num_channels()) {
    // One special case: we can't reduce from 3 to 2 components, since
    // that would require adding an alpha channel.
    if (pnmimage.get_num_channels() == 3 && num_channels == 2) {
      return;
    }

    gobj_cat.info()
      << "Downgrading " << get_name() << " from "
      << pnmimage.get_num_channels() << " components to "
      << num_channels << ".\n";
    pnmimage.set_num_channels(num_channels);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_txo_file
//       Access: Private
//  Description: Called internally when read() detects a txo file.
////////////////////////////////////////////////////////////////////
bool Texture::
read_txo_file(const Filename &fullpath) {
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
  bool success = read_txo(*in, fullpath);
  vfs->close_read_file(in);
  
  set_fullpath(fullpath);
  clear_alpha_fullpath();
  _keep_ram_image = false;
  
  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write_txo_file
//       Access: Private
//  Description: Called internally when write() detects a txo
//               filename.
////////////////////////////////////////////////////////////////////
bool Texture::
write_txo_file(const Filename &fullpath) const {
  Filename filename = Filename::binary_filename(fullpath);
  ofstream out;
  if (!filename.open_write(out)) {
    gobj_cat.error()
      << "Unable to open " << filename << "\n";
    return false;
  }
  
#ifdef HAVE_ZLIB
  if (fullpath.get_extension() == "pz") {
    OCompressStream compressor(&out, false);
    return write_txo(compressor);
  }
#endif  // HAVE_ZLIB
  return write_txo(out, fullpath);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::filter_2d_mipmap_pages
//       Access: Public
//  Description: Generates the next mipmap level from the previous
//               one.  If there are multiple pages (e.g. a cube map),
//               generates each page independently.
//
//               x_size and y_size are the size of the previous level.
//               They need not be a power of 2, or even a multiple of
//               2.
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
//       Access: Public
//  Description: Generates the next mipmap level from the previous
//               one, treating all the pages of the level as a single
//               3-d block of pixels.
//
//               x_size, y_size, and z_size are the size of the
//               previous level.  They need not be a power of 2, or
//               even a multiple of 2.
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

  } else {
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

      switch (texture_type) {
      case TT_1d_texture:
      case TT_2d_texture:
        if (alpha_filename.empty()) {
          me = TexturePool::load_texture(filename, primary_file_num_channels);
        } else {
          me = TexturePool::load_texture(filename, alpha_filename,
                                         primary_file_num_channels, alpha_file_channel);
        }
        break;

      case TT_3d_texture:
        me = TexturePool::load_3d_texture(filename);
        break;

      case TT_cube_map:
        me = TexturePool::load_cube_map(filename);
        break;
      }
    }
  }

  if (me == (Texture *)NULL) {
    // Oops, we couldn't load the texture; we'll just return NULL.
    // But we do need a dummy texture to read in and ignore all of the
    // attributes.
    PT(Texture) dummy = new Texture("");
    dummy->fillin(scan, manager, has_rawdata);

  } else {
    me->set_name(name);
    me->fillin(scan, manager, has_rawdata);
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

  // We use the setters here, instead of directly assigning these
  // values, so that we will correctly update _properties_modified
  // only if any of these changes.

  set_wrap_u((WrapMode)scan.get_uint8());
  set_wrap_v((WrapMode)scan.get_uint8());
  set_wrap_w((WrapMode)scan.get_uint8());
  set_minfilter((FilterType)scan.get_uint8());
  set_magfilter((FilterType)scan.get_uint8());
  set_anisotropic_degree(scan.get_int16());
  Colorf border_color;
  border_color.read_datagram(scan);
  set_border_color(border_color);

  if (manager->get_file_minor_ver() >= 1) {
    set_compression((CompressionMode)scan.get_uint8());
  }

  Format format = (Format)scan.get_uint8();
  int num_components = scan.get_uint8();

  if (num_components == get_num_components()) {
    // Only reset the format if the number of components hasn't
    // changed, since if the number of components has changed our
    // texture no longer matches what it was when the bam was
    // written.
    set_format(format);
  }

  if (has_rawdata) {
    // In the rawdata case, we must always set the format.
    _format = format;
    _num_components = num_components;
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
    set_pad_size();
    ++_image_modified;
    ++_properties_modified;
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
  // Write out the texture's raw pixel data if (a) the current Bam
  // Texture Mode requires that, or (b) there's no filename, so the
  // file can't be loaded up from disk, but the raw pixel data is
  // currently available in RAM.

  // Otherwise, we just write out the filename, and assume whoever
  // loads the bam file later will have access to the image file on
  // disk.
  BamTextureMode file_texture_mode = manager->get_file_texture_mode();
  bool has_rawdata =
    (file_texture_mode == BTM_rawdata || (has_ram_image() && get_filename().empty()));
  if (has_rawdata && !has_ram_image()) {
    get_ram_image();
    if (!has_ram_image()) {
      // No image data after all.
      has_rawdata = false;
    }
  }

  bool has_bam_dir = !manager->get_filename().empty();
  Filename bam_dir = manager->get_filename().get_dirname();
  Filename filename = get_filename();
  Filename alpha_filename = get_alpha_filename();


  switch (file_texture_mode) {
  case BTM_unchanged:
  case BTM_rawdata:
    break;

  case BTM_fullpath:
    filename = get_fullpath();
    alpha_filename = get_alpha_fullpath();
    break;

  case BTM_relative:
    filename = get_fullpath();
    alpha_filename = get_alpha_fullpath();
    bam_dir.make_absolute();
    if (!has_bam_dir || !filename.make_relative_to(bam_dir, true)) {
      if (filename.find_on_searchpath(get_texture_path()) == -1) {
        filename.find_on_searchpath(get_model_path());
      }
    }
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Texture file " << get_filename()
        << " found as " << filename << "\n";
    }
    if (!has_bam_dir || !alpha_filename.make_relative_to(bam_dir, true)) {
      if (alpha_filename.find_on_searchpath(get_texture_path()) == -1) {
        alpha_filename.find_on_searchpath(get_model_path());
      }
    }
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Alpha image " << get_alpha_filename()
        << " found as " << alpha_filename << "\n";
    }
    break;

  case BTM_basename:
    filename = filename.get_basename();
    alpha_filename = alpha_filename.get_basename();
    break;

  default:
    gobj_cat.error()
      << "Unsupported bam-texture-mode: " << (int)file_texture_mode << "\n";
  }

  if (filename.empty()) {
    // If we don't have a filename, we have to store rawdata anyway.
    has_rawdata = true;
  }

  me.add_string(get_name());
  me.add_string(filename);
  me.add_string(alpha_filename);
  me.add_uint8(_primary_file_num_channels);
  me.add_uint8(_alpha_file_channel);
  me.add_uint8(has_rawdata);
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

  me.add_uint8(_format);
  me.add_uint8(_num_components);

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

