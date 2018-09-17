/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrmlNode.h
 * @author drose
 * @date 1999-06-23
 */

#ifndef VRMLNODE_H
#define VRMLNODE_H

#include "pandatoolbase.h"

#include "vrmlNodeType.h"
#include "pvector.h"
#include "pmap.h"

class VrmlNode {
public:
  VrmlNode(const VrmlNodeType *type);
  ~VrmlNode();

  const VrmlFieldValue &get_value(const char *field_name) const;

  void output(std::ostream &out, int indent) const;

  class Field {
  public:
    Field() { }
    Field(const VrmlNodeType::NameTypeRec *type, const VrmlFieldValue &value) :
      _type(type), _value(value) { }
    const VrmlNodeType::NameTypeRec *_type;
    VrmlFieldValue _value;
  };

  typedef std::vector<Field> Fields;
  Fields _fields;

  int _use_count;

  const VrmlNodeType *_type;
};

inline std::ostream &operator << (std::ostream &out, const VrmlNode &node) {
  node.output(out, 0);
  return out;
}

class Declaration {
public:
  SFNodeRef _node;

  void output(std::ostream &out, int indent) const;
};

inline std::ostream &operator << (std::ostream &out, const Declaration &dec) {
  dec.output(out, 0);
  return out;
}

typedef pvector<Declaration> VrmlScene;

std::ostream &operator << (std::ostream &out, const VrmlScene &scene);

#endif
