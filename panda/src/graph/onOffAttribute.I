// Filename: onOffAttribute.I
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH OnOffAttribute::
OnOffAttribute(bool is_on) {
  _is_on = is_on;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH OnOffAttribute::
OnOffAttribute(const OnOffAttribute &copy) :
  NodeAttribute(copy),
  _is_on(copy._is_on)
{
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void OnOffAttribute::
operator = (const OnOffAttribute &copy) {
  NodeAttribute::operator = (copy);
  _is_on = copy._is_on;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::set_on
//       Access: Public
//  Description: Changes the attribute to an 'on' attribute;
//               this turns on the attribute for all nodes at this
//               point and below.  However, this particular function
//               does not change the attribute value itself, and is
//               thus only appropriate for derived attribute types
//               that do not actually carry an attribute value.
//               Derived types that *do* have an attribute value
//               should override this function to accept a value
//               parameter of the appropriate type.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void OnOffAttribute::
set_on() {
  _is_on = true;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::set_off
//       Access: Public
//  Description: Changes the attribute to an 'off' attribute; this
//               turns off the attribute for all nodes at this point
//               and below.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void OnOffAttribute::
set_off() {
  _is_on = false;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::is_on
//       Access: Public
//  Description: Returns true if the attribute is on, false if it is
//               not.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool OnOffAttribute::
is_on() const {
  return _is_on;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::is_off
//       Access: Public
//  Description: Returns true if the attribute is off, false if it is
//               not.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool OnOffAttribute::
is_off() const {
  return !_is_on;
}
