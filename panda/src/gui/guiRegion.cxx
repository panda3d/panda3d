// Filename: guiRegion.cxx
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "config_gui.h"
#include "guiRegion.h"

TypeHandle GuiRegion::_type_handle;

GuiRegion::~GuiRegion(void) {
#ifdef _DEBUG
  if (gui_cat->is_debug())
    gui_cat->debug() << "deleting region '" << *this << "'" << endl;
#endif
}
