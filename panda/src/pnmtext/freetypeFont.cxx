// Filename: freetypeFont.cxx
// Created by:  drose (07Sep03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "freetypeFont.h"

#ifdef HAVE_FREETYPE

#include "config_pnmtext.h"
#include "config_util.h"
#include "config_express.h"
#include "virtualFileSystem.h"

FT_Library FreetypeFont::_ft_library;
bool FreetypeFont::_ft_initialized = false;
bool FreetypeFont::_ft_ok = false;

// This constant determines how big a particular point size font
// appears to be.  By convention, 10 points is 1 unit (e.g. 1 foot)
// high.
const float FreetypeFont::_points_per_unit = 10.0f;

// A universal typographic convention.
const float FreetypeFont::_points_per_inch = 72.0f;

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
FreetypeFont::
FreetypeFont() {
  _font_loaded = false;

  _point_size = text_point_size;
  _tex_pixels_per_unit = text_pixels_per_unit;
  _scale_factor = text_scale_factor;

  _line_height = 1.0f;
  _space_advance = 0.25f;

  if (!_ft_initialized) {
    initialize_ft_library();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::load_font
//       Access: Protected
//  Description: This method accepts the name of some font file
//               that FreeType can read, along with face_index,
//               indicating which font within the file to load
//               (usually 0).
////////////////////////////////////////////////////////////////////
bool FreetypeFont::
load_font(const Filename &font_filename, int face_index) {
  if (!_ft_ok) {
    pnmtext_cat.error()
      << "Unable to read font " << font_filename
      << ": FreeType library not initialized properly.\n";
    return false;
  }

  unload_font();

  bool exists = false;
  int error;
  Filename path(font_filename);
  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->resolve_filename(path, get_model_path());
    exists = vfs->read_file(path, _raw_font_data);
    if (exists) {
      error = FT_New_Memory_Face(_ft_library, 
                                 (const FT_Byte *)_raw_font_data.data(),
                                 _raw_font_data.length(),
                                 face_index, &_face);
    }
  } else {
    path.resolve_filename(get_model_path());
    exists = path.exists();
    if (exists) {
      string os_specific = path.to_os_specific();
      error = FT_New_Face(_ft_library, os_specific.c_str(),
                          face_index, &_face);
    }
  }

  if (!exists) {
    pnmtext_cat.error()
      << "Unable to find font file " << font_filename << "\n";
  } else {
    if (error == FT_Err_Unknown_File_Format) {
      pnmtext_cat.error()
        << "Unable to read font " << font_filename << ": unknown file format.\n";
    } else if (error) {
      pnmtext_cat.error()
        << "Unable to read font " << font_filename << ": invalid.\n";

    } else {
      return font_loaded();
    }
  }
  
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::load_font
//       Access: Protected
//  Description: This method accepts a table of data representing
//               the font file, loaded from some source other than a
//               filename on disk.
////////////////////////////////////////////////////////////////////
bool FreetypeFont::
load_font(const char *font_data, int data_length, int face_index) {
  if (!_ft_ok) {
    pnmtext_cat.error()
      << "Unable to read font: FreeType library not initialized properly.\n";
    return false;
  }

  unload_font();

  int error;
  error = FT_New_Memory_Face(_ft_library, 
                             (const FT_Byte *)font_data, data_length,
                             face_index, &_face);

  if (error == FT_Err_Unknown_File_Format) {
    pnmtext_cat.error()
      << "Unable to read font: unknown file format.\n";
  } else if (error) {
    pnmtext_cat.error()
      << "Unable to read font: invalid.\n";
    
  } else {
    return font_loaded();
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::unload_font
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void FreetypeFont::
unload_font() {
  if (_font_loaded) {
    FT_Done_Face(_face);
    _font_loaded = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::copy_bitmap_to_pnmimage
//       Access: Protected
//  Description: Copies a bitmap as rendered by FreeType into a
//               PNMImage, so it can be rescaled.
////////////////////////////////////////////////////////////////////
void FreetypeFont::
copy_bitmap_to_pnmimage(const FT_Bitmap &bitmap, PNMImage &image) {
  if (bitmap.pixel_mode == ft_pixel_mode_grays && 
      bitmap.num_grays == (int)image.get_maxval() + 1) {
    // This is the easy case: we can copy the rendered glyph
    // directly into our image, one pixel at a time.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < bitmap.rows; yi++) {
      for (int xi = 0; xi < bitmap.width; xi++) {
        image.set_gray_val(xi, yi, buffer_row[xi]);
      }
      buffer_row += bitmap.pitch;
    }
    
  } else if (bitmap.pixel_mode == ft_pixel_mode_mono) {
    // This is a little bit more work: we have to expand the
    // one-bit-per-pixel bitmap into a one-byte-per-pixel image.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < bitmap.rows; yi++) {
      xelval maxval = image.get_maxval();
      int bit = 0x80;
      unsigned char *b = buffer_row;
      for (int xi = 0; xi < bitmap.width; xi++) {
        if (*b & bit) {
          image.set_gray_val(xi, yi, maxval);
        } else {
          image.set_gray_val(xi, yi, 0);
        }
        bit >>= 1;
        if (bit == 0) {
          ++b;
          bit = 0x80;
        }
      }
      
      buffer_row += bitmap.pitch;
    }
    
    
  } else if (bitmap.pixel_mode == ft_pixel_mode_grays) {
    // Here we must expand a grayscale pixmap with n levels of gray
    // into our 256-level texture.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < bitmap.rows; yi++) {
      for (int xi = 0; xi < bitmap.width; xi++) {
        image.set_gray(xi, yi, (float)buffer_row[xi] / (bitmap.num_grays - 1));
      }
      buffer_row += bitmap.pitch;
    }
    
  } else {
    pnmtext_cat.error()
      << "Unexpected pixel mode in bitmap: " << (int)bitmap.pixel_mode << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::font_loaded
//       Access: Private
//  Description: Called after a font has been successfully loaded,
//               either from disk or from memory image.
////////////////////////////////////////////////////////////////////
bool FreetypeFont::
font_loaded() {
  string name = _face->family_name;
  if (_face->style_name != NULL) {
    name += " ";
    name += _face->style_name;
  }
  set_name(name);
  
  pnmtext_cat.info()
    << "Loaded font " << name << "\n";
  _font_loaded = true;
  reset_scale();

  if (pnmtext_cat.is_debug()) {
    pnmtext_cat.debug()
      << name << " has " << _face->num_charmaps << " charmaps:\n";
    for (int i = 0; i < _face->num_charmaps; i++) {
      pnmtext_cat.debug(false) << " " << (void *)_face->charmaps[i];
    }
    pnmtext_cat.debug(false) << "\n";
    pnmtext_cat.debug()
      << "default charmap is " << (void *)_face->charmap << "\n";
  }
  if (_face->charmap == NULL) {
    // If for some reason FreeType didn't set us up a charmap,
    // then set it up ourselves.
    if (_face->num_charmaps == 0) {
      pnmtext_cat.warning()
        << name << " has no charmaps available.\n";
    } else {
      pnmtext_cat.warning()
        << name << " has no default Unicode charmap.\n";
      if (_face->num_charmaps > 1) {
        pnmtext_cat.warning()
          << "Arbitrarily choosing first of " 
          << _face->num_charmaps << " charmaps.\n";
      }
      FT_Set_Charmap(_face, _face->charmaps[0]);
    }
  }

  return true;
}
 
////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::reset_scale
//       Access: Private
//  Description: Resets the font based on the current values for
//               _point_size, _tex_pixels_per_unit, and _scale_factor.
//               Returns true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool FreetypeFont::
reset_scale() {
  // The font may be rendered larger (by a factor of _scale_factor),
  // and then reduced into the texture.  Hence the difference between
  // _font_pixels_per_unit and _tex_pixels_per_unit.
  _font_pixels_per_unit = _tex_pixels_per_unit * _scale_factor;

  float units_per_inch = (_points_per_inch / _points_per_unit);
  int dpi = (int)(_font_pixels_per_unit * units_per_inch);
  
  int error = FT_Set_Char_Size(_face,
                               (int)(_point_size * 64), (int)(_point_size * 64),
                               dpi, dpi);
  if (error) {
    // If we were unable to set a particular char size, perhaps we
    // have a non-scalable font.  Try to figure out the closest
    // available size.
    int desired_height = (int)(_font_pixels_per_unit * _point_size / _points_per_unit + 0.5f);
    int best_size = -1;
    if (_face->num_fixed_sizes > 0) {
      best_size = 0;
      int best_diff = abs(desired_height - _face->available_sizes[0].height);
      for (int i = 1; i < _face->num_fixed_sizes; i++) {
        int diff = abs(desired_height - _face->available_sizes[i].height);
        if (diff < best_diff) {
          best_size = i;
          best_diff = diff;
        }
      }
    }
    if (best_size >= 0) {
      int pixel_height = _face->available_sizes[best_size].height;
      int pixel_width = _face->available_sizes[best_size].width;
      error = FT_Set_Pixel_Sizes(_face, pixel_width, pixel_height);
      if (!error) {
        pnmtext_cat.info()
          << "Using " << pixel_height << "-pixel font for "
          << get_name() << "\n";

        _font_pixels_per_unit = pixel_height * _points_per_unit / _point_size;
        _tex_pixels_per_unit = _font_pixels_per_unit;
      }
    }
  }

  if (error) {
    pnmtext_cat.warning()
      << "Unable to set " << get_name() 
      << " to " << _point_size << "pt at " << dpi << " dpi.\n";
    _line_height = 1.0f;
    return false;
  }

  _line_height = _face->size->metrics.height / (_font_pixels_per_unit * 64.0f);

  // Determine the correct width for a space.
  error = FT_Load_Char(_face, ' ', FT_LOAD_DEFAULT);
  if (error) {
    // Space isn't defined.  Oh well.
    _space_advance = 0.25f * _line_height;

  } else {
    _space_advance = _face->glyph->advance.x / (_font_pixels_per_unit * 64.0f);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FreetypeFont::initialize_ft_library
//       Access: Private, Static
//  Description: Should be called exactly once to initialize the
//               FreeType library.
////////////////////////////////////////////////////////////////////
void FreetypeFont::
initialize_ft_library() {
  if (!_ft_initialized) {
    int error = FT_Init_FreeType(&_ft_library);
    _ft_initialized = true;
    if (error) {
      pnmtext_cat.error()
        << "Unable to initialize FreeType; dynamic fonts will not load.\n";
    } else {
      _ft_ok = true;
    }
  }
}

#endif  // HAVE_FREETYPE
