// Filename: eggAlphaMode.I
// Created by:  drose (20Jan99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggAlphaMode::
EggAlphaMode() {
  _alpha_mode = AM_unspecified;
  _draw_order = 0;
  _has_draw_order = false;
}


////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggAlphaMode::
EggAlphaMode(const EggAlphaMode &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::Copy assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggAlphaMode &EggAlphaMode::
operator = (const EggAlphaMode &copy) {
  _alpha_mode = copy._alpha_mode;
  _draw_order = copy._draw_order;
  _has_draw_order = copy._has_draw_order;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::set_alpha_mode
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void EggAlphaMode::
set_alpha_mode(AlphaMode mode) {
  _alpha_mode = mode;
}


////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::get_alpha_mode
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggAlphaMode::AlphaMode EggAlphaMode::
get_alpha_mode() const {
  return _alpha_mode;
}


////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::set_draw_order
//       Access: Public
//  Description: Sets the "draw-order" flag associated with this
//               object.  This specifies a particular order in which
//               objects of this type should be drawn, within the
//               specified bin.  If a bin is not explicitly specified,
//               "fixed" is used.  See also set_bin().
////////////////////////////////////////////////////////////////////
INLINE void EggAlphaMode::
set_draw_order(int order) {
  _draw_order = order;
  _has_draw_order = true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::get_draw_order
//       Access: Public
//  Description: Returns the "draw-order" flag as set for this
//               particular object.  See set_draw_order().
////////////////////////////////////////////////////////////////////
INLINE int EggAlphaMode::
get_draw_order() const {
  return _draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::has_draw_order
//       Access: Public
//  Description: Returns true if the draw-order flag has been set for
//               this particular object.  See set_draw_order().
////////////////////////////////////////////////////////////////////
INLINE bool EggAlphaMode::
has_draw_order() const {
  return _has_draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::clear_draw_order
//       Access: Public
//  Description: Removes the draw-order flag from this particular
//               object.  See set_draw_order().
////////////////////////////////////////////////////////////////////
INLINE void EggAlphaMode::
clear_draw_order() {
  _has_draw_order = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::set_bin
//       Access: Public
//  Description: Sets the "bin" string for this particular object.
//               This names a particular bin in which the object
//               should be rendered.  The exact meaning of a bin is
//               implementation defined, but generally a GeomBin
//               matching each bin name must also be specifically
//               added to the rendering engine (e.g. the
//               CullTraverser) in use for this to work.  See also
//               set_draw_order().
////////////////////////////////////////////////////////////////////
INLINE void EggAlphaMode::
set_bin(const string &bin) {
  _bin = bin;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::get_bin
//       Access: Public
//  Description: Returns the bin name that has been set for this
//               particular object, if any.  See set_bin().
////////////////////////////////////////////////////////////////////
INLINE string EggAlphaMode::
get_bin() const {
  return _bin;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::has_bin
//       Access: Public
//  Description: Returns true if a bin name has been set for this
//               particular object.  See set_bin().
////////////////////////////////////////////////////////////////////
INLINE bool EggAlphaMode::
has_bin() const {
  return !_bin.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::clear_bin
//       Access: Public
//  Description: Removes the bin name that was set for this particular
//               object.  See set_bin().
////////////////////////////////////////////////////////////////////
INLINE void EggAlphaMode::
clear_bin() {
  _bin = string();
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool EggAlphaMode::
operator != (const EggAlphaMode &other) const {
  return !(*this == other);
}
