// Filename: geomBinAttribute.cxx
// Created by:  drose (07Apr00)
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

#if defined(WIN32_VC) && !defined(NO_PCH)
#include "cull_headers.h"
#endif

#pragma hdrstop

#if !defined(WIN32_VC) || defined(NO_PCH)
#include "geomBinAttribute.h"
#include "geomBinTransition.h"

#include <indent.h>
#include <string.h>
#endif

#include <graphicsStateGuardianBase.h>

TypeHandle GeomBinAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle GeomBinAttribute::
get_handle() const {
  return GeomBinTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated GeomBinAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *GeomBinAttribute::
make_copy() const {
  return new GeomBinAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated GeomBinAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *GeomBinAttribute::
make_initial() const {
  return new GeomBinAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               GeomBinTransition.
////////////////////////////////////////////////////////////////////
void GeomBinAttribute::
set_value_from(const OnOffTransition *other) {
  const GeomBinTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
  _draw_order = ot->_draw_order;
  nassertv(!_value.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int GeomBinAttribute::
compare_values(const OnOffAttribute *other) const {
  const GeomBinAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  if (_value != ot->_value) {
    return strcmp(_value.c_str(), ot->_value.c_str());
  }
  return _draw_order - ot->_draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void GeomBinAttribute::
output_value(ostream &out) const {
  out << _value << ":" << _draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void GeomBinAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << ":" << _draw_order << "\n";
}
