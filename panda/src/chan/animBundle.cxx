// Filename: animBundle.cxx
// Created by:  drose (21Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "animBundle.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle AnimBundle::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimBundle::output
//       Access: Public, Virtual
//  Description: Writes a one-line description of the bundle.
////////////////////////////////////////////////////////////////////
void AnimBundle::
output(ostream &out) const {
  out << get_type() << " " << get_name() << ", " << get_num_frames() 
      << " frames at " << get_base_frame_rate() << " fps";
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundle::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void AnimBundle::
write_datagram(BamWriter *manager, Datagram &me)
{
  AnimGroup::write_datagram(manager, me);
  me.add_float64(_fps);
  me.add_uint16(_num_frames);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundle::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void AnimBundle::
fillin(DatagramIterator& scan, BamReader* manager)
{
  AnimGroup::fillin(scan, manager);
  _fps = scan.get_float64();
  _num_frames = scan.get_uint16();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundle::make_AnimBundle
//       Access: Protected
//  Description: Factory method to generate a AnimBundle object
////////////////////////////////////////////////////////////////////
TypedWriteable* AnimBundle::
make_AnimBundle(const FactoryParams &params)
{
  AnimBundle *me = new AnimBundle;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundle::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a AnimBundle object
////////////////////////////////////////////////////////////////////
void AnimBundle::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_AnimBundle);
}

