/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppMakeProperty.h
 * @author rdb
 * @date 2014-09-18
 */

#ifndef CPPMAKEPROPERTY_H
#define CPPMAKEPROPERTY_H

#include "dtoolbase.h"

#include "cppDeclaration.h"
#include "cppIdentifier.h"

/**
 * This is a MAKE_PROPERTY() declaration appearing within a class body.  It
 * means to generate a property within Python, replacing (for instance)
 * get_something()/set_something() with a synthetic 'something' attribute.
 *
 * This is an example of a simple property (MAKE_PROPERTY is defined as
 * the built-in __make_property):
 * @@code
 *   Thing get_thing() const;
 *   void set_thing(const Thing &);
 *
 *   MAKE_PROPERTY(thing, get_thing, set_thing);
 * @@endcode
 * The setter may be omitted to make the property read-only.
 *
 * There is also a secondary macro that allows the property to be set to a
 * cleared state using separate clear functions.  In the scripting language,
 * this would be represented by a "null" value, or an "optional" construct in
 * languages that have no notion of a null value.
 *
 * @@code
 *   bool has_thing() const;
 *   Thing get_thing() const;
 *   void set_thing(const Thing &);
 *   void clear_thing();
 *   MAKE_PROPERTY2(thing, has_thing, get_thing, set_thing, clear_thing);
 * @@endcode
 * As with MAKE_PROPERTY, both the setter and clearer can be omitted to create
 * a read-only property.
 *
 * Thirdly, there is a variant called MAKE_SEQ_PROPERTY.  It takes a length
 * function as argument and the getter and setter take an index as first
 * argument:
 * @@code
 *   size_t get_num_things() const;
 *   Thing &get_thing(size_t i) const;
 *   void set_thing(size_t i, Thing value) const;
 *   void remove_thing(size_t i) const;
 *
 *   MAKE_SEQ_PROPERTY(get_num_things, get_thing, set_thing, remove_thing);
 * @@endcode
 *
 * Lastly, there is the possibility to have properties with key/value
 * associations, often called a "map" or "dictionary" in scripting languages:
 * @@code
 *   bool has_thing(string key) const;
 *   Thing &get_thing(string key) const;
 *   void set_thing(string key, Thing value) const;
 *   void clear_thing(string key) const;
 *
 *   MAKE_MAP_PROPERTY(things, has_thing, get_thing, set_thing, clear_thing);
 * @@endcode
 * You may also replace the "has" function with a "find" function that returns
 * an index.  If the returned index is negative (or in the case of an unsigned
 * integer, the maximum value), the item is assumed not to be present in the
 * mapping.
 *
 * It is also possible to use both MAKE_SEQ_PROPERTY and MAKE_MAP_PROPERTY on
 * the same property name.  This implies that this property has both a
 * sequence and mapping interface.
 */
class CPPMakeProperty : public CPPDeclaration {
public:
  enum Type {
    T_normal = 0x0,
    T_sequence = 0x1,
    T_mapping = 0x2,
  };

  CPPMakeProperty(CPPIdentifier *ident, Type type,
                  CPPScope *current_scope, const CPPFile &file);

  virtual std::string get_simple_name() const;
  virtual std::string get_local_name(CPPScope *scope = nullptr) const;
  virtual std::string get_fully_scoped_name() const;

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;

  virtual SubType get_subtype() const;
  virtual CPPMakeProperty *as_make_property();

  CPPIdentifier *_ident;
  Type _type;
  CPPFunctionGroup *_length_function;
  CPPFunctionGroup *_has_function;
  CPPFunctionGroup *_get_function;
  CPPFunctionGroup *_set_function;
  CPPFunctionGroup *_clear_function;
  CPPFunctionGroup *_del_function;
  CPPFunctionGroup *_insert_function;
  CPPFunctionGroup *_get_key_function;
};

#endif
