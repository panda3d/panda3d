// Filename: xFileDataNodeTemplate.h
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

#ifndef XFILEDATANODETEMPLATE_H
#define XFILEDATANODETEMPLATE_H

#include "pandatoolbase.h"
#include "xFileDataNode.h"
#include "xFileTemplate.h"
#include "xFileParseData.h"
#include "pointerTo.h"
#include "pta_int.h"
#include "pta_double.h"

////////////////////////////////////////////////////////////////////
//       Class : XFileDataNodeTemplate
// Description : A data element that represents a combination of
//               multiple data elements as defined by a template.  The
//               individual data elements of the template may be
//               obtained by walking through the children of this
//               object.
////////////////////////////////////////////////////////////////////
class XFileDataNodeTemplate : public XFileDataNode {
public:
  XFileDataNodeTemplate(XFile *x_file, const string &name,
                          XFileTemplate *xtemplate);

  INLINE XFileTemplate *get_template() const;

  virtual bool is_complex_object() const;

  void add_parse_double(PTA_double double_list);
  void add_parse_int(PTA_int int_list);
  void add_parse_string(const string &str);
  bool finalize_parse_data();

  virtual bool add_element(XFileDataObject *element);

  virtual void write_text(ostream &out, int indent_level) const;
  virtual void write_data(ostream &out, int indent_level,
                          const char *separator) const;

protected:
  virtual int get_num_elements() const;
  virtual const XFileDataObject *get_element(int n) const;
  virtual const XFileDataObject *get_element(const string &name) const;

private:
  PT(XFileTemplate) _template;

  XFileParseDataList _parse_data_list;
  
  typedef pvector< PT(XFileDataObject) > NestedElements;
  NestedElements _nested_elements;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileDataNode::init_type();
    register_type(_type_handle, "XFileDataNodeTemplate",
                  XFileDataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileDataNodeTemplate.I"

#endif
  



