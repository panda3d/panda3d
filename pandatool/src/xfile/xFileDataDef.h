// Filename: xFileDataDef.h
// Created by:  drose (03Oct04)
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

#ifndef XFILEDATADEF_H
#define XFILEDATADEF_H

#include "pandatoolbase.h"
#include "namable.h"
#include "xFileNode.h"
#include "xFileArrayDef.h"
#include "xFileTemplate.h"
#include "pvector.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : XFileDataDef
// Description : A definition of a single data element appearing
//               within a template record.
////////////////////////////////////////////////////////////////////
class XFileDataDef : public XFileNode {
public:
  enum Type {
    T_word,
    T_dword,
    T_float,
    T_double,
    T_char,
    T_uchar,
    T_sword,
    T_sdword,
    T_string,
    T_cstring,
    T_unicode,
    T_template,
  };

  INLINE XFileDataDef(Type type, const string &name, 
                      XFileTemplate *xtemplate = NULL);
  virtual ~XFileDataDef();

  virtual void clear();
  void add_array_def(const XFileArrayDef &array_def);

  virtual void write_text(ostream &out, int indent_level) const;

private:
  Type _type;
  PT(XFileTemplate) _template;
  
  typedef pvector<XFileArrayDef> ArrayDef;
  ArrayDef _array_def;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileNode::init_type();
    register_type(_type_handle, "XFileDataDef",
                  XFileNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileDataDef.I"

#endif
  


