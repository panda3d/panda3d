// Filename: geomBinTransition.cxx
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "geomBinTransition.h"
#include "geomBinAttribute.h"

#include <indent.h>

TypeHandle GeomBinTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated GeomBinTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *GeomBinTransition::
make_copy() const {
  return new GeomBinTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated GeomBinAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *GeomBinTransition::
make_attrib() const {
  return new GeomBinAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another GeomBinTransition.
////////////////////////////////////////////////////////////////////
void GeomBinTransition::
set_value_from(const OnOffTransition *other) {
  const GeomBinTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
  _draw_order = ot->_draw_order; 
  nassertv(_value != (GeomBin *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int GeomBinTransition::
compare_values(const OnOffTransition *other) const {
  const GeomBinTransition *ot;
  DCAST_INTO_R(ot, other, false);
  if (_value != ot->_value) {
    return (int)(_value - ot->_value);
  }
  return _draw_order - ot->_draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void GeomBinTransition::
output_value(ostream &out) const {
  nassertv(_value != (GeomBin *)NULL);
  out << *_value << ":" << _draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void GeomBinTransition::
write_value(ostream &out, int indent_level) const {
  nassertv(_value != (GeomBin *)NULL);
  indent(out, indent_level) << *_value << ":" << _draw_order << "\n";
}
