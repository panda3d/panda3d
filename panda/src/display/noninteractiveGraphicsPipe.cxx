// Filename: noninteractiveGraphicsPipe.cxx
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

#if defined(WIN32_VC) && !defined(NO_PCH)
#include "display_headers.h"
#endif

#pragma hdrstop

#if !defined(WIN32_VC) || defined(NO_PCH)
#include "noninteractiveGraphicsPipe.h"
#include "config_display.h"
#endif

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle NoninteractiveGraphicsPipe::_type_handle;

NoninteractiveGraphicsPipe::NoninteractiveGraphicsPipe(const PipeSpecifier& spec)
  : GraphicsPipe(spec) {}

NoninteractiveGraphicsPipe::~NoninteractiveGraphicsPipe(void) {}

NoninteractiveGraphicsPipe::NoninteractiveGraphicsPipe(void) {
  display_cat.error()
    << "NoninteractiveGraphicsPipe should not be created with default constructor" << endl;
}

NoninteractiveGraphicsPipe::NoninteractiveGraphicsPipe(const NoninteractiveGraphicsPipe&) {
  display_cat.error()
    << "NoninteractiveGraphicsPipes should not be copied" << endl;
}

NoninteractiveGraphicsPipe& NoninteractiveGraphicsPipe::operator=(const NoninteractiveGraphicsPipe&) {
  display_cat.error()
  << "NoninteractiveGraphicsPipes should not be assigned" << endl;
  return *this;
}

TypeHandle NoninteractiveGraphicsPipe::get_class_type(void) {
  return _type_handle;
}

void NoninteractiveGraphicsPipe::init_type(void) {
  GraphicsPipe::init_type();
  register_type(_type_handle, "NoninteractiveGraphicsPipe",
                GraphicsPipe::get_class_type());
}

TypeHandle NoninteractiveGraphicsPipe::get_type(void) const {
  return get_class_type();
}
