// Filename: allAttributesWrapper.h
// Created by:  drose (21Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ALLATTRIBUTESWRAPPER_H
#define ALLATTRIBUTESWRAPPER_H

#include <pandabase.h>

#include "nodeAttributes.h"

#include <updateSeq.h>
#include <pointerTo.h>

class AllTransitionsWrapper;

////////////////////////////////////////////////////////////////////
// 	 Class : AllAttributesWrapper
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AllAttributesWrapper {
public:
  typedef AllTransitionsWrapper TransitionWrapper;
  typedef AllAttributesWrapper AttributeWrapper;

  INLINE AllAttributesWrapper();
  INLINE AllAttributesWrapper(const NodeAttributes &attrib);
  INLINE AllAttributesWrapper(const AllAttributesWrapper &copy);
  INLINE void operator = (const AllAttributesWrapper &copy);
  INLINE static AllAttributesWrapper 
  init_from(const AllTransitionsWrapper &trans);
  INLINE ~AllAttributesWrapper();

  INLINE bool is_empty() const;
  INLINE PT(NodeAttribute) set_attribute(TypeHandle handle,
					 NodeAttribute *trans);
  INLINE PT(NodeAttribute) set_attribute(NodeAttribute *trans);
  INLINE PT(NodeAttribute) clear_attribute(TypeHandle handle);
  INLINE bool has_attribute(TypeHandle handle) const;
  INLINE NodeAttribute *get_attribute(TypeHandle handle) const;

  INLINE const NodeAttributes &get_attributes() const;
  INLINE NodeAttributes &get_attributes();

  INLINE bool is_initial() const;
  INLINE int compare_to(const AllAttributesWrapper &other) const;

  INLINE void make_initial();
  void apply_in_place(const AllTransitionsWrapper &trans);
  void apply_from(const AllAttributesWrapper &other, 
		  const AllTransitionsWrapper &trans);
  NodeAttributes *apply(const AllTransitionsWrapper &trans) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  NodeAttributes _attrib;
};

INLINE ostream &operator << (ostream &out, const AllAttributesWrapper &a) {
  a.output(out);
  return out;
}

template<class Attribute>
INLINE bool 
get_attribute_into(Attribute *&ptr, const AllAttributesWrapper &attrib,
		   TypeHandle transition_type);

#include "allAttributesWrapper.I"

#endif
