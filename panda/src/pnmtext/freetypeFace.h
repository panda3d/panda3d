// Filename: freetypeFont.h
// Created by:  gogg (16Nov09)
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

#ifndef FREETYPEFACE_H
#define FREETYPEFACE_H

#include "pandabase.h"

#ifdef HAVE_FREETYPE

//#include "config_pnmtext.h"
//#include "filename.h"
//#include "pvector.h"
//#include "pmap.h"
//#include "pnmImage.h"
#include "typedReferenceCount.h"
#include "namable.h"

#include <ft2build.h>
#include FT_FREETYPE_H

////////////////////////////////////////////////////////////////////
//       Class : FreetypeFont
// Description : This is a reference-counted wrapper for the
//               freetype font face object (FT_Face).
//               It's used by the FreetypeFont class to store a face
//               that can be shared between copied instances.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PNMTEXT FreetypeFace : public TypedReferenceCount, public Namable {
public:
  FreetypeFace();

PUBLISHED:
  ~FreetypeFace();

  INLINE FT_Face get_face();
  INLINE void set_face(FT_Face face);

private:
  FT_Face _face;


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
};


#include "freetypeFace.I"

#endif  // HAVE_FREETYPE

#endif
