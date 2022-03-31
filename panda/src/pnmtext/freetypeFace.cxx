/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file freetypeFace.cxx
 * @author gogg
 * @date 2009-11-16
 */

#include "freetypeFace.h"

#ifdef HAVE_FREETYPE

#include "config_pnmtext.h"

FT_Library FreetypeFace::_ft_library;
bool FreetypeFace::_ft_initialized = false;
bool FreetypeFace::_ft_ok = false;

TypeHandle FreetypeFace::_type_handle;

/**
 *
 */
FreetypeFace::
FreetypeFace() : _lock("FreetypeFace::_lock") {
  _face = nullptr;
  _char_size = 0;
  _dpi = 0;
  _pixel_width = 0;
  _pixel_height = 0;

  if (!_ft_initialized) {
    initialize_ft_library();
  }
}

/**
 *
 */
FreetypeFace::
~FreetypeFace() {
  if (_face != nullptr){
    FT_Done_Face(_face);
  }
}

/**
 * Retrieves the internal freetype face, and also acquires the lock.  The
 * freetype face is set to the indicated size, either as a char_size and dpi,
 * or as a specific pixel_width and height, before returning.
 *
 * You must call release_face() when you are done using it, to release the
 * lock.
 */
FT_Face FreetypeFace::
acquire_face(int char_size, int dpi, int pixel_width, int pixel_height) {
  _lock.acquire();

  if (pixel_height != 0) {
    if (pixel_height != _pixel_height || pixel_width != _pixel_width) {
      _char_size = 0;
      _dpi = 0;
      _pixel_height = pixel_height;
      _pixel_width = pixel_width;
      FT_Set_Pixel_Sizes(_face, _pixel_width, _pixel_height);
    }
  } else {
    if (char_size != _char_size || dpi != _dpi) {
      _char_size = char_size;
      _dpi = dpi;
      _pixel_height = 0;
      _pixel_width = 0;
      if (_char_size != 0) {
        FT_Set_Char_Size(_face, _char_size, _char_size, _dpi, _dpi);
      }
    }
  }

  return _face;
}

/**
 * Releases the lock acquired by a previous call to acquire_face(), and allows
 * another thread to use the face.
 */
void FreetypeFace::
release_face(FT_Face face) {
  nassertv(_face == face);
  _lock.release();
}

/**
 * Replaces the internal freetype face.
 */
void FreetypeFace::
set_face(FT_Face face) {
  MutexHolder holder(_lock);

  if (_face != nullptr){
    FT_Done_Face(_face);
  }
  _face = face;
  _char_size = 0;
  _dpi = 0;
  _pixel_width = 0;
  _pixel_height = 0;

  _name = _face->family_name;
  if (_face->style_name != nullptr) {
    _name += " ";
    _name += _face->style_name;
  }

  pnmtext_cat.info()
    << "Loaded font " << _name << "\n";

  if (pnmtext_cat.is_debug()) {
    pnmtext_cat.debug()
      << _name << " has " << _face->num_charmaps << " charmaps:\n";
    for (int i = 0; i < _face->num_charmaps; i++) {
      pnmtext_cat.debug(false) << " " << (void *)_face->charmaps[i];
    }
    pnmtext_cat.debug(false) << "\n";
    pnmtext_cat.debug()
      << "default charmap is " << (void *)_face->charmap << "\n";
  }
  if (_face->charmap == nullptr) {
    // If for some reason FreeType didn't set us up a charmap, then set it up
    // ourselves.
    if (_face->num_charmaps == 0) {
      pnmtext_cat.warning()
        << _name << " has no charmaps available.\n";
    } else {
      pnmtext_cat.warning()
        << _name << " has no default Unicode charmap.\n";
      if (_face->num_charmaps > 1) {
        pnmtext_cat.warning()
          << "Arbitrarily choosing first of "
          << _face->num_charmaps << " charmaps.\n";
      }
      FT_Set_Charmap(_face, _face->charmaps[0]);
    }
  }
}

/**
 * Should be called exactly once to initialize the FreeType library.
 */
void FreetypeFace::
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
