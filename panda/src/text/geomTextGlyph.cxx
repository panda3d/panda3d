// Filename: geomTextGlyph.cxx
// Created by:  drose (11Feb02)
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

#include "geomTextGlyph.h"

#ifdef HAVE_FREETYPE

#include "datagramIterator.h"
#include "bamReader.h"

TypeHandle GeomTextGlyph::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomTextGlyph::
operator = (const GeomTextGlyph &copy) {
  GeomTristrip::operator = (copy);
  if (_glyph != copy._glyph) {
    if (_glyph != (DynamicTextGlyph *)NULL) {
      _glyph->_geom_count--;
    }
    _glyph = copy._glyph;
    if (_glyph != (DynamicTextGlyph *)NULL) {
      _glyph->_geom_count++;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTextGlyph::
~GeomTextGlyph() {
  if (_glyph != (DynamicTextGlyph *)NULL) {
    _glyph->_geom_count--;
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
  GeomTextGlyph *me = new GeomTextGlyph((DynamicTextGlyph *)NULL);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  me->make_dirty();
  me->config();
  return me;
}

#endif  // HAVE_FREETYPE
