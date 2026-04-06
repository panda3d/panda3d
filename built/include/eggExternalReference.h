/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggExternalReference.h
 * @author drose
 * @date 1999-02-11
 */

#ifndef EGGEXTERNALREFERENCE_H
#define EGGEXTERNALREFERENCE_H

#include "pandabase.h"

#include "eggFilenameNode.h"

/**
 * Defines a reference to another egg file which should be inserted at this
 * point.
 */
class EXPCL_PANDA_EGG EggExternalReference : public EggFilenameNode {
PUBLISHED:
  explicit EggExternalReference(const std::string &node_name, const std::string &filename);
  EggExternalReference(const EggExternalReference &copy);
  EggExternalReference &operator = (const EggExternalReference &copy);

  virtual void write(std::ostream &out, int indent_level) const;

  virtual std::string get_default_extension() const;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggFilenameNode::init_type();
    register_type(_type_handle, "EggExternalReference",
                  EggFilenameNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggExternalReference.I"

#endif
