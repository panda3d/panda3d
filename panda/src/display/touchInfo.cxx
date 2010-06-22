// Filename: touchInfo.cxx
// Created by:  Walt Destler (May 25, 2010)
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

#include "touchInfo.h"

TouchInfo::TouchInfo(){
	_x = 0;
	_y = 0;
	_id = 0;
	_flags = 0;
}

LONG TouchInfo::get_x(){
	return _x;
}

LONG TouchInfo::get_y(){
	return _y;
}

DWORD TouchInfo::get_id(){
	return _id;
}

DWORD TouchInfo::get_flags(){
	return _flags;
}

void TouchInfo::set_x(LONG x){
	_x = x;
}

void TouchInfo::set_y(LONG y){
	_y = y;
}

void TouchInfo::set_id(DWORD id){
	_id = id;
}

void TouchInfo::set_flags(DWORD flags){
	_flags = flags;
}
