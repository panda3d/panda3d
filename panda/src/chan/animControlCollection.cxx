// Filename: animControlCollection.cxx
// Created by:  drose (22Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "animControlCollection.h"


////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::Constructor
//       Access: Public
//  Description: Returns the AnimControl associated with the given
//               name, or NULL if no such control has been associated.
////////////////////////////////////////////////////////////////////
AnimControlCollection::
AnimControlCollection() {
  _last_started_control = (AnimControl *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
AnimControlCollection::
~AnimControlCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::store_anim
//       Access: Public
//  Description: Associates the given AnimControl with this collection
//               under the given name.  The AnimControl will remain
//               associated until a new AnimControl is associated with
//               the same name later, or until unbind_anim() is called
//               with this name.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
store_anim(AnimControl *control, const string &name) {
  Controls::iterator ci = _controls.find(name);
  if (ci == _controls.end()) {
    _controls.insert(Controls::value_type(name, control));
  } else {
    if (_last_started_control == (*ci).second) {
      _last_started_control = (AnimControl *)NULL;
    }
    (*ci).second = control;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::find_anim
//       Access: Public
//  Description: Returns the AnimControl associated with the given
//               name, or NULL if no such control has been associated.
////////////////////////////////////////////////////////////////////
AnimControl *AnimControlCollection::
find_anim(const string &name) const {
  Controls::const_iterator ci = _controls.find(name);
  if (ci == _controls.end()) {
    return (AnimControl *)NULL;
  }
  return (*ci).second;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::unbind_anim
//       Access: Public
//  Description: Removes the AnimControl associated with the given
//               name, if any.  Returns true if an AnimControl was
//               removed, false if there was no AnimControl with the
//               indicated name.
////////////////////////////////////////////////////////////////////
bool AnimControlCollection::
unbind_anim(const string &name) {
  Controls::iterator ci = _controls.find(name);
  if (ci == _controls.end()) {
    return false;
  }
  if (_last_started_control == (*ci).second) {
    _last_started_control = (AnimControl *)NULL;
  }
  _controls.erase(ci);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::get_num_anims
//       Access: Public
//  Description: Returns the number of AnimControls associated with
//               this collection.
////////////////////////////////////////////////////////////////////
int AnimControlCollection:: 
get_num_anims() const {
  return _controls.size();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::clear_anims
//       Access: Public
//  Description: Disassociates all anims from this collection.
////////////////////////////////////////////////////////////////////
void AnimControlCollection:: 
clear_anims() {
  _controls.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::play_all
//       Access: Public
//  Description: Starts all animations playing.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
play_all() {
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    (*ci).second->play();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::play_all
//       Access: Public
//  Description: Starts all animations playing.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
play_all(int from, int to) {
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    (*ci).second->play(from, to);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::loop_all
//       Access: Public
//  Description: Starts all animations looping.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
loop_all(bool restart) {
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    (*ci).second->loop(restart);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::loop_all
//       Access: Public
//  Description: Starts all animations looping.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
loop_all(bool restart, int from, int to) {
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    (*ci).second->loop(restart, from, to);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::stop_all
//       Access: Public
//  Description: Stops all currently playing animations.  Returns true
//               if any animations were stopped, false if none were
//               playing.
////////////////////////////////////////////////////////////////////
bool AnimControlCollection::
stop_all() {
  bool any = false;
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    if ((*ci).second->is_playing()) {
      any = true;
      (*ci).second->stop();
    }
  }

  return any;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::pose_all
//       Access: Public
//  Description: Sets all animations to the indicated frame.
////////////////////////////////////////////////////////////////////
void AnimControlCollection::
pose_all(int frame) {
  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    (*ci).second->pose(frame);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControlCollection::which_anim_playing
//       Access: Public
//  Description: Returns the name of the bound AnimControl currently
//               playing, if any.  If more than one AnimControl is
//               currently playing, returns all of the names separated
//               by spaces.
////////////////////////////////////////////////////////////////////
string AnimControlCollection::
which_anim_playing() const {
  string result;

  Controls::const_iterator ci;
  for (ci = _controls.begin(); ci != _controls.end(); ++ci) {
    if ((*ci).second->is_playing()) {
      if (!result.empty()) {
        result += " ";
      }
      result += (*ci).first;
    }
  }

  return result;
}
