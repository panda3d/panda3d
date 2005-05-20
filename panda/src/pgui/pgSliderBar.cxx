// Filename: pgSliderBar.cxx
// Created by:  masad (19Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pgSliderBar.h"
#include "pgMouseWatcherParameter.h"

#include "throw_event.h"

TypeHandle PGSliderBar::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGSliderBar::
PGSliderBar(const string &name) :
  PGItem(name),
  //  _slider("slider"),
  _left("left"),
  _right("right")
{
  _slider = NULL;
  _range = 100.0;
  // _value is a range from (-1 to +1) that represents slider
  _value = 0.0;
  // _mapped_value is a mapping of (-1<->1) into (0<->1)
  _mapped_value = 0.5*_value + 0.5;
  // _update_position is mapping of _mapped_value wrt slider width
  _update_position = 0.0; //will be set when _width is defined
  _speed = 0.05;
  _scale = 0.05;
  _bar_state = -1;
  _update_slider = false;
  _slider_only = true;
  _negative_mapping = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGSliderBar::
~PGSliderBar() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::Copy Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
PGSliderBar::
PGSliderBar(const PGSliderBar &copy) :
  PGItem(copy),
  _value(copy._value),
  _range(copy._range),
  //  _slider(copy._slider),
  _left(copy._left),
  _right(copy._right)
{
  _bar_state = -1;
  _slider = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *PGSliderBar::
make_copy() const {
  return new PGSliderBar(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::has_cull_callback
//       Access: Protected, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool PGSliderBar::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::cull_callback
//       Access: Protected, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool PGSliderBar::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  update();
  return PGItem::cull_callback(trav, data);
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::setup
//       Access: Public
//  Description: Creates a PGSliderBar with the indicated dimensions,
//               with the indicated maximum range.
////////////////////////////////////////////////////////////////////
void PGSliderBar::
setup(float width, float height, float range) {
  //set_state(0);
  //clear_state_def(0);

  _width = 0.5 * width; // quick reference to find the left and right max points
  //set_frame(-0.5f * width, 0.5f * width, -0.5f * height, 0.5f * height);
  set_range(range);

  NodePath current = NodePath(this);
  if (!_slider) {
    _slider = new PGSliderButton("slider");
    _slider_button = current.attach_new_node(_slider);
  }
    
  _slider->set_slider_bar(this);
  //_slider->setup(_slider->get_name());
  _slider->setup("");
  _slider->set_drag_n_drop(true);
  _slider->set_frame(-1,1,-1.5,1.5);
  _slider_button.set_scale(_scale);
  _slider_button.set_pos(0, 0, 0); // center it

  // if left or right button to control slider desired, create them
  if (!_slider_only) {
    _left_button = current.attach_new_node(&_left);
    _left.set_slider_bar(this);
    _right_button = current.attach_new_node(&_right);
    _right.set_slider_bar(this);
    _left.setup(_left.get_name());
    _right.setup(_right.get_name());
    _left_button.set_scale(0.5);
    _left_button.set_pos(-(_width+1), 0, -0.25);
    _right_button.set_scale(0.5);
    _right_button.set_pos((_width+0.5), 0, -0.25);
  }

  PGFrameStyle style;
  style.set_width(0.3f, 0.3f);

  /*
  style.set_color(0.6f, 0.6f, 0.6f, 1.0f);
  style.set_type(PGFrameStyle::T_bevel_in);
  //set_frame_style(0, style);
  */

  style.set_color(0.8f, 0.8f, 0.8f, 1.0f);
  style.set_type(PGFrameStyle::T_bevel_out);
  _slider->set_frame_style(PGButton::S_ready, style);
  style.set_type(PGFrameStyle::T_bevel_in);
  _slider->set_frame_style(PGButton::S_depressed, style);
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::press
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void PGSliderBar::
press(const MouseWatcherParameter &param, bool background) {
  PGItem::press(param, background);
  //pgui_cat.info() << get_name() << "::" << param << endl;
  //pgui_cat.info() << _slider->get_name() << "::press:" << _slider_button.get_x() << endl;

  // translate the mouse param position into frame space
  LPoint2f mouse_point = param.get_mouse();
  LVector3f result(mouse_point[0], mouse_point[1], 0);
  result = get_frame_inv_xform().xform_point(result);
  _update_slider = true;
  _update_position = result[0];
  //_slider_button.set_x(result[0]);
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::drag
//       Access: Public, Virtual
//  Description: This is a hook function, called when the user 
//               is trying to drag the slider button 
////////////////////////////////////////////////////////////////////
void PGSliderBar::
drag(const MouseWatcherParameter &param) {
  //pgui_cat.info() << get_name() << "::" << param << endl;
  //pgui_cat.info() << _slider->get_name() << "::drag:" << _slider_button.get_x() << endl;

  // translate the mouse param position into frame space
  LPoint2f mouse_point = param.get_mouse();
  LVector3f result(mouse_point[0], mouse_point[1], 0);
  result = get_frame_inv_xform().xform_point(result);
  // keep the slider button within slider bar
  if (result[0] < -_width)
    result[0] = -_width;
  if (result[0] > _width)
    result[0] = _width;
  _update_slider = true;
  _update_position = result[0];
  //_slider_button.set_x(result[0]);
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBar::update
//       Access: Private
//  Description: Computes the appropriate size of the bar frame
//               according to the percentage completed.
////////////////////////////////////////////////////////////////////
void PGSliderBar:: 
update() {
  //  int state = get_state();

  // need left and right button input if they exist
  if (!_slider_only) {
    // Handle left and right button presses
    if (_left.is_button_down()) {
      // move the slider to the left
      float x = _slider_button.get_x() - _speed;
      _update_slider = true;
      _update_position = max(x, -_width);
      //_slider_button.set_x(max(x, -_width));
    }
    
    if (_right.is_button_down()) {
      // move the slider to the right
      float x = _slider_button.get_x() + _speed;
      _update_slider = true;
      _update_position = min(x, _width);
      //_slider_button.set_x(min(x, _width));
    }
  }

  // press() and drag() update schedules this values that need to be
  // applied here so that value of current slider position as a ratio
  // of range can be updated
  if (_update_slider) {
    //pgui_cat.info() << "mouse_point: " << _update_position << endl;
    if (!_slider_button.is_empty())
      _slider_button.set_x(_update_position);
    _mapped_value = (_update_position + _width)/(2*_width);
    _value = _negative_mapping ? ((_mapped_value-0.5)*2) : _mapped_value;
    _update_slider = false;
    throw_event(get_click_event());
  }
}
