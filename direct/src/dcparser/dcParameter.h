// Filename: dcParameter.h
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

#ifndef DCPARAMETER_H
#define DCPARAMETER_H

#include "dcbase.h"
#include "dcPackerInterface.h"

#ifdef HAVE_PYTHON

#undef HAVE_LONG_LONG  // NSPR and Python both define this.
#include <Python.h>

// We only need these headers if we are also building a Python interface.
#include "datagram.h"
#include "datagramIterator.h"

#endif  // HAVE_PYTHON

class DCSimpleParameter;
class DCClassParameter;
class DCArrayParameter;
class DCTypedefParameter;
class HashGenerator;

////////////////////////////////////////////////////////////////////
//       Class : DCParameter
// Description : Represents the type specification for a single
//               parameter within a field specification.  This may be
//               a simple type, or it may be a class or an array
//               reference.
//
//               This may also be a typedef reference to another type,
//               which has the same properties as the referenced type,
//               but a different name.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCParameter : public DCPackerInterface {
protected:
  DCParameter();
public:
  virtual ~DCParameter();

PUBLISHED:
  virtual DCSimpleParameter *as_simple_parameter();
  virtual DCClassParameter *as_class_parameter();
  virtual DCArrayParameter *as_array_parameter();
  virtual DCTypedefParameter *as_typedef_parameter();

  virtual DCParameter *make_copy() const=0;

public:
  virtual void output(ostream &out, bool brief) const=0;
  virtual void generate_hash(HashGenerator &hash) const;
};

#endif
