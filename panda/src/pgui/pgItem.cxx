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

#include "pgTop.h"
#include "pgItem.h"
#include "pgMouseWatcherParameter.h"

#include "namedNode.h"
#include "throw_event.h"
#include "string_utils.h"
#include "arcChain.h"
#include "transformTransition.h"
#include "sceneGraphReducer.h"
#include "directRenderTraverser.h"
#include "allTransitionsWrapper.h"

#ifdef HAVE_AUDIO
#include "audioSound.h"
#endif

TypeHandle PGItem::_type_handle;
PT(TextNode) PGItem::_text_node;
PGItem *PGItem::_focus_item = (PGItem *)NULL;

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
  _flags = F_active;
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

  if (_focus_item == this) {
    _focus_item = (PGItem *)NULL;
  }
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
  _flags(copy._flags)
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
  _flags = copy._flags;
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
//     Function: PGItem::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this node by the indicated
//               matrix, if it means anything to do so.  For most
//               kinds of nodes, this does nothing.
////////////////////////////////////////////////////////////////////
void PGItem::
xform(const LMatrix4f &mat) {
  // Transform the frame.
  LPoint3f ll(_frame[0], 0.0, _frame[2]);
  LPoint3f ur(_frame[1], 0.0, _frame[3]);
  ll = ll * mat;
  ur = ur * mat;
  _frame.set(ll[0], ur[0], ll[2], ur[2]);

  // Transform the individual states and their frame styles.
  StateDefs::iterator di;
  for (di = _state_defs.begin(); di != _state_defs.end(); ++di) {
    Node *node = (*di)._node;
    if (node != (Node *)NULL) {
      // Apply the matrix to the previous transform on all the arcs
      // here.
      int num_children = node->get_num_children(RenderRelation::get_class_type());
      for (int i = 0; i < num_children; i++) {
        NodeRelation *arc = node->get_child(RenderRelation::get_class_type(), i);
        if (arc != (*di)._frame_arc) {
          LMatrix4f arc_mat;
          TransformTransition *tt;
          if (!get_transition_into(tt, arc)) {
            // No previous transform.
            arc_mat = mat;
          } else {
            arc_mat = tt->get_matrix() * mat;
          }
          tt = new TransformTransition(arc_mat);
          arc->set_transition(tt);

          // Now flatten the transform into the subgraph.
          SceneGraphReducer gr;
          gr.apply_transitions(arc);
        }
      }
    }

    // Transform the frame style too.
    if ((*di)._frame_style.xform(mat)) {
      (*di)._frame_stale = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::activate_region
//       Access: Public, Virtual
//  Description: Applies the indicated scene graph transform and order
//               as determined by the traversal from PGTop.
////////////////////////////////////////////////////////////////////
void PGItem::
activate_region(PGTop *, const LMatrix4f &transform, int sort) {
  // Transform all four vertices, and get the new bounding box.  This
  // way the region works (mostly) even if has been rotated.
  LPoint3f ll(_frame[0], 0.0, _frame[2]);
  LPoint3f lr(_frame[1], 0.0, _frame[2]);
  LPoint3f ul(_frame[0], 0.0, _frame[3]);
  LPoint3f ur(_frame[1], 0.0, _frame[3]);
  ll = ll * transform;
  lr = lr * transform;
  ul = ul * transform;
  ur = ur * transform;
  _region->set_frame(min(min(ll[0], lr[0]), min(ul[0], ur[0])),
                     max(max(ll[0], lr[0]), max(ul[0], ur[0])),
                     min(min(ll[2], lr[2]), min(ul[2], ur[2])),
                     max(max(ll[2], lr[2]), max(ul[2], ur[2])));
                     
  _region->set_sort(sort);
  _region->set_active(true);
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::draw_item
//       Access: Public, Virtual
//  Description: Called by the PGTop's traversal to draw this
//               particular item.
////////////////////////////////////////////////////////////////////
void PGItem::
draw_item(PGTop *top, GraphicsStateGuardian *gsg, 
          const AllAttributesWrapper &attrib) {
  if (has_state_def(get_state())) {
    // This item has a current state definition that we should use
    // to render the item.
    Node *def = get_state_def(get_state());
    
    // We'll use a normal DirectRenderTraverser to do the rendering
    // of the subgraph.
    DirectRenderTraverser drt(gsg, RenderRelation::get_class_type());
    drt.set_view_frustum_cull(false);
    drt.traverse(def, attrib, AllTransitionsWrapper());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::enter
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse enters the region.
////////////////////////////////////////////////////////////////////
void PGItem::
enter(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_enter_event();
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::exit
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse exits the region.
////////////////////////////////////////////////////////////////////
void PGItem::
exit(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_exit_event();
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::focus_in
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               widget gets the keyboard focus.
////////////////////////////////////////////////////////////////////
void PGItem::
focus_in() {
  string event = get_focus_in_event();
  play_sound(event);
  throw_event(event);
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::focus_out
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               widget loses the keyboard focus.
////////////////////////////////////////////////////////////////////
void PGItem::
focus_out() {
  string event = get_focus_out_event();
  play_sound(event);
  throw_event(event);
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::press
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void PGItem::
press(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_press_event(param.get_button());
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::release
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button previously depressed with
//               press() is released.
////////////////////////////////////////////////////////////////////
void PGItem::
release(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_release_event(param.get_button());
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::set_active
//       Access: Published, Virtual
//  Description: Sets whether the PGItem is active for mouse watching.
//               This is not necessarily related to the
//               active/inactive appearance of the item, which is
//               controlled by set_state(), but it does affect whether
//               it responds to mouse events.
////////////////////////////////////////////////////////////////////
void PGItem::
set_active(bool active) {
  if (active) {
    _flags |= F_active;
  } else {
    _flags &= ~F_active;
    // Deactivating the item automatically defocuses it too.
    if (get_focus()) {
      set_focus(false);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::set_focus
//       Access: Published, Virtual
//  Description: Sets whether the PGItem currently has keyboard focus.
//               This simply means that the item may respond to
//               keyboard events as well as to mouse events; precisely
//               what this means is up to the individual item.  
//
//               Only one PGItem in the world is allowed to have focus
//               at any given time.  Setting the focus on any other
//               item automatically disables the focus from the
//               previous item.
////////////////////////////////////////////////////////////////////
void PGItem::
set_focus(bool focus) {
  if (focus) {
    if (!get_active()) {
      // Cannot set focus on an inactive item.
      return;
    }

    // Set the keyboard focus to this item.
    if (_focus_item != this) {
      if (_focus_item != (PGItem *)NULL) {
        // Clear the focus from whatever item currently has it.
        _focus_item->set_focus(false);
      }
      _focus_item = this;
    }
    if (!get_focus()) {
      focus_in();
      _flags |= F_focus;
    }

  } else {
    if (_focus_item == this) {
      // Remove this item from the focus.
      _focus_item = (PGItem *)NULL;
    }

    if (get_focus()) {
      focus_out();
      _flags &= ~F_focus;
    }
  }
  _region->set_keyboard(focus);
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

  remove_all_children(_state_defs[state]._node);

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
  nassertr(state >= 0 && state < 1000, (Node *)NULL);  // Sanity check.
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

#ifdef HAVE_AUDIO
////////////////////////////////////////////////////////////////////
//     Function: PGItem::set_sound
//       Access: Published
//  Description: Sets the sound that will be played whenever the
//               indicated event occurs.
////////////////////////////////////////////////////////////////////
void PGItem::
set_sound(const string &event, AudioSound *sound) {
  _sounds[event] = sound;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::clear_sound
//       Access: Published
//  Description: Removes the sound associated with the indicated
//               event.
////////////////////////////////////////////////////////////////////
void PGItem::
clear_sound(const string &event) {
  _sounds.erase(event);
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::get_sound
//       Access: Published
//  Description: Returns the sound associated with the indicated
//               event, or NULL if there is no associated sound.
////////////////////////////////////////////////////////////////////
AudioSound *PGItem::
get_sound(const string &event) const {
  Sounds::const_iterator si = _sounds.find(event);
  if (si != _sounds.end()) {
    return (*si).second;
  }
  return (AudioSound *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::has_sound
//       Access: Published
//  Description: Returns true if there is a sound associated with the
//               indicated event, or false otherwise.
////////////////////////////////////////////////////////////////////
bool PGItem::
has_sound(const string &event) const {
  return (_sounds.count(event) != 0);
}
#endif  // HAVE_AUDIO

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

    // The default TextNode is aligned to the left, for the
    // convenience of PGEntry.
    _text_node->set_align(TM_ALIGN_LEFT);
  }
  return _text_node;
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::play_sound
//       Access: Protected
//  Description: Plays the sound associated with the indicated event,
//               if there is one.
////////////////////////////////////////////////////////////////////
void PGItem::
play_sound(const string &event) {
#ifdef HAVE_AUDIO
  Sounds::const_iterator si = _sounds.find(event);
  if (si != _sounds.end()) {
    AudioSound *sound = (*si).second;
    sound->play();
  }
#endif  // HAVE_AUDIO
}

////////////////////////////////////////////////////////////////////
//     Function: PGItem::remove_all_children
//       Access: Protected, Static
//  Description: An internal utility function to remove all the
//               children of the indicated Node.
////////////////////////////////////////////////////////////////////
void PGItem::
remove_all_children(Node *node) {
  if (node != (Node *)NULL) {
    int num_children = 
      node->get_num_children(RenderRelation::get_class_type());
    while (num_children > 0) {
      nassertv(num_children == node->get_num_children(RenderRelation::get_class_type()));
      NodeRelation *arc = 
        node->get_child(RenderRelation::get_class_type(), num_children - 1);
      remove_arc(arc);
      num_children--;
    }
  }
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
      }
      _state_defs[state]._frame_arc = (NodeRelation *)NULL;
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
