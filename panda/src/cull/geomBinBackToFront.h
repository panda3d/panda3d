// Filename: geomBinBackToFront.h
// Created by:  drose (13Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GEOMBINBACKTOFRONT_H
#define GEOMBINBACKTOFRONT_H

#include <pandabase.h>

#include "geomBin.h"

#include <pointerTo.h>

#include <set>

////////////////////////////////////////////////////////////////////
// 	 Class : GeomBinBackToFront
// Description : This kind of GeomBin renders its GeomNodes in order
//               according to distance from the camera plane,
//               beginning with the farthest away.  It's particularly
//               appropriate for rendering transparent geometry, but
//               it may also be useful for rendering simple scenes
//               without a Z-buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomBinBackToFront : public GeomBin {
PUBLISHED:
  INLINE GeomBinBackToFront(const string &name);

public:
  virtual void clear_current_states();
  virtual void record_current_state(GraphicsStateGuardian *gsg,
				    CullState *cs, int draw_order,
				    CullTraverser *trav);

  virtual void draw(CullTraverser *trav);

private:
  class NodeEntry {
  public:
    INLINE NodeEntry(float distance, const PT(CullState) &state, 
		     const ArcChain &arc_chain, bool is_direct);
    INLINE NodeEntry(const NodeEntry &copy);
    INLINE void operator = (const NodeEntry &copy);

    INLINE bool operator < (const NodeEntry &other) const;

    INLINE void draw(CullTraverser *trav) const;

  private:
    float _distance;
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
    register_type(_type_handle, "GeomBinBackToFront",
                  GeomBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "geomBinBackToFront.I"

#endif
