// Filename: multiTransition.I
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "multiTransitionHelpers.h"
#include "multiAttribute.h"

#include <indent.h>

template<class Property, class NameClass>
TypeHandle MultiTransition<Property, NameClass>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
MultiTransition<Property, NameClass>::
MultiTransition() {
  _default_dir = TD_identity;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
MultiTransition<Property, NameClass>::
MultiTransition(const MultiTransition &copy) :
  NodeTransition(copy),
  _properties(copy._properties),
  _default_dir(copy._default_dir)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiTransition<Property, NameClass>::
operator = (const MultiTransition &copy) {
  NodeTransition::operator = (copy);
  _properties = copy._properties;
  _default_dir = copy._default_dir;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::clear
//       Access: Public
//  Description: Removes all properties from the set, and resets it to
//               its initial state.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiTransition<Property, NameClass>::
clear() {
  _properties.clear();
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::is_identity
//       Access: Public
//  Description: Returns true if this transition does not have any
//               effect.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
bool MultiTransition<Property, NameClass>::
is_identity() const {
  if (_default_dir != TD_identity) {
    return false;
  }

  Properties::const_iterator pi;
  for (pi = _properties.begin(); pi != _properties.end(); ++pi) {
    if ((*pi).second != TD_identity) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::set_default_dir
//       Access: Public
//  Description: Sets the default direction on the MultiTransition.
//               This controls whether the MultiTransition applies
//               itself by completely replacing everything that went
//               before, or by adding to whatever has gone before.
//
//               When this is set to either TD_on or TD_off, all
//               properties which are not explicitly mentioned are
//               assumed to be 'on' or 'off' transitions,
//               respectively; when it is set to TD_identity, the
//               default, all properties which are not explicitly
//               mentions are assumed to be identity transitions and
//               are left unchanged.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE void MultiTransition<Property, NameClass>::
set_default_dir(TransitionDirection dir) {
  _default_dir = dir;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::get_default_dir
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE TransitionDirection MultiTransition<Property, NameClass>::
get_default_dir() const {
  return _default_dir;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::set_identity
//       Access: Public
//  Description: Changes the indicated property to an identity
//               transition: this property will not be changed by the
//               application of the MultiTransition.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiTransition<Property, NameClass>::
set_identity(const Property &property) {
  _properties[property] = TD_identity;
  state_changed();  
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::set_on
//       Access: Public
//  Description: Changes the indicated property to an 'on' transition:
//               this property will be explicitly turned on by the
//               application of the MultiTransition.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiTransition<Property, NameClass>::
set_on(const Property &property) {
  _properties[property] = TD_on;
  state_changed();  
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::set_off
//       Access: Public
//  Description: Changes the indicated property to an 'off' transition:
//               this property will be explicitly turned off by the
//               application of the MultiTransition.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiTransition<Property, NameClass>::
set_off(const Property &property) {
  _properties[property] = TD_off;
  state_changed();  
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::is_identity
//       Access: Public
//  Description: Returns true if this property has been indicated as
//               an identity transition, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
bool MultiTransition<Property, NameClass>::
is_identity(const Property &property) const {
  Properties::const_iterator pi;
  pi = _properties.find(property);
  if (pi != _properties.end()) {
    return (*pi).second == TD_identity;
  }

  // The property is not mentioned.  It's implicitly identity only if
  // the default_dir flag is identity.
  return (_default_dir == TD_identity);
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::is_on
//       Access: Public
//  Description: Returns true if this property has been indicated as
//               an 'on' transition, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
bool MultiTransition<Property, NameClass>::
is_on(const Property &property) const {
  Properties::const_iterator pi;
  pi = _properties.find(property);
  if (pi != _properties.end()) {
    return (*pi).second == TD_on;
  }

  // The property is not mentioned.  It's implicitly 'on' only if the
  // default_dir flag is TD_on.
  return (_default_dir == TD_on);
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::is_on
//       Access: Public
//  Description: Returns true if this property has been indicated as
//               an 'off' transition, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
bool MultiTransition<Property, NameClass>::
is_off(const Property &property) const {
  Properties::const_iterator pi;
  pi = _properties.find(property);
  if (pi != _properties.end()) {
    return (*pi).second == TD_off;
  }

  // The property is not mentioned.  It's implicitly 'off' only if the
  // default_dir flag is TD_off.
  return (_default_dir == TD_off);
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::make_identity
//       Access: Public, Virtual
//  Description: This is a pure virtual function and normally would
//               not need a body, except that VC++ doesn't seem happy
//               about instantiating the template otherwise.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
NodeTransition *MultiTransition<Property, NameClass>::
make_identity() const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::compose
//       Access: Public, Virtual
//  Description: Returns a new transition that corresponds to the
//               composition of this transition with the second
//               transition (which must be of an equivalent type).
//               This may return the same pointer as either source
//               transition.  Applying the transition returned from
//               this function to an attribute attribute will produce
//               the same effect as applying each transition
//               separately.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
NodeTransition *MultiTransition<Property, NameClass>::
compose(const NodeTransition *other) const {
  const MultiTransition<Property, NameClass> *ot;
  DCAST_INTO_R(ot, other, NULL);

  if (ot->_priority < _priority) {
    // The other transition too low-priority; the result is unchanged.
    return (MultiTransition<Property, NameClass> *)this;

  } else if (ot->_priority > _priority) {
    // The other priority is higher: the result is the same as the other.
    return (MultiTransition<Property, NameClass> *)ot;

  } else if (ot->_default_dir != TD_identity) {
    // The other transition is a complete-world transition: the result
    // is the same as other.
    return (MultiTransition<Property, NameClass> *)ot;

  } else if (is_identity()) {
    // We're identity: the result is the same as other.
    return (MultiTransition<Property, NameClass> *)ot;

  } else if (ot->is_identity()) {
    // The other transition is identity: it has no effect.
    return (MultiTransition<Property, NameClass> *)this;

  } else {
    // Neither of us is identity.  Build and return a new list.
    MultiTransition<Property, NameClass> *result;
    DCAST_INTO_R(result, make_identity(), NULL);
    result->_default_dir = _default_dir;

    dmap_compose(_properties.begin(), _properties.end(),
		 ot->_properties.begin(), ot->_properties.end(),
		 inserter(result->_properties, result->_properties.begin()));

    return result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::invert
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
NodeTransition *MultiTransition<Property, NameClass>::
invert() const {
  if (is_identity()) {
    return (MultiTransition<Property, NameClass> *)this;
  }

  MultiTransition<Property, NameClass> *result;
  DCAST_INTO_R(result, make_identity(), NULL);

  result->_default_dir = TD_identity;

  dmap_invert(_properties.begin(), _properties.end(),
	      inserter(result->_properties, result->_properties.begin()));

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::apply
//       Access: Public, Virtual
//  Description: Returns a new attribute (or possibly the same
//               attribute) that represents the effect of applying this
//               indicated transition to the indicated attribute.  The
//               source attribute may be NULL, indicating the initial
//               attribute.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
NodeAttribute *MultiTransition<Property, NameClass>::
apply(const NodeAttribute *attrib) const {
  
  MultiAttribute<Property, NameClass> *result;
  if (attrib == (const NodeAttribute *)NULL) {
    DCAST_INTO_R(result, make_attrib(), NULL);
  } else {
    DCAST_INTO_R(result, (NodeAttribute *)attrib, NULL);
  }    

  if (_priority < result->_priority || is_identity()) {
    // The priority is too low to affect the attribute, or we're
    // identity.
    return result;
  }

  if (result->get_count() > 1) {
    // Copy on write.
    DCAST_INTO_R(result, result->make_copy(), NULL);
  }

  result->_priority = _priority;

  MultiAttribute<Property, NameClass>::Properties temp;
  if (_default_dir == TD_identity) {
    // In this case, the normal case, the transition does not specify
    // completely what the attributes should be.
    bmap_apply(result->_properties.begin(), result->_properties.end(),
	       _properties.begin(), _properties.end(),
	       false, TD_on,
	       inserter(temp, temp.begin()));
    result->_properties_is_on = true;

  } else if (_default_dir == TD_off) {
    // In this case, the transition completely specifies what the
    // attributes should be; any property not explicitly mentioned by
    // the transition should be turned off.
    bmap_apply(result->_properties.begin(), result->_properties.end(),
	       _properties.begin(), _properties.end(),
	       true, TD_on,
	       inserter(temp, temp.begin()));
    result->_properties_is_on = true;

  } else { // _default_dir == TD_on
    // In this case, the transition completely specifies what the
    // attributes should be; any property not explicitly mentioned by
    // the transition should be turned on.
    bmap_apply(result->_properties.begin(), result->_properties.end(),
	       _properties.begin(), _properties.end(),
	       true, TD_off,
	       inserter(temp, temp.begin()));
    result->_properties_is_on = false;
  }

  result->_properties.swap(temp);

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiTransition<Property, NameClass>::
output(ostream &out) const {
  Properties::const_iterator pi;

  bool any_identity = false;
  for (pi = _properties.begin(); pi != _properties.end(); ++pi) {
    if ((*pi).second == TD_identity) {
      if (!any_identity) {
	out << " identity(";
	any_identity = true;
      } else {
	out << ", ";
      }
      output_property(out, (*pi).first);
    }
  }
  if (any_identity) {
    out << ")";
  }

  bool any_on = false;
  for (pi = _properties.begin(); pi != _properties.end(); ++pi) {
    if ((*pi).second == TD_on) {
      if (!any_on) {
	out << " on(";
	any_on = true;
      } else {
	out << ", ";
      }
      output_property(out, (*pi).first);
    }
  }
  if (any_on) {
    out << ")";
  }

  bool any_off = false;
  for (pi = _properties.begin(); pi != _properties.end(); ++pi) {
    if ((*pi).second == TD_off) {
      if (!any_off) {
	out << " off(";
	any_off = true;
      } else {
	out << ", ";
      }
      output_property(out, (*pi).first);
    }
  }
  if (any_off) {
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiTransition<Property, NameClass>::
write(ostream &out, int indent_level) const {
  Properties::const_iterator pi;

  bool any_identity = false;
  for (pi = _properties.begin(); pi != _properties.end(); ++pi) {
    if ((*pi).second == TD_identity) {
      if (!any_identity) {
	indent(out, indent_level) << "identity:\n";
	any_identity = true;
      }
      write_property(out, (*pi).first, indent_level + 2);
    }
  }

  bool any_on = false;
  for (pi = _properties.begin(); pi != _properties.end(); ++pi) {
    if ((*pi).second == TD_on) {
      if (!any_on) {
	indent(out, indent_level) << "on:\n";
	any_on = true;
      }
      write_property(out, (*pi).first, indent_level + 2);
    }
  }

  bool any_off = false;
  for (pi = _properties.begin(); pi != _properties.end(); ++pi) {
    if ((*pi).second == TD_off) {
      if (!any_off) {
	indent(out, indent_level) << "off:\n";
	any_off = true;
      }
      write_property(out, (*pi).first, indent_level + 2);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::internal_compare_to
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
int MultiTransition<Property, NameClass>::
internal_compare_to(const NodeTransition *other) const {
  const MultiTransition<Property, NameClass> *ot;
  DCAST_INTO_R(ot, other, false);

  if (_default_dir != ot->_default_dir) {
    return (int)_default_dir - (int)ot->_default_dir;
  }

  return dmap_compare(_properties.begin(), _properties.end(),
		      ot->_properties.begin(), ot->_properties.end());
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::output_property
//       Access: Public, Virtual
//  Description: This is a pure virtual function and normally would
//               not need a body, except that VC++ doesn't seem happy
//               about instantiating the template otherwise.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiTransition<Property, NameClass>::
output_property(ostream &, const Property &) const {
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::write_property
//       Access: Public, Virtual
//  Description: This is a pure virtual function and normally would
//               not need a body, except that VC++ doesn't seem happy
//               about instantiating the template otherwise.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiTransition<Property, NameClass>::
write_property(ostream &, const Property &, int) const {
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::size
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE MultiTransition<Property, NameClass>::size_type MultiTransition<Property, NameClass>::
size() const {
  return _properties.size();
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::begin
//       Access: Public
//  Description: Returns an iterator that begins the sequence of
//               properties in the transition.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE MultiTransition<Property, NameClass>::const_iterator MultiTransition<Property, NameClass>::
begin() const {
  return _properties.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: MultiTransition::end
//       Access: Public
//  Description: Returns an iterator that marks the end of the
//               sequence of properties in the transition.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE MultiTransition<Property, NameClass>::const_iterator MultiTransition<Property, NameClass>::
end() const {
  return _properties.end();
}

