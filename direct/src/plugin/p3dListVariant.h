// Filename: p3dListVariant.h
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

#ifndef P3DLISTVARIANT_H
#define P3DLISTVARIANT_H

#include "p3d_plugin_common.h"
#include "p3dVariant.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DListVariant
// Description : A variant type that contains a list of other variants.
////////////////////////////////////////////////////////////////////
class P3DListVariant : public P3DVariant {
public:
  P3DListVariant();
  P3DListVariant(P3DVariant * const elements[], int num_elements);
  P3DListVariant(const P3DListVariant &copy);

public:
  virtual ~P3DListVariant();

  virtual P3DVariant *make_copy(); 
  virtual bool get_bool() const;
  virtual void make_string(string &value) const;
  virtual int get_list_length() const;
  virtual P3DVariant *get_list_item(int n) const;

  void append_item(P3DVariant *item);

  virtual TiXmlElement *make_xml() const;

private:
  typedef vector<P3DVariant *> Elements;
  Elements _elements;
};

#endif

