// Filename: multiAttribute.T
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

template<class Property, class NameClass>
TypeHandle MultiAttribute<Property, NameClass>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
MultiAttribute<Property, NameClass>::
MultiAttribute() {
  _properties_is_on = true;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
MultiAttribute<Property, NameClass>::
MultiAttribute(const MultiAttribute &copy) :
  NodeAttribute(copy),
  _properties(copy._properties),
  _properties_is_on(copy._properties_is_on)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiAttribute<Property, NameClass>::
operator = (const MultiAttribute &copy) {
  NodeAttribute::operator = (copy);
  _properties = copy._properties;
  _properties_is_on = copy._properties_is_on;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::set_on
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiAttribute<Property, NameClass>::
set_on(const Property &property) {
  if (_properties_is_on) {
    set_property(property);
  } else {
    clear_property(property);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::set_off
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiAttribute<Property, NameClass>::
set_off(const Property &property) {
  if (_properties_is_on) {
    clear_property(property);
  } else {
    set_property(property);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::is_on
//       Access: Public
//  Description: Returns true if the particular properties member is
//               on, false if it is not.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
bool MultiAttribute<Property, NameClass>::
is_on(const Property &property) const {
  if (_properties_is_on) {
    return has_property(property);
  } else {
    return !has_property(property);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::is_off
//       Access: Public
//  Description: Returns true if the particular properties member is
//               off, false if it is not.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
bool MultiAttribute<Property, NameClass>::
is_off(const Property &property) const {
  if (_properties_is_on) {
    return !has_property(property);
  } else {
    return has_property(property);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::properties_is_on
//       Access: Public
//  Description: Returns true if inclusion in the set traversible via
//               begin()/end() indicates that the given property is
//               on, the default.  Returns false if inclusion in the
//               set indicates that the given property is off.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE_GRAPH bool MultiAttribute<Property, NameClass>::
get_properties_is_on() const {
  return _properties_is_on;
}


////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiAttribute<Property, NameClass>::
output(ostream &out) const {
  if (!_properties.empty()) {
    Properties::const_iterator pi = _properties.begin();
    output_property(out, *pi);
    ++pi;
    while (pi != _properties.end()) {
      out << " ";
      output_property(out, *pi);
      ++pi;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiAttribute<Property, NameClass>::
write(ostream &out, int indent_level) const {
  Properties::const_iterator pi;
  for (pi = _properties.begin(); pi != _properties.end(); ++pi) {
    write_property(out, *pi, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::internal_compare_to
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
int MultiAttribute<Property, NameClass>::
internal_compare_to(const NodeAttribute *other) const {
  const MultiAttribute<Property, NameClass> *ot;
  DCAST_INTO_R(ot, other, false);

  return bmap_compare(_properties.begin(), _properties.end(),
                      ot->_properties.begin(), ot->_properties.end());
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::output_property
//       Access: Public, Virtual
//  Description: This is a pure virtual function and normally would
//               not need a body, except that VC++ doesn't seem happy
//               about instantiating the template otherwise.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiAttribute<Property, NameClass>::
output_property(ostream &, const Property &) const {
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::write_property
//       Access: Public, Virtual
//  Description: This is a pure virtual function and normally would
//               not need a body, except that VC++ doesn't seem happy
//               about instantiating the template otherwise.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
void MultiAttribute<Property, NameClass>::
write_property(ostream &, const Property &, int) const {
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::set_property
//       Access: Private
//  Description: An internal function that adds the indicated property
//               to the sorted vector, if it is not already there.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE_GRAPH void MultiAttribute<Property, NameClass>::
set_property(const Property &prop) {
  bool found_flag = false;
  Properties::iterator pi = find_property(prop, found_flag);
  if (!found_flag) {
    _properties.insert(pi, prop);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::set_property
//       Access: Private
//  Description: An internal function that returns true if the
//               indicated property is a member of the sorted vector,
//               false if not.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE_GRAPH bool MultiAttribute<Property, NameClass>::
has_property(const Property &prop) const {
  bool found_flag = false;
  ((MultiAttribute<Property, NameClass> *)this)->find_property(prop, found_flag);
  return found_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::clear_property
//       Access: Private
//  Description: An internal function that removes the indicated
//               property from the sorted vector.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE_GRAPH void MultiAttribute<Property, NameClass>::
clear_property(const Property &prop) {
  bool found_flag = false;
  Properties::iterator pi = find_property(prop, found_flag);
  if (found_flag) {
    _properties.erase(pi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::find_property
//       Access: Private
//  Description: An internal function that searches for the indicated
//               property within the sorted vector.  If the property
//               is found, found_flag is set true, and its iterator is
//               returned; otherwise, found_flag is unchanged, and the
//               iterator at which the property should be inserted is
//               returned.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE_GRAPH MultiAttribute<Property, NameClass>::Properties::iterator MultiAttribute<Property, NameClass>::
find_property(const Property &prop, bool &found_flag) {
  return binary_search_property(_properties.begin(), _properties.end(),
                                prop, found_flag);
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::binary_search_property
//       Access: Private
//  Description: An internal function that implements a binary search
//               for a particular property on the sorted vector.  If
//               the property is found, found_flag is set true, and
//               its iterator is returned; otherwise, found_flag is
//               unchanged, and the iterator at which the property
//               should be inserted is returned.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
MultiAttribute<Property, NameClass>::Properties::iterator MultiAttribute<Property, NameClass>::
binary_search_property(Properties::iterator begin, Properties::iterator end,
                       const Property &prop, bool &found_flag) {
  if (begin == end) {
    return begin;
  }
  Properties::iterator mid = begin + (end - begin) / 2;
  if (prop < *mid) {
    return binary_search_property(begin, mid, prop, found_flag);

  } else if (*mid < prop) {
    return binary_search_property(mid + 1, end, prop, found_flag);

  } else { // *mid == prop
    found_flag = true;
    return mid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE_GRAPH MultiAttribute<Property, NameClass>::size_type MultiAttribute<Property, NameClass>::
size() const {
  return _properties.size();
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::begin
//       Access: Public
//  Description: Returns an iterator that begins the sequence of
//               properties in the attribute.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE_GRAPH MultiAttribute<Property, NameClass>::const_iterator MultiAttribute<Property, NameClass>::
begin() const {
  return _properties.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: MultiAttribute::end
//       Access: Public
//  Description: Returns an iterator that marks the end of the
//               sequence of properties in the attribute.
////////////////////////////////////////////////////////////////////
template<class Property, class NameClass>
INLINE_GRAPH MultiAttribute<Property, NameClass>::const_iterator MultiAttribute<Property, NameClass>::
end() const {
  return _properties.end();
}
