// Filename: geomBinGroup.cxx
// Created by:  drose (13Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "geomBinGroup.h"
#include "cullTraverser.h"

#include <indent.h>
#include <nodeAttributes.h>
#include <graphicsStateGuardian.h>

TypeHandle GeomBinGroup::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::clear_current_states
//       Access: Public, Virtual
//  Description: Called each frame by the CullTraverser to reset the
//               list of CullStates that were added last frame, in
//               preparation for defining a new set of CullStates
//               visible this frame.
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
clear_current_states() {
  SubBins::iterator sbi;
  for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
    (*sbi)->clear_current_states();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinUnsorted::record_current_state
//       Access: Public, Virtual
//  Description: Called each frame by the CullTraverser to indicated
//               that the given CullState (and all of its current
//               GeomNodes) is visible this frame.
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
record_current_state(GraphicsStateGuardian *gsg, CullState *cs, 
		     int draw_order, CullTraverser *trav) {
  int index = choose_bin(cs);
  if (index >= 0 && index < get_num_bins()) {
    get_bin(index)->record_current_state(gsg, cs, draw_order, trav);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::draw
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
draw(CullTraverser *trav) {
  SubBins::iterator sbi;
  for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
    (*sbi)->draw(trav);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
output(ostream &out) const {
  out << get_type() << " " << get_name()
      << ", " << _sub_bins.size() << " bins.";
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << " " << get_name() << " {\n";

  SubBins::const_iterator sbi;
  for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
    (*sbi)->write(out, indent_level + 2);
  }

  indent(out, indent_level) << "}\n";
}
