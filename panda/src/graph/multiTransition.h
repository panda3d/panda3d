// Filename: multiTransition.h
// Created by:  drose (23Mar00)
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

#ifndef MULTITRANSITION_H
#define MULTITRANSITION_H

#include "pandabase.h"

#include "nodeTransition.h"
#include "transitionDirection.h"
#include "multiTransitionHelpers.h"

#include "indent.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//       Class : MultiTransition
// Description : This is an abstract template class that encapsulates
//               states that may have any number of subcomponents, in
//               no particular order, any of which may be on or off at
//               a given time.  The classic example of this is a
//               LightTransition, which may represent any arbitrary
//               set of lights that are to be considered on at a given
//               time.
//
//               This is a template class; it templates on the type of
//               thing that is to be on or off, as well as a class
//               that encodes the name of the thing.  This class
//               (which might be the same as the Property class)
//               should contain a static method called
//               "get_class_name()" which returns the string name of
//               the Property class.  This is necessary to allow the
//               MultiTransition it initialize its TypeHandle
//               properly.
//
//               Also see MultiNodeTransition.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
class MultiTransition : public NodeTransition {
private:
  typedef pair<Property, TransitionDirection> Element;

  // This has to be a vector and not a map, so we can safely access
  // the iterators outside of PANDA.DLL.
  typedef pvector<Element> Properties;

  // We use this as an STL function object for sorting the vector in
  // order by its property.
  class SortByFirstOfPair {
  public:
    INLINE_GRAPH bool operator ()(const Element &a, const Element &b) const;
  };

protected:
  MultiTransition();
  MultiTransition(const MultiTransition &copy);
  void operator = (const MultiTransition &copy);

PUBLISHED:
  void clear();
  bool is_identity() const;

  // Typically a MultiTransition will be in "incomplete" mode; that
  // is, it presents an incomplete view of the state that we desire.
  // That means that components which are not mentioned in a
  // particular transition will not be affected by the transition; if
  // they were off, they will remain off; if they were on, they will
  // remain on.

  // Sometimes you want a transition to be able to turn off all
  // components (for instance, to disable lighting by turning off all
  // lights) or to turn on all components.  To achieve either effect,
  // set the default direction, which indicates whether to implicitly
  // turn on or off (or leave alone) any property not explicitly
  // mentioned in the transition.
  INLINE_GRAPH void set_default_dir(TransitionDirection dir);
  INLINE_GRAPH TransitionDirection get_default_dir() const;

  void set_identity(const Property &prop);
  void set_on(const Property &prop);
  void set_off(const Property &prop);

  bool is_identity(const Property &prop) const;
  bool is_on(const Property &prop) const;
  bool is_off(const Property &prop) const;

public:
  virtual NodeTransition *make_identity() const=0;

  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;
  virtual NodeAttribute *apply(const NodeAttribute *attrib) const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;

  virtual void output_property(ostream &out, const Property &prop) const;
  virtual void write_property(ostream &out, const Property &prop,
                              int indent_level) const;

public:
  // These functions and typedefs allow one to peruse all of the
  // Properties in the transition.
  typedef Properties::const_iterator iterator;
  typedef Properties::const_iterator const_iterator;
  typedef Properties::value_type value_type;
  typedef Properties::size_type size_type;

  INLINE_GRAPH size_type size() const;
  INLINE_GRAPH const_iterator begin() const;
  INLINE_GRAPH const_iterator end() const;

private:
  Properties _properties;
  TransitionDirection _default_dir;

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
    register_type(_type_handle,
                  string("MultiTransition<")+NameClass::get_class_name()+">",
                  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "multiTransition.T"

#endif
