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

  INLINE NullAttributeWrapper();
  INLINE NullAttributeWrapper(const NullAttributeWrapper &copy);
  INLINE void operator = (const NullAttributeWrapper &copy);
  INLINE static NullAttributeWrapper
  init_from(const NullTransitionWrapper &trans);

  INLINE bool is_initial() const;
  INLINE int compare_to(const NullAttributeWrapper &other) const;

  INLINE void make_initial();
  INLINE void apply_in_place(const NullTransitionWrapper &trans);

  INLINE void output(ostream &out) const;
  INLINE void write(ostream &out, int indent_level = 0) const;
};

INLINE ostream &operator << (ostream &out, const NullAttributeWrapper &naw) {
  naw.output(out);
  return out;
}

#include "nullAttributeWrapper.I"

#endif
