/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullPlanes.h
 * @author drose
 * @date 2005-08-23
 */

#ifndef CULLPLANES_H
#define CULLPLANES_H

#include "pandabase.h"
#include "referenceCount.h"
#include "nodePath.h"
#include "boundingHexahedron.h"
#include "boundingPlane.h"
#include "pointerTo.h"
#include "luse.h"
#include "deletedChain.h"

class ClipPlaneAttrib;
class OccluderEffect;

/**
 * This represents the set of clip planes and/or occluders that are definitely
 * in effect for the current node of the CullTraverserData, as well as on all
 * child nodes.  Any clip planes and occluders in this list may be safely
 * culled against.
 *
 * This does not include the clip planes that are in effect now, but might
 * later be turned off by a child node, since we can't safely cull against
 * such clip planes.
 *
 * The bounding volumes in this object are transformed for each level of the
 * scene graph.
 */
class EXPCL_PANDA_PGRAPH CullPlanes : public ReferenceCount {
protected:
  INLINE CullPlanes();
  INLINE CullPlanes(const CullPlanes &copy);
  INLINE void operator = (const CullPlanes &copy);

public:
  INLINE ~CullPlanes();
  ALLOC_DELETED_CHAIN(CullPlanes);

  INLINE bool is_empty() const;

  static CPT(CullPlanes) make_empty();
  CPT(CullPlanes) xform(const LMatrix4 &mat) const;
  CPT(CullPlanes) apply_state(const CullTraverser *trav,
                              const CullTraverserData *data,
                              const ClipPlaneAttrib *net_attrib,
                              const ClipPlaneAttrib *off_attrib,
                              const OccluderEffect *node_effect) const;
  CPT(CullPlanes) do_cull(int &result, CPT(RenderState) &state,
                          const GeometricBoundingVolume *node_gbv) const;

  CPT(CullPlanes) remove_plane(const NodePath &clip_plane) const;
  CPT(CullPlanes) remove_occluder(const NodePath &occluder) const;

  void write(std::ostream &out) const;

private:
  typedef pmap<NodePath, PT(BoundingPlane) > Planes;
  Planes _planes;

  typedef pmap<NodePath, PT(BoundingHexahedron) > Occluders;
  Occluders _occluders;
};

// We can safely redefine this as a no-op.
template<>
INLINE void PointerToBase<CullPlanes>::update_type(To *ptr) {}

#include "cullPlanes.I"

#endif
