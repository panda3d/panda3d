/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcField_ext.h
 * @author CFSworks
 * @date 2019-07-03
 */

#ifndef DCFIELD_EXT_H
#define DCFIELD_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "dcField.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for DCField, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<DCField> : public ExtensionBase<DCField> {
public:
  bool pack_args(DCPacker &packer, PyObject *sequence) const;
  PyObject *unpack_args(DCPacker &packer) const;

  void receive_update(DCPacker &packer, PyObject *distobj) const;

  Datagram client_format_update(DOID_TYPE do_id, PyObject *args) const;
  Datagram ai_format_update(DOID_TYPE do_id, CHANNEL_TYPE to_id, CHANNEL_TYPE from_id,
                            PyObject *args) const;
  Datagram ai_format_update_msg_type(DOID_TYPE do_id, CHANNEL_TYPE to_id, CHANNEL_TYPE from_id,
                            int msg_type, PyObject *args) const;

  static std::string get_pystr(PyObject *value);
};

#endif  // HAVE_PYTHON

#endif  // DCFIELD_EXT_H
