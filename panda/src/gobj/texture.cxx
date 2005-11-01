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
#include "texturePool.h"
#include "textureContext.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "string_utils.h"
#include "preparedGraphicsObjects.h"
#include "pnmImage.h"
#include "virtualFileSystem.h"

#include <stddef.h>


TypeHandle Texture::_type_handle;

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
  _magfilter = FT_linear;
  _minfilter = FT_linear;
  _wrap_u = WM_repeat;
  _wrap_v = WM_repeat;
  _wrap_w = WM_repeat;
  _anisotropic_degree = 1;
  _keep_ram_image = true;
  _all_dirty_flags = 0;
  _border_color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _match_framebuffer_format = false;

  _texture_type = TT_2d_texture;
  _x_size = 0;
  _y_size = 1;
  _z_size = 1;
  set_format(F_rgb);
  set_component_type(T_unsigned_byte);

  _loaded_from_disk = false;
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
  _loaded_from_disk(copy._loaded_from_disk),
  _wrap_u(copy._wrap_u),
  _wrap_v(copy._wrap_v),
  _wrap_w(copy._wrap_w),
  _minfilter(copy._minfilter),
  _magfilter(copy._magfilter),
  _anisotropic_degree(copy._anisotropic_degree),
  _keep_ram_image(copy._keep_ram_image),
  _border_color(copy._border_color),
  _match_framebuffer_format(copy._match_framebuffer_format),
  _all_dirty_flags(0),
  _image(copy._image)
{
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
//               If the Texture is an AviTexture, the resulting
//               duplicate may be animated independently of the
//               original.
////////////////////////////////////////////////////////////////////
PT(Texture) Texture::
make_copy() {
  return new Texture(*this);
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
  _loaded_from_disk = false;
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
//
//               This also implicitly sets keep_ram_image to true.
////////////////////////////////////////////////////////////////////
void Texture::
generate_normalization_cube_map(int size) {
  setup_cube_map(size, T_unsigned_byte, F_rgb);
  PTA_uchar image = modify_ram_image();

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
//     Function: Texture::read
//       Access: Published, Virtual
//  Description: Reads the texture from the indicated filename.  If
//               num_channels is not 0, it specifies the number of
//               components to downgrade the image to if it is greater
//               than this number.
//
//               This also implicitly sets keep_ram_image to false.
////////////////////////////////////////////////////////////////////
bool Texture::
read(const Filename &fullpath, int z, int primary_file_num_channels) {
  PNMImage image;

  if (!image.read(fullpath, NULL, false)) {
    gobj_cat.error()
      << "Texture::read() - couldn't read: " << fullpath << endl;
    return false;
  }

  if (!has_name()) {
    set_name(fullpath.get_basename_wo_extension());
  }
  if (!has_filename()) {
    set_filename(fullpath);
    clear_alpha_filename();
  }

  set_fullpath(fullpath);
  clear_alpha_fullpath();

  // Check to see if we need to scale it.
  consider_rescale(image);
  consider_downgrade(image, primary_file_num_channels);

  _primary_file_num_channels = image.get_num_channels();
  _alpha_file_channel = 0;

  return load(image, z);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read
//       Access: Published, Virtual
//  Description: Combine a 3-component image with a grayscale image
//               to get a 4-component image
//
//               This also implicitly sets keep_ram_image to false.
////////////////////////////////////////////////////////////////////
bool Texture::
read(const Filename &fullpath, const Filename &alpha_fullpath,
     int z, int primary_file_num_channels, int alpha_file_channel) {
  PNMImage image;
  if (!image.read(fullpath, NULL, false)) {
    gobj_cat.error()
      << "Texture::read() - couldn't read: " << fullpath << endl;
    return false;
  }

  PNMImage alpha_image;
  if (!alpha_image.read(alpha_fullpath, NULL, true)) {
    gobj_cat.error()
      << "Texture::read() - couldn't read (alpha): " << alpha_fullpath << endl;
    return false;
  }

  if (!has_name()) {
    set_name(fullpath.get_basename_wo_extension());
  }
  if (!has_filename()) {
    set_filename(fullpath);
    set_alpha_filename(alpha_fullpath);
  }

  set_fullpath(fullpath);
  set_alpha_fullpath(alpha_fullpath);

  consider_rescale(image);

  // The grayscale (alpha channel) image must be the same size as the
  // main image.
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

  consider_downgrade(image, primary_file_num_channels);

  _primary_file_num_channels = image.get_num_channels();

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

  return load(image, z);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write
//       Access: Published
//  Description: Writes the texture to the indicated filename.
////////////////////////////////////////////////////////////////////
bool Texture::
write(const Filename &name, int z) const {
  nassertr(has_ram_image(), false);
  PNMImage pnmimage;
  if (!store(pnmimage, z)) {
    return false;
  }

  if (!pnmimage.write(name)) {
    gobj_cat.error()
      << "Texture::write() - couldn't write: " << name << endl;
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read_pages
//       Access: Published
//  Description: Automatically reads in a sequence of pages, for the
//               purpose of reading in a 3-d texture or a cube map
//               texture.  The filename should contain a sequence of
//               one or more hash marks ("#") which will be filled in
//               with the z value of each page, zero-based.  If z_size
//               is specified, the reading will stop there; otherwise,
//               all found textures will be loaded, until a gap in the
//               sequence is encountered.
//
//               If more than one hash mark is used, the numbers will
//               be padded with zeroes if necessary to the
//               corresponding number of digits.
////////////////////////////////////////////////////////////////////
bool Texture::
read_pages(Filename fullpath_pattern, int z_size) {
  fullpath_pattern.set_pattern(true);
  if (!fullpath_pattern.has_hash()) {
    gobj_cat.error()
      << "Template " << fullpath_pattern << " contains no hash marks.\n";
    return false;
  }

  clear_ram_image();

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

  if (z_size != 0) {
    set_z_size(z_size);
    for (int z = 0; z < z_size; z++) {
      if (!read(fullpath_pattern.get_filename_index(z), z)) {
        return false;
      }
    }
  } else {
    set_z_size(0);
    int z = 0;
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

    Filename file = fullpath_pattern.get_filename_index(z);
    while (vfs->exists(file)) {
      if (!read(file, z)) {
        return false;
      }
      ++z;

      file = fullpath_pattern.get_filename_index(z);
    }
  }

  set_fullpath(fullpath_pattern);
  clear_alpha_fullpath();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write_pages
//       Access: Published
//  Description: Automatically writes out a sequence of pages, for the
//               purpose of writing out a 3-d texture or a cube map
//               texture.  The filename should contain a sequence of
//               one or more hash marks ("#") which will be filled in
//               with the z value of each page, zero-based.
//
//               If more than one hash mark is used, the numbers will
//               be padded with zeroes if necessary to the
//               corresponding number of digits.
////////////////////////////////////////////////////////////////////
bool Texture::
write_pages(Filename fullpath_pattern) {
  fullpath_pattern.set_pattern(true);
  if (!fullpath_pattern.has_hash()) {
    gobj_cat.error()
      << "Template " << fullpath_pattern << " contains no hash marks.\n";
    return false;
  }

  for (int z = 0; z < _z_size; z++) {
    if (!write(fullpath_pattern.get_filename_index(z), z)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::load
//       Access: Published, Virtual
//  Description: Fills the texture system RAM data from the
//               already-read PNMImage.
//
//               For a 3-d texture or a cube map, this must be called
//               multiple times, one for each page (z value).  Cube
//               maps have exactly 6 pages, while 3-d textures can
//               have any number and can dynamically grow as each page
//               is loaded.  For the first page loaded, this also sets
//               the texture parameters; for subsequent pages, the
//               texture parameters must match those which were loaded
//               previously.
//
//               This also implicitly sets keep_ram_image to false if
//               a filename has been set, or true if one has not been
//               set.
////////////////////////////////////////////////////////////////////
bool Texture::
load(const PNMImage &pnmimage, int z) {
  if (!reconsider_z_size(z)) {
    return false;
  }
  nassertr(z >= 0 && z < _z_size, false);

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

  _loaded_from_disk = true;
  modify_ram_image();
  _keep_ram_image = !has_filename();

  // Now copy the pixel data from the PNMImage into our internal
  // _image component.
  bool has_alpha = pnmimage.has_alpha();
  bool is_grayscale = pnmimage.is_grayscale();
    
  if (maxval == 255) {
    // Most common case: one byte per pixel, and the source image
    // shows a maxval of 255.  No scaling is necessary.
    int idx = get_expected_ram_page_size() * z;
    
    for (int j = _y_size-1; j >= 0; j--) {
      for (int i = 0; i < _x_size; i++) {
        if (is_grayscale) {
          store_unscaled_byte(idx, pnmimage.get_gray_val(i, j));
        } else {
          store_unscaled_byte(idx, pnmimage.get_blue_val(i, j));
          store_unscaled_byte(idx, pnmimage.get_green_val(i, j));
          store_unscaled_byte(idx, pnmimage.get_red_val(i, j));
        }
        if (has_alpha) {
          store_unscaled_byte(idx, pnmimage.get_alpha_val(i, j));
        }
      }
    }

    nassertr((size_t)idx == get_expected_ram_page_size() * (z + 1), false);
    
  } else if (maxval == 65535) {
    // Another possible case: two bytes per pixel, and the source
    // image shows a maxval of 65535.  Again, no scaling is necessary.
    int idx = get_expected_ram_page_size() * z;
    
    for (int j = _y_size-1; j >= 0; j--) {
      for (int i = 0; i < _x_size; i++) {
        if (is_grayscale) {
          store_unscaled_short(idx, pnmimage.get_gray_val(i, j));
        } else {
          store_unscaled_short(idx, pnmimage.get_blue_val(i, j));
          store_unscaled_short(idx, pnmimage.get_green_val(i, j));
          store_unscaled_short(idx, pnmimage.get_red_val(i, j));
        }
        if (has_alpha) {
          store_unscaled_short(idx, pnmimage.get_alpha_val(i, j));
        }
      }
    }

    nassertr((size_t)idx == get_expected_ram_page_size() * (z + 1), false);
    
  } else if (maxval <= 255) {
    // A less common case: one byte per pixel, but the maxval is
    // something other than 255.  In this case, we should scale the
    // pixel values up to the appropriate amount.
    int idx = get_expected_ram_page_size() * z;
    double scale = 255.0 / (double)maxval;
    
    for (int j = _y_size-1; j >= 0; j--) {
      for (int i = 0; i < _x_size; i++) {
        if (is_grayscale) {
          store_scaled_byte(idx, pnmimage.get_gray_val(i, j), scale);
        } else {
          store_scaled_byte(idx, pnmimage.get_blue_val(i, j), scale);
          store_scaled_byte(idx, pnmimage.get_green_val(i, j), scale);
          store_scaled_byte(idx, pnmimage.get_red_val(i, j), scale);
        }
        if (has_alpha) {
          store_scaled_byte(idx, pnmimage.get_alpha_val(i, j), scale);
        }
      }
    }

    nassertr((size_t)idx == get_expected_ram_page_size() * (z + 1), false);
    
  } else {
    // Another uncommon case: two bytes per pixel, and the maxval is
    // something other than 65535.  Again, we must scale the pixel
    // values.
    int idx = get_expected_ram_page_size() * z;
    double scale = 65535.0 / (double)maxval;
    
    for (int j = _y_size-1; j >= 0; j--) {
      for (int i = 0; i < _x_size; i++) {
        if (is_grayscale) {
          store_scaled_short(idx, pnmimage.get_gray_val(i, j), scale);
        } else {
          store_scaled_short(idx, pnmimage.get_blue_val(i, j), scale);
          store_scaled_short(idx, pnmimage.get_green_val(i, j), scale);
          store_scaled_short(idx, pnmimage.get_red_val(i, j), scale);
        }
        if (has_alpha) {
          store_scaled_short(idx, pnmimage.get_alpha_val(i, j), scale);
        }
      }
    }

    nassertr((size_t)idx == get_expected_ram_page_size() * (z + 1), false);
  }

  mark_dirty(DF_image);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::store
//       Access: Published
//  Description: Saves the zth level of the texture to the indicated
//               PNMImage, but does not write it to disk.
////////////////////////////////////////////////////////////////////
bool Texture::
store(PNMImage &pnmimage, int z) const {
  nassertr(has_ram_image(), false);
  nassertr(z >= 0 && z < _z_size, false);

  if (_component_type == T_unsigned_byte) {
    pnmimage.clear(_x_size, _y_size, _num_components);
    bool has_alpha = pnmimage.has_alpha();
    bool is_grayscale = pnmimage.is_grayscale();

    int idx = get_expected_ram_page_size() * z;

    for (int j = _y_size-1; j >= 0; j--) {
      for (int i = 0; i < _x_size; i++) {
        if (is_grayscale) {
          pnmimage.set_gray(i, j, get_unsigned_byte(idx));
        } else {
          pnmimage.set_blue(i, j, get_unsigned_byte(idx));
          pnmimage.set_green(i, j, get_unsigned_byte(idx));
          pnmimage.set_red(i, j, get_unsigned_byte(idx));
        }
        if (has_alpha) {
          pnmimage.set_alpha(i, j, get_unsigned_byte(idx));
        }
      }
    }

    nassertr((size_t)idx == get_expected_ram_page_size() * (z + 1), false);

    return true;

  } else if (_component_type == T_unsigned_short) {
    pnmimage.clear(_x_size, _y_size, _num_components, 65535);
    bool has_alpha = pnmimage.has_alpha();
    bool is_grayscale = pnmimage.is_grayscale();

    int idx = get_expected_ram_page_size() * z;

    for (int j = _y_size-1; j >= 0; j--) {
      for (int i = 0; i < _x_size; i++) {
        if (is_grayscale) {
          pnmimage.set_gray(i, j, get_unsigned_short(idx));
        } else {
          pnmimage.set_blue(i, j, get_unsigned_short(idx));
          pnmimage.set_green(i, j, get_unsigned_short(idx));
          pnmimage.set_red(i, j, get_unsigned_short(idx));
        }
        if (has_alpha) {
          pnmimage.set_alpha(i, j, get_unsigned_short(idx));
        }
      }
    }

    nassertr((size_t)idx == get_expected_ram_page_size() * (z + 1), false);

    return true;
  }

  gobj_cat.error()
    << "Couldn't write image for " << get_name()
    << "; inappropriate data type " << (int)_component_type << ".\n";
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
load_related(const PT(InternalName) &suffix) const {
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
    res = TexturePool::load_texture(main, alph, 
                                    _primary_file_num_channels,
                                    _alpha_file_channel);
  } else {
    res = TexturePool::load_texture(main,
                                    _primary_file_num_channels);
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
    mark_dirty(DF_wrap);
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
    mark_dirty(DF_wrap); 
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
    mark_dirty(DF_wrap); 
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
    if (is_mipmap(_minfilter) != is_mipmap(filter)) {
      mark_dirty(DF_filter | DF_mipmap);
    } else {
      mark_dirty(DF_filter);
    }
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
    mark_dirty(DF_filter);
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
    mark_dirty(DF_filter);
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
    mark_dirty(DF_border);
    _border_color = color;
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
  return !_image.empty();
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
  if (_loaded_from_disk && !has_ram_image() && has_filename()) {
    reload_ram_image();
  }

  return _image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::modify_ram_image
//       Access: Published
//  Description: Returns a modifiable pointer to the system-RAM image.
//               If the RAM image has been dumped, creates a new one.
//
//               This also implicitly sets keep_ram_image to true.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
modify_ram_image() {
  if (_image.empty()) {
    make_ram_image();
  }

  mark_dirty(DF_image);
  _keep_ram_image = true;
  return _image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::make_ram_image
//       Access: Published
//  Description: Discards the current system-RAM image for the
//               texture, if any, and allocates a new buffer of the
//               appropriate size.  Returns the new buffer.
//
//               This also implicitly sets keep_ram_image to true.
////////////////////////////////////////////////////////////////////
PTA_uchar Texture::
make_ram_image() {
  _image = PTA_uchar::empty_array(get_expected_ram_image_size());
  mark_dirty(DF_image);
  _keep_ram_image = true;
  return _image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_ram_image
//       Access: Published
//  Description: Replaces the current system-RAM image with the new
//               data.
//
//               This also implicitly sets keep_ram_image to true.
////////////////////////////////////////////////////////////////////
void Texture::
set_ram_image(PTA_uchar image) {
  nassertv(image.size() == get_expected_ram_image_size());
  if (_image != image) {
    _image = image;
    mark_dirty(DF_image);
  }
  _keep_ram_image = true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_ram_image
//       Access: Published
//  Description: Discards the current system-RAM image.
////////////////////////////////////////////////////////////////////
void Texture::
clear_ram_image() {
  _image.clear();
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
  case F_stencil_index:
  case F_depth_component:
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
//       Access: Public, Static
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
//       Access: Public
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

  if (tc != (TextureContext *)NULL) {
    // Now that we have a new TextureContext with zero dirty flags, our
    // intersection of all dirty flags must be zero.  This doesn't mean
    // that some other contexts aren't still dirty, but at least one
    // context isn't.
    _all_dirty_flags = 0;

    if (!keep_texture_ram && !_keep_ram_image) {
      // Once we have prepared the texture, we can generally safely
      // remove the pixels from main RAM.  The GSG is now responsible
      // for remembering what it looks like.

      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Dumping RAM for texture " << get_name() << "\n";
      }
      _image.clear();
    }
  }
  return tc;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::mark_dirty
//       Access: Public
//  Description: Sets the indicated dirty bits on for all texture
//               contexts that share this Texture.  Does not change
//               the bits that are not on.  This presumably will
//               inform the GSG that the texture properties have
//               changed.  See also TextureContext::mark_dirty().
//
//               Normally, this does not need to be called directly;
//               changing the properties on the texture will
//               automatically call this.  However, if you fiddle with
//               the texture image directly, you may need to
//               explicitly call mark_dirty(Texture::DF_image).
////////////////////////////////////////////////////////////////////
void Texture::
mark_dirty(int flags_to_set) {
  if ((_all_dirty_flags & flags_to_set) == flags_to_set) {
    // If all the texture contexts already share these bits, no need
    // to do anything else.
    return;
  }

  // Otherwise, iterate through the contexts and mark them all dirty.
  Contexts::iterator ci;
  for (ci = _contexts.begin(); ci != _contexts.end(); ++ci) {
    TextureContext *tc = (*ci).second;
    if (tc != (TextureContext *)NULL) {
      tc->mark_dirty(flags_to_set);
    }
  }

  _all_dirty_flags |= flags_to_set;
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
  if (_texture_type == TT_1d_texture || _texture_type == TT_2d_texture) {
    gobj_cat.info()
      << "Reloading texture " << get_name() << "\n";
    
    make_ram_image();
    if (has_alpha_fullpath()) {
      read(get_fullpath(), get_alpha_fullpath(),
           0, _primary_file_num_channels, _alpha_file_channel);
    } else {
      read(get_fullpath(), 0, _primary_file_num_channels);
    }
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
    if (!_image.is_null() && new_size > _image.size()) {
      _image.insert(_image.end(), new_size - _image.size(), 0);
      nassertr(_image.size() == new_size, false);
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
  if (!_loaded_from_disk || num_components != _num_components) {
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

  if (!_loaded_from_disk) {
#ifndef NDEBUG
    if (_texture_type == TT_1d_texture) {
      nassertr(y_size == 1, false);
    } else if (_texture_type == TT_cube_map) {
      nassertr(x_size == y_size, false);
    }
#endif
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
        << " level " << z << ".\n";
      return false;
    }
  }

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
//     Function: Texture::consider_rescale
//       Access: Private
//  Description: Scales the PNMImage according to the whims of the
//               Config.prc file.
////////////////////////////////////////////////////////////////////
void Texture::
consider_rescale(PNMImage &pnmimage) {
  int new_x_size = pnmimage.get_x_size();
  int new_y_size = pnmimage.get_y_size();

  switch (textures_power_2) {
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

  switch (textures_square) {
  case ATS_down:
    new_x_size = new_y_size = min(new_x_size, new_y_size);
    break;

  case ATS_up:
    new_x_size = new_y_size = max(new_x_size, new_y_size);
    break;

  case ATS_none:
    break;
  }

  if (max_texture_dimension > 0) {
    new_x_size = min(new_x_size, (int)max_texture_dimension);
    new_y_size = min(new_y_size, (int)max_texture_dimension);
  }

  if (pnmimage.get_x_size() != new_x_size ||
      pnmimage.get_y_size() != new_y_size) {
    gobj_cat.info()
      << "Automatically rescaling " << get_name() << " from "
      << pnmimage.get_x_size() << " by " << pnmimage.get_y_size() << " to "
      << new_x_size << " by " << new_y_size << "\n";

    PNMImage scaled(new_x_size, new_y_size, pnmimage.get_num_channels(),
                    pnmimage.get_maxval(), pnmimage.get_type());
    scaled.quick_filter_from(pnmimage);
    pnmimage = scaled;
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
    me = new Texture("");

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
  _wrap_u = (WrapMode)scan.get_uint8();
  _wrap_v = (WrapMode)scan.get_uint8();
  _wrap_w = (WrapMode)scan.get_uint8();
  _minfilter = (FilterType)scan.get_uint8();
  _magfilter = (FilterType)scan.get_uint8();
  _anisotropic_degree = scan.get_int16();

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
    _loaded_from_disk = false;
    
    PN_uint32 u_size = scan.get_uint32();
    
    // fill the _image buffer with image data
    string temp_buff = scan.extract_bytes(u_size);
    _image = PTA_uchar::empty_array((int) u_size);
    for (PN_uint32 u_idx=0; u_idx < u_size; ++u_idx) {
      _image[(int)u_idx] = (uchar) temp_buff[u_idx];
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
  bool has_bam_dir = !manager->get_filename().empty();
  Filename bam_dir = manager->get_filename().get_dirname();
  Filename filename = get_filename();
  Filename alpha_filename = get_alpha_filename();

  // Write out the texture's raw pixel data if (a) the current Bam
  // Texture Mode requires that, or (b) there's no filename, so the
  // file can't be loaded up from disk, but the raw pixel data is
  // currently available in RAM.

  // Otherwise, we just write out the filename, and assume whoever
  // loads the bam file later will have access to the image file on
  // disk.
  bool has_rawdata = 
    (bam_texture_mode == BTM_rawdata || (has_ram_image() && filename.empty()));

  switch (bam_texture_mode) {
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
    if (!has_bam_dir || filename.find_on_searchpath(bam_dir) == -1) {
      if (filename.find_on_searchpath(get_texture_path()) == -1) {
	filename.find_on_searchpath(get_model_path());
      }
    }
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Texture file " << get_filename()
	<< " found as " << filename << "\n";
    }
    if (!has_bam_dir || alpha_filename.find_on_searchpath(bam_dir) == -1) {
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
      << "Unsupported bam-texture-mode: " << (int)bam_texture_mode << "\n";
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

    me.add_uint32(_image.size());
    me.append_data(_image, _image.size());
  }
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
