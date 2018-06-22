/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataNodeReference.h
 * @author drose
 * @date 2004-10-08
 */

#ifndef XFILEDATANODEREFERENCE_H
#define XFILEDATANODEREFERENCE_H

#include "pandatoolbase.h"
#include "xFileDataNodeTemplate.h"
#include "pointerTo.h"

/**
 * This is a nested reference to an instance of a template object, declared
 * via the syntax:
 *
 * { InstanceName }
 *
 * in the X File.
 */
class XFileDataNodeReference : public XFileDataNode {
public:
  XFileDataNodeReference(XFileDataNodeTemplate *object);

  INLINE XFileTemplate *get_template() const;
  INLINE XFileDataNodeTemplate *get_object() const;

  virtual bool is_reference() const;
  virtual bool is_complex_object() const;

  virtual void write_text(std::ostream &out, int indent_level) const;

protected:
  virtual int get_num_elements() const;
  virtual XFileDataObject *get_element(int n);
  virtual XFileDataObject *get_element(const std::string &name);

private:
  PT(XFileDataNodeTemplate) _object;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileDataNode::init_type();
    register_type(_type_handle, "XFileDataNodeReference",
                  XFileDataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileDataNodeReference.I"

#endif
