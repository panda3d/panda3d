// Filename: geomBinTransition.I
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomBinTransition::
GeomBinTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomBinTransition::
GeomBinTransition(const string &bin, int draw_order) : 
  OnOffTransition(TD_on),
  _value(bin),
  _draw_order(draw_order)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::off
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomBinTransition GeomBinTransition::
off() {
  GeomBinTransition result;
  result.set_off();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::set_on
//       Access: Public
//  Description: Changes the GeomBinTransition to render the particular
//               bin.
////////////////////////////////////////////////////////////////////
INLINE void GeomBinTransition::
set_on(const string &bin, int draw_order) {
  nassertv(!bin.empty());
  _value = bin;
  _draw_order = draw_order;
  OnOffTransition::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::get_bin
//       Access: Public
//  Description: Returns the bin name that the GeomBinTransition
//               represents.  It is only valid to call this if is_on()
//               has returned true.
////////////////////////////////////////////////////////////////////
INLINE string GeomBinTransition::
get_bin() const {
  nassertr(is_on(), string());
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::get_draw_order
//       Access: Public
//  Description: Returns the draw order associated with the
//               GeomBinTransition.  It is only valid to call this if
//               is_on() has returned true.  This draw order has
//               meaning only if the associated bin (see get_bin()) is
//               of type GeomBinFixed.
////////////////////////////////////////////////////////////////////
INLINE int GeomBinTransition::
get_draw_order() const {
  nassertr(is_on(), 0);
  return _draw_order;
}

