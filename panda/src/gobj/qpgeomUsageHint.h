// Filename: qpgeomUsageHint.h
// Created by:  drose (18Mar05)
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

#ifndef qpGEOMUSAGEHINT_H
#define qpGEOMUSAGEHINT_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : qpGeomUsageHint
// Description : This class exists just to provide scoping for the
//               UsageHint enumerated type, which is used by
//               GeomVertexData, GeomVertexArrayData, and
//               GeomPrimitive.
//
//               The usage hint describes to the rendering backend how
//               often the data in question will be modified and/or
//               rendered.  It allows the backend to make appropriate
//               choices about what part of memory the data should be
//               stored in.
//
//               The hint is provided as a performance optimization
//               only, and does not constraing actual usage; although
//               it may be an important optimization.
//
//               In general, the hint may only be specified at the
//               time the data object is constructed.  If you need to
//               change it, you must create a new object (but in many
//               cases you can just assign the same internal data
//               pointer to the new object, to keep the same
//               client-side memory).
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomUsageHint {
PUBLISHED:
  enum UsageHint {
    // UH_client: don't attempt to upload the data; always keep it on
    // the client.
    UH_client,

    // UH_stream: the data will be created once, used to render a few
    // times, and then discarded.  This should be used for short-lived
    // temporary arrays.
    UH_stream,

    // UH_static: the data will be created once, and used to render
    // many times, without modification.  This is the most common
    // case, since typically vertex data is not directly animated
    // (this is not related to scene graph animation, e.g. from
    // adjusting transforms on a node).
    UH_static,

    // UH_dynamic: the data will be repeatedly modified and
    // re-rendered.  This is for data that will be modified at
    // runtime, such as animated or soft-skinned vertices.
    UH_dynamic,
  };
};

#endif

