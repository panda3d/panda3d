/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movingPart.I
 * @author drose
 * @date 1999-02-22
 */

#include "animChannelFixed.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

template<class SwitchType>
TypeHandle MovingPart<SwitchType>::_type_handle;

// We don't need to explicitly call MovingPart::init_type(), because it is an
// abstract class and therefore must have derived objects.  Its derived
// objects will call init_type() for us.



/**
 * Normally, you'd use make_copy() or copy_subgraph() to make a copy of this.
 */
template<class SwitchType>
INLINE MovingPart<SwitchType>::
MovingPart(const MovingPart<SwitchType> &copy) :
  MovingPartBase(copy),
  _value(copy._value),
  _default_value(copy._default_value)
{
}

/**
 *
 */
template<class SwitchType>
INLINE MovingPart<SwitchType>::
MovingPart(PartGroup *parent, const std::string &name,
           const ValueType &default_value) :
  MovingPartBase(parent, name),
  _value(default_value),
  _default_value(default_value)
{
}

/**
 *
 */
template<class SwitchType>
INLINE MovingPart<SwitchType>::
MovingPart() {
}

/**
 * Returns the TypeHandle associated with the ValueType we are concerned with.
 * This is provided to allow a bit of run-time checking that joints and
 * channels are matching properly in type.
 */
template<class SwitchType>
TypeHandle MovingPart<SwitchType>::
get_value_type() const {
  return get_type_handle(ValueType);
}



/**
 * Creates and returns a new AnimChannel that is not part of any hierarchy,
 * but that returns the default value associated with this part.
 */
template<class SwitchType>
AnimChannelBase *MovingPart<SwitchType>::
make_default_channel() const {
  return new AnimChannelFixed<SwitchType>(get_name(), _default_value);
}

/**
 * Outputs a very brief description of the channel's current value.
 */
template<class SwitchType>
void MovingPart<SwitchType>::
output_value(std::ostream &out) const {
  SwitchType::output_value(out, _value);
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
template<class SwitchType>
void MovingPart<SwitchType>::
write_datagram(BamWriter *manager, Datagram &me) {
  MovingPartBase::write_datagram(manager, me);
  SwitchType::write_datagram(me, _value);
  SwitchType::write_datagram(me, _default_value);
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
template<class SwitchType>
void MovingPart<SwitchType>::
fillin(DatagramIterator &scan, BamReader *manager) {
  MovingPartBase::fillin(scan, manager);
  SwitchType::read_datagram(scan, _value);
  SwitchType::read_datagram(scan, _default_value);
}
