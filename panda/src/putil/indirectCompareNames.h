// Filename: indirectCompareNames.h
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef INDIRECTCOMPARENAMES_H
#define INDIRECTCOMPARENAMES_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
// 	 Class : IndirectCompareNames
// Description : An STL function object class, this is intended to be
//               used on any ordered collection of pointers to classes
//               that define a get_name() method, particularly for
//               things that derive from Namable.  It defines the
//               order of the pointers by case-sensitive name
//               comparison.
////////////////////////////////////////////////////////////////////
template<class ObjectType>
class IndirectCompareNames {
public:
  INLINE bool operator () (const ObjectType *a, const ObjectType *b) const;
};

#include "indirectCompareNames.I"

#endif

