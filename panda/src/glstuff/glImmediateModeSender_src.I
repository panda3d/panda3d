/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glImmediateModeSender_src.I
 * @author drose
 * @date 2005-08-15
 */

/**
 *
 */
INLINE CLP(ImmediateModeSender)::
CLP(ImmediateModeSender)() {
}

/**
 * The ComponentSender becomes the owner of the GeomVertexReader pointer, and
 * will delete it when it is done.
 */
INLINE CLP(ImmediateModeSender)::ComponentSender::
ComponentSender(GeomVertexReader *reader) :
  _reader(reader)
{
}

/**
 * Specifies the vertex index of the next vertex to send.  If this is not
 * called, the next consecutive vertex will be sent.
 */
INLINE void CLP(ImmediateModeSender)::ComponentSender::
set_vertex(int vertex_index) {
  _reader->set_row(vertex_index);
}


/**
 *
 */
INLINE CLP(ImmediateModeSender)::ComponentSender1f::
ComponentSender1f(GeomVertexReader *reader, Func1f *func) :
  ComponentSender(reader),
  _func(func)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::ComponentSender2f::
ComponentSender2f(GeomVertexReader *reader, Func2f *func) :
  ComponentSender(reader),
  _func(func)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::ComponentSender3f::
ComponentSender3f(GeomVertexReader *reader, Func3f *func) :
  ComponentSender(reader),
  _func(func)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::ComponentSender4f::
ComponentSender4f(GeomVertexReader *reader, Func4f *func) :
  ComponentSender(reader),
  _func(func)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::TexcoordSender1f::
TexcoordSender1f(GeomVertexReader *reader, TexcoordFunc1f *func,
                 int stage_index) :
  ComponentSender(reader),
  _func(func),
  _stage_index(stage_index)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::TexcoordSender2f::
TexcoordSender2f(GeomVertexReader *reader, TexcoordFunc2f *func,
                 int stage_index) :
  ComponentSender(reader),
  _func(func),
  _stage_index(stage_index)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::TexcoordSender3f::
TexcoordSender3f(GeomVertexReader *reader, TexcoordFunc3f *func,
                 int stage_index) :
  ComponentSender(reader),
  _func(func),
  _stage_index(stage_index)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::TexcoordSender4f::
TexcoordSender4f(GeomVertexReader *reader, TexcoordFunc4f *func,
                 int stage_index) :
  ComponentSender(reader),
  _func(func),
  _stage_index(stage_index)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::VectorSender1f::
VectorSender1f(GeomVertexReader *reader, VectorFunc *func) :
  ComponentSender(reader),
  _func(func)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::VectorSender2f::
VectorSender2f(GeomVertexReader *reader, VectorFunc *func) :
  ComponentSender(reader),
  _func(func)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::VectorSender3f::
VectorSender3f(GeomVertexReader *reader, VectorFunc *func) :
  ComponentSender(reader),
  _func(func)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::VectorSender4f::
VectorSender4f(GeomVertexReader *reader, VectorFunc *func) :
  ComponentSender(reader),
  _func(func)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::VectorSender1ui::
VectorSender1ui(GeomVertexReader *reader, VectorUintFunc *func) :
  ComponentSender(reader),
  _func(func)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::VectorSender2fui::
VectorSender2fui(GeomVertexReader *reader, VectorUintFunc *func) :
  ComponentSender(reader),
  _func(func)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::VectorSender3fui::
VectorSender3fui(GeomVertexReader *reader, VectorUintFunc *func) :
  ComponentSender(reader),
  _func(func)
{
}

/**
 *
 */
INLINE CLP(ImmediateModeSender)::VectorSender4fui::
VectorSender4fui(GeomVertexReader *reader, VectorUintFunc *func) :
  ComponentSender(reader),
  _func(func)
{
}
