/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buffered_datagramconnection.cxx
 * @author drose
 * @date 2007-03-05
 */

#include "buffered_datagramconnection.h"

TypeHandle Buffered_DatagramConnection::_type_handle;

/**
 * send the message
 */
bool Buffered_DatagramConnection::
SendMessage(const Datagram &msg) {
  if (IsConnected()) {
    // printf(" DO SendMessage %d\n",msg.get_length());

    int val = _Writer.AddData(msg.get_data(), msg.get_length(), *this);
    if (val >= 0) {
      return true;
    }

    // Raise an exception to give us more information at the python level
    nativenet_cat.warning() << "Buffered_DatagramConnection::SendMessage->Error On Write--Out Buffer = " << _Writer.AmountBuffered() << "\n";

    ClearAll();
  }

  return false;
}
