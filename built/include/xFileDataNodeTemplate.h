/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataNodeTemplate.h
 * @author drose
 * @date 2004-10-03
 */

#ifndef XFILEDATANODETEMPLATE_H
#define XFILEDATANODETEMPLATE_H

#include "pandatoolbase.h"
#include "xFileDataNode.h"
#include "xFileTemplate.h"
#include "xFileParseData.h"
#include "pointerTo.h"
#include "pta_int.h"
#include "pta_double.h"

/**
 * This is a node which contains all of the data elements defined by a
 * template.  See XFileTemplate for the definition of the template; this class
 * only contains the data members for a particular instance of a template.
 */
class XFileDataNodeTemplate : public XFileDataNode {
public:
  XFileDataNodeTemplate(XFile *x_file, const std::string &name,
                        XFileTemplate *xtemplate);

  void zero_fill();

  virtual bool is_complex_object() const;

  void add_parse_double(PTA_double double_list);
  void add_parse_int(PTA_int int_list);
  void add_parse_string(const std::string &str);
  bool finalize_parse_data();

  virtual bool add_element(XFileDataObject *element);

  virtual void write_text(std::ostream &out, int indent_level) const;
  virtual void write_data(std::ostream &out, int indent_level,
                          const char *separator) const;

protected:
  virtual int get_num_elements() const;
  virtual XFileDataObject *get_element(int n);
  virtual XFileDataObject *get_element(const std::string &name);

private:
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
