// Filename: dcField.h
// Created by:  drose (11Oct00)
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

#ifndef DCFIELD_H
#define DCFIELD_H

#include "dcbase.h"
#include "dcPackerInterface.h"
#include "dcPython.h"

class DCPacker;
class DCAtomicField;
class DCMolecularField;
class DCParameter;
class DCSwitch;
class HashGenerator;

////////////////////////////////////////////////////////////////////
//       Class : DCField
// Description : A single field of a Distributed Class, either atomic
//               or molecular.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCField : public DCPackerInterface {
public:
  DCField(const string &name = string());
  virtual ~DCField();

PUBLISHED:
  int get_number() const;

  virtual DCField *as_field();
  virtual DCAtomicField *as_atomic_field();
  virtual DCMolecularField *as_molecular_field();
  virtual DCParameter *as_parameter();
  virtual DCSwitch *as_switch();

  string format_data(const string &packed_data);
  string parse_string(const string &formatted_string);

  bool validate_ranges(const string &packed_data) const;

  bool has_default_value() const;
  const string &get_default_value() const;

  bool is_required() const;
  bool is_broadcast() const;
  bool is_p2p() const;
  bool is_ram() const;
  bool is_db() const;
  bool is_clsend() const;
  bool is_clrecv() const;
  bool is_ownsend() const;
  bool is_airecv() const;

  bool compare_flags(const DCField &other) const;

#ifdef HAVE_PYTHON
  bool pack_args(DCPacker &packer, PyObject *sequence) const;
  PyObject *unpack_args(DCPacker &packer) const;

  void receive_update(DCPacker &packer, PyObject *distobj) const;

  Datagram client_format_update(int do_id, PyObject *args) const;
  Datagram ai_format_update(int do_id, int to_id, int from_id,
                            PyObject *args) const;
#endif 

public:
  virtual void output(ostream &out, bool brief) const=0;
  virtual void write(ostream &out, bool brief, int indent_level) const=0;
  virtual void generate_hash(HashGenerator &hashgen) const;
  virtual bool pack_default_value(DCPackData &pack_data, bool &pack_error) const;

  void set_number(int number);
  void set_default_value(const string &default_value);

  enum Flags {
    F_required        = 0x0001,
    F_broadcast       = 0x0002,
    F_p2p             = 0x0004,
    F_ram             = 0x0008,
    F_db              = 0x0010,
    F_clsend          = 0x0020,
    F_clrecv          = 0x0040,
    F_ownsend         = 0x0080,
    F_airecv          = 0x0100,
  };
  void add_flag(enum Flags flag);
  void set_flags(int flags);
  int get_flags() const;

protected:
  void refresh_default_value();
  void output_flags(ostream &out) const;

protected:
  int _number;
  bool _default_value_stale;
  bool _has_default_value;

private:
  int _flags;  // A bitmask union of any of the above values.

  string _default_value;
};

#endif
