// Filename: glImmediateModeSender_src.I
// Created by:  drose (15Aug05)
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


////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CLP(ImmediateModeSender)::
CLP(ImmediateModeSender)() {
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::ComponentSender::Constructor
//       Access: Public
//  Description: The ComponentSender becomes the owner of the
//               GeomVertexReader pointer, and will delete it when it
//               is done.
////////////////////////////////////////////////////////////////////
INLINE CLP(ImmediateModeSender)::ComponentSender::
ComponentSender(GeomVertexReader *reader) :
  _reader(reader)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::ComponentSender::set_vertex
//       Access: Public
//  Description: Specifies the vertex index of the next vertex to
//               send.  If this is not called, the next consecutive
//               vertex will be sent.
////////////////////////////////////////////////////////////////////
INLINE void CLP(ImmediateModeSender)::ComponentSender::
set_vertex(int vertex_index) {
  _reader->set_row(vertex_index);
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::ComponentSender1f::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLP(ImmediateModeSender)::ComponentSender1f::
ComponentSender1f(GeomVertexReader *reader, Func1f *func) :
  ComponentSender(reader),
  _func(func)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::ComponentSender2f::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLP(ImmediateModeSender)::ComponentSender2f::
ComponentSender2f(GeomVertexReader *reader, Func2f *func) :
  ComponentSender(reader),
  _func(func)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::ComponentSender3f::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLP(ImmediateModeSender)::ComponentSender3f::
ComponentSender3f(GeomVertexReader *reader, Func3f *func) :
  ComponentSender(reader),
  _func(func)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::ComponentSender4f::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLP(ImmediateModeSender)::ComponentSender4f::
ComponentSender4f(GeomVertexReader *reader, Func4f *func) :
  ComponentSender(reader),
  _func(func)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::TexcoordSender1f::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLP(ImmediateModeSender)::TexcoordSender1f::
TexcoordSender1f(GeomVertexReader *reader, TexcoordFunc1f *func, 
                 int stage_index) :
  ComponentSender(reader),
  _func(func),
  _stage_index(stage_index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::TexcoordSender2f::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLP(ImmediateModeSender)::TexcoordSender2f::
TexcoordSender2f(GeomVertexReader *reader, TexcoordFunc2f *func, 
                 int stage_index) :
  ComponentSender(reader),
  _func(func),
  _stage_index(stage_index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::TexcoordSender3f::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLP(ImmediateModeSender)::TexcoordSender3f::
TexcoordSender3f(GeomVertexReader *reader, TexcoordFunc3f *func, 
                 int stage_index) :
  ComponentSender(reader),
  _func(func),
  _stage_index(stage_index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::TexcoordSender4f::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLP(ImmediateModeSender)::TexcoordSender4f::
TexcoordSender4f(GeomVertexReader *reader, TexcoordFunc4f *func, 
                 int stage_index) :
  ComponentSender(reader),
  _func(func),
  _stage_index(stage_index)
{
}
