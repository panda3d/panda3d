/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glImmediateModeSender_src.cxx
 * @author drose
 * @date 2005-08-15
 */

#ifdef SUPPORT_IMMEDIATE_MODE

/**
 *
 */
CLP(ImmediateModeSender)::
~CLP(ImmediateModeSender)() {
  clear();
}

/**
 * Removes (and deletes) all of the senders from the object.
 */
void CLP(ImmediateModeSender)::
clear() {
  ComponentSenders::iterator si;
  for (si = _senders.begin(); si != _senders.end(); ++si) {
    delete (*si);
  }
  _senders.clear();
}

/**
 * Specifies the vertex index of the next vertex to send.  If this is not
 * called, the next consecutive vertex will be sent.
 */
void CLP(ImmediateModeSender)::
set_vertex(int vertex_index) {
  ComponentSenders::iterator si;
  for (si = _senders.begin(); si != _senders.end(); ++si) {
    (*si)->set_vertex(vertex_index);
  }
}

/**
 * Sends the next vertex to the OpenGL API.
 */
void CLP(ImmediateModeSender)::
issue_vertex() {
  ComponentSenders::iterator si;
  for (si = _senders.begin(); si != _senders.end(); ++si) {
    (*si)->issue_vertex();
  }
}

/**
 * Creates a new ComponentSender for the named data column, if it exists in
 * the vertex data, and adds it to the list of senders for this object.
 *
 * The four function pointers are the four variants on the function pointer
 * for the possible number of components of the data column.  The appropriate
 * pointer will be used, depending on the number of components the data column
 * actually uses.
 *
 * The return value is true if the column is added, false if it is not for
 * some reason (for instance, the named column doesn't exist in the vertex
 * data).
 */
bool CLP(ImmediateModeSender)::
add_column(const GeomVertexDataPipelineReader *data_reader, const InternalName *name,
           Func1f *func1f, Func2f *func2, Func3f *func3, Func4f *func4) {
  if (data_reader->has_column(name)) {
    GeomVertexReader *reader = new GeomVertexReader(data_reader, name);
    ComponentSender *sender = nullptr;
    const GeomVertexColumn *column = reader->get_column();
    switch (column->get_num_components()) {
    case 1:
      if (func1f != nullptr) {
        sender = new ComponentSender1f(reader, func1f);
      }
      break;

    case 2:
      if (func2 != nullptr) {
        sender = new ComponentSender2f(reader, func2);
      }
      break;

    case 3:
      if (func3 != nullptr) {
        sender = new ComponentSender3f(reader, func3);
      }
      break;

    case 4:
      if (func4 != nullptr) {
        sender = new ComponentSender4f(reader, func4);
      }
      break;
    }

    if (sender != nullptr) {
      // Ok, we've got a valid sender; add it to the list.
      _senders.push_back(sender);
      return true;

    } else {
      // We didn't get a valid sender; clean up and return.
      delete reader;
    }
  }

  return false;
}

/**
 * Creates a new ComponentSender for the named data column, if it exists in
 * the vertex data, and adds it to the list of senders for this object.
 *
 * This works like add_column(), but it specifically handles a texcoord-style
 * column, which requires one additional parameter to OpenGL: the texture
 * stage.
 *
 * The return value is true if the column is added, false if it is not for
 * some reason (for instance, the named column doesn't exist in the vertex
 * data).
 */
bool CLP(ImmediateModeSender)::
add_texcoord_column(const GeomVertexDataPipelineReader *data_reader,
                    const InternalName *name, int stage_index,
                    TexcoordFunc1f *func1f, TexcoordFunc2f *func2,
                    TexcoordFunc3f *func3, TexcoordFunc4f *func4) {
  if (data_reader->has_column(name)) {
    GeomVertexReader *reader = new GeomVertexReader(data_reader, name);
    ComponentSender *sender = nullptr;
    const GeomVertexColumn *column = reader->get_column();
    switch (column->get_num_components()) {
    case 1:
      sender = new TexcoordSender1f(reader, func1f, stage_index);
      break;

    case 2:
      sender = new TexcoordSender2f(reader, func2, stage_index);
      break;

    case 3:
      sender = new TexcoordSender3f(reader, func3, stage_index);
      break;

    case 4:
      sender = new TexcoordSender4f(reader, func4, stage_index);
      break;
    }

    if (sender != nullptr) {
      // Ok, we've got a valid sender; add it to the list.
      _senders.push_back(sender);
      return true;

    } else {
      // We didn't get a valid sender; clean up and return.
      delete reader;
    }
  }

  return false;
}

/**
 * Creates a new ComponentSender for the named data column, if it exists in
 * the vertex data, and adds it to the list of senders for this object.
 *
 * This works like add_column(), but it specifically handles a function that
 * accepts as its first parameter the size (number of components) of the
 * floating-point vector, followed by the address of the vector.
 *
 * The return value is true if the column is added, false if it is not for
 * some reason (for instance, the named column doesn't exist in the vertex
 * data).
 */
bool CLP(ImmediateModeSender)::
add_vector_column(const GeomVertexDataPipelineReader *data_reader, const InternalName *name,
                  VectorFunc *func) {
  if (data_reader->has_column(name)) {
    GeomVertexReader *reader = new GeomVertexReader(data_reader, name);
    ComponentSender *sender = nullptr;
    const GeomVertexColumn *column = reader->get_column();
    switch (column->get_num_components()) {
    case 1:
      sender = new VectorSender1f(reader, func);
      break;

    case 2:
      sender = new VectorSender2f(reader, func);
      break;

    case 3:
      sender = new VectorSender3f(reader, func);
      break;

    case 4:
      sender = new VectorSender4f(reader, func);
      break;
    }

    if (sender != nullptr) {
      // Ok, we've got a valid sender; add it to the list.
      _senders.push_back(sender);
      return true;

    } else {
      // We didn't get a valid sender; clean up and return.
      delete reader;
    }
  }

  return false;
}

/**
 * Creates a new ComponentSender for the named data column, if it exists in
 * the vertex data, and adds it to the list of senders for this object.
 *
 * This works like add_vector_column(), but handles a function that receives a
 * vector of unsigned ints.
 *
 * The return value is true if the column is added, false if it is not for
 * some reason (for instance, the named column doesn't exist in the vertex
 * data).
 */
bool CLP(ImmediateModeSender)::
add_vector_uint_column(const GeomVertexDataPipelineReader *data_reader,
                       const InternalName *name, VectorUintFunc *func) {
  if (data_reader->has_column(name)) {
    GeomVertexReader *reader = new GeomVertexReader(data_reader, name);
    ComponentSender *sender = nullptr;
    const GeomVertexColumn *column = reader->get_column();
    switch (column->get_num_components()) {
    case 1:
      sender = new VectorSender1ui(reader, func);
      break;

    case 2:
      sender = new VectorSender2fui(reader, func);
      break;

    case 3:
      sender = new VectorSender3fui(reader, func);
      break;

    case 4:
      sender = new VectorSender4fui(reader, func);
      break;
    }

    if (sender != nullptr) {
      // Ok, we've got a valid sender; add it to the list.
      _senders.push_back(sender);
      return true;

    } else {
      // We didn't get a valid sender; clean up and return.
      delete reader;
    }
  }

  return false;
}

/**
 * Adds a new ComponentSender to the list of senders for this object.  The
 * GLImmediateModeSender object becomes the owner of the ComponentSender
 * pointer and will delete it when it is done.
 */
void CLP(ImmediateModeSender)::
add_sender(ComponentSender *sender) {
  _senders.push_back(sender);
}

/**
 *
 */
CLP(ImmediateModeSender)::ComponentSender::
~ComponentSender() {
  delete _reader;
}

/**
 *
 */
void CLP(ImmediateModeSender)::ComponentSender1f::
issue_vertex() {
  PN_stdfloat d = _reader->get_data1f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d);
}

/**
 *
 */
void CLP(ImmediateModeSender)::ComponentSender2f::
issue_vertex() {
  const LVecBase2 &d = _reader->get_data2();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d[0], d[1]);
}

/**
 *
 */
void CLP(ImmediateModeSender)::ComponentSender3f::
issue_vertex() {
  const LVecBase3 &d = _reader->get_data3();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d[0], d[1], d[2]);
}

/**
 *
 */
void CLP(ImmediateModeSender)::ComponentSender4f::
issue_vertex() {
  const LVecBase4 &d = _reader->get_data4();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d[0], d[1], d[2], d[3]);
}

/**
 *
 */
void CLP(ImmediateModeSender)::TexcoordSender1f::
issue_vertex() {
  PN_stdfloat d = _reader->get_data1f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ", stage " << _stage_index
      << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(GL_TEXTURE0 + _stage_index, d);
}

/**
 *
 */
void CLP(ImmediateModeSender)::TexcoordSender2f::
issue_vertex() {
  const LVecBase2 &d = _reader->get_data2();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ", stage " << _stage_index
      << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(GL_TEXTURE0 + _stage_index, d[0], d[1]);
}

/**
 *
 */
void CLP(ImmediateModeSender)::TexcoordSender3f::
issue_vertex() {
  const LVecBase3 &d = _reader->get_data3();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ", stage " << _stage_index
      << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(GL_TEXTURE0 + _stage_index, d[0], d[1], d[2]);
}

/**
 *
 */
void CLP(ImmediateModeSender)::TexcoordSender4f::
issue_vertex() {
  const LVecBase4 &d = _reader->get_data4();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ", stage " << _stage_index
      << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(GL_TEXTURE0 + _stage_index, d[0], d[1], d[2], d[3]);
}

/**
 *
 */
void CLP(ImmediateModeSender)::VectorSender1f::
issue_vertex() {
  PN_stdfloat d = _reader->get_data1f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(1, &d);
}

/**
 *
 */
void CLP(ImmediateModeSender)::VectorSender2f::
issue_vertex() {
  const LVecBase2 &d = _reader->get_data2();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(2, d.get_data());
}

/**
 *
 */
void CLP(ImmediateModeSender)::VectorSender3f::
issue_vertex() {
  const LVecBase3 &d = _reader->get_data3();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(3, d.get_data());
}

/**
 *
 */
void CLP(ImmediateModeSender)::VectorSender4f::
issue_vertex() {
  const LVecBase4 &d = _reader->get_data4();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(4, d.get_data());
}

/**
 *
 */
void CLP(ImmediateModeSender)::VectorSender1ui::
issue_vertex() {
  int d = _reader->get_data1i();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(1, (const GLuint *)&d);
}

/**
 *
 */
void CLP(ImmediateModeSender)::VectorSender2fui::
issue_vertex() {
  const LVecBase2i &d = _reader->get_data2i();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d[0] << " "
      << d[1] << "\n";
  }
#endif  // NDEBUG

  (*_func)(2, (const GLuint *)d.get_data());
}

/**
 *
 */
void CLP(ImmediateModeSender)::VectorSender3fui::
issue_vertex() {
  const LVecBase3i &d = _reader->get_data3i();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d[0] << " "
      << d[1] << " " << d[2] << "\n";
  }
#endif  // NDEBUG

  (*_func)(3, (const GLuint *)d.get_data());
}

/**
 *
 */
void CLP(ImmediateModeSender)::VectorSender4fui::
issue_vertex() {
  const LVecBase4i &d = _reader->get_data4i();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d[0] << " "
      << d[1] << " " << d[2] << " " << d[3] << "\n";
  }
#endif  // NDEBUG

  (*_func)(4, (const GLuint *)d.get_data());
}

#endif  // SUPPORT_IMMEDIATE_MODE
