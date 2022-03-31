/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannel.I
 * @author drose
 * @date 1999-02-22
 */

template<class SwitchType>
TypeHandle AnimChannel<SwitchType>::_type_handle;

// We don't need to explicitly call AnimChannel::init_type(), because it is an
// abstract class and therefore must have derived objects.  Its derived
// objects will call init_type() for us.


/**
 * Don't use this constructor.  It exists only so that AnimChannelFixed may
 * define itself outside of the hierarchy.  Normally, an AnimChannel must be
 * created as part of a hierarchy.
 */
template<class SwitchType>
INLINE AnimChannel<SwitchType>::
AnimChannel(const std::string &name)
  : AnimChannelBase(name) {
}

/**
 * Creates a new AnimChannel, just like this one, without copying any
 * children.  The new copy is added to the indicated parent.  Intended to be
 * called by make_copy() only.
 */
template<class SwitchType>
INLINE AnimChannel<SwitchType>::
AnimChannel(AnimGroup *parent, const AnimChannel &copy) :
  AnimChannelBase(parent, copy)
{
}

/**
 * This is the normal constructor, which automatically places the AnimChannel
 * in the previously-created hierarchy.
 */
template<class SwitchType>
INLINE AnimChannel<SwitchType>::
AnimChannel(AnimGroup *parent, const std::string &name)
  : AnimChannelBase(parent, name) {
}

/**
 *
 */
template<class SwitchType>
INLINE AnimChannel<SwitchType>::
~AnimChannel() {
}

/**
 * Returns the value associated with the current frame, with no scale or share
 * components.  This only makes sense for a matrix-type channel, although for
 * fiddly technical reasons the function exists for all channels.
 */
template<class SwitchType>
void AnimChannel<SwitchType>::
get_value_no_scale_shear(int frame, ValueType &value) {
  get_value(frame, value);
}

/**
 * Returns the x, y, and z scale components associated with the current frame.
 * As above, this only makes sense for a matrix-type channel.
 */
template<class SwitchType>
void AnimChannel<SwitchType>::
get_scale(int, LVecBase3 &scale) {
  nassertv(false);
}

/**
 * Returns the h, p, and r components associated with the current frame.  As
 * above, this only makes sense for a matrix-type channel.
 */
template<class SwitchType>
void AnimChannel<SwitchType>::
get_hpr(int, LVecBase3 &hpr) {
  nassertv(false);
}

/**
 * Returns the rotation component associated with the current frame, expressed
 * as a quaternion.  As above, this only makes sense for a matrix-type
 * channel.
 */
template<class SwitchType>
void AnimChannel<SwitchType>::
get_quat(int, LQuaternion &quat) {
  nassertv(false);
}

/**
 * Returns the x, y, and z translation components associated with the current
 * frame.  As above, this only makes sense for a matrix-type channel.
 */
template<class SwitchType>
void AnimChannel<SwitchType>::
get_pos(int, LVecBase3 &pos) {
  nassertv(false);
}

/**
 * Returns the a, b, and c shear components associated with the current frame.
 * As above, this only makes sense for a matrix-type channel.
 */
template<class SwitchType>
void AnimChannel<SwitchType>::
get_shear(int, LVecBase3 &shear) {
  nassertv(false);
}


/**
 * Returns the TypeHandle associated with the ValueType we return.  This is
 * provided to allow a bit of run-time checking that joints and channels are
 * matching properly in type.
 */
template<class SwitchType>
TypeHandle AnimChannel<SwitchType>::
get_value_type() const {
  return get_type_handle(ValueType);
}
