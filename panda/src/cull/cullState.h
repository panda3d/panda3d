// Filename: cullState.h
// Created by:  drose (07Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CULLSTATE_H
#define CULLSTATE_H

#include <pandabase.h>

#include "config_cull.h"

#include <geomNode.h>
#include <allTransitionsWrapper.h>
#include <pointerTo.h>
#include <pointerToArray.h>
#include <updateSeq.h>
#include <referenceCount.h>
#include <arcChain.h>

#include "pmap.h"
#include "pvector.h"

class GraphicsStateGuardian;
class GeomBin;

////////////////////////////////////////////////////////////////////
//       Class : CullState
// Description : This is the basic grouping unit of the CullTraverser.
//               A single CullState object represents all of the
//               GeomNodes throughout the scene graph that share an
//               identical state.  The CullTraverser collects these as
//               it traverses, and tries to cache them between
//               subsequent frames.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullState : public ReferenceCount {
public:
  typedef pvector<ArcChain> CurrentGeomNodes;
  typedef pvector<ArcChain> CurrentDirectNodes;

public:
  INLINE CullState(const AllTransitionsWrapper &trans);
  INLINE ~CullState();

  INLINE int compare_to(const CullState &other) const;

  bool check_currency(Node *node,
                      const AllTransitionsWrapper &trans,
                      UpdateSeq as_of);

  INLINE void mark_verified(Node *node, UpdateSeq now);

  INLINE void clear_current_nodes();
  INLINE void record_current_geom_node(const ArcChain &arc_chain);
  INLINE void record_current_direct_node(const ArcChain &arc_chain);
  INLINE bool is_empty() const;
  INLINE int count_current_nodes() const;
  INLINE int get_empty_frames_count() const;

  INLINE void apply_to(const AllTransitionsWrapper &initial_state);
  INLINE const AllTransitionsWrapper &get_attributes() const;
  INLINE const AllTransitionsWrapper &get_transitions() const;

  INLINE bool has_bin() const;
  INLINE GeomBin *get_bin() const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:
  // Functions and typedefs to support treating the CullState as a
  // read-only STL container of current nodes.  Beware!  This
  // interface is not safe to use outside of PANDA.DLL.
  typedef CurrentGeomNodes::const_iterator geom_iterator;
  typedef CurrentGeomNodes::const_iterator geom_const_iterator;
  typedef CurrentGeomNodes::value_type geom_value_type;
  typedef CurrentGeomNodes::size_type geom_size_type;

  INLINE geom_size_type geom_size() const;
  INLINE geom_iterator geom_begin() const;
  INLINE geom_iterator geom_end() const;

  typedef CurrentDirectNodes::const_iterator direct_iterator;
  typedef CurrentDirectNodes::const_iterator direct_const_iterator;
  typedef CurrentDirectNodes::value_type direct_value_type;
  typedef CurrentDirectNodes::size_type direct_size_type;

  INLINE direct_size_type direct_size() const;
  INLINE direct_iterator direct_begin() const;
  INLINE direct_iterator direct_end() const;

private:
  AllTransitionsWrapper _trans;
  AllTransitionsWrapper _attrib;

  typedef pmap<Node *, UpdateSeq> Verified;
  Verified _verified;

  CurrentGeomNodes _current_geom_nodes;
  CurrentDirectNodes _current_direct_nodes;
  int _empty_frames_count;

  GeomBin *_bin;
  friend class GeomBin;
};

INLINE ostream &operator << (ostream &out, const CullState &bs) {
  bs.output(out);
  return out;
}

#include "cullState.I"

#endif
