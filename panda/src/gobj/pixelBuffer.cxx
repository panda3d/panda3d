// Filename: pixelBuffer.cxx
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

#include "pixelBuffer.h"
#include "config_gobj.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle PixelBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PixelBuffer::
PixelBuffer() {
  _xsize = 0;
  _ysize = 0;
  _format = F_rgb;
  _type = T_unsigned_byte;
  _num_components = 3;
  _component_width = 1;
  _image = PTA_uchar();

  _loaded = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PixelBuffer::
PixelBuffer(int xsize, int ysize, int components, int component_width, 
            Type type, Format format, bool allocate_ram) {
  _xsize = xsize;
  _ysize = ysize;
  _num_components = components;
  _component_width = component_width;
  _type = type;
  _format = format;

  if (allocate_ram) {
    int num_pixels = _xsize * _ysize * _num_components * _component_width;
    _image = PTA_uchar::empty_array((size_t)num_pixels);
  }
  _loaded = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PixelBuffer::
PixelBuffer(const PixelBuffer &copy) :
  _xsize(copy._xsize),
  _ysize(copy._ysize),
  _num_components(copy._num_components),
  _component_width(copy._component_width),
  _format(copy._format),
  _type(copy._type),
  _loaded(copy._loaded),
  _image(copy._image)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PixelBuffer::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PixelBuffer::
operator = (const PixelBuffer &copy) {
  _xsize = copy._xsize;
  _ysize = copy._ysize;
  _num_components = copy._num_components;
  _component_width = copy._component_width;
  _format = copy._format;
  _type = copy._type;
  _loaded = copy._loaded;
  _image = copy._image;
}

////////////////////////////////////////////////////////////////////
//     Function: config
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void PixelBuffer::config(void)
{
  ImageBuffer::config();
}

////////////////////////////////////////////////////////////////////
//     Function: read
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool PixelBuffer::
read(const Filename &name) {
  PNMImage pnmimage;

  if (!pnmimage.read(name)) {
    gobj_cat.error()
      << "PixelBuffer::read() - couldn't read: " << name << endl;
    return false;
  }

  set_name(name.get_basename_wo_extension());
  set_filename(name);
  clear_alpha_filename();
  return load(pnmimage);
}

////////////////////////////////////////////////////////////////////
//     Function: write
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool PixelBuffer::
write(const Filename &name) const {
  PNMImage pnmimage;
  if (!store(pnmimage)) {
    return false;
  }

  Filename tname;
  if (name.empty()) {
    tname = get_filename();
  } else {
    tname = name;
  }

  if (!pnmimage.write(tname)) {
    gobj_cat.error()
      << "PixelBuffer::write() - couldn't write: " << name << endl;
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: load
//       Access: Public
//  Description: Extracts the image data from the given PNMImage and
//               stores it in the _image member, as an unadorned array
//               of pixel values.  Note that we now store pixel
//               components in the order B, G, R, A.
////////////////////////////////////////////////////////////////////
bool PixelBuffer::load(const PNMImage& pnmimage)
{
  int num_components = pnmimage.get_num_channels();

  if (!_loaded || num_components != _num_components) {
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

  _xsize = pnmimage.get_x_size();
  _ysize = pnmimage.get_y_size();
  _num_components = num_components;
  _component_width = 1;
  _loaded = true;

  // Now copy the pixel data from the PNMImage into our internal
  // _image component.
  bool has_alpha = pnmimage.has_alpha();
  bool is_grayscale = pnmimage.is_grayscale();
  xelval maxval = pnmimage.get_maxval();
    
  if (maxval == 255) {
    // Most common case: one byte per pixel, and the source image
    // shows a maxval of 255.  No scaling is necessary.
    _type = T_unsigned_byte;
    _image = PTA_uchar::empty_array((int)(_xsize * _ysize * _num_components));
    int idx = 0;
    
    for (int j = _ysize-1; j >= 0; j--) {
      for (int i = 0; i < _xsize; i++) {
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
    
  } else if (maxval == 65535) {
    // Another possible case: two bytes per pixel, and the source
    // image shows a maxval of 65535.  Again, no scaling is necessary.
    _type = T_unsigned_short;
    _image = PTA_uchar::empty_array(_xsize * _ysize * _num_components * 2);
    _component_width = 2;
    int idx = 0;
    
    for (int j = _ysize-1; j >= 0; j--) {
      for (int i = 0; i < _xsize; i++) {
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
    
  } else if (maxval <= 255) {
    // A less common case: one byte per pixel, but the maxval is
    // something other than 255.  In this case, we should scale the
    // pixel values up to the appropriate amount.
    _type = T_unsigned_byte;
    _image = PTA_uchar::empty_array(_xsize * _ysize * _num_components);
    int idx = 0;
    double scale = 255.0 / (double)maxval;
    
    for (int j = _ysize-1; j >= 0; j--) {
      for (int i = 0; i < _xsize; i++) {
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
    
  } else {
    // Another uncommon case: two bytes per pixel, and the maxval is
    // something other than 65535.  Again, we must scale the pixel
    // values.
    _type = T_unsigned_short;
    _image = PTA_uchar::empty_array(_xsize * _ysize * _num_components * 2);
    _component_width = 2;
    int idx = 0;
    double scale = 65535.0 / (double)maxval;
    
    for (int j = _ysize-1; j >= 0; j--) {
      for (int i = 0; i < _xsize; i++) {
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
  }

  config();
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: store
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool PixelBuffer::
store(PNMImage &pnmimage) const {
  if (_type == T_unsigned_byte) {
    pnmimage.clear(_xsize, _ysize, _num_components);
    bool has_alpha = pnmimage.has_alpha();
    bool is_grayscale = pnmimage.is_grayscale();

    int idx = 0;
    for (int j = _ysize-1; j >= 0; j--) {
      for (int i = 0; i < _xsize; i++) {
        if (is_grayscale) {
          pnmimage.set_gray(i, j, get_unsigned_byte(idx));
        } else {
          pnmimage.set_blue(i, j, get_unsigned_byte(idx));
          pnmimage.set_green(i, j, get_unsigned_byte(idx));
          pnmimage.set_red(i, j, get_unsigned_byte(idx));
        }
        if (has_alpha)
          pnmimage.set_alpha(i, j, get_unsigned_byte(idx));
      }
    }
    return true;

  } else if (_type == T_unsigned_short) {
    pnmimage.clear(_xsize, _ysize, _num_components, 65535);
    bool has_alpha = pnmimage.has_alpha();
    bool is_grayscale = pnmimage.is_grayscale();

    int idx = 0;
    for (int j = _ysize-1; j >= 0; j--) {
      for (int i = 0; i < _xsize; i++) {
        if (is_grayscale) {
          pnmimage.set_gray(i, j, get_unsigned_short(idx));
        } else {
          pnmimage.set_blue(i, j, get_unsigned_short(idx));
          pnmimage.set_green(i, j, get_unsigned_short(idx));
          pnmimage.set_red(i, j, get_unsigned_short(idx));
        }
        if (has_alpha)
          pnmimage.set_alpha(i, j, get_unsigned_short(idx));
      }
    }
    return true;
  }

  gobj_cat.error()
    << "Couldn't write image for " << get_name()
    << "; inappropriate type " << (int)_type << ".\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: copy
//       Access:
//  Description: Deep copy of pixel buffer
////////////////////////////////////////////////////////////////////
void PixelBuffer::
copy(const PixelBuffer *pb) {
  nassertv(pb != NULL);
  _xsize = pb->_xsize;
  _ysize = pb->_ysize;
  _num_components = pb->_num_components;
  _component_width = pb->_component_width;
  _format = pb->_format;

  if (!pb->_image.is_null()) {
    _image = PTA_uchar::empty_array(0);
    _image.v() = pb->_image.v();
  } else {
    _image = PTA_uchar();
  }

}
