// Filename: multiAttribute.h
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MULTIATTRIBUTE_H
#define MULTIATTRIBUTE_H

#include <pandabase.h>

#include "nodeAttribute.h"
#include "multiTransitionHelpers.h"

#include <indent.h>
#include <algorithm>

template<class Property, class NameClass>
class MultiTransition;

////////////////////////////////////////////////////////////////////
// 	 Class : MultiAttribute
// Description : 
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
class MultiAttribute : public NodeAttribute {
private:
  typedef vector<Property> Properties;

protected:
  MultiAttribute();
  MultiAttribute(const MultiAttribute &copy);
  void operator = (const MultiAttribute &copy);

PUBLISHED:
  void set_on(const Property &prop);
  void set_off(const Property &prop);

  bool is_on(const Property &prop) const;
  bool is_off(const Property &prop) const;

  INLINE bool get_properties_is_on() const;

public:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeAttribute *other) const;

  virtual void output_property(ostream &out, const Property &prop) const=0;
  virtual void write_property(ostream &out, const Property &prop,
			      int indent_level) const=0;

private:
  INLINE void set_property(const Property &prop);
  INLINE bool has_property(const Property &prop) const;
  INLINE void clear_property(const Property &prop);

  INLINE Properties::iterator
  find_property(const Property &prop, bool &found_flag);

  Properties::iterator
  binary_search_property(Properties::iterator begin, Properties::iterator end,
			 const Property &prop, bool &found_flag);

public:
  // These functions and typedefs allow one to peruse all of the
  // Properties in the transition.  Remember to export the
  // vector<Property> template class if you intend to use these
  // outside of PANDA.DLL.
  typedef Properties::const_iterator iterator;
  typedef Properties::const_iterator const_iterator;
  typedef Properties::value_type value_type;
  typedef Properties::size_type size_type;

  INLINE size_type size() const;
  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

private:
  // The following is a set of properties listed in the attribute.  If
  // _properties_is_on is true, the default, it is a list of all 'on'
  // properties; all others are off.  If _properties_is_on is false,
  // it is a list of all 'off' properties; all others are on.
  Properties _properties;
  bool _properties_is_on;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeAttribute::init_type();
    register_type(_type_handle, 
		  string("MultiAttribute<")+NameClass::get_class_name()+">",
		  NodeAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
friend class MultiTransition<Property, NameClass>;
};

#include "multiAttribute.I"

#endif
