// Filename: pgItem.cxx
// Created by:  drose (02Jul01)
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

#include "pgItem.h"

#include "namedNode.h"
#include "throw_event.h"
#include "string_utils.h"
#include "arcChain.h"

TypeHandle PGItem::_type_handle;
PT(TextNode) PGItem::_text_node;

////////////////////////////////////////////////////////////////////
//     Function: PGItem::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGItem::
PGItem(const string &name) : NamedNode(name)
{
  _has_frame = false;
  _frame.set(0, 0, 0, 0);
  _region = new PGMouseWatcherRegion(this);
  _state = 0;
  _active = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGItem::
~PGItem() {
  nassertv(_region->_item == this);
  _region->_item = (PGItem *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PGItem::
PGItem(const PGItem &copy) :
  NamedNode(copy),
  _has_frame(copy._has_frame),
  _frame(copy._frame),
  _state(copy._state),
  _active(copy._active)
{
  _region = new PGMouseWatcherRegion(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PGItem::
operator = (const PGItem &copy) {
  NamedNode::operator = (copy);
  _has_frame = copy._has_frame;
  _frame = copy._frame;
  _state = copy._state;
  _active = copy._active;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *PGItem::
make_copy() const {
  return new PGItem(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::activate_region
//       Access: Public, Virtual
//  Description: Applies the indicated scene graph transform and order
//               as determined by the traversal from PGTop.
////////////////////////////////////////////////////////////////////
void PGItem::
activate_region(const LMatrix4f &transform, int sort) {
  LPoint3f ll(_frame[0], 0.0, _frame[2]);
  LPoint3f ur(_frame[1], 0.0, _frame[3]);
  ll = ll * transform;
  ur = ur * transform;
  _region->set_frame(ll[0], ur[0], ll[2], ur[2]);
  _region->set_sort(sort);
  _region->set_active(true);
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::enter
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse enters the region.
////////////////////////////////////////////////////////////////////
void PGItem::
enter() {
  throw_event(get_enter_event());
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::exit
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse exits the region.
////////////////////////////////////////////////////////////////////
void PGItem::
exit() {
  throw_event(get_exit_event());
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::button_down
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void PGItem::
button_down(ButtonHandle button) {
  throw_event(get_button_down_event(), button.get_name());
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::button_up
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button previously depressed with
//               button_down() is release.  The bool is_within flag is
//               true if the button was released while the mouse was
//               still within the region, or false if it was released
//               outside the region.
////////////////////////////////////////////////////////////////////
void PGItem::
button_up(ButtonHandle button, bool is_within) {
  throw_event(get_button_up_event(), 
              EventParameter(button.get_name()), 
              EventParameter(is_within));
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_num_state_defs
//       Access: Published
//  Description: Returns one more than the highest-numbered state def
//               that was ever assigned to the PGItem.  The complete
//               set of state defs assigned may then be retrieved by
//               indexing from 0 to (get_num_state_defs() - 1).
//
//               This is only an upper limit on the actual number of
//               state defs, since there may be holes in the list.
////////////////////////////////////////////////////////////////////
int PGItem::
get_num_state_defs() const {
  return _state_defs.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::has_state_def
//       Access: Published
//  Description: Returns true if get_state_def() has ever been called
//               for the indicated state (thus defining a render
//               subgraph for this state index), false otherwise.
////////////////////////////////////////////////////////////////////
bool PGItem::
has_state_def(int state) const {
  if (state < 0 || state >= (int)_state_defs.size()) {
    return false;
  }
  return (_state_defs[state]._node != (Node *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::clear_state_def
//       Access: Published
//  Description: Resets the NodePath assigned to the indicated state
//               to its initial default, with only a frame
//               representation if appropriate.
////////////////////////////////////////////////////////////////////
void PGItem::
clear_state_def(int state) {
  if (state < 0 || state >= (int)_state_defs.size()) {
    return;
  }

  Node *def = _state_defs[state]._node;
  if (def != (Node *)NULL) {
    // Remove all the children from this node.
    int num_children = def->get_num_children(RenderRelation::get_class_type());
    while (num_children > 0) {
      nassertv(num_children == def->get_num_children(RenderRelation::get_class_type()));
      NodeRelation *arc = 
        def->get_child(RenderRelation::get_class_type(), num_children - 1);
      remove_arc(arc);
      num_children--;
    }
  }

  _state_defs[state]._frame_arc = (NodeRelation *)NULL;
  _state_defs[state]._frame_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_state_def
//       Access: Published
//  Description: Returns the Node that is the root of the subgraph
//               that will be drawn when the PGItem is in the
//               indicated state.  The first time this is called for a
//               particular state index, it may create the Node.
////////////////////////////////////////////////////////////////////
Node *PGItem::
get_state_def(int state) {
  nassertr(state < 1000, (Node *)NULL);  // Sanity check.
  slot_state_def(state);

  if (_state_defs[state]._node == (Node *)NULL) {
    // Create a new node.
    _state_defs[state]._node = new NamedNode("state_" + format_string(state));
    _state_defs[state]._frame_stale = true;
  }

  if (_state_defs[state]._frame_stale) {
    update_frame(state);
  }

  return _state_defs[state]._node;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::instance_to_state_def
//       Access: Published
//  Description: Parents an instance of the bottom node of the
//               indicated ArcChain (normally a NodePath) to the
//               indicated state index.
////////////////////////////////////////////////////////////////////
void PGItem::
instance_to_state_def(int state, const ArcChain &chain) {
  if (chain.empty()) {
    // If the chain is empty, quietly do nothing.
    return;
  }

  NodeRelation *new_arc = 
    new RenderRelation(get_state_def(state), chain.node());

  if (chain.has_arcs()) {
    // If the chain has an arc, copy the transitions from it.
    new_arc->copy_transitions_from(chain.arc());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_frame_style
//       Access: Published
//  Description: Returns the kind of frame that will be drawn behind
//               the item when it is in the indicated state.
////////////////////////////////////////////////////////////////////
PGFrameStyle PGItem::
get_frame_style(int state) {
  if (state < 0 || state >= (int)_state_defs.size()) {
    return PGFrameStyle();
  }
  return _state_defs[state]._frame_style;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::set_frame_style
//       Access: Published
//  Description: Changes the kind of frame that will be drawn behind
//               the item when it is in the indicated state.
////////////////////////////////////////////////////////////////////
void PGItem::
set_frame_style(int state, const PGFrameStyle &style) {
  // Get the state def node, mainly to ensure that this state is
  // slotted and listed as having been defined.
  Node *def = get_state_def(state);
  nassertv(def != (Node *)NULL);

  _state_defs[state]._frame_style = style;
  _state_defs[state]._frame_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_text_node
//       Access: Published, Static
//  Description: Returns the TextNode object that will be used by all
//               PGItems to generate default labels given a string.
//               This can be loaded with the default font, etc.
////////////////////////////////////////////////////////////////////
TextNode *PGItem::
get_text_node() {
  if (_text_node == (TextNode *)NULL) {
    _text_node = new TextNode("pguiText");
    _text_node->freeze();
    _text_node->set_text_color(0.0, 0.0, 0.0, 1.0);
    _text_node->set_align(TM_ALIGN_CENTER);
  }
  return _text_node;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::slot_state_def
//       Access: Private
//  Description: Ensures there is a slot in the array for the given
//               state definition.
////////////////////////////////////////////////////////////////////
void PGItem::
slot_state_def(int state) {
  while (state >= (int)_state_defs.size()) {
    StateDef def;
    def._frame_stale = true;
    _state_defs.push_back(def);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::update_frame
//       Access: Private
//  Description: Generates a new instance of the frame geometry for
//               the indicated state.
////////////////////////////////////////////////////////////////////
void PGItem::
update_frame(int state) {
  // First, remove the old frame geometry, if any.
  if (state >= 0 && state < (int)_state_defs.size()) {
    NodeRelation *old_arc = _state_defs[state]._frame_arc;
    if (old_arc != (NodeRelation *)NULL) {
      if (old_arc->is_attached()) {
        remove_arc(old_arc);
        _state_defs[state]._frame_arc = (NodeRelation *)NULL;
      }
    }
  }

  // We must turn off the stale flag first, before we call
  // get_state_def(), to prevent get_state_def() from being a
  // recursive call.
  _state_defs[state]._frame_stale = false;

  // Now create new frame geometry.
  if (has_frame()) {
    Node *node = get_state_def(state);
    nassertv(node != (Node *)NULL);
    
    _state_defs[state]._frame_arc = 
      _state_defs[state]._frame_style.generate_into(node, _frame);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::mark_frames_stale
//       Access: Private
//  Description: Marks all the frames in all states stale, so that
//               they will be regenerated the next time each state is
//               requested.
////////////////////////////////////////////////////////////////////
void PGItem::
mark_frames_stale() {
  StateDefs::iterator di;
  for (di = _state_defs.begin(); di != _state_defs.end(); ++di) {
    // Remove the old frame, if any.
    NodeRelation *old_arc = (*di)._frame_arc;
    if (old_arc != (NodeRelation *)NULL) {
      if (old_arc->is_attached()) {
        remove_arc(old_arc);
        (*di)._frame_arc = (NodeRelation *)NULL;
      }
    }

    (*di)._frame_stale = true;
  }
}
