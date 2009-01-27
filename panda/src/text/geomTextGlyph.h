// Filename: geomTextGlyph.h
// Created by:  drose (31Mar05)
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

#ifndef GEOMTEXTGLYPH_H
#define GEOMTEXTGLYPH_H

#include "pandabase.h"
#include "geom.h"

#ifdef HAVE_FREETYPE

#include "dynamicTextGlyph.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomTextGlyph
// Description : This is a specialization on Geom for containing a
//               primitive intended to represent a DynamicTextGlyph.
//               Its sole purpose is to maintain the geom count on the
//               glyph, so we can determine the actual usage count on
//               a dynamic glyph (and thus know when it is safe to
//               recycle the glyph).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_TEXT GeomTextGlyph : public Geom {
public:
  GeomTextGlyph(DynamicTextGlyph *glyph,
                const GeomVertexData *data);
  GeomTextGlyph(const GeomVertexData *data);
  GeomTextGlyph(const GeomTextGlyph &copy);
  void operator = (const GeomTextGlyph &copy);
  virtual ~GeomTextGlyph();
  ALLOC_DELETED_CHAIN(GeomTextGlyph);

  virtual Geom *make_copy() const;
  virtual bool copy_primitives_from(const Geom *other);
  void count_geom(const Geom *other);

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  typedef pvector< PT(DynamicTextGlyph) > Glyphs;
  Glyphs _glyphs;

public:
  static void register_with_read_factory();
  static TypedWritable *make_GeomTextGlyph(const FactoryParams &params);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Geom::init_type();
    register_type(_type_handle, "GeomTextGlyph",
                  Geom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "geomTextGlyph.I"

#else  // HAVE_FREETYPE

// Without Freetype, a GeomTextGlyph is really just an ordinary Geom.
typedef Geom GeomTextGlyph;

#endif  // HAVE_FREETYPE

#endif // GEOMTEXTGLYPH_H
