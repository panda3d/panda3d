// Filename: movingPart.I
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

#include "animChannelFixed.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

template<class SwitchType>
TypeHandle MovingPart<SwitchType>::_type_handle;

// We don't need to explicitly call MovingPart::init_type(), because
// it is an abstract class and therefore must have derived objects.
// Its derived objects will call init_type() for us.



////////////////////////////////////////////////////////////////////
//     Function: MovingPart::Copy Constructor
//       Access: Protected
//  Description: Normally, you'd use make_copy() or copy_subgraph() to
//               make a copy of this.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
INLINE MovingPart<SwitchType>::
MovingPart(const MovingPart<SwitchType> &copy) :
  MovingPartBase(copy),
  _value(copy._value),
  _initial_value(copy._initial_value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPart::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class SwitchType>
INLINE MovingPart<SwitchType>::
MovingPart(PartGroup *parent, const string &name,
           const ValueType &initial_value) :
  MovingPartBase(parent, name),
  _value(initial_value),
  _initial_value(initial_value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPart::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class SwitchType>
INLINE MovingPart<SwitchType>::
MovingPart(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPart::get_value_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle associated with the ValueType
//               we are concerned with.  This is provided to allow a
//               bit of run-time checking that joints and channels are
//               matching properly in type.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
TypeHandle MovingPart<SwitchType>::
get_value_type() const {
  return get_type_handle(ValueType);
}



////////////////////////////////////////////////////////////////////
//     Function: MovingPart::make_initial_channel
//       Access: Public, Virtual
//  Description: Creates and returns a new AnimChannel that is not
//               part of any hierarchy, but that returns the default
//               value associated with this part.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
AnimChannelBase *MovingPart<SwitchType>::
make_initial_channel() const {
  return new AnimChannelFixed<SwitchType>(get_name(), _initial_value);
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPart::output_value
//       Access: Public, Virtual
//  Description: Outputs a very brief description of the channel's
//               current value.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void MovingPart<SwitchType>::
output_value(ostream &out) const {
  SwitchType::output_value(out, _value);
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPart::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void MovingPart<SwitchType>::
write_datagram(BamWriter *manager, Datagram &me)
{
  MovingPartBase::write_datagram(manager, me);
  SwitchType::write_datagram(me, _value);
  SwitchType::write_datagram(me, _initial_value);
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPart::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void MovingPart<SwitchType>::
fillin(DatagramIterator& scan, BamReader* manager)
{
  MovingPartBase::fillin(scan, manager);
  SwitchType::read_datagram(scan, _value);
  SwitchType::read_datagram(scan, _initial_value);
}



