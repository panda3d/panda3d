// Filename: animChannel.I
// Created by:  drose (22Feb99)
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
TypeHandle AnimChannel<SwitchType>::_type_handle;

// We don't need to explicitly call AnimChannel::init_type(), because
// it is an abstract class and therefore must have derived objects.
// Its derived objects will call init_type() for us.


////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::Protected constructor
//       Access: Protected
//  Description: Don't use this constructor.  It exists only so that
//               AnimChannelFixed may define itself outside of the
//               hierarchy.  Normally, an AnimChannel must be created
//               as part of a hierarchy.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
INLINE AnimChannel<SwitchType>::
AnimChannel(const string &name)
  : AnimChannelBase(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::Copy Constructor
//       Access: Protected
//  Description: Creates a new AnimChannel, just like this one,
//               without copying any children.  The new copy is added
//               to the indicated parent.  Intended to be called by
//               make_copy() only.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
INLINE AnimChannel<SwitchType>::
AnimChannel(AnimGroup *parent, const AnimChannel &copy) : 
  AnimChannelBase(parent, copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::Constructor
//       Access: Public
//  Description: This is the normal constructor, which automatically
//               places the AnimChannel in the previously-created
//               hierarchy.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
INLINE AnimChannel<SwitchType>::
AnimChannel(AnimGroup *parent, const string &name)
  : AnimChannelBase(parent, name) {
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class SwitchType>
INLINE AnimChannel<SwitchType>::
~AnimChannel() {
}

#if defined(WIN32_VC) || defined(WIN64_VC)
////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::get_value
//       Access: Public, Virtual
//  Description: Gets the value of the channel at the indicated frame.
//               This is a pure virtual function and normally would
//               not need a function body, except that VC++ seems to
//               be unhappy about instantiating the template without
//               it.
//
//               However, GCC seems to get confused when it *is*
//               defined.  So this whole thing is protected within an
//               ifdef.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannel<SwitchType>::
get_value(int, TYPENAME AnimChannel<SwitchType>::ValueType &) {
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::get_value_no_scale_share
//       Access: Public, Virtual
//  Description: Returns the value associated with the current frame,
//               with no scale or share components.  This only makes
//               sense for a matrix-type channel, although for fiddly
//               technical reasons the function exists for all
//               channels.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannel<SwitchType>::
get_value_no_scale_shear(int frame, ValueType &value) {
  get_value(frame, value);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::get_scale
//       Access: Public, Virtual
//  Description: Returns the x, y, and z scale components associated
//               with the current frame.  As above, this only makes
//               sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannel<SwitchType>::
get_scale(int, LVecBase3 &scale) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::get_hpr
//       Access: Public, Virtual
//  Description: Returns the h, p, and r components associated
//               with the current frame.  As above, this only makes
//               sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannel<SwitchType>::
get_hpr(int, LVecBase3 &hpr) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::get_quat
//       Access: Public, Virtual
//  Description: Returns the rotation component associated with the
//               current frame, expressed as a quaternion.  As above,
//               this only makes sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannel<SwitchType>::
get_quat(int, LQuaternion &quat) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::get_pos
//       Access: Public, Virtual
//  Description: Returns the x, y, and z translation components
//               associated with the current frame.  As above, this
//               only makes sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannel<SwitchType>::
get_pos(int, LVecBase3 &pos) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::get_shear
//       Access: Public, Virtual
//  Description: Returns the a, b, and c shear components associated
//               with the current frame.  As above, this only makes
//               sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannel<SwitchType>::
get_shear(int, LVecBase3 &shear) {
  nassertv(false);
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannel::get_value_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle associated with the ValueType
//               we return.  This is provided to allow a bit of
//               run-time checking that joints and channels are
//               matching properly in type.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
TypeHandle AnimChannel<SwitchType>::
get_value_type() const {
  return get_type_handle(ValueType);
}


