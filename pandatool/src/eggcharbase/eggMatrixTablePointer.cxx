// Filename: eggMatrixTablePointer.cxx
// Created by:  drose (26Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggMatrixTablePointer.h"

#include <eggXfmAnimData.h>
#include <eggXfmSAnim.h>

TypeHandle EggMatrixTablePointer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggMatrixTablePointer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggMatrixTablePointer::
EggMatrixTablePointer(EggObject *object) {
  _table = DCAST(EggTable, object);

  if (_table != (EggTable *)NULL) {
    // Now search for the child named "xform".  This contains the
    // actual table data.
    EggGroupNode::iterator ci;
    bool found = false;
    for (ci = _table->begin(); ci != _table->end() && !found; ++ci) {
      EggNode *child = (*ci);
      if (child->get_name() == "xform") {
	if (child->is_of_type(EggXfmSAnim::get_class_type())) {
	  _xform = DCAST(EggXfmSAnim, child);
	  found = true;

	} else if (child->is_of_type(EggXfmAnimData::get_class_type())) {
	  // Quietly replace old-style XfmAnim tables with new-style
	  // XfmSAnim tables.
	  PT(EggXfmAnimData) anim = DCAST(EggXfmAnimData, child);
	  _xform = new EggXfmSAnim(*anim);
	  _table->replace(ci, _xform.p());
	  found = true;
	}
      }
    }
  }
}
