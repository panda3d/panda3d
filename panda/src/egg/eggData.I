// Filename: eggData.I
// Created by:  drose (11Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggData::
EggData() {
  _coordsys = CS_default;
}


////////////////////////////////////////////////////////////////////
//     Function: EggData::Copy constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggData::
EggData(const EggData &copy) : 
  EggGroupNode(copy), 
  _coordsys(copy._coordsys), 
  _egg_filename(copy._egg_filename) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::Copy assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggData &EggData::
operator = (const EggData &copy) {
  EggGroupNode::operator = (copy);
  _coordsys = copy._coordsys;
  _egg_filename = copy._egg_filename;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggData::get_coordinate_system
//       Access: Public
//  Description: Returns the coordinate system in which the egg file
//               is defined.
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem EggData::
get_coordinate_system() const {
  return _coordsys;
}


////////////////////////////////////////////////////////////////////
//     Function: EggData::set_egg_filename
//       Access: Public
//  Description: Sets the directory in which the egg file is
//               considered to reside.  This is also implicitly set by
//               read().
////////////////////////////////////////////////////////////////////
INLINE void EggData::
set_egg_filename(const Filename &directory) {
  _egg_filename = directory;
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::get_egg_filename
//       Access: Public
//  Description: Returns the directory in which the egg file is
//               considered to reside.
////////////////////////////////////////////////////////////////////
INLINE const Filename &EggData::
get_egg_filename() const {
  return _egg_filename;
}

