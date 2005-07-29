// Filename: animChannel.I
// Created by:  drose (22Feb99)
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

#ifdef WIN32_VC
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
//     Function: AnimChannel::get_value_no_scale
//       Access: Public, Virtual
//  Description: Returns the value associated with the current frame,
//               with no scale components.  This only makes sense for
//               a matrix-type channel, although for fiddly technical
//               reasons the function exists for all channels.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannel<SwitchType>::
get_value_no_scale(int frame, ValueType &value) {
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
get_scale(int, float scale[3]) {
  scale[0] = 1.0f;
  scale[1] = 1.0f;
  scale[2] = 1.0f;
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


