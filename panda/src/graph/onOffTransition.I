// Filename: onOffTransition.I
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE OnOffTransition::
OnOffTransition(TransitionDirection direction) {
  _direction = direction;   
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE OnOffTransition::
OnOffTransition(const OnOffTransition &copy) :
  NodeTransition(copy),
  _direction(copy._direction)
{
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void OnOffTransition::
operator = (const OnOffTransition &copy) {
  NodeTransition::operator = (copy);
  _direction = copy._direction;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::set_identity
//       Access: Public
//  Description: Changes the transition to an identity transition;
//               this has no effect on the attribute for any nodes at
//               this point and below.
////////////////////////////////////////////////////////////////////
INLINE void OnOffTransition::
set_identity() {
  _direction = TD_identity;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::set_on
//       Access: Public
//  Description: Changes the transition to an 'on' transition;
//               this turns on the attribute for all nodes at this
//               point and below.  However, this particular function
//               does not change the attribute value itself, and is
//               thus only appropriate for derived transition types
//               that do not actually carry an attribute value.
//               Derived types that *do* have an attribute value
//               should override this function to accept a value
//               parameter of the appropriate type.
////////////////////////////////////////////////////////////////////
INLINE void OnOffTransition::
set_on() {
  _direction = TD_on;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::set_off
//       Access: Public
//  Description: Changes the transition to an 'off' transition; this
//               turns off the attribute for all nodes at this point
//               and below.
////////////////////////////////////////////////////////////////////
INLINE void OnOffTransition::
set_off() {
  _direction = TD_off;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::is_identity
//       Access: Public
//  Description: Returns true if this is the identity transition,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool OnOffTransition::
is_identity() const {
  return _direction == TD_identity;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::is_on
//       Access: Public
//  Description: Returns true if this transition is 'on', false
//               otherwise.  If the derived transition type carries a
//               value, it should define a get_value() function which
//               allows you to query the on value.
////////////////////////////////////////////////////////////////////
INLINE bool OnOffTransition::
is_on() const {
  return _direction == TD_on;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::is_off
//       Access: Public
//  Description: Returns true if this transition is the 'off'
//               transition, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool OnOffTransition::
is_off() const {
  return _direction == TD_off;
}
