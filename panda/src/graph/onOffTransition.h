// Filename: onOffTransition.h
// Created by:  drose (20Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef ONOFFTRANSITION_H
#define ONOFFTRANSITION_H

#include <pandabase.h>

#include "nodeTransition.h"
#include "transitionDirection.h"

class NodeAttribute;
class NodeRelation;

////////////////////////////////////////////////////////////////////
//       Class : OnOffTransition
// Description : This is an abstract class that encapsulates a family
//               of transitions for states that are either on or off
//               (and, when on, may or may not have some particular
//               value).  This is typically useful for things like
//               Texture, which have a definite value when on, and
//               also have a definite off condition.
//
//               See also OnTransition, which is used for states that
//               are always on in some sense, or whose value itself
//               can encode the case of being off.
//
//               The transition may be either 'on', turning on a
//               particular state, 'off', turning *off* a state that
//               was previously turned on, or 'identity', not
//               affecting the state.  If two 'on' transitions are
//               composed together, the second one overrides the
//               first.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA OnOffTransition : public NodeTransition {
protected:
  INLINE_GRAPH OnOffTransition(TransitionDirection direction = TD_identity);
  INLINE_GRAPH OnOffTransition(const OnOffTransition &copy);
  INLINE_GRAPH void operator = (const OnOffTransition &copy);

PUBLISHED:
  INLINE_GRAPH void set_identity();
  INLINE_GRAPH void set_on();
  INLINE_GRAPH void set_off();

  INLINE_GRAPH bool is_identity() const;
  INLINE_GRAPH bool is_on() const;
  INLINE_GRAPH bool is_off() const;

public:
  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;
  virtual NodeAttribute *apply(const NodeAttribute *attrib) const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;

  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

private:
  TransitionDirection _direction;

public:
  //No Read/Write factory methods are defined since this is
  //only an abstract clas
  virtual void write_datagram(BamWriter* manager, Datagram &me);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeTransition::init_type();
    register_type(_type_handle, "OnOffTransition",
                  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class OnOffAttribute;
};

#ifndef DONT_INLINE_GRAPH
#include "onOffTransition.I"
#endif

#endif
