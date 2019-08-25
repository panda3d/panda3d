/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcClass_ext.h
 * @author CFSworks
 * @date 2019-07-03
 */

#ifndef DCCLASS_EXT_H
#define DCCLASS_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "dcClass.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for DCClass, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<DCClass> : public ExtensionBase<DCClass> {
public:
  bool has_class_def() const;
  void set_class_def(PyObject *class_def);
  PyObject *get_class_def() const;
  bool has_owner_class_def() const;
  void set_owner_class_def(PyObject *owner_class_def);
  PyObject *get_owner_class_def() const;

  void receive_update(PyObject *distobj, DatagramIterator &di) const;
  void receive_update_broadcast_required(PyObject *distobj, DatagramIterator &di) const;
  void receive_update_broadcast_required_owner(PyObject *distobj, DatagramIterator &di) const;
  void receive_update_all_required(PyObject *distobj, DatagramIterator &di) const;
  void receive_update_other(PyObject *distobj, DatagramIterator &di) const;

  void direct_update(PyObject *distobj, const std::string &field_name,
                     const vector_uchar &value_blob);
  void direct_update(PyObject *distobj, const std::string &field_name,
                     const Datagram &datagram);
  bool pack_required_field(Datagram &datagram, PyObject *distobj,
                           const DCField *field) const;
  bool pack_required_field(DCPacker &packer, PyObject *distobj,
                           const DCField *field) const;



  Datagram client_format_update(const std::string &field_name,
                                DOID_TYPE do_id, PyObject *args) const;
  Datagram ai_format_update(const std::string &field_name, DOID_TYPE do_id,
                            CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, PyObject *args) const;
  Datagram ai_format_update_msg_type(const std::string &field_name, DOID_TYPE do_id,
                            CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, int msg_type, PyObject *args) const;
  Datagram ai_format_generate(PyObject *distobj, DOID_TYPE do_id,
                              ZONEID_TYPE parent_id, ZONEID_TYPE zone_id,
                              CHANNEL_TYPE district_channel_id,
                              CHANNEL_TYPE from_channel_id,
                              PyObject *optional_fields) const;
  Datagram client_format_generate_CMU(PyObject *distobj, DOID_TYPE do_id,
                                      ZONEID_TYPE zone_id,
                                      PyObject *optional_fields) const;

private:
  /**
   * Implementation of DCClass::PythonClassDefs which actually stores the
   * Python pointers.  This needs to be defined here rather than on DCClass
   * itself, since DCClass cannot include Python.h or call Python functions.
   */
  class PythonClassDefsImpl : public DCClass::PythonClassDefs {
  public:
    virtual ~PythonClassDefsImpl() {
      Py_XDECREF(_class_def);
      Py_XDECREF(_owner_class_def);
    }

    PyObject *_class_def = nullptr;
    PyObject *_owner_class_def = nullptr;
  };

  PythonClassDefsImpl *do_get_defs() const;
};

#endif  // HAVE_PYTHON

#endif  // DCCLASS_EXT_H
