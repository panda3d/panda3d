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

#ifdef HAVE_PYTHON
  bool pack_args(Datagram &datagram, PyObject *sequence) const;
  PyObject *unpack_args(DatagramIterator &iterator) const;

  void receive_update(PyObject *distobj, DatagramIterator &iterator) const;

  Datagram client_format_update(int do_id, PyObject *args) const;
  Datagram ai_format_update(int do_id, int to_id, int from_id, PyObject *args) const;
#endif 

public:
  virtual void write(ostream &out, bool brief, int indent_level) const=0;
  virtual void generate_hash(HashGenerator &hashgen) const;

  void set_number(int number);

protected:
  int _number;
};

#endif
