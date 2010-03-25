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

AwMouseAndKeyboard::AwMouseAndKeyboard(GraphicsWindow *window, int device, const string &name):
MouseAndKeyboard(window,device,name)
{
	//do nothing
}


void AwMouseAndKeyboard::do_transmit_data(DataGraphTraverser *trav, const DataNodeTransmit &input, DataNodeTransmit &output){

	MouseAndKeyboard::do_transmit_data(trav,input,output);

		int num_events = _button_events->get_num_events();
		for (int i = 0; i < num_events; i++) {
			const ButtonEvent &be = _button_events->get_event(i);
			string event_name = be._button.get_name();
			printf("Button pressed: %s ", event_name);
			if(be._type == ButtonEvent::T_down ) printf(" down ");
			if(be._type == ButtonEvent::T_repeat ) printf(" repeat ");
			if(be._type == ButtonEvent::T_resume_down ) printf(" resume down ");
			if(be._type == ButtonEvent::T_resume_down ) printf(" up ");
		}
}
