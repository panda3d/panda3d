// Filename: cullPlanes.h
// Created by:  drose (23Aug05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CULLPLANES_H
#define CULLPLANES_H

#include "pandabase.h"
#include "referenceCount.h"
#include "nodePath.h"
#include "clipPlaneAttrib.h"
#include "boundingPlane.h"
#include "pointerTo.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : CullPlanes
// Description : This represents the set of clip planes that are
//               definitely in effect for the current node of the
//               CullTraverserData, as well as on all child nodes.
//               Any clip planes in this list may be safely culled
//               against.
//
//               This does not include the clip planes that are in
//               effect now, but might later be turned off by a child
//               node, since we can't safely cull against such clip
//               planes.
//
//               The bounding volumes in this object are transformed
//               for each level of the scene graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullPlanes : public ReferenceCount {
protected:
  INLINE CullPlanes();
  INLINE CullPlanes(const CullPlanes &copy);
  INLINE void operator = (const CullPlanes &copy); 

public:
  INLINE ~CullPlanes();

  INLINE bool is_empty() const;

  static CPT(CullPlanes) make_empty();
  CPT(CullPlanes) xform(const LMatrix4f &mat) const;
  CPT(CullPlanes) apply_state(const CullTraverser *trav, 
                              const CullTraverserData *data,
                              const ClipPlaneAttrib *net_attrib,
                              const ClipPlaneAttrib *off_attrib) const;
  CPT(CullPlanes) do_cull(int &result, CPT(RenderState) &state,
                          const GeometricBoundingVolume *node_gbv) const;

  CPT(CullPlanes) remove_plane(const NodePath &clip_plane) const;

  void write(ostream &out) const;

private:
  typedef pmap<NodePath, PT(BoundingPlane) > Planes;
  Planes _planes;
};

#include "cullPlanes.I"

#endif
