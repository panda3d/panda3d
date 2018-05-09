/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClientControlMessage.cxx
 * @author drose
 * @date 2000-07-09
 */

#include "config_pstatclient.h"
#include "pStatClientControlMessage.h"
#include "pStatClientVersion.h"

#include "datagram.h"
#include "datagramIterator.h"

/**
 *
 */
PStatClientControlMessage::
PStatClientControlMessage() {
  _type = T_invalid;
}

/**
 * Writes the message into the indicated datagram.
 */
void PStatClientControlMessage::
encode(Datagram &datagram) const {
  datagram.clear();
  datagram.add_uint8(_type);
  switch (_type) {
  case T_hello:
    datagram.add_string(_client_hostname);
    datagram.add_string(_client_progname);
    datagram.add_uint16(_major_version);
    datagram.add_uint16(_minor_version);
    break;

  case T_define_collectors:
    {
      datagram.add_uint16(_collectors.size());
      for (int i = 0; i < (int)_collectors.size(); i++) {
        _collectors[i]->write_datagram(datagram);
      }
    }
    break;

  case T_define_threads:
    {
      datagram.add_uint16(_first_thread_index);
      datagram.add_uint16(_names.size());
      for (int i = 0; i < (int)_names.size(); i++) {
        datagram.add_string(_names[i]);
      }
    }
    break;

  default:
    pstats_cat.error()
      << "Invalid PStatClientControlMessage::Type " << (int)_type << "\n";
  }
}

/**
 * Extracts the message from the indicated datagram.  Returns true on success,
 * false on error.
 */
bool PStatClientControlMessage::
decode(const Datagram &datagram, PStatClientVersion *version) {
  DatagramIterator source(datagram);
  _type = (Type)source.get_uint8();

  switch (_type) {
  case T_hello:
    _client_hostname = source.get_string();
    _client_progname = source.get_string();
    if (source.get_remaining_size() == 0) {
      _major_version = 1;
      _minor_version = 0;
    } else {
      _major_version = source.get_uint16();
      _minor_version = source.get_uint16();
    }
    break;

  case T_define_collectors:
    {
      int num = source.get_uint16();
      _collectors.clear();
      for (int i = 0; i < num; i++) {
        PStatCollectorDef *def = new PStatCollectorDef;
        def->read_datagram(source, version);
        _collectors.push_back(def);
      }
    }
    break;

  case T_define_threads:
    {
      _first_thread_index = source.get_uint16();
      int num = source.get_uint16();
      _names.clear();
      for (int i = 0; i < num; i++) {
        _names.push_back(source.get_string());
      }
    }
    break;

  case T_datagram:
    // Not, strictly speaking, a control message.
    return false;

  default:
    pstats_cat.error()
      << "Read invalid PStatClientControlMessage type: " << (int)_type << "\n";
    _type = T_invalid;
    return false;
  }

  return true;
}
