// Filename: pStatClientControlMessage.cxx
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "config_pstats.h"
#include "pStatClientControlMessage.h"

#include <datagram.h>
#include <datagramIterator.h>

////////////////////////////////////////////////////////////////////
//     Function: PStatClientControlMessage::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PStatClientControlMessage::
PStatClientControlMessage() {
  _type = T_invalid;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientControlMessage::encode
//       Access: Public
//  Description: Writes the message into the indicated datagram.
////////////////////////////////////////////////////////////////////
void PStatClientControlMessage::
encode(Datagram &datagram) const {
  datagram.clear();
  datagram.add_uint8(_type);
  switch (_type) {
  case T_hello:
    datagram.add_string(_client_hostname);
    datagram.add_string(_client_progname);
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

////////////////////////////////////////////////////////////////////
//     Function: PStatClientControlMessage::decode
//       Access: Public
//  Description: Extracts the message from the indicated datagram.
//               Returns true on success, false on error.
////////////////////////////////////////////////////////////////////
bool PStatClientControlMessage::
decode(const Datagram &datagram) {
  DatagramIterator source(datagram);
  _type = (Type)source.get_uint8();
  
  switch (_type) {
  case T_hello:
    _client_hostname = source.get_string();
    _client_progname = source.get_string();
    break;

  case T_define_collectors:
    {
      int num = source.get_uint16();
      _collectors.clear();
      for (int i = 0; i < num; i++) {
	PStatCollectorDef *def = new PStatCollectorDef;
	def->read_datagram(source);
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

  default:
    pstats_cat.error()
      << "Read invalid PStatClientControlMessage type: " << (int)_type << "\n";
    _type = T_invalid;
    return false;
  }

  return true;
}
