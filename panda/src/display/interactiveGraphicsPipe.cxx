// Filename: interactiveGraphicsPipe.cxx
// Created by:  cary (10Mar99)
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

#include "interactiveGraphicsPipe.h"
#include "config_display.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle InteractiveGraphicsPipe::_type_handle;

InteractiveGraphicsPipe::InteractiveGraphicsPipe(const PipeSpecifier& spec)
  : GraphicsPipe(spec) {}

InteractiveGraphicsPipe::InteractiveGraphicsPipe(void) {
  display_cat.error()
    << "InteractiveGraphicsPipe should not be created with default constructor" << endl;
}

InteractiveGraphicsPipe::InteractiveGraphicsPipe(const InteractiveGraphicsPipe&) {
  display_cat.error()
    << "InteractiveGraphicsPipes should not be copied" << endl;
}

InteractiveGraphicsPipe& InteractiveGraphicsPipe::operator=(const InteractiveGraphicsPipe&) {
  display_cat.error()
  << "InteractiveGraphicsPipes should not be assigned" << endl;
  return *this;
}

InteractiveGraphicsPipe::~InteractiveGraphicsPipe(void) {}

TypeHandle InteractiveGraphicsPipe::get_class_type(void) {
  return _type_handle;
}

void InteractiveGraphicsPipe::init_type(void) {
  GraphicsPipe::init_type();
  register_type(_type_handle, "InteractiveGraphicsPipe",
                GraphicsPipe::get_class_type());
}

TypeHandle InteractiveGraphicsPipe::get_type(void) const {
  return get_class_type();
}
