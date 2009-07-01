// Filename: p3dVariant.h
// Created by:  drose (30Jun09)
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

#ifndef P3DVARIANT_H
#define P3DVARIANT_H

#include "p3d_plugin_common.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DVariant
// Description : The C++ implementation of P3D_variant, corresponding
//               to a single atomic value that is passed around
//               between scripting languages.  This is an abstract
//               base class; the actual implementations are provided
//               by the various specialized classes, below.
////////////////////////////////////////////////////////////////////
class P3DVariant : public P3D_variant {
protected:
  inline P3DVariant(P3D_variant_type type);
  inline P3DVariant(const P3DVariant &copy);

public:
  virtual ~P3DVariant();

  virtual P3DVariant *make_copy()=0; 
  virtual bool get_bool() const=0;
  virtual int get_int() const;
  virtual double get_float() const;

  int get_string_length() const;
  int extract_string(char *buffer, int buffer_length) const;
  virtual void make_string(string &value) const=0;

  virtual int get_list_length() const;
  virtual P3DVariant *get_list_item(int n) const;
};

#include "p3dVariant.I"

#endif

