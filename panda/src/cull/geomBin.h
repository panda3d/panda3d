// Filename: geomBin.h
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GEOMBIN_H
#define GEOMBIN_H

#include <pandabase.h>

#include "cullState.h"
#include "cullStateLookup.h"

#include <geomNode.h>
#include <allTransitionsWrapper.h>
#include <pointerTo.h>
#include <indirectCompareTo.h>
#include <typedReferenceCount.h>
#include <namable.h>

#include <set>

class GeomNode;
class CullTraverser;

////////////////////////////////////////////////////////////////////
// 	 Class : GeomBin
// Description : This is an abstract class that defines the interface
//               for a number of different kinds of bins that may be
//               assigned to a CullTraverser.  The traverser will
//               assign GeomNodes to the bins according to the various
//               GeomBinTransitions encountered in the scene graph;
//               the individual GeomBins are then responsible for
//               sorting and rendering the geometry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomBin : public TypedReferenceCount, public Namable {
public:
  INLINE GeomBin(const string &name, CullTraverser *traverser = NULL, 
		 int sort = 0);
  virtual ~GeomBin();

  void attach_to(CullTraverser *traverser, int sort = 0);
  PT(GeomBin) detach();

  INLINE CullTraverser *get_traverser() const;
  INLINE int get_sort() const;
  INLINE void set_sort(int sort);

  virtual void clear_current_states()=0;
  virtual void record_current_state(GraphicsStateGuardian *gsg,
				    CullState *cs, int draw_order,
				    CullTraverser *trav)=0;
  virtual void remove_state(CullState *cs);

  virtual void draw(CullTraverser *trav)=0;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  INLINE void claim_cull_state(CullState *cs);
  INLINE void disclaim_cull_state(CullState *cs);

  CullTraverser *_traverser;
  int _sort;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "GeomBin",
                  TypedReferenceCount::get_class_type(),
		  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const GeomBin &bin) {
  bin.output(out);
  return out;
}

#include "geomBin.I"

#endif
