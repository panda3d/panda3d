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
#include "pta_int.h"
#include "pta_double.h"

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
  XFileDataObjectTemplate(XFile *x_file, const string &name,
                          XFileTemplate *xtemplate);

  INLINE XFileTemplate *get_template() const;

  virtual void write_text(ostream &out, int indent_level) const;

public:
  void add_parse_object(XFileDataObjectTemplate *object, bool reference);
  void add_parse_double(PTA_double double_list, char separator);
  void add_parse_int(PTA_int int_list, char separator);
  void add_parse_string(const string &str, char separator);
  void add_parse_separator(char separator);
  bool finalize_parse_data();

private:
  PT(XFileTemplate) _template;

  // This class is used to fill up the data as the values are parsed.
  // It only has a temporary lifespan; it will be converted into
  // actual data by finalize_parse_data().
  enum ParseFlags {
    PF_object     = 0x001,
    PF_reference  = 0x002,
    PF_double     = 0x004,
    PF_int        = 0x008,
    PF_string     = 0x010,
    PF_comma      = 0x020,
    PF_semicolon  = 0x040,
  };

  class ParseData {
  public:
    PT(XFileDataObjectTemplate) _object;
    PTA_double _double_list;
    PTA_int _int_list;
    string _string;
    int _parse_flags;
  };

  typedef pvector<ParseData> ParseDataList;
  ParseDataList _parse_data_list;
  
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
  



