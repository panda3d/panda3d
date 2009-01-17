// Filename: geomTextGlyph.cxx
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

#include "geomTextGlyph.h"

#ifdef HAVE_FREETYPE

#include "datagramIterator.h"
#include "bamReader.h"
#include "indent.h"

TypeHandle GeomTextGlyph::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTextGlyph::
GeomTextGlyph(DynamicTextGlyph *glyph, const GeomVertexData *data) :
  Geom(data)
{
  // Initially, there is only one glyph in the Geom.  There might be
  // additional Glyphs later when we flatten the graph and call
  // Geom::unify().
  if (glyph != (DynamicTextGlyph *)NULL) {
    _glyphs.reserve(1);
    _glyphs.push_back(glyph);
    glyph->_geom_count++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTextGlyph::
GeomTextGlyph(const GeomVertexData *data) :
  Geom(data)
{
  // With this constructor, there are no glyphs initially.
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTextGlyph::
GeomTextGlyph(const GeomTextGlyph &copy) :
  Geom(copy),
  _glyphs(copy._glyphs)
{
  Glyphs::iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    DynamicTextGlyph *glyph = (*gi);
    nassertv(glyph != (DynamicTextGlyph *)NULL);
    glyph->_geom_count++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomTextGlyph::
operator = (const GeomTextGlyph &copy) {
  Geom::operator = (copy);
  
  Glyphs::iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    DynamicTextGlyph *glyph = (*gi);
    nassertv(glyph != (DynamicTextGlyph *)NULL);
    glyph->_geom_count--;
    nassertv((*gi)->_geom_count >= 0);
  }
  _glyphs = copy._glyphs;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    DynamicTextGlyph *glyph = (*gi);
    nassertv(glyph != (DynamicTextGlyph *)NULL);
    glyph->_geom_count++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTextGlyph::
~GeomTextGlyph() {
  Glyphs::iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    DynamicTextGlyph *glyph = (*gi);
    nassertv(glyph != (DynamicTextGlyph *)NULL);
    glyph->_geom_count--;
    nassertv(glyph->_geom_count >= 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Geom that is a shallow copy
//               of this one.  It will be a different Geom pointer,
//               but its internal data may or may not be shared with
//               that of the original Geom.
////////////////////////////////////////////////////////////////////
Geom *GeomTextGlyph::
make_copy() const {
  return new GeomTextGlyph(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::copy_primitives_from
//       Access: Public, Virtual
//  Description: Copies the primitives from the indicated Geom into
//               this one.  This does require that both Geoms contain
//               the same fundamental type primitives, both have a
//               compatible shade model, and both use the same
//               GeomVertexData.  Both Geoms must also be the same
//               specific class type (i.e. if one is a GeomTextGlyph,
//               they both must be.)
//
//               Returns true if the copy is successful, or false
//               otherwise (because the Geoms were mismatched).
////////////////////////////////////////////////////////////////////
bool GeomTextGlyph::
copy_primitives_from(const Geom *other) {
  if (!Geom::copy_primitives_from(other)) {
    return false;
  }

  const GeomTextGlyph *tother;
  DCAST_INTO_R(tother, other, false);

  // Also copy the glyph pointers.
  Glyphs::const_iterator gi;
  for (gi = tother->_glyphs.begin(); gi != tother->_glyphs.end(); ++gi) {
    _glyphs.push_back(*gi);
    (*gi)->_geom_count++;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::count_geom
//       Access: Public
//  Description: Records the reference count of the other Geom within
//               this Geom, as if the primitives were copied in via
//               copy_primitives_from() (but does not actually copy
//               any primitives).  This is particularly necessary for
//               GeomTextGlyph's reference counting mechanism.
//
//               Does nothing if the other Geom is not a
//               GeomTextGlyph.
////////////////////////////////////////////////////////////////////
void GeomTextGlyph::
count_geom(const Geom *other) {
  if (other->is_of_type(GeomTextGlyph::get_class_type())) {
    const GeomTextGlyph *tother;
    DCAST_INTO_V(tother, other);
    
    Glyphs::const_iterator gi;
    for (gi = tother->_glyphs.begin(); gi != tother->_glyphs.end(); ++gi) {
      _glyphs.push_back(*gi);
      (*gi)->_geom_count++;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomTextGlyph::
output(ostream &out) const {
  Geom::output(out);
  out << ", glyphs: [";
  Glyphs::const_iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    DynamicTextGlyph *glyph = (*gi);
    nassertv(glyph != (DynamicTextGlyph *)NULL);
    out << " " << glyph->get_character();
  }
  out << " ]";
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomTextGlyph::
write(ostream &out, int indent_level) const {
  Geom::write(out, indent_level);
  indent(out, indent_level)
    << "Glyphs: [";
  Glyphs::const_iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    DynamicTextGlyph *glyph = (*gi);
    nassertv(glyph != (DynamicTextGlyph *)NULL);
    out << " " << glyph->get_character();
  }
  out << " ]\n";
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomTextGlyph object
////////////////////////////////////////////////////////////////////
void GeomTextGlyph::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomTextGlyph);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::make_GeomTextGlyph
//       Access: Public
//  Description: Factory method to generate a GeomTextGlyph object
////////////////////////////////////////////////////////////////////
TypedWritable* GeomTextGlyph::
make_GeomTextGlyph(const FactoryParams &params) {
  GeomTextGlyph *me = new GeomTextGlyph((DynamicTextGlyph *)NULL, 
                                        (GeomVertexData *)NULL);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

#endif  // HAVE_FREETYPE
