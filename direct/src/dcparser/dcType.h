// Filename: dcType.h
// Created by:  drose (15Jun04)
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

#ifndef DCTYPE_H
#define DCTYPE_H

#include "dcbase.h"
#include "dcPackerInterface.h"

#ifdef HAVE_PYTHON

#undef HAVE_LONG_LONG  // NSPR and Python both define this.
#include <Python.h>

// We only need these headers if we are also building a Python interface.
#include "datagram.h"
#include "datagramIterator.h"

#endif  // HAVE_PYTHON

class DCSimpleType;
class DCClassType;
class DCArrayType;
class DCTypedefType;
class HashGenerator;

////////////////////////////////////////////////////////////////////
//       Class : DCType
// Description : Represents the type specification for a single
//               parameter within a field specification.  This may be
//               a simple type, or it may be a class or an array
//               reference.
//
//               This may also be a typedef reference to another type,
//               which has the same properties as the referenced type,
//               but a different name.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCType : public DCPackerInterface {
protected:
  DCType();
public:
  virtual ~DCType();

PUBLISHED:
  virtual string get_name() const;

  virtual DCSimpleType *as_simple_type();
  virtual DCClassType *as_class_type();
  virtual DCArrayType *as_array_type();
  virtual DCTypedefType *as_typedef_type();

  virtual DCType *make_copy() const=0;

public:
  virtual void output(ostream &out, const string &parameter_name, 
                      bool brief) const=0;
  virtual void generate_hash(HashGenerator &hash) const;

#ifdef HAVE_PYTHON
  virtual void pack_arg(Datagram &datagram, PyObject *item) const=0;
  virtual PyObject *unpack_arg(DatagramIterator &iterator) const=0;
#endif

};

#endif
