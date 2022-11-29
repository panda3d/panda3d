/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClient_ext.h
 * @author rdb
 * @date 2022-11-23
 */

#ifndef PSTATCLIENT_EXT_H
#define PSTATCLIENT_EXT_H

#include "dtoolbase.h"

#if defined(HAVE_PYTHON) && defined(DO_PSTATS)

#include "extension.h"
#include "pStatClient.h"
#include "py_panda.h"

typedef struct _frame PyFrameObject;

/**
 * This class defines the extension methods for PStatClient, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<PStatClient> : public ExtensionBase<PStatClient> {
public:
  INLINE static bool connect(const std::string &hostname = std::string(), int port = -1);
  INLINE static void disconnect();

  bool client_connect(std::string hostname, int port);
  void client_disconnect();

private:
  static int trace_callback(PyObject *py_thread, PyFrameObject *frame,
                            int what, PyObject *arg);
};

#include "pStatClient_ext.I"

#endif  // HAVE_PYTHON && DO_PSTATS

#endif  // PSTATCLIENT_EXT_H
