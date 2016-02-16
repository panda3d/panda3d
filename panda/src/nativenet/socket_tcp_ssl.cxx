/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file socket_tcp_ssl.cxx
 * @author drose
 * @date 2007-03-01
 */

#include "socket_tcp_ssl.h"

#ifdef HAVE_OPENSSL

SSL_CTX *global_ssl_ctx;
TypeHandle Socket_TCP_SSL::_type_handle;

#endif  // HAVE_OPENSSL
