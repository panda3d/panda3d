// Filename: dcPackerInterface.h
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

#ifndef DCPACKERINTERFACE_H
#define DCPACKERINTERFACE_H

#include "dcbase.h"
#include "dcSubatomicType.h"

class DCPackData;

////////////////////////////////////////////////////////////////////
//       Class : DCPackerInterface
// Description : This defines the internal interface for packing
//               values into a DCField.  The various different DC
//               objects inherit from this.  
//
//               Normally these methods are called only by the
//               DCPacker object; the user wouldn't normally call
//               these directly.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCPackerInterface {
public:
  virtual ~DCPackerInterface();

  virtual bool has_nested_fields() const;
  virtual int get_num_nested_fields() const;
  virtual DCPackerInterface *get_nested_field(int n) const;
  virtual size_t get_length_bytes() const;

  virtual DCSubatomicType get_pack_type() const;
  virtual bool pack_double(DCPackData &pack_data, double value) const;
  virtual bool pack_int(DCPackData &pack_data, int value) const;
  virtual bool pack_int64(DCPackData &pack_data, PN_int64 value) const;
  virtual bool pack_string(DCPackData &pack_data, const string &value) const;
};

#endif
