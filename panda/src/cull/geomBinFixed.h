// Filename: geomBinFixed.h
// Created by:  drose (14Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GEOMBINFIXED_H
#define GEOMBINFIXED_H

#include <pandabase.h>

#include "geomBin.h"

#include <pointerTo.h>

#include <set>

////////////////////////////////////////////////////////////////////
// 	 Class : GeomBinFixed
// Description : This kind of GeomBin renders its GeomNodes in a
//               user-specified order according to the draw_order
//               specified at each GeomBinTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomBinFixed : public GeomBin {
PUBLISHED:
  INLINE GeomBinFixed(const string &name);

public:
  virtual void clear_current_states();
  virtual void record_current_state(GraphicsStateGuardian *gsg,
				    CullState *cs, int draw_order,
				    CullTraverser *trav);

  virtual void draw(CullTraverser *trav);

private:
  class NodeEntry {
  public:
    INLINE NodeEntry(int draw_order, const PT(CullState) &state, 
		     const ArcChain &arc_chain, bool is_direct);
    INLINE NodeEntry(const NodeEntry &copy);
    INLINE void operator = (const NodeEntry &copy);

    INLINE bool operator < (const NodeEntry &other) const;

    INLINE void draw(CullTraverser *trav) const;

  private:
    int _draw_order;
    PT(CullState) _state;
    ArcChain _arc_chain;
    bool _is_direct;
  };

  typedef multiset<NodeEntry> NodeEntries;
  NodeEntries _node_entries;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomBin::init_type();
    register_type(_type_handle, "GeomBinFixed",
                  GeomBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "geomBinFixed.I"

#endif
