// Filename: pStatCollectorDef.cxx
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "pStatCollectorDef.h"

#include <datagram.h>
#include <datagramIterator.h>


////////////////////////////////////////////////////////////////////
//     Function: PStatCollectorDef::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PStatCollectorDef::
PStatCollectorDef() {
  _index = 0;
  _parent_index = 0;
  _suggested_color.set(0.0, 0.0, 0.0);
  _sort = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollectorDef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PStatCollectorDef::
PStatCollectorDef(int index, const string &name) : 
  _index(index),
  _name(name)
{
  _parent_index = 0;
  _suggested_color.set(0.0, 0.0, 0.0);
  _sort = -1;
}


////////////////////////////////////////////////////////////////////
//     Function: PStatCollectorDef::write_datagram
//       Access: Public
//  Description: Writes the definition of the collectorDef to the
//               datagram.
////////////////////////////////////////////////////////////////////
void PStatCollectorDef::
write_datagram(Datagram &destination) const {
  destination.add_int16(_index);
  destination.add_string(_name);
  destination.add_int16(_parent_index);
  _suggested_color.write_datagram(destination);
  destination.add_int16(_sort);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollectorDef::read_datagram
//       Access: Public
//  Description: Extracts the collectorDef definition from the datagram.
////////////////////////////////////////////////////////////////////
void PStatCollectorDef::
read_datagram(DatagramIterator &source) {
  _index = source.get_int16();
  _name = source.get_string();
  _parent_index = source.get_int16();
  _suggested_color.read_datagram(source);

  if (source.get_remaining_size() > 0) {
    _sort = source.get_int16();
  }
}
