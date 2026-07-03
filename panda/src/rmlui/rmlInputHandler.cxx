/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlInputHandler.cxx
 * @author rdb
 * @date 2011-12-20
 */

#include "rmlInputHandler.h"
#include "buttonEventList.h"
#include "dataGraphTraverser.h"
#include "linmath_events.h"
#include "keyboardButton.h"
#include "mouseButton.h"

#ifndef CPPPARSER
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Context.h>
#include <mutex>
using namespace Rml::Input;
#endif

TypeHandle RmlInputHandler::_type_handle;

// ---------------------------------------------------------------------------
// Keymap — built lazily on the first call to get_rml_key(), protected by a
// static mutex.  A file-scope pmap avoids static-initialization-order issues.
// ---------------------------------------------------------------------------
static pmap<int, int> s_keymap;

/**
 * Populates the Panda ButtonHandle → RmlUi KeyIdentifier map.
 * Called once, under keymap_lock, on the first call to get_rml_key().
 */
static void
build_keymap() {
  s_keymap[KeyboardButton::space().get_index()]         = KI_SPACE;
  s_keymap[KeyboardButton::backspace().get_index()]     = KI_BACK;
  s_keymap[KeyboardButton::tab().get_index()]           = KI_TAB;
  s_keymap[KeyboardButton::enter().get_index()]         = KI_RETURN;
  s_keymap[KeyboardButton::escape().get_index()]        = KI_ESCAPE;
  s_keymap[KeyboardButton::end().get_index()]           = KI_END;
  s_keymap[KeyboardButton::home().get_index()]          = KI_HOME;
  s_keymap[KeyboardButton::left().get_index()]          = KI_LEFT;
  s_keymap[KeyboardButton::up().get_index()]            = KI_UP;
  s_keymap[KeyboardButton::right().get_index()]         = KI_RIGHT;
  s_keymap[KeyboardButton::down().get_index()]          = KI_DOWN;
  s_keymap[KeyboardButton::insert().get_index()]        = KI_INSERT;
  s_keymap[KeyboardButton::del().get_index()]           = KI_DELETE;
  s_keymap[KeyboardButton::caps_lock().get_index()]     = KI_CAPITAL;
  s_keymap[KeyboardButton::f1().get_index()]            = KI_F1;
  s_keymap[KeyboardButton::f2().get_index()]            = KI_F2;
  s_keymap[KeyboardButton::f3().get_index()]            = KI_F3;
  s_keymap[KeyboardButton::f4().get_index()]            = KI_F4;
  s_keymap[KeyboardButton::f5().get_index()]            = KI_F5;
  s_keymap[KeyboardButton::f6().get_index()]            = KI_F6;
  s_keymap[KeyboardButton::f7().get_index()]            = KI_F7;
  s_keymap[KeyboardButton::f8().get_index()]            = KI_F8;
  s_keymap[KeyboardButton::f9().get_index()]            = KI_F9;
  s_keymap[KeyboardButton::f10().get_index()]           = KI_F10;
  s_keymap[KeyboardButton::f11().get_index()]           = KI_F11;
  s_keymap[KeyboardButton::f12().get_index()]           = KI_F12;
  s_keymap[KeyboardButton::f13().get_index()]           = KI_F13;
  s_keymap[KeyboardButton::f14().get_index()]           = KI_F14;
  s_keymap[KeyboardButton::f15().get_index()]           = KI_F15;
  s_keymap[KeyboardButton::f16().get_index()]           = KI_F16;
  s_keymap[KeyboardButton::help().get_index()]          = KI_HELP;
  s_keymap[KeyboardButton::lcontrol().get_index()]      = KI_LCONTROL;
  s_keymap[KeyboardButton::lshift().get_index()]        = KI_LSHIFT;
  s_keymap[KeyboardButton::num_lock().get_index()]      = KI_NUMLOCK;
  s_keymap[KeyboardButton::page_down().get_index()]     = KI_NEXT;
  s_keymap[KeyboardButton::page_up().get_index()]       = KI_PRIOR;
  s_keymap[KeyboardButton::pause().get_index()]         = KI_PAUSE;
  s_keymap[KeyboardButton::print_screen().get_index()]  = KI_SNAPSHOT;
  s_keymap[KeyboardButton::rcontrol().get_index()]      = KI_RCONTROL;
  s_keymap[KeyboardButton::rshift().get_index()]        = KI_RSHIFT;
  s_keymap[KeyboardButton::scroll_lock().get_index()]   = KI_SCROLL;

  s_keymap[KeyboardButton::ascii_key(';').get_index()]  = KI_OEM_1;
  s_keymap[KeyboardButton::ascii_key('=').get_index()]  = KI_OEM_PLUS;
  s_keymap[KeyboardButton::ascii_key(',').get_index()]  = KI_OEM_COMMA;
  s_keymap[KeyboardButton::ascii_key('-').get_index()]  = KI_OEM_MINUS;
  s_keymap[KeyboardButton::ascii_key('.').get_index()]  = KI_OEM_PERIOD;
  s_keymap[KeyboardButton::ascii_key('/').get_index()]  = KI_OEM_2;
  s_keymap[KeyboardButton::ascii_key('`').get_index()]  = KI_OEM_3;
  s_keymap[KeyboardButton::ascii_key('[').get_index()]  = KI_OEM_4;
  s_keymap[KeyboardButton::ascii_key('\\').get_index()] = KI_OEM_5;
  s_keymap[KeyboardButton::ascii_key(']').get_index()]  = KI_OEM_6;
  s_keymap[KeyboardButton::ascii_key('<').get_index()]  = KI_OEM_102;

  for (char c = 'a'; c <= 'z'; ++c) {
    s_keymap[KeyboardButton::ascii_key(c).get_index()] = (c - 'a') + KI_A;
  }
  for (char c = '0'; c <= '9'; ++c) {
    s_keymap[KeyboardButton::ascii_key(c).get_index()] = (c - '0') + KI_0;
  }
}

/**
 *
 */
void RmlInputHandler::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "RmlInputHandler",
                DataNode::get_class_type());
}

/**
 *
 */
RmlInputHandler::
RmlInputHandler(const std::string &name) :
  DataNode(name),
  _mouse_xy(-1)
{
  _pixel_xy_input = define_input("pixel_xy", EventStoreVec2::get_class_type());
  _button_events_input = define_input("button_events", ButtonEventList::get_class_type());
}

/**
 *
 */
RmlInputHandler::
~RmlInputHandler() {
}

/**
 * Maps a Panda ButtonHandle to an RmlUi KeyIdentifier, or KI_UNKNOWN if
 * the key has no RmlUi equivalent.
 *
 * The keymap is built on the first call after Panda's button registry is
 * fully initialised.  std::call_once ensures a single thread-safe build.
 */
int RmlInputHandler::
get_rml_key(const ButtonHandle handle) {
  static std::once_flag built;
  std::call_once(built, build_keymap);
  auto it = s_keymap.find(handle.get_index());
  return (it != s_keymap.end()) ? it->second : 0;
}

/**
 * Accumulates keyboard and mouse events from the data graph each frame.
 * Called on the data graph traversal thread; events are flushed into RmlUi
 * by update_context() on the cull thread.
 */
void RmlInputHandler::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &input,
                 DataNodeTransmit &) {
  MutexHolder holder(_lock);

  if (input.has_data(_pixel_xy_input)) {
    const EventStoreVec2 *pixel_xy;
    DCAST_INTO_V(pixel_xy, input.get_data(_pixel_xy_input).get_ptr());
    LVecBase2 p = pixel_xy->get_value();
    bool reentered = !_mouse_in_window;
    if (p != _mouse_xy || reentered) {
      // Force a move on re-entry even at the same pixel, so RmlUi re-activates
      // the mouse and rebuilds the hover chain that the leave cleared.
      _mouse_xy_changed = true;
      _mouse_xy = p;
    }
    // A pending leave is stale once the pointer is back inside; drop it so the
    // move above isn't immediately undone in the same flush.
    if (reentered) {
      _mouse_left = false;
    }
    _mouse_in_window = true;
  } else if (_mouse_in_window) {
    // The pointer left the window/region this tick: pixel_xy is no longer
    // transmitted.  Queue a single mouse-leave so RmlUi clears hover state.
    _mouse_in_window = false;
    _mouse_left = true;
  }

  if (input.has_data(_button_events_input)) {
    const ButtonEventList *evts;
    DCAST_INTO_V(evts, input.get_data(_button_events_input).get_ptr());
    int n = evts->get_num_events();
    for (int i = 0; i < n; ++i) {
      const ButtonEvent &be = evts->get_event(i);
      int rml_key = KI_UNKNOWN;

      switch (be._type) {
      case ButtonEvent::T_down:
        // matches() (not ==) so that lcontrol/lshift/etc. resolve to their
        // generic control/shift/... alias; == compares only the button index.
        if (be._button.matches(KeyboardButton::control())) {
          _modifiers |= KM_CTRL;
        } else if (be._button.matches(KeyboardButton::shift())) {
          _modifiers |= KM_SHIFT;
        } else if (be._button.matches(KeyboardButton::alt())) {
          _modifiers |= KM_ALT;
        } else if (be._button.matches(KeyboardButton::meta())) {
          _modifiers |= KM_META;
        } else if (be._button == MouseButton::wheel_up()) {
          _wheel_delta -= 1.0f;
        } else if (be._button == MouseButton::wheel_down()) {
          _wheel_delta += 1.0f;
        } else if (be._button == MouseButton::one()) {
          _mouse_button_events.push_back({0, true});
        } else if (be._button == MouseButton::two()) {
          _mouse_button_events.push_back({1, true});
        } else if (be._button == MouseButton::three()) {
          _mouse_button_events.push_back({2, true});
        } else if (be._button == MouseButton::four()) {
          _mouse_button_events.push_back({3, true});
        } else if (be._button == MouseButton::five()) {
          _mouse_button_events.push_back({4, true});
        }
        rml_key = get_rml_key(be._button);
        if (rml_key != KI_UNKNOWN) _key_events.push_back({rml_key, true});
        break;

      case ButtonEvent::T_repeat:
        rml_key = get_rml_key(be._button);
        if (rml_key != KI_UNKNOWN) _repeated_keys.push_back(rml_key);
        break;

      case ButtonEvent::T_up:
        if (be._button.matches(KeyboardButton::control())) {
          _modifiers &= ~KM_CTRL;
        } else if (be._button.matches(KeyboardButton::shift())) {
          _modifiers &= ~KM_SHIFT;
        } else if (be._button.matches(KeyboardButton::alt())) {
          _modifiers &= ~KM_ALT;
        } else if (be._button.matches(KeyboardButton::meta())) {
          _modifiers &= ~KM_META;
        } else if (be._button == MouseButton::one()) {
          _mouse_button_events.push_back({0, false});
        } else if (be._button == MouseButton::two()) {
          _mouse_button_events.push_back({1, false});
        } else if (be._button == MouseButton::three()) {
          _mouse_button_events.push_back({2, false});
        } else if (be._button == MouseButton::four()) {
          _mouse_button_events.push_back({3, false});
        } else if (be._button == MouseButton::five()) {
          _mouse_button_events.push_back({4, false});
        }
        rml_key = get_rml_key(be._button);
        if (rml_key != KI_UNKNOWN) _key_events.push_back({rml_key, false});
        break;

      case ButtonEvent::T_keystroke:
        // Filter control characters; pass everything else as Unicode.
        if (be._keycode > 0x1F &&
            (be._keycode < 0x7F || be._keycode > 0x9F)) {
          _text_input.push_back(be._keycode);
        }
        break;

      default:
        break;
      }
    }
  }
}

/**
 * Flushes accumulated input into the RmlUi context, then calls Update().
 * Called by RmlRegion::do_cull before calling Render().
 */
void RmlInputHandler::
update_context(Rml::Context *context, int xoffs, int yoffs) {
  MutexHolder holder(_lock);

  for (auto &[key, down] : _key_events) {
    if (down) {
      context->ProcessKeyDown((KeyIdentifier) key, _modifiers);
    } else {
      context->ProcessKeyUp((KeyIdentifier) key, _modifiers);
    }
  }
  _key_events.clear();

  for (int key : _repeated_keys) {
    context->ProcessKeyUp((KeyIdentifier) key, _modifiers);
    context->ProcessKeyDown((KeyIdentifier) key, _modifiers);
  }
  _repeated_keys.clear();

  for (int cp : _text_input) {
    context->ProcessTextInput((Rml::Character) cp);
  }
  _text_input.clear();

  if (_mouse_xy_changed) {
    _mouse_xy_changed = false;
    context->ProcessMouseMove(
      (int) _mouse_xy.get_x() - xoffs,
      (int) _mouse_xy.get_y() - yoffs,
      _modifiers);
  }

  for (auto &[btn, down] : _mouse_button_events) {
    if (down) {
      context->ProcessMouseButtonDown(btn, _modifiers);
    } else {
      context->ProcessMouseButtonUp(btn, _modifiers);
    }
  }
  _mouse_button_events.clear();

  if (_wheel_delta != 0.0f) {
    context->ProcessMouseWheel(_wheel_delta, _modifiers);
    _wheel_delta = 0.0f;
  }

  if (_mouse_left) {
    _mouse_left = false;
    // Clears hover/active styling on all elements; RmlUi re-activates the mouse
    // on the next ProcessMouseMove when the pointer returns.
    context->ProcessMouseLeave();
  }

  context->Update();
}
