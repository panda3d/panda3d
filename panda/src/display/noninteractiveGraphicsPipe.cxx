// Filename: noninteractiveGraphicsPipe.cxx
// Created by:  cary (10Mar99)
// 
////////////////////////////////////////////////////////////////////

#include "noninteractiveGraphicsPipe.h"
#include "config_display.h"

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
