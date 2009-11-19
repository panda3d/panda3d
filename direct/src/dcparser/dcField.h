// Filename: dcField.h
// Created by:  drose (11Oct00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef DCFIELD_H
#define DCFIELD_H

#include "dcbase.h"
#include "dcPackerInterface.h"
#include "dcKeywordList.h"
#include "dcPython.h"

#ifdef WITHIN_PANDA
#include "pStatCollector.h"
#endif

class DCPacker;
class DCAtomicField;
class DCMolecularField;
class DCParameter;
class DCSwitch;
class DCClass;
class HashGenerator;

////////////////////////////////////////////////////////////////////
//       Class : DCField
// Description : A single field of a Distributed Class, either atomic
//               or molecular.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCField : public DCPackerInterface, public DCKeywordList {
public:
  DCField();
  DCField(const string &name, DCClass *dclass);
  virtual ~DCField();

PUBLISHED:
  INLINE int get_number() const;
  INLINE DCClass *get_class() const;

  virtual DCField *as_field();
  virtual const DCField *as_field() const;
  virtual DCAtomicField *as_atomic_field();
  virtual const DCAtomicField *as_atomic_field() const;
  virtual DCMolecularField *as_molecular_field();
  virtual const DCMolecularField *as_molecular_field() const;
  virtual DCParameter *as_parameter();
  virtual const DCParameter *as_parameter() const;

  string format_data(const string &packed_data, bool show_field_names = true);
  string parse_string(const string &formatted_string);

  bool validate_ranges(const string &packed_data) const;

  INLINE bool has_default_value() const;
  INLINE const string &get_default_value() const;

  INLINE bool is_bogus_field() const;

  INLINE bool is_required() const;
  INLINE bool is_broadcast() const;
  INLINE bool is_ram() const;
  INLINE bool is_db() const;
  INLINE bool is_clsend() const;
  INLINE bool is_clrecv() const;
  INLINE bool is_ownsend() const;
  INLINE bool is_ownrecv() const;
  INLINE bool is_airecv() const;

  INLINE void output(ostream &out) const;
  INLINE void write(ostream &out, int indent_level) const;

#ifdef HAVE_PYTHON
  bool pack_args(DCPacker &packer, PyObject *sequence) const;
  PyObject *unpack_args(DCPacker &packer) const;

  void receive_update(DCPacker &packer, PyObject *distobj) const;

  Datagram client_format_update(DOID_TYPE do_id, PyObject *args) const;
  Datagram ai_format_update(DOID_TYPE do_id, CHANNEL_TYPE to_id, CHANNEL_TYPE from_id,
                            PyObject *args) const;
  Datagram ai_format_update_msg_type(DOID_TYPE do_id, CHANNEL_TYPE to_id, CHANNEL_TYPE from_id,
                            int msg_type, PyObject *args) const;
#endif 

public:
  virtual void output(ostream &out, bool brief) const=0;
  virtual void write(ostream &out, bool brief, int indent_level) const=0;
  virtual void generate_hash(HashGenerator &hashgen) const;
  virtual bool pack_default_value(DCPackData &pack_data, bool &pack_error) const;
  virtual void set_name(const string &name);

  INLINE void set_number(int number);
  INLINE void set_class(DCClass *dclass);
  INLINE void set_default_value(const string &default_value);

#ifdef HAVE_PYTHON
  static string get_pystr(PyObject *value);
#endif

protected:
  void refresh_default_value();

protected:
  DCClass *_dclass;
  int _number;
  bool _default_value_stale;
  bool _has_default_value;
  bool _bogus_field;

private:
  string _default_value;

#ifdef WITHIN_PANDA
  PStatCollector _field_update_pcollector;
#endif
};

INLINE ostream &operator << (ostream &out, const DCField &field) {
  field.output(out);
  return out;
}

#include "dcField.I"

#endif
