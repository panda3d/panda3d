// Filename: dataValve.I
// Created by:  drose (05Feb01)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: DataValve::Control::Constructor
//       Access: Published
//  Description: A unique Control can be assigned to each child of the
//               DataValve, or the same Control may be shared between
//               different children as desired.  Each Control may be
//               explicitly set on or off to enable or disable all
//               data flow to that child, respectively; it may also be
//               set to a "buttons" state, in which it enables or
//               disables data flow to the child based on the current
//               state of the modifier buttons as held by the user.
//
//               In addition to being assigned to control all data to
//               a particular child, a particular Control may be set
//               to control only data of a particular type to a
//               particular child.  See set_control() and
//               set_fine_control() on DataValve.
////////////////////////////////////////////////////////////////////
INLINE DataValve::Control::
Control() {
  _state = S_on;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::Control::set_on
//       Access: Published
//  Description: Sets the state on this particular Control to be on
//               until further notice.  Child nodes guarded by this
//               Control will receive data.
////////////////////////////////////////////////////////////////////
INLINE void DataValve::Control::
set_on() {
  _state = S_on;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::Control::set_off
//       Access: Published
//  Description: Sets the state on this particular Control to be off
//               until further notice.  Child nodes guarded by this
//               Control will not receive data.
////////////////////////////////////////////////////////////////////
INLINE void DataValve::Control::
set_off() {
  _state = S_off;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::Control::set_buttons
//       Access: Published
//  Description: Sets the state on this particular Control to be
//               dependent on the current set of buttons being held
//               down by the user.  The ModifierButtons object here
//               must exactly match the ModifierButtons object set on
//               the DataValve in order and number of buttons.
//
//               The best way to generate the ModifierButtons list is
//               to create a new ModifierButtons based on the value
//               returned by DataValve::get_modifier_buttons(), then
//               explicitly call button_up() and button_down() on each
//               button that you want to be up or down.
////////////////////////////////////////////////////////////////////
INLINE void DataValve::Control::
set_buttons(const ModifierButtons &mods) {
  _state = S_buttons;
  _mods = mods;
}


////////////////////////////////////////////////////////////////////
//     Function: DataValve::set_default_control
//       Access: Public
//  Description: Sets the Control that will apply to each child that
//               does not have a particular control set via
//               set_control().
////////////////////////////////////////////////////////////////////
INLINE void DataValve::
set_default_control(DataValve::Control *control) {
  _default_control = control;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::get_default_control
//       Access: Public
//  Description: Returns the Control that will apply to each child
//               that does not have a particular control set via
//               set_control().
////////////////////////////////////////////////////////////////////
INLINE DataValve::Control *DataValve::
get_default_control() const {
  return _default_control;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::set_modifier_buttons
//       Access: Public
//  Description: Specifies the set of buttons that the DataValve will
//               monitor as modifier buttons.  The exact same set of
//               ModifierButtons must be specified on the DataValve as
//               well as on all of its Controls in order to
//               successfully use modifier buttons to switch the
//               valves automatically.
////////////////////////////////////////////////////////////////////
INLINE void DataValve::
set_modifier_buttons(const ModifierButtons &mods) {
  _mods = mods;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::get_modifier_buttons
//       Access: Public
//  Description: Returns the set of ModifierButtons currently being
//               monitored.  See set_modifier_buttons().
////////////////////////////////////////////////////////////////////
INLINE const ModifierButtons &DataValve::
get_modifier_buttons() const {
  return _mods;
}
