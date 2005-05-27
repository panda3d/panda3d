// Filename: cConnectionRepository.cxx
// Created by:  drose (17May04)
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

#include "cConnectionRepository.h"
#include "dcmsgtypes.h"
#include "dcClass.h"
#include "dcPacker.h"

#include "config_distributed.h"
#include "httpChannel.h"
#include "urlSpec.h"
#include "datagramIterator.h"
#include "throw_event.h"
#include "pStatTimer.h"


const string CConnectionRepository::_overflow_event_name = "CRDatagramOverflow";

#ifndef CPPPARSER
PStatCollector CConnectionRepository::_update_pcollector("App:Show code:readerPollTask:Update");
#endif  // CPPPARSER

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
CConnectionRepository::
CConnectionRepository() :
#ifdef HAVE_PYTHON
  _python_repository(NULL),
#endif
#ifdef HAVE_SSL
  _http_conn(NULL),
#endif
#ifdef HAVE_NSPR
  _cw(&_qcm, 0),
  _qcr(&_qcm, 0),
#endif
  _client_datagram(true),
  _simulated_disconnect(false),
  _verbose(distributed_cat.is_spam()),
  _msg_channel(0),
  _msg_sender(0),
  _sec_code(0),
  _msg_type(0)
{
#if defined(HAVE_NSPR) && defined(SIMULATE_NETWORK_DELAY)
  if (min_lag != 0.0 || max_lag != 0.0) {
    _qcr.start_delay(min_lag, max_lag);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
CConnectionRepository::
~CConnectionRepository() {
  disconnect();
}

#ifdef HAVE_SSL
////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::set_connection_http
//       Access: Published
//  Description: Once a connection has been established via the HTTP
//               interface, gets the connection and uses it.  The
//               supplied HTTPChannel object must have a connection
//               available via get_connection().
////////////////////////////////////////////////////////////////////
void CConnectionRepository::
set_connection_http(HTTPChannel *channel) {
  disconnect();
  nassertv(channel->is_connection_ready());
  _http_conn = channel->get_connection();
}
#endif  // HAVE_SSL


#ifdef HAVE_NSPR
////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::try_connect_nspr
//       Access: Published
//  Description: Uses NSPR to try to connect to the server and port
//               named in the indicated URL.  Returns true if
//               successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool CConnectionRepository::
try_connect_nspr(const URLSpec &url) {
  disconnect();

  _nspr_conn = 
    _qcm.open_TCP_client_connection(url.get_server(), url.get_port(),
                                    game_server_timeout_ms);

  if (_nspr_conn != (Connection *)NULL) {
    _nspr_conn->set_no_delay(true);
    _qcr.add_connection(_nspr_conn);
    return true;
  }

  return false;
}
#endif  // HAVE_NSPR

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::check_datagram
//       Access: Published
//  Description: Returns true if a new datagram is available, false
//               otherwise.  If the return value is true, the new
//               datagram may be retrieved via get_datagram(), or
//               preferably, with get_datagram_iterator() and
//               get_msg_type().
////////////////////////////////////////////////////////////////////
bool CConnectionRepository::
check_datagram() {
  if (_simulated_disconnect) {
    return false;
  }
  while (do_check_datagram()) {
#ifndef NDEBUG
    if (get_verbose()) {
      describe_message(nout, "receive ", _dg.get_message());
    }
#endif  // NDEBUG

    // Start breaking apart the datagram.
    _di = DatagramIterator(_dg);

    if (!_client_datagram) {
      _msg_channel = _di.get_uint64();
      _msg_sender = _di.get_uint64();
      _sec_code = _di.get_uint8();
      
#ifdef HAVE_PYTHON
      // For now, we need to stuff this field onto the Python
      // structure, to support legacy code that expects to find it
      // there.
      if (_python_repository != (PyObject *)NULL) {
        PyObject *value = PyLong_FromUnsignedLongLong(_msg_sender);
        PyObject_SetAttrString(_python_repository, "msgSender", value);
        Py_DECREF(value);
      }
#endif  // HAVE_PYTHON
    }

    _msg_type = _di.get_uint16();
    // Is this a message that we can process directly?
    switch (_msg_type) {
#ifdef HAVE_PYTHON
    case CLIENT_OBJECT_UPDATE_FIELD:
    case STATESERVER_OBJECT_UPDATE_FIELD:
      if (!handle_update_field()) {
        return false;
      }
      break;
#endif  // HAVE_PYTHON

    default:
      // Some unknown message; let the caller deal with it.
      return true;
    }
  }

  // No datagrams available.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::is_connected
//       Access: Published
//  Description: Returns true if the connection to the gameserver is
//               established and still good, false if we are not
//               connected.  A false value means either (a) we never
//               successfully connected, (b) we explicitly called
//               disconnect(), or (c) we were connected, but the
//               connection was spontaneously lost.
////////////////////////////////////////////////////////////////////
bool CConnectionRepository::
is_connected() {
#ifdef HAVE_NSPR
  if (_nspr_conn) {
    if (_qcm.reset_connection_available()) {
      PT(Connection) reset_connection;
      if (_qcm.get_reset_connection(reset_connection)) {
        _qcm.close_connection(reset_connection);
        if (reset_connection == _nspr_conn) {
          // Whoops, lost our connection.
          _nspr_conn = NULL;
          return false;
        }
      }
    }
    return true;
  }
#endif  // HAVE_NSPR

#ifdef HAVE_SSL
  if (_http_conn) {
    if (!_http_conn->is_closed()) {
      return true;
    }

    // Connection lost.
    delete _http_conn;
    _http_conn = NULL;
  }
#endif  // HAVE_SSL

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::send_datagram
//       Access: Published
//  Description: Queues the indicated datagram for sending to the
//               server.  It may not get send immediately if
//               collect_tcp is in effect; call flush() to guarantee
//               it is sent now.
////////////////////////////////////////////////////////////////////
bool CConnectionRepository::
send_datagram(const Datagram &dg) {
  if (_simulated_disconnect) {
    distributed_cat.warning()
      << "Unable to send datagram during simulated disconnect.\n";
    return false;
  }

#ifndef NDEBUG
  if (get_verbose()) {
    describe_message(nout, "send ", dg.get_message());
  }
#endif  // NDEBUG

#ifdef HAVE_NSPR
  if (_nspr_conn) {
    _cw.send(dg, _nspr_conn);
    return true;
  }
#endif  // HAVE_NSPR

#ifdef HAVE_SSL
  if (_http_conn) {
    if (!_http_conn->send_datagram(dg)) {
      distributed_cat.warning()
        << "Could not send datagram.\n";
      return false;
    }

    return true;
  }
#endif  // HAVE_SSL

  distributed_cat.warning()
    << "Unable to send datagram after connection is closed.\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::consider_flush
//       Access: Published
//  Description: Sends the most recently queued data if enough time
//               has elapsed.  This only has meaning if
//               set_collect_tcp() has been set to true.
////////////////////////////////////////////////////////////////////
bool CConnectionRepository::
consider_flush() {
  if (_simulated_disconnect) {
    return false;
  }

#ifdef HAVE_NSPR
  if (_nspr_conn) {
    return _nspr_conn->consider_flush();
  }
#endif  // HAVE_NSPR

#ifdef HAVE_SSL
  if (_http_conn) {
    return _http_conn->consider_flush();
  }
#endif  // HAVE_SSL

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::flush
//       Access: Published
//  Description: Sends the most recently queued data now.  This only
//               has meaning if set_collect_tcp() has been set to
//               true.
////////////////////////////////////////////////////////////////////
bool CConnectionRepository::
flush() {
  if (_simulated_disconnect) {
    return false;
  }

#ifdef HAVE_NSPR
  if (_nspr_conn) {
    return _nspr_conn->flush();
  }
#endif  // HAVE_NSPR

#ifdef HAVE_SSL
  if (_http_conn) {
    return _http_conn->flush();
  }
#endif  // HAVE_SSL

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::disconnect
//       Access: Published
//  Description: Closes the connection to the server.
////////////////////////////////////////////////////////////////////
void CConnectionRepository::
disconnect() {
#ifdef HAVE_NSPR
  if (_nspr_conn) {
    _qcm.close_connection(_nspr_conn);
    _nspr_conn = NULL;
  }
#endif  // HAVE_NSPR

#ifdef HAVE_SSL
  if (_http_conn) {
    _http_conn->close();
    delete _http_conn;
    _http_conn = NULL;
  }
#endif  // HAVE_SSL

  _simulated_disconnect = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::do_check_datagram
//       Access: Private
//  Description: The private implementation of check_datagram(), this
//               gets one datagram if it is available.
////////////////////////////////////////////////////////////////////
bool CConnectionRepository::
do_check_datagram() {
#ifdef HAVE_NSPR
  if (_nspr_conn) {
    _nspr_conn->consider_flush();
    if (_qcr.get_overflow_flag()) {
      throw_event(get_overflow_event_name());
      _qcr.reset_overflow_flag();
    }
    return (_qcr.data_available() && _qcr.get_data(_dg));
  }
#endif  // HAVE_NSPR

#ifdef HAVE_SSL
  if (_http_conn) {
    _http_conn->consider_flush();
    return _http_conn->receive_datagram(_dg);
  }
#endif  // HAVE_SSL

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::handle_update_field
//       Access: Private
//  Description: Directly handles an update message on a field.
//               Python never touches the datagram; it just gets its
//               distributed method called with the appropriate
//               parameters.  Returns true if everything is ok, false
//               if there was an error processing the field's update
//               method.
////////////////////////////////////////////////////////////////////
bool CConnectionRepository::
handle_update_field() {
#ifdef HAVE_PYTHON
  PStatTimer timer(_update_pcollector);
  unsigned int do_id = _di.get_uint32();
  if (_python_repository != (PyObject *)NULL) {
    PyObject *doId2do =
      PyObject_GetAttrString(_python_repository, "doId2do");
    nassertr(doId2do != NULL, false);

    #ifdef USE_PYTHON_2_2_OR_EARLIER
    PyObject *doId = PyInt_FromLong(do_id);
    #else
    PyObject *doId = PyLong_FromUnsignedLong(do_id);
    #endif
    PyObject *distobj = PyDict_GetItem(doId2do, doId);
    Py_DECREF(doId);
    Py_DECREF(doId2do);

    if (distobj != NULL) {
      PyObject *dclass_obj = PyObject_GetAttrString(distobj, "dclass");
      nassertr(dclass_obj != NULL, false);

      PyObject *dclass_this = PyObject_GetAttrString(dclass_obj, "this");
      Py_DECREF(dclass_obj);
      nassertr(dclass_this != NULL, false);

      DCClass *dclass = (DCClass *)PyInt_AsLong(dclass_this);
      Py_DECREF(dclass_this);

      // It's a good idea to ensure the reference count to distobj is
      // raised while we call the update method--otherwise, the update
      // method might get into trouble if it tried to delete the
      // object from the doId2do map.
      Py_INCREF(distobj);
      dclass->receive_update(distobj, _di); 
      Py_DECREF(distobj);
      
      if (PyErr_Occurred()) {
        return false;
      }
    }
  }
#endif  // HAVE_PYTHON  

  return true;
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::describe_message
//       Access: Private
//  Description: Unpacks the message and reformats it for user
//               consumption, writing a description on the indicated
//               output stream.
////////////////////////////////////////////////////////////////////
void CConnectionRepository::
describe_message(ostream &out, const string &prefix, 
                 const string &message_data) const {
  DCPacker packer;
  packer.set_unpack_data(message_data);
  CHANNEL_TYPE do_id;
  int msg_type;
  bool is_update = false;

  if (!_client_datagram) {
    packer.RAW_UNPACK_CHANNEL();  // msg_channel
    packer.RAW_UNPACK_CHANNEL();  // msg_sender
    packer.raw_unpack_uint8();    // sec_code
    msg_type = packer.raw_unpack_uint16();
    is_update = (msg_type == STATESERVER_OBJECT_UPDATE_FIELD);
    
  } else {
    msg_type = packer.raw_unpack_uint16();
    is_update = (msg_type == CLIENT_OBJECT_UPDATE_FIELD);
  }

  if (!is_update) {
    out << prefix << "message " << msg_type << "\n";

  } else {
    // It's an update message.  Figure out what dclass the object is
    // based on its doId, so we can decode the rest of the message.
    do_id = packer.raw_unpack_uint32();
    DCClass *dclass = NULL;

#ifdef HAVE_PYTHON
    if (_python_repository != (PyObject *)NULL) {
      PyObject *doId2do =
        PyObject_GetAttrString(_python_repository, "doId2do");
      nassertv(doId2do != NULL);

      #ifdef USE_PYTHON_2_2_OR_EARLIER
      PyObject *doId = PyInt_FromLong(do_id);
      #else
      PyObject *doId = PyLong_FromUnsignedLong(do_id);
      #endif
      PyObject *distobj = PyDict_GetItem(doId2do, doId);
      Py_DECREF(doId);
      Py_DECREF(doId2do);

      if (distobj != NULL) {
        PyObject *dclass_obj = PyObject_GetAttrString(distobj, "dclass");
        nassertv(dclass_obj != NULL);

        PyObject *dclass_this = PyObject_GetAttrString(dclass_obj, "this");
        Py_DECREF(dclass_obj);
        nassertv(dclass_this != NULL);
        
        dclass = (DCClass *)PyInt_AsLong(dclass_this);
        Py_DECREF(dclass_this);
      }
    }
#endif  // HAVE_PYTHON  

    int field_id = packer.raw_unpack_uint16();

    if (dclass == (DCClass *)NULL) {
      out << prefix << "update for unknown object " << do_id 
          << ", field " << field_id << "\n";

    } else {
      out << prefix << "update for " << dclass->get_name()
          << " " << do_id << ": ";
      DCField *field = dclass->get_field_by_index(field_id);
      if (field == (DCField *)NULL) {
        out << "unknown field " << field_id << "\n";
        
      } else {
        out << field->get_name();
        packer.begin_unpack(field);
        packer.unpack_and_format(out);
        packer.end_unpack();
        out << "\n";
      }
    }
  }
}
#endif  // NDEBUG

