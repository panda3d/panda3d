// Filename: nullAttributeWrapper.h
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NULLATTRIBUTEWRAPPER_H
#define NULLATTRIBUTEWRAPPER_H

#include <pandabase.h>

#include "nodeAttribute.h"

#include <updateSeq.h>
#include <pointerTo.h>

class NullTransitionWrapper;

////////////////////////////////////////////////////////////////////
// 	 Class : NullAttributeWrapper
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NullAttributeWrapper {
public:
  typedef NullTransitionWrapper TransitionWrapper;
  typedef NullAttributeWrapper AttributeWrapper;

  INLINE_GRAPH NullAttributeWrapper();
  INLINE_GRAPH NullAttributeWrapper(const NullAttributeWrapper &copy);
  INLINE_GRAPH void operator = (const NullAttributeWrapper &copy);
  INLINE_GRAPH static NullAttributeWrapper
  init_from(const NullTransitionWrapper &trans);

  INLINE_GRAPH bool is_initial() const;
  INLINE_GRAPH int compare_to(const NullAttributeWrapper &other) const;

  INLINE_GRAPH void make_initial();
  INLINE_GRAPH void apply_in_place(const NullTransitionWrapper &trans);

  INLINE_GRAPH void output(ostream &out) const;
  INLINE_GRAPH void write(ostream &out, int indent_level = 0) const;
};

INLINE_GRAPH ostream &operator << (ostream &out, const NullAttributeWrapper &naw);

#ifndef DONT_INLINE_GRAPH
#include "nullAttributeWrapper.I"
#endif

#endif
