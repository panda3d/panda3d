// Filename: glVertexBufferContext_src.I
// Created by:  drose (17Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CLP(VertexBufferContext)::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CLP(VertexBufferContext)::
CLP(VertexBufferContext)(CLP(GraphicsStateGuardian) *glgsg,
                         PreparedGraphicsObjects *pgo, 
                         GeomVertexArrayData *data) :
  VertexBufferContext(pgo, data),
  _glgsg(glgsg)
{
  _index = 0;
}
