// Filename: dataRelation.cxx
// Created by:  drose (08Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "dataRelation.h"
#include "transformTransition.h"

TypeHandle DataRelation::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DataRelation::make_arc
//       Access: Public, Static
//  Description: This function is passed to the Factory to make a new
//               DataRelation by type.  Don't try to call this
//               function directly.
////////////////////////////////////////////////////////////////////
NodeRelation *DataRelation::
make_arc(const FactoryParams &) {
  return new DataRelation;
}
