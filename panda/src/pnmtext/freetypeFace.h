/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file freetypeFace.h
 * @author gogg
 * @date 2009-11-16
 */

#ifndef FREETYPEFACE_H
#define FREETYPEFACE_H

#include "pandabase.h"

#ifdef HAVE_FREETYPE

#include "typedReferenceCount.h"
#include "namable.h"
#include "pmutex.h"
#include "mutexHolder.h"

#include <ft2build.h>
#include FT_FREETYPE_H

/**
 * This is a reference-counted wrapper for the freetype font face object
 * (FT_Face). It's used by the FreetypeFont class to store a face that can be
 * shared between copied instances.
 */
class EXPCL_PANDA_PNMTEXT FreetypeFace : public TypedReferenceCount, public Namable {
public:
  FreetypeFace();
  ~FreetypeFace();

  FT_Face acquire_face(int char_size, int dpi, int pixel_width, int pixel_height);
  void release_face(FT_Face face);

  void set_face(FT_Face face);

private:
  static void initialize_ft_library();

private:
  // This is provided as a permanent storage for the raw font data, if needed.
  std::string _font_data;

  std::string _name;
  FT_Face _face;
  int _char_size;
  int _dpi;
  int _pixel_width;
  int _pixel_height;
  Mutex _lock;

  static FT_Library _ft_library;
  static bool _ft_initialized;
  static bool _ft_ok;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "FreetypeFace",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class FreetypeFont;
};


#include "freetypeFace.I"

#endif  // HAVE_FREETYPE

#endif
