// Filename: mouseWatcherRegion.cxx
// Created by:  drose (13Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "mouseWatcherRegion.h"

#include <indent.h>


TypeHandle MouseWatcherRegion::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
output(ostream &out) const {
  out << get_name() << " lrbt = " << _frame;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << get_name() << " lrbt = " << _frame << "\n";
}

