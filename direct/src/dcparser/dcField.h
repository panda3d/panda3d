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

#ifdef HAVE_PYTHON

#undef HAVE_LONG_LONG  // NSPR and Python both define this.
#include <Python.h>

// We only need these headers if we are also building a Python interface.
#include "datagram.h"
#include "datagramIterator.h"

#endif  // HAVE_PYTHON

class DCAtomicField;
class DCMolecularField;
class HashGenerator;

////////////////////////////////////////////////////////////////////
//       Class : DCField
// Description : A single field of a Distributed Class, either atomic
//               or molecular.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCField : public DCPackerInterface {
PUBLISHED:
  int get_number() const;
  const string &get_name() const;

  virtual DCAtomicField *as_atomic_field();
  virtual DCMolecularField *as_molecular_field();

#ifdef HAVE_PYTHON
  void pack_args(Datagram &datagram, PyObject *tuple) const;
  PyObject *unpack_args(DatagramIterator &iterator) const;

  void receive_update(PyObject *distobj, DatagramIterator &iterator) const;

  Datagram client_format_update(int do_id, PyObject *args) const;
  Datagram ai_format_update(int do_id, int to_id, int from_id, PyObject *args) const;
#endif 

public:
  DCField(const string &name);
  virtual ~DCField();
  virtual void write(ostream &out, bool brief, int indent_level) const=0;
  virtual void generate_hash(HashGenerator &hash) const;

protected:
  int _number;
  string _name;

public:
#ifdef HAVE_PYTHON
  virtual bool do_pack_args(Datagram &datagram, PyObject *tuple, int &index) const=0;
  virtual bool do_unpack_args(pvector<PyObject *> &args, DatagramIterator &iterator) const=0;
#endif

  friend class DCClass;
};

#endif
