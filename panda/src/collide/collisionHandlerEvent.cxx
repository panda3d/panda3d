// Filename: collisionHandlerEvent.cxx
// Created by:  drose (27Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "collisionHandlerEvent.h"
#include "config_collide.h"

#include <eventParameter.h>
#include <throw_event.h>

TypeHandle CollisionHandlerEvent::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::Constructor
//       Access: Public
//  Description: The default CollisionHandlerEvent will throw no
//               events.  Its pattern strings must first be set via a
//               call to set_in_pattern() and/or set_out_pattern().
////////////////////////////////////////////////////////////////////
CollisionHandlerEvent::
CollisionHandlerEvent() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::begin_group
//       Access: Public, Virtual
//  Description: Will be called by the CollisionTraverser before a new
//               traversal is begun.  It instructs the handler to
//               reset itself in preparation for a number of
//               CollisionEntries to be sent.
////////////////////////////////////////////////////////////////////
void CollisionHandlerEvent::
begin_group() {
  if (collide_cat.is_spam()) {
    collide_cat.spam()
      << "begin_group.\n";
  }
  _last_colliding.swap(_current_colliding);
  _current_colliding.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::add_entry
//       Access: Public, Virtual
//  Description: Called between a begin_group() .. end_group()
//               sequence for each collision that is detected.
////////////////////////////////////////////////////////////////////
void CollisionHandlerEvent::
add_entry(CollisionEntry *entry) {
  nassertv(entry != (CollisionEntry *)NULL);

  // Record this particular entry for later.  This will keep track of
  // all the unique pairs of node/node intersections.
  bool inserted = _current_colliding.insert(entry).second;

  if (collide_cat.is_spam()) {
    collide_cat.spam()
      << "Detected collision from " << (void *)entry->get_from_node()
      << " to " << (void *)entry->get_into_node()
      << ", inserted = " << inserted << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::end_group
//       Access: Public, Virtual
//  Description: Called by the CollisionTraverser at the completion of
//               all collision detections for this traversal.  It
//               should do whatever finalization is required for the
//               handler.
////////////////////////////////////////////////////////////////////
void CollisionHandlerEvent::
end_group() {
  // Now compare the list of entries we collected this frame with
  // those we kept from the last time.  Each new entry represents a
  // new 'in' event; each missing entry represents a new 'out' event.

  if (collide_cat.is_spam()) {
    collide_cat.spam()
      << "end_group.\n"
      << "current_colliding has " << _current_colliding.size()
      << " entries, last_colliding has " << _last_colliding.size() 
      << "\n";
  }

  Colliding::iterator ca, cb;

  ca = _current_colliding.begin();
  cb = _last_colliding.begin();

  SortEntries order;
  while (ca != _current_colliding.end() && cb != _last_colliding.end()) {
    if (order(*ca, *cb)) {
      // Here's an element in a but not in b.  That's a newly entered
      // intersection.
      throw_event_pattern(_in_pattern, *ca);
      ++ca;

    } else if (order(*cb, *ca)) {
      // Here's an element in b but not in a.  That's a newly exited
      // intersection.
      throw_event_pattern(_out_pattern, *cb);
      ++cb;

    } else {
      // This element is in both b and a.  It hasn't changed.
      throw_event_pattern(_again_pattern, *cb);
      ++ca;
      ++cb;
    }
  }

  while (ca != _current_colliding.end()) {
    // Here's an element in a but not in b.  That's a newly entered
    // intersection.
    throw_event_pattern(_in_pattern, *ca);
    ++ca;
  }

  while (cb != _last_colliding.end()) {
    // Here's an element in b but not in a.  That's a newly exited
    // intersection.
    throw_event_pattern(_out_pattern, *cb);
    ++cb;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::clear
//       Access: Public
//  Description: Empties the list of elements that all colliders are
//               known to be colliding with.  No "out" events will be
//               thrown; if the same collision is detected next frame,
//               a new "in" event will be thrown for each collision.
//
//               This can be called each frame to defeat the
//               persistent "in" event mechanism, which prevents the
//               same "in" event from being thrown repeatedly.
//               However, also see set_again_pattern(), which can be
//               used to set the event that is thrown when a collision
//               is detected for two or more consecutive frames.
////////////////////////////////////////////////////////////////////
void CollisionHandlerEvent::
clear() {
  _last_colliding.clear();
  _current_colliding.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::throw_event_pattern
//       Access: Private
//  Description: Throws an event matching the indicated pattern.
////////////////////////////////////////////////////////////////////
void CollisionHandlerEvent::
throw_event_pattern(const string &pattern, CollisionEntry *entry) {
  if (pattern.empty()) {
    return;
  }

  string event;
  for (size_t p = 0; p < pattern.size(); ++p) {
    if (pattern[p] == '%') {
      string cmd = pattern.substr(p + 1, 2);
      p += 2;
      if (cmd == "fn") {
        event += entry->get_from_node()->get_name();

      } else if (cmd == "in") {
        event += entry->get_into_node()->get_name();

      } else if (cmd == "ft") {
        event += 
          (!entry->get_from()->is_tangible() ? 'i' : 't');

      } else if (cmd == "it") {
        event += 
          (entry->has_into() && !entry->get_into()->is_tangible() ? 'i' : 't');

      } else if (cmd == "ig") {
        event += 
          (entry->has_into() ? 'c' : 'g');

      } else {
        collide_cat.error()
          << "Invalid symbol in event_pattern: %" << cmd << "\n";
      }
    } else {
      event += pattern[p];
    }
  }

  if (!event.empty()) {
    throw_event(event, EventParameter(entry));
  }
}
