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
#include "config_distributed.h"
#include "httpChannel.h"
#include "urlSpec.h"

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
CConnectionRepository::
CConnectionRepository() :
#ifdef HAVE_SSL
  _http_conn(NULL),
#endif
#ifdef HAVE_NSPR
  _cw(&_qcm, 0),
  _qcr(&_qcm, 0),
#endif
  _simulated_disconnect(false)
{
#ifdef HAVE_NSPR
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
//               datagram may be retrieved via get_datagram().
////////////////////////////////////////////////////////////////////
bool CConnectionRepository::
check_datagram() {
  if (_simulated_disconnect) {
    return false;
  }

  return do_check_datagram();
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
