// Filename: AwMouseAndKeyboard.cxx
// Created by:  Bei Yang (Mar2010)
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

#include "config_awesomium.h"
#include "AwMouseAndKeyboard.h"
#include "dataNodeTransmit.h"

TypeHandle AwMouseAndKeyboard::_type_handle;

AwMouseAndKeyboard::AwMouseAndKeyboard(const string &name):
DataNode(name)
{
  _button_events_input = define_input("button_events", ButtonEventList::get_class_type());
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());
}


void AwMouseAndKeyboard::do_transmit_data(DataGraphTraverser *trav, const DataNodeTransmit &input, DataNodeTransmit &output){

  if (input.has_data(_button_events_input)) {
    const ButtonEventList *button_events;
    DCAST_INTO_V(button_events, input.get_data(_button_events_input).get_ptr());
    
    int num_events = button_events->get_num_events();
    for (int i = 0; i < num_events; i++) {
      const ButtonEvent &be = button_events->get_event(i);
      string event_name = be._button.get_name();
      printf("Button Event! : %s with code %i and index %i ", event_name.c_str(), be._keycode, be._button.get_index());
      if(be._type == ButtonEvent::T_down) printf("down");
      if(be._type == ButtonEvent::T_repeat) printf("repeat");
      if(be._type == ButtonEvent::T_up) printf("up");
      if(be._type == ButtonEvent::T_resume_down) printf("T_resume_down");
      printf("\n");
    }
  }

}
