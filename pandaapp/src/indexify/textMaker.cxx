// Filename: textMaker.cxx
// Created by:  drose (03Apr02)
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

#include "textMaker.h"
#include "textGlyph.h"
#include "filename.h"
#include "pnmImage.h"

FT_Library TextMaker::_ft_library;
bool TextMaker::_ft_initialized = false;
bool TextMaker::_ft_ok = false;

////////////////////////////////////////////////////////////////////
//     Function: TextMaker::Constructor
//       Access: Public
//  Description: The constructor expects the name of some font file
//               that FreeType can read, along with face_index,
//               indicating which font within the file to load
//               (usually 0).
////////////////////////////////////////////////////////////////////
TextMaker::
TextMaker(const Filename &font_filename, int face_index) {
  _is_valid = false;

  if (!_ft_initialized) {
    initialize_ft_library();
  }
  if (!_ft_ok) {
    nout
      << "Unable to read font " << font_filename
      << ": FreeType library not initialized properly.\n";
    return;
  }

  Filename path(font_filename);
  if (!path.exists()) {
    nout << "Unable to find font file " << font_filename << "\n";
  } else {
    string os_specific = path.to_os_specific();
    
    int error = FT_New_Face(_ft_library,
                            os_specific.c_str(),
                            face_index,
                            &_face);
    if (error == FT_Err_Unknown_File_Format) {
      nout << "Unable to read font " << font_filename << ": unknown file format.\n";
    } else if (error) {
      nout << "Unable to read font " << font_filename << ": invalid.\n";

    } else {
      string name = _face->family_name;
      if (_face->style_name != NULL) {
        name += " ";
        name += _face->style_name;
      }
      set_name(name);

      error = FT_Select_Charmap(_face, ft_encoding_unicode);
      if (error) {
        error = FT_Select_Charmap(_face, ft_encoding_latin_2);
      }
      if (error) {
        nout << "Unable to select ISO encoding for " << get_name() << ".\n";
        FT_Done_Face(_face);

      } else {
        nout << "Loaded font " << get_name() << "\n";
        _is_valid = true;
        _scale_factor = 0.0;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextMaker::Constructor
//       Access: Public
//  Description: This constructor works as above, but it takes the
//               font data from an in-memory buffer instead of from a
//               named file.
////////////////////////////////////////////////////////////////////
TextMaker::
TextMaker(const char *font_data, int font_data_size, int face_index) {
  _is_valid = false;

  if (!_ft_initialized) {
    initialize_ft_library();
  }
  if (!_ft_ok) {
    nout
      << "Unable to read internal font: FreeType library not initialized properly.\n";
    return;
  }

  int error = FT_New_Memory_Face(_ft_library,
                                 (const FT_Byte *)font_data, font_data_size,
                                 face_index,
                                 &_face);
  if (error == FT_Err_Unknown_File_Format) {
    nout << "Unable to read internal font: unknown file format.\n";
  } else if (error) {
    nout << "Unable to read internal font: invalid.\n";
    
  } else {
    string name = _face->family_name;
    if (_face->style_name != NULL) {
      name += " ";
      name += _face->style_name;
    }
    set_name(name);
    
    error = FT_Select_Charmap(_face, ft_encoding_unicode);
    if (error) {
      error = FT_Select_Charmap(_face, ft_encoding_latin_2);
    }
    if (error) {
      nout << "Unable to select ISO encoding for " << get_name() << ".\n";
      FT_Done_Face(_face);
      
    } else {
      _is_valid = true;
      _scale_factor = 0.0;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextMaker::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextMaker::
~TextMaker() {
  empty_cache();

  if (_is_valid) {
    FT_Done_Face(_face);
    _is_valid = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextMaker::is_valid
//       Access: Public
//  Description: Returns true if the TextMaker is valid and ready to
//               generate text, false otherwise.
////////////////////////////////////////////////////////////////////
bool TextMaker::
is_valid() const {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: TextMaker::set_pixel_size
//       Access: Public
//  Description: Specifies the pixel size of the font to use to
//               generate future text.  If the scale_factor is
//               specified, it is the factor by which to scale up the
//               font glyphs internally before reducing them to the
//               final output pixel_size; this works around problems
//               FreeType has with antialiasing small pixel sizes.
//
//               If the font contains one or more fixed-size fonts
//               instead of a scalable font, this will ignore the
//               supplied scale_factor and automatically choose the
//               scale_factor to produce the requested pixel_size
//               output from the closest matching input size.
////////////////////////////////////////////////////////////////////
void TextMaker::
set_pixel_size(int pixel_size, double scale_factor) {
  nassertv(_is_valid);
  reset_scale(pixel_size, scale_factor);
  empty_cache();
}

////////////////////////////////////////////////////////////////////
//     Function: TextMaker::generate_into
//       Access: Public
//  Description: Generates text into the indicated image at the
//               indicated position.  Currently, text is centered
//               horizontally, with the baseline on the y position.
////////////////////////////////////////////////////////////////////
void TextMaker::
generate_into(const string &text, PNMImage &dest_image, int x, int y) {
  // First, measure the total width in pixels, so we can center.
  int width = 0;
  string::const_iterator ti;
  for (ti = text.begin(); ti != text.end(); ++ti) {
    int ch = (unsigned char)(*ti);
    TextGlyph *glyph = get_glyph(ch);
    width += glyph->get_advance();
  }

  int xp = x - (width / 2);
  int yp = y;

  // Now place the text.
  for (ti = text.begin(); ti != text.end(); ++ti) {
    int ch = (unsigned char)(*ti);
    TextGlyph *glyph = get_glyph(ch);
    glyph->place(dest_image, xp, yp);
    xp += glyph->get_advance();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextMaker::get_glyph
//       Access: Private
//  Description: Returns the glyph for the indicated index, or NULL if
//               it is not defined in the font.
////////////////////////////////////////////////////////////////////
TextGlyph *TextMaker::
get_glyph(int character) {
  int glyph_index = FT_Get_Char_Index(_face, character);

  Glyphs::iterator gi;
  gi = _glyphs.find(glyph_index);
  if (gi != _glyphs.end()) {
    return (*gi).second;
  }

  TextGlyph *glyph = make_glyph(glyph_index);
  _glyphs.insert(Glyphs::value_type(glyph_index, glyph));
  return glyph;
}

////////////////////////////////////////////////////////////////////
//     Function: TextMaker::make_glyph
//       Access: Private
//  Description: Creates a new TextGlyph object for the indicated
//               index, if possible.
////////////////////////////////////////////////////////////////////
TextGlyph *TextMaker::
make_glyph(int glyph_index) {
  int error = FT_Load_Glyph(_face, glyph_index, FT_LOAD_RENDER);
  if (error) {
    nout << "Unable to render glyph " << glyph_index << "\n";
    return (TextGlyph *)NULL;
  }

  FT_GlyphSlot slot = _face->glyph;
  FT_Bitmap &bitmap = slot->bitmap;

  double advance = slot->advance.x / 64.0;

  if (bitmap.width == 0 || bitmap.rows == 0) {
    // If we got an empty bitmap, it's a special case.
    TextGlyph *glyph = new TextGlyph(advance);
    glyph->rescale(_scale_factor);
    return glyph;

  } else {
    TextGlyph *glyph = new TextGlyph(advance);
    PNMImage &glyph_image = glyph->_image;
    glyph_image.clear(bitmap.width, bitmap.rows, 1);

    if (bitmap.pixel_mode == ft_pixel_mode_mono) {
      // This is a little bit more work: we have to expand the
      // one-bit-per-pixel bitmap into a one-byte-per-pixel texture.
      unsigned char *buffer_row = bitmap.buffer;
      for (int yi = 0; yi < bitmap.rows; yi++) {
        int bit = 0x80;
        unsigned char *b = buffer_row;
        for (int xi = 0; xi < bitmap.width; xi++) {
          if (*b & bit) {
            glyph_image.set_gray(xi, yi, 0.0);
          } else {
            glyph_image.set_gray(xi, yi, 1.0);
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
          double value = 
            (double)buffer_row[xi] / (double)(bitmap.num_grays - 1);
          glyph_image.set_gray(xi, yi, 1.0 - value);
        }
        buffer_row += bitmap.pitch;
      }

    } else {
      nout << "Unexpected pixel mode in bitmap: " << (int)bitmap.pixel_mode << "\n";
    }

    glyph->_top = slot->bitmap_top;
    glyph->_left = slot->bitmap_left;

    glyph->rescale(_scale_factor);
    return glyph;
  }
}
 
////////////////////////////////////////////////////////////////////
//     Function: TextMaker::reset_scale
//       Access: Private
//  Description: Sets the font to use the appropriate scale pixels,
//               and sets _scale_factor accordingly.
////////////////////////////////////////////////////////////////////
bool TextMaker::
reset_scale(int pixel_size, double scale_factor) {
  _scale_factor = scale_factor;
  int want_pixel_size = (int)(pixel_size * _scale_factor + 0.5);

  int error = FT_Set_Pixel_Sizes(_face, want_pixel_size, want_pixel_size);
  if (error) {
    // If we were unable to set a particular char size, perhaps we
    // have a non-scalable font.  Try to figure out the closest
    // available size.
    int best_size = -1;
    if (_face->num_fixed_sizes > 0) {
      best_size = 0;
      int best_diff = abs(want_pixel_size - _face->available_sizes[0].height);
      for (int i = 1; i < _face->num_fixed_sizes; i++) {
        int diff = abs(want_pixel_size - _face->available_sizes[i].height);
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
        _scale_factor = (double)pixel_height / (double)pixel_size;
      }
    }
  }

  if (error) {
    nout
      << "Unable to set " << get_name() 
      << " to " << pixel_size << " pixels.\n";
    return false;
  }

  _line_height = _face->size->metrics.height / 64.0;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TextMaker::empty_cache
//       Access: Private
//  Description: Empties the cache of previously-generated glyphs.
////////////////////////////////////////////////////////////////////
void TextMaker::
empty_cache() {
  Glyphs::iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    TextGlyph *glyph = (*gi).second;
    delete glyph;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextMaker::initialize_ft_library
//       Access: Private, Static
//  Description: Should be called exactly once to initialize the
//               FreeType library.
////////////////////////////////////////////////////////////////////
void TextMaker::
initialize_ft_library() {
  if (!_ft_initialized) {
    int error = FT_Init_FreeType(&_ft_library);
    _ft_initialized = true;
    if (error) {
      nout << "Unable to initialize FreeType.\n";
    } else {
      _ft_ok = true;
    }
  }
}
