/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggComment.h
 * @author drose
 * @date 1999-01-20
 */

#ifndef EGGCOMMENT_H
#define EGGCOMMENT_H

#include "pandabase.h"

#include "eggNode.h"

/**
 * A comment that appears in an egg file within a <Comment> entry.
 */
class EXPCL_PANDA_EGG EggComment : public EggNode {
PUBLISHED:
  INLINE explicit EggComment(const std::string &node_name, const std::string &comment);
  INLINE EggComment(const EggComment &copy);

  // You can use the string operators to directly set and manipulate the
  // comment.

  INLINE EggComment &operator = (const std::string &comment);
  INLINE EggComment &operator = (const EggComment &copy);

  INLINE operator const std::string & () const;

  // Or, you can set and get it explicitly.

  INLINE void set_comment(const std::string &comment);
  INLINE std::string get_comment() const;

  virtual void write(std::ostream &out, int indent_level) const;

private:
  std::string _comment;


public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    register_type(_type_handle, "EggComment",
                  EggNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggComment.I"

#endif
