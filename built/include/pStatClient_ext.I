/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClient_ext.I
 * @author rdb
 * @date 2022-11-29
 */

/**
 * Attempts to establish a connection to the indicated PStatServer.  Returns
 * true if successful, false on failure.
 */
INLINE bool Extension<PStatClient>::
connect(const std::string &hostname, int port) {
  PStatClient *client = PStatClient::get_global_pstats();
  return invoke_extension<PStatClient>(client).client_connect(hostname, port);
}

/**
 * Closes the connection previously established.
 */
INLINE void Extension<PStatClient>::
disconnect() {
  PStatClient *client = PStatClient::get_global_pstats();
  invoke_extension<PStatClient>(client).client_disconnect();
}
