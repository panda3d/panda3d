// Filename: animChannelFixed.I
// Created by:  drose (24Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
has_changed(int, double, int, double) {
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
