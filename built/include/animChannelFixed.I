/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelFixed.I
 * @author drose
 * @date 1999-02-24
 */

template<class SwitchType>
TypeHandle AnimChannelFixed<SwitchType>::_type_handle;

/**
 * Creates a new AnimChannelFixed, just like this one, without copying any
 * children.  The new copy is added to the indicated parent.  Intended to be
 * called by make_copy() only.
 */
template<class SwitchType>
INLINE AnimChannelFixed<SwitchType>::
AnimChannelFixed(AnimGroup *parent, const AnimChannelFixed<SwitchType> &copy) :
  AnimChannel<SwitchType>(parent, copy),
  _value(copy._value)
{
}

/**
 *
 */
template<class SwitchType>
INLINE AnimChannelFixed<SwitchType>::
AnimChannelFixed(const std::string &name, const ValueType &value)
  : AnimChannel<SwitchType>(name),
    _value(value) {
}


/**
 *
 */
template<class SwitchType>
bool AnimChannelFixed<SwitchType>::
has_changed(int, double, int, double) {
  return false;
}


/**
 *
 */
template<class SwitchType>
void AnimChannelFixed<SwitchType>::
get_value(int, ValueType &value) {
  value = _value;
}


/**
 *
 */
template<class SwitchType>
void AnimChannelFixed<SwitchType>::
output(std::ostream &out) const {
  AnimChannel<SwitchType>::output(out);
  out << " = " << _value;
}
