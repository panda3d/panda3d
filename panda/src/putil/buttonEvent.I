// Filename: buttonEvent.I
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent() :
  _button(ButtonHandle::none()),
  _down(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent(ButtonHandle button, bool down) :
  _button(button),
  _down(down)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent(ButtonHandle button, bool down, ModifierButtons mods) :
  _button(button),
  _down(down),
  _mods(mods)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent(const ButtonEvent &copy) :
  _button(copy._button),
  _down(copy._down),
  _mods(copy._mods)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ButtonEvent::
operator = (const ButtonEvent &copy) {
  _button = copy._button;
  _down = copy._down;
  ((ModifierButtons &)_mods) = copy._mods;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ButtonEvent::
operator == (const ButtonEvent &other) const {
  return (_button == other._button &&
	  _down == other._down &&
	  _mods == other._mods);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ButtonEvent::
operator != (const ButtonEvent &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Ordering Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ButtonEvent::
operator < (const ButtonEvent &other) const {
  if (_button != other._button) {
    return _button < other._button;
  }

  if (_down != other._down) {
    return _down < other._down;
  }

  return _mods < other._mods;
}
