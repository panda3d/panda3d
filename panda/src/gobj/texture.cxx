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
//     Function: Texture::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Texture::
~Texture() {
  release_all();
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
  _texture_type = texture_type;
  _x_size = x_size;
  _y_size = y_size;
  _z_size = z_size;
  if (_texture_type == TT_cube_map) {
    _z_size = 6;
  }
  set_component_type(component_type);
  set_format(format);

  clear_ram_image();
  _loaded_from_disk = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read
//       Access: Published
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

  if (!image.read(fullpath)) {
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
//       Access: Published
//  Description: Combine a 3-component image with a grayscale image
//               to get a 4-component image
//
//               This also implicitly sets keep_ram_image to false.
////////////////////////////////////////////////////////////////////
bool Texture::
read(const Filename &fullpath, const Filename &alpha_fullpath,
     int z, int primary_file_num_channels, int alpha_file_channel) {
  PNMImage image;
  if (!image.read(fullpath)) {
    gobj_cat.error()
      << "Texture::read() - couldn't read: " << fullpath << endl;
    return false;
  }

  PNMImage alpha_image;
  if (!alpha_image.read(alpha_fullpath)) {
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
read_pages(const Filename &fullpath_template, int z_size) {
  string fp = fullpath_template.get_fullpath();
  size_t hash = fp.rfind('#');
  if (hash == string::npos) {
    gobj_cat.error()
      << "Template " << fullpath_template << " contains no hash marks.\n";
    return false;
  }

  // Count the number of hash marks.
  size_t num_hash = 1;
  while (hash >= num_hash && fp[hash - num_hash] == '#') {
    num_hash++;
  }

  string prefix = fp.substr(0, hash - num_hash + 1);
  string suffix = fp.substr(hash + 1);

  clear_ram_image();

  if (z_size != 0) {
    set_z_size(z_size);
    for (int z = 0; z < z_size; z++) {
      ostringstream strm;
      strm << prefix << setw(num_hash) << setfill('0') << z << suffix;
      if (!read(strm.str(), z)) {
        return false;
      }
    }
  } else {
    set_z_size(0);
    int z = 0;
    ostringstream strm;
    strm << prefix << setw(num_hash) << setfill('0') << z << suffix;
    Filename file(strm.str());
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

    while (vfs->exists(file)) {
      if (!read(file, z)) {
        return false;
      }
      ++z;

      ostringstream strm;
      strm << prefix << setw(num_hash) << setfill('0') << z << suffix;
      file = Filename(strm.str());
    }
  }

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
write_pages(const Filename &fullpath_template) {
  string fp = fullpath_template.get_fullpath();
  size_t hash = fp.rfind('#');
  if (hash == string::npos) {
    gobj_cat.error()
      << "Template " << fullpath_template << " contains no hash marks.\n";
    return false;
  }

  // Count the number of hash marks.
  size_t num_hash = 1;
  while (hash >= num_hash && fp[hash - num_hash] == '#') {
    num_hash++;
  }

  string prefix = fp.substr(0, hash - num_hash + 1);
  string suffix = fp.substr(hash + 1);

  for (int z = 0; z < _z_size; z++) {
    ostringstream strm;
    strm << prefix << setw(num_hash) << setfill('0') << z << suffix;
    if (!write(strm.str(), z)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::load
//       Access: Published
//  Description: Fills the texture system RAM data from the
//               already-read PNMImage.
//
//               For a 3-d texture or a cube map, this must be called
//               multiple times, one for each page (z value).  Cube
//               maps have exactly 6 pages, while 3-d textures can
//               have any number and can dynamically grow as each page
//               is loaded.  For the first page loaded (or when
//               reloading z == 0), this also sets the texture
//               parameters; for subsequent pages, the texture
//               parameters must match those which were loaded
//               previously.
//
//               This also implicitly sets keep_ram_image to false if
//               a filename has been set, or true if one has not been
//               set.
////////////////////////////////////////////////////////////////////
bool Texture::
load(const PNMImage &pnmimage, int z) {
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

  nassertr(z >= 0 && z < _z_size, false);

  int num_components = pnmimage.get_num_channels();
  ComponentType component_type = T_unsigned_byte;

  xelval maxval = pnmimage.get_maxval();
  if (maxval > 255) {
    component_type = T_unsigned_short;
  }

  if (!_loaded_from_disk || num_components != _num_components) {
    // Come up with a default format based on the number of channels.
    // But only do this the first time the file is loaded, or if the
    // number of channels in the image changes on subsequent loads.

    switch (pnmimage.get_color_type()) {
    case PNMImage::CT_grayscale:
      _format = F_luminance;
      break;
      
    case PNMImage::CT_two_channel:
      _format = F_luminance_alpha;
      break;
      
    case PNMImage::CT_color:
      _format = F_rgb;
      break;
      
    case PNMImage::CT_four_channel:
      _format = F_rgba;
      break;
      
    default:
      // Eh?
      nassertr(false, false);
      _format = F_rgb;
    };
  }

  if (!_loaded_from_disk || z == 0) {
    if (_texture_type == TT_1d_texture) {
      nassertr(pnmimage.get_y_size() == 1, false);
    }
    _x_size = pnmimage.get_x_size();
    _y_size = pnmimage.get_y_size();
    _num_components = num_components;
    set_component_type(component_type);

  } else {
    if (_x_size != pnmimage.get_x_size() ||
        _y_size != pnmimage.get_y_size() ||
        _component_type != component_type) {
      gobj_cat.error()
        << "Texture properties have changed for texture " << get_name()
        << " level " << z << ".\n";
      return false;
    }
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
  if (_loaded_from_disk && !has_ram_image() && has_filename() &&
      (_texture_type == TT_1d_texture || _texture_type == TT_2d_texture)) {
    // Now we have to reload the texture image.
    gobj_cat.info()
      << "Reloading texture " << get_name() << "\n";

    make_ram_image();
    if (has_alpha_fullpath()) {
      read(get_fullpath(), get_alpha_fullpath());
    } else {
      read(get_fullpath());
    }
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
  if (!has_ram_image()) {
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
    prepared_objects->release_texture(tc);
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
    prepared_objects->release_texture(tc);
  }

  // Now that we've called release_texture() on every known context,
  // the _contexts list should have completely emptied itself.
  nassertr(_contexts.empty(), num_freed);

  return num_freed;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_format
//       Access: Public
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
//       Access: Public
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
  if (tc != (TextureContext *)NULL) {
    _contexts[prepared_objects] = tc;

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
    (*ci).second->mark_dirty(flags_to_set);
  }

  _all_dirty_flags |= flags_to_set;
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

  } else {
    return FT_invalid;
  }
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
//     Function: Texture::up_to_power_2
//       Access: Private, Static
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
//       Access: Private, Static
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
  BamReader::get_factory()->register_factory(get_class_type(), make_Texture);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::make_Texture
//       Access: Protected
//  Description: Factory method to generate a Texture object
////////////////////////////////////////////////////////////////////
TypedWritable *Texture::
make_Texture(const FactoryParams &params) {
  //The process of making a texture is slightly
  //different than making other Writable objects.
  //That is because all creation of Textures should
  //be done through calls to TexturePool, which ensures
  //that any loads of the same Texture, refer to the
  //same memory
  DatagramIterator scan;
  BamReader *manager;
  bool has_rawdata = false;

  parse_params(params, scan, manager);

  // Get the filenames so we can look up the file on disk first.
  string name = scan.get_string();
  Filename filename = scan.get_string();
  Filename alpha_filename = scan.get_string();

  int primary_file_num_channels = 0;  
  int alpha_file_channel = 0;  

  if (manager->get_file_minor_ver() == 2) {
    // We temporarily had a version that stored the number of channels
    // here.
    primary_file_num_channels = scan.get_uint8();

  } else if (manager->get_file_minor_ver() >= 3) {
    primary_file_num_channels = scan.get_uint8();
    alpha_file_channel = scan.get_uint8();
  }

  // from minor version 5, read the rawdata mode, else carry on
  if (manager->get_file_minor_ver() >= 5) {
    has_rawdata = scan.get_bool();
  }

  Texture *me = NULL;
  if (has_rawdata) {
    // then create a Texture and don't load from the file
    me = new Texture;

  } else {
    if (filename.empty()) {
      // This texture has no filename; since we don't have an image to
      // load, we can't actually create the texture.
      gobj_cat.info()
        << "Cannot create texture '" << name << "' with no filename.\n";

    } else {
      // This texture does have a filename, so try to load it from disk.
      if (alpha_filename.empty()) {
        me = TexturePool::load_texture(filename, primary_file_num_channels);
      } else {
        me = TexturePool::load_texture(filename, alpha_filename, 
                                       primary_file_num_channels, alpha_file_channel);
      }
    }
  }

  if (me == (Texture *)NULL) {
    // Oops, we couldn't load the texture; we'll just return NULL.
    // But we do need a dummy texture to read in and ignore all of the
    // attributes.
    PT(Texture) dummy = new Texture;
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

  if (manager->get_file_minor_ver() < 17) {
    _texture_type = TT_2d_texture;
  } else {
    _texture_type = (TextureType)scan.get_uint8();
  }

  _wrap_u = (WrapMode)scan.get_uint8();
  _wrap_v = (WrapMode)scan.get_uint8();
  if (manager->get_file_minor_ver() < 17) {
    _wrap_w = WM_repeat;
  } else {
    _wrap_w = (WrapMode)scan.get_uint8();
  }
  _minfilter = (FilterType)scan.get_uint8();
  _magfilter = (FilterType)scan.get_uint8();
  _anisotropic_degree = scan.get_int16();

  bool has_pbuffer = true;
  if (manager->get_file_minor_ver() < 17) {
    has_pbuffer = scan.get_bool();
  }
  if (has_pbuffer) {
    Format format = (Format)scan.get_uint8();
    int num_channels = -1;
    num_channels = scan.get_uint8();

    if (num_channels == get_num_components()) {
      // Only reset the format if the number of components hasn't
      // changed, since if the number of components has changed our
      // texture no longer matches what it was when the bam was
      // written.
      set_format(format);
    }
    
    if (has_rawdata) {
      // In the rawdata case, we must always set the format.
      _format = format;
      _x_size = scan.get_uint32();
      _y_size = scan.get_uint32();
      if (manager->get_file_minor_ver() < 17) {
        _z_size = 1;
      } else {
        _z_size = scan.get_uint32();
      }
      _component_type = (ComponentType)scan.get_uint8();
      _num_components = scan.get_uint8();
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
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void Texture::
write_datagram(BamWriter *manager, Datagram &me) {
  bool has_rawdata = (bam_texture_mode == BTM_rawdata);

  Filename filename = get_filename();
  Filename alpha_filename = get_alpha_filename();

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
    filename.find_on_searchpath(get_texture_path()) ||
      filename.find_on_searchpath(get_model_path());
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Texture file " << get_filename() << " found as " << filename << "\n";
    }
    alpha_filename.find_on_searchpath(get_texture_path()) ||
      alpha_filename.find_on_searchpath(get_model_path());
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Alpha image " << get_alpha_filename() << " found as " << alpha_filename << "\n";
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

  // The data beginning at this point is handled by fillin().
  me.add_uint8(_texture_type);
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
    me.add_uint8(_num_components);
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
