// Filename: animChannelFixed.I
// Created by:  drose (24Feb99)
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

template<class SwitchType>
TypeHandle AnimChannelFixed<SwitchType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::Copy Constructor
//       Access: Protected
//  Description: Creates a new AnimChannelFixed, just like this one,
//               without copying any children.  The new copy is added
//               to the indicated parent.  Intended to be called by
//               make_copy() only.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
INLINE AnimChannelFixed<SwitchType>::
AnimChannelFixed(AnimGroup *parent, const AnimChannelFixed<SwitchType> &copy) : 
  AnimChannel<SwitchType>(parent, copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class SwitchType>
INLINE AnimChannelFixed<SwitchType>::
AnimChannelFixed(const string &name, const ValueType &value)
  : AnimChannel<SwitchType>(name),
    _value(value) {
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::has_changed
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class SwitchType>
bool AnimChannelFixed<SwitchType>::
has_changed(double, double) {
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::get_value
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannelFixed<SwitchType>::
get_value(int, ValueType &value) {
  value = _value;
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannelFixed<SwitchType>::
output(ostream &out) const {
  AnimChannel<SwitchType>::output(out);
  out << " = " << _value;
}
