// Filename: qpgeomTextGlyph.cxx
// Created by:  drose (31Mar05)
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

TypeHandle qpGeomTextGlyph::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: qpGeomTextGlyph::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomTextGlyph::
operator = (const qpGeomTextGlyph &copy) {
  qpGeom::operator = (copy);
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
//     Function: qpGeomTextGlyph::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTextGlyph::
~qpGeomTextGlyph() {
  if (_glyph != (DynamicTextGlyph *)NULL) {
    _glyph->_geom_count--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTextGlyph::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated qpGeom that is a shallow copy
//               of this one.  It will be a different qpGeom pointer,
//               but its internal data may or may not be shared with
//               that of the original qpGeom.
////////////////////////////////////////////////////////////////////
qpGeom *qpGeomTextGlyph::
make_copy() const {
  return new qpGeomTextGlyph(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTextGlyph::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a qpGeomTextGlyph object
////////////////////////////////////////////////////////////////////
void qpGeomTextGlyph::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_qpGeomTextGlyph);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTextGlyph::make_qpGeomTextGlyph
//       Access: Public
//  Description: Factory method to generate a qpGeomTextGlyph object
////////////////////////////////////////////////////////////////////
TypedWritable* qpGeomTextGlyph::
make_qpGeomTextGlyph(const FactoryParams &params) {
  qpGeomTextGlyph *me = new qpGeomTextGlyph((DynamicTextGlyph *)NULL);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  me->make_dirty();
  me->config();
  return me;
}

#endif  // HAVE_FREETYPE
