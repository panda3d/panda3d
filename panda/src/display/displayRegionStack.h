// Filename: displayRegionStack.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef DISPLAYREGIONSTACK_H
#define DISPLAYREGIONSTACK_H

#include <pandabase.h>

#include "displayRegion.h"

class GraphicsStateGuardian;

////////////////////////////////////////////////////////////////////
// 	 Class : DisplayRegionStack
// Description : An instance of this kind of object is returned by
//               GraphicsStateGuardian::push_display_region().  It
//               holds the information needed to restore the previous
//               display region in the subsequent matching call to
//               pop_display_region().
////////////////////////////////////////////////////////////////////
class DisplayRegionStack {
public:
  INLINE DisplayRegionStack();
  INLINE ~DisplayRegionStack();
  INLINE DisplayRegionStack(const DisplayRegionStack &copy);
  INLINE void operator =(const DisplayRegionStack &copy);
 
private:
  CPT(DisplayRegion) _display_region;
  int _stack_level;
  friend class GraphicsStateGuardian;
};

#include "displayRegionStack.I"

#endif
