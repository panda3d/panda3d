// Filename: vrmlNode.h
// Created by:  drose (23Jun99)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////

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

  void output(ostream &out, int indent) const;

  class Field {
  public:
    Field() { }
    Field(const VrmlNodeType::NameTypeRec *type, const VrmlFieldValue &value) :
      _type(type), _value(value) { }
    const VrmlNodeType::NameTypeRec *_type;
    VrmlFieldValue _value;
  };

  typedef vector<Field> Fields;
  Fields _fields;

  int _use_count;

  const VrmlNodeType *_type;
};

inline ostream &operator << (ostream &out, const VrmlNode &node) {
  node.output(out, 0);
  return out;
}

class Declaration {
public:
  SFNodeRef _node;

  void output(ostream &out, int indent) const;
};

inline ostream &operator << (ostream &out, const Declaration &dec) {
  dec.output(out, 0);
  return out;
}

typedef pvector<Declaration> VrmlScene;

ostream &operator << (ostream &out, const VrmlScene &scene);

#endif
