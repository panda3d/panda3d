// Filename: animControlCollection.cxx
// Created by:  drose (22Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


#include "animControlCollection.h"


////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::Constructor
//       Access: Published
//  Description: Returns the AnimControl associated with the given
//               name, or NULL if no such control has been associated.
////////////////////////////////////////////////////////////////////
AnimControlCollection::
AnimControlCollection() {
  _last_started_control = (AnimControl *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
AnimControlCollection::
~AnimControlCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::store_anim
//       Access: Published
//  Description: Associates the given AnimControl with this collection
//               under the given name.  The AnimControl will remain
//               associated until a new AnimControl is associated with
//               the same name later, or until unbind_anim() is called
//               with this name.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
store_anim(AnimControl *control, const string &name) {
  ControlsByName::iterator ci = _controls_by_name.find(name);

  if (ci == _controls_by_name.end()) {
    // Insert a new control.
    size_t index = _controls.size();
    ControlDef cdef;
    cdef._control = control;
    cdef._name = name;
    _controls.push_back(cdef);
    _controls_by_name.insert(ControlsByName::value_type(name, index));

  } else {
    // Replace an existing control.
    size_t index = (*ci).second;
    nassertv(index < _controls.size());
    nassertv(_controls[index]._name == name);
    if (_last_started_control == _controls[index]._control) {
      _last_started_control = (AnimControl *)NULL;
    }
    _controls[index]._control = control;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::find_anim
//       Access: Published
//  Description: Returns the AnimControl associated with the given
//               name, or NULL if no such control has been associated.
////////////////////////////////////////////////////////////////////
AnimControl *AnimControlCollection::
find_anim(const string &name) const {
  ControlsByName::const_iterator ci = _controls_by_name.find(name);
  if (ci == _controls_by_name.end()) {
    return (AnimControl *)NULL;
  }
  size_t index = (*ci).second;
  nassertr(index < _controls.size(), NULL);
  nassertr(_controls[index]._name == name, NULL);
  return _controls[index]._control;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::unbind_anim
//       Access: Published
//  Description: Removes the AnimControl associated with the given
//               name, if any.  Returns true if an AnimControl was
//               removed, false if there was no AnimControl with the
//               indicated name.
////////////////////////////////////////////////////////////////////
bool AnimControlCollection::
unbind_anim(const string &name) {
  ControlsByName::iterator ci = _controls_by_name.find(name);
  if (ci == _controls_by_name.end()) {
    return false;
  }
  size_t index = (*ci).second;
  nassertr(index < _controls.size(), false);
  nassertr(_controls[index]._name == name, false);

  if (_last_started_control == _controls[index]._control) {
    _last_started_control = (AnimControl *)NULL;
  }
  _controls_by_name.erase(ci);

  _controls.erase(_controls.begin() + index);

  // Now slide all the index numbers down.
  for (ci = _controls_by_name.begin();
       ci != _controls_by_name.end();
       ++ci) {
    if ((*ci).second > index) {
      (*ci).second--;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::get_num_anims
//       Access: Published
//  Description: Returns the number of AnimControls associated with
//               this collection.
////////////////////////////////////////////////////////////////////
int AnimControlCollection::
get_num_anims() const {
  return _controls.size();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::get_anim
//       Access: Published
//  Description: Returns the nth AnimControl associated with
//               this collection.
////////////////////////////////////////////////////////////////////
AnimControl *AnimControlCollection::
get_anim(int n) const {
  nassertr(n >= 0 && n < (int)_controls.size(), NULL);
  return _controls[n]._control;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::get_anim_name
//       Access: Published
//  Description: Returns the name of the nth AnimControl associated
//               with this collection.
////////////////////////////////////////////////////////////////////
string AnimControlCollection::
get_anim_name(int n) const {
  nassertr(n >= 0 && n < (int)_controls.size(), string());
  return _controls[n]._name;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::clear_anims
//       Access: Published
//  Description: Disassociates all anims from this collection.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
clear_anims() {
  _controls.clear();
  _controls_by_name.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::play_all
//       Access: Published
//  Description: Starts all animations playing.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
play_all() {
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    (*ci)._control->play();
    _last_started_control = (*ci)._control;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::play_all
//       Access: Published
//  Description: Starts all animations playing.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
play_all(int from, int to) {
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    (*ci)._control->play(from, to);
    _last_started_control = (*ci)._control;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::loop_all
//       Access: Published
//  Description: Starts all animations looping.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
loop_all(bool restart) {
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    (*ci)._control->loop(restart);
    _last_started_control = (*ci)._control;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::loop_all
//       Access: Published
//  Description: Starts all animations looping.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
loop_all(bool restart, int from, int to) {
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    (*ci)._control->loop(restart, from, to);
    _last_started_control = (*ci)._control;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::stop_all
//       Access: Published
//  Description: Stops all currently playing animations.  Returns true
//               if any animations were stopped, false if none were
//               playing.
////////////////////////////////////////////////////////////////////
bool AnimControlCollection::
stop_all() {
  bool any = false;
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    if ((*ci)._control->is_playing()) {
      any = true;
      (*ci)._control->stop();
    }
  }

  return any;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::pose_all
//       Access: Published
//  Description: Sets all animations to the indicated frame.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
pose_all(int frame) {
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    (*ci)._control->pose(frame);
    _last_started_control = (*ci)._control;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::which_anim_playing
//       Access: Published
//  Description: Returns the name of the bound AnimControl currently
//               playing, if any.  If more than one AnimControl is
//               currently playing, returns all of the names separated
//               by spaces.
////////////////////////////////////////////////////////////////////
string AnimControlCollection::
which_anim_playing() const {
  string result;

  Controls::const_iterator ci;
  for (ci = _controls.begin(); 
       ci != _controls.end(); 
       ++ci) {
    if ((*ci)._control->is_playing()) {
      if (!result.empty()) {
        result += " ";
      }
      result += (*ci)._name;
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
output(ostream &out) const {
  out << _controls.size() << " anims.";
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
write(ostream &out) const {
  ControlsByName::const_iterator ci;
  for (ci = _controls_by_name.begin(); 
       ci != _controls_by_name.end(); 
       ++ci) {
    out << (*ci).first << ": " << *_controls[(*ci).second]._control << "\n";
  }
}
