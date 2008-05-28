// Filename: eggComment.h
// Created by:  drose (20Jan99)
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

#ifndef EGGCOMMENT_H
#define EGGCOMMENT_H

#include "pandabase.h"

#include "eggNode.h"

////////////////////////////////////////////////////////////////////
//       Class : EggComment
// Description : A comment that appears in an egg file within a
//               <Comment> entry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggComment : public EggNode {
PUBLISHED:
  INLINE EggComment(const string &node_name, const string &comment);
  INLINE EggComment(const EggComment &copy);

  // You can use the string operators to directly set and manipulate
  // the comment.

  INLINE EggComment &operator = (const string &comment);
  INLINE EggComment &operator = (const EggComment &copy);

  INLINE operator const string & () const;

  // Or, you can set and get it explicitly.

  INLINE void set_comment(const string &comment);
  INLINE string get_comment() const;

  virtual void write(ostream &out, int indent_level) const;

private:
  string _comment;


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
