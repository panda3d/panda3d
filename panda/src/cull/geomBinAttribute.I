// Filename: geomBinAttribute.I
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomBinAttribute::
GeomBinAttribute() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomBinAttribute::
GeomBinAttribute(const string &bin, int draw_order) {
  set_on(bin, draw_order);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::set_on
//       Access: Public
//  Description: Changes the GeomBinAttribute to render the particular
//               geomBin.
////////////////////////////////////////////////////////////////////
INLINE void GeomBinAttribute::
set_on(const string &bin, int draw_order) {
  nassertv(!bin.empty());
  _value = bin;
  _draw_order = draw_order;
  OnOffAttribute::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::get_bin
//       Access: Public
//  Description: Returns the bin name that the GeomBinAttribute
//               represents.  It is only valid to call this if is_on()
//               has returned true.
////////////////////////////////////////////////////////////////////
INLINE const string &GeomBinAttribute::
get_bin() const {
  nassertr(is_on(), _value);
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinAttribute::get_draw_order
//       Access: Public
//  Description: Returns the draw order associated with the
//               GeomBinAttribute.  It is only valid to call this if
//               is_on() has returned true.  This draw order has
//               meaning only if the associated bin (see get_bin()) is
//               of type GeomBinFixed.
////////////////////////////////////////////////////////////////////
INLINE int GeomBinAttribute::
get_draw_order() const {
  nassertr(is_on(), 0);
  return _draw_order;
}

