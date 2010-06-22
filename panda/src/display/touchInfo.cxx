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

int TouchInfo::get_x(){
  return _x;
}

int TouchInfo::get_y(){
  return _y;
}

int TouchInfo::get_id(){
  return _id;
}

int TouchInfo::get_flags(){
  return _flags;
}

void TouchInfo::set_x(int x){
  _x = x;
}

void TouchInfo::set_y(int y){
  _y = y;
}

void TouchInfo::set_id(int id){
  _id = id;
}

void TouchInfo::set_flags(int flags){
  _flags = flags;
}
