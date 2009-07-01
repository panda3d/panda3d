// Filename: p3dListValue.h
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

#ifndef P3DLISTVALUE_H
#define P3DLISTVALUE_H

#include "p3d_plugin_common.h"
#include "p3dValue.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DListValue
// Description : A value type that contains a list of other values.
////////////////////////////////////////////////////////////////////
class P3DListValue : public P3DValue {
public:
  P3DListValue();
  P3DListValue(P3DValue * const elements[], int num_elements);
  P3DListValue(const P3DListValue &copy);

public:
  virtual ~P3DListValue();

  virtual P3DValue *make_copy(); 
  virtual bool get_bool() const;
  virtual void make_string(string &value) const;
  virtual int get_list_length() const;
  virtual P3DValue *get_list_item(int n) const;

  void append_item(P3DValue *item);

  virtual TiXmlElement *make_xml() const;

private:
  typedef vector<P3DValue *> Elements;
  Elements _elements;
};

#endif

