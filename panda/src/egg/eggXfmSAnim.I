// Filename: eggXfmSAnim.I
// Created by:  drose (19Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggXfmSAnim::
EggXfmSAnim(const string &name, CoordinateSystem cs) : EggGroupNode(name) {
  _has_fps = false;
  _coordsys = cs;
}


////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::Copy constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggXfmSAnim::
EggXfmSAnim(const EggXfmSAnim &copy) 
  : EggGroupNode(copy), 
    _fps(copy._fps),
    _has_fps(copy._has_fps),
    _order(copy._order),
    _coordsys(copy._coordsys) {
}


////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::Copy assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggXfmSAnim &EggXfmSAnim::
operator = (const EggXfmSAnim &copy) {
  EggGroupNode::operator = (copy);
  _fps = copy._fps;
  _has_fps = copy._has_fps;
  _order = copy._order;
  _coordsys = copy._coordsys;

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::set_fps
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void EggXfmSAnim::
set_fps(double fps) {
  _fps = fps;
  _has_fps = true;
}


////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::clear_fps
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void EggXfmSAnim::
clear_fps() {
  _has_fps = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::has_fps
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool EggXfmSAnim::
has_fps() const {
  return _has_fps;
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::get_fps
//       Access: Public
//  Description: This is only valid if has_fps() returns true.
////////////////////////////////////////////////////////////////////
INLINE double EggXfmSAnim::
get_fps() const {
  nassertr(has_fps(), 0.0);
  return _fps;
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::set_order
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void EggXfmSAnim::
set_order(const string &order) {
  _order = order;
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::clear_order
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void EggXfmSAnim::
clear_order() {
  _order = "";
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::has_order
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool EggXfmSAnim::
has_order() const {
  return !_order.empty();
}
 
////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::get_order
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const string &EggXfmSAnim::
get_order() const {
  return _order;
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::get_coordinate_system
//       Access: Public
//  Description: Returns the coordinate system this table believes it
//               is defined within.  This should always match the
//               coordinate system of the EggData structure that owns
//               it.  It is necessary to store it here because the
//               meaning of the h, p, and r columns depends on the
//               coordinate system.
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem EggXfmSAnim::
get_coordinate_system() const {
  return _coordsys;
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::clear_data
//       Access: Public
//  Description: Removes all data from the table.  It does this by
//               removing all of its children.
////////////////////////////////////////////////////////////////////
INLINE void EggXfmSAnim::
clear_data() {
  EggGroupNode::clear();
}
