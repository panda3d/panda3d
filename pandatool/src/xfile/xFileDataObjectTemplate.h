// Filename: xFileDataObjectTemplate.h
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

#ifndef XFILEDATAOBJECTTEMPLATE_H
#define XFILEDATAOBJECTTEMPLATE_H

#include "pandatoolbase.h"
#include "xFileDataObject.h"
#include "xFileTemplate.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : XFileDataObjectTemplate
// Description : A data element that represents a combination of
//               multiple data elements as defined by a template.  The
//               individual data elements of the template may be
//               obtained by walking through the children of this
//               object.
////////////////////////////////////////////////////////////////////
class XFileDataObjectTemplate : public XFileDataObject {
public:
  XFileDataObjectTemplate(XFileTemplate *xtemplate, const string &name);

  INLINE XFileTemplate *get_template() const;

  virtual void write_text(ostream &out, int indent_level) const;

private:
  PT(XFileTemplate) _template;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileDataObject::init_type();
    register_type(_type_handle, "XFileDataObjectTemplate",
                  XFileDataObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileDataObjectTemplate.I"

#endif
  



