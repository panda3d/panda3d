// Filename: xFileTemplate.h
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

#ifndef XFILETEMPLATE_H
#define XFILETEMPLATE_H

#include "pandatoolbase.h"
#include "xFileNode.h"
#include "windowsGuid.h"

class XFileDataDef;

////////////////////////////////////////////////////////////////////
//       Class : XFileTemplate
// Description : A template definition in the X file.  This defines
//               the data structures that may be subsequently read.
////////////////////////////////////////////////////////////////////
class XFileTemplate : public XFileNode {
public:
  XFileTemplate(const string &name, const WindowsGuid &guid);

  virtual void write_text(ostream &out, int indent_level) const;

private:
  WindowsGuid _guid;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileNode::init_type();
    register_type(_type_handle, "XFileTemplate",
                  XFileNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileTemplate.I"

#endif
  


