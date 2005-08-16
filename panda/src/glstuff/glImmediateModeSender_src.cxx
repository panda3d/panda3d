// Filename: glImmediateModeSender_src.cxx
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
//     Function: CLP(ImmediateModeSender)::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CLP(ImmediateModeSender)::
~CLP(ImmediateModeSender)() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::clear
//       Access: Public
//  Description: Removes (and deletes) all of the senders from the
//               object.
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::
clear() {
  ComponentSenders::iterator si;
  for (si = _senders.begin(); si != _senders.end(); ++si) {
    delete (*si);
  }
  _senders.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::set_vertex
//       Access: Public
//  Description: Specifies the vertex index of the next vertex to
//               send.  If this is not called, the next consecutive
//               vertex will be sent.
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::
set_vertex(int vertex_index) {
  ComponentSenders::iterator si;
  for (si = _senders.begin(); si != _senders.end(); ++si) {
    (*si)->set_vertex(vertex_index);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::issue_vertex
//       Access: Public
//  Description: Sends the next vertex to the OpenGL API.
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::
issue_vertex() {
  ComponentSenders::iterator si;
  for (si = _senders.begin(); si != _senders.end(); ++si) {
    (*si)->issue_vertex();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::add_column
//       Access: Public
//  Description: Creates a new ComponentSender for the named data
//               column, if it exists in the vertex data, and adds it
//               to the list of senders for this object.
//
//               The four function pointers are the four variants on
//               the function pointer for the possible number of
//               components of the data column.  The appropriate
//               pointer will be used, depending on the number of
//               components the data column actually uses.
//
//               The return value is true if the column is added,
//               false if it is not for some reason (for instance, the
//               named column doesn't exist in the vertex data).
////////////////////////////////////////////////////////////////////
bool CLP(ImmediateModeSender)::
add_column(const GeomVertexData *vertex_data, const InternalName *name,
           Func1f *func1f, Func2f *func2f, Func3f *func3f, Func4f *func4f) {
  if (vertex_data->has_column(name)) {
    GeomVertexReader *reader = new GeomVertexReader(vertex_data, name);
    ComponentSender *sender = NULL;
    const GeomVertexColumn *column = reader->get_column();
    switch (column->get_num_components()) {
    case 1:
      if (func1f != (Func1f *)NULL) {
        sender = new ComponentSender1f(reader, func1f);
      }
      break;

    case 2:
      if (func2f != (Func2f *)NULL) {
        sender = new ComponentSender2f(reader, func2f);
      }
      break;

    case 3:
      if (func3f != (Func3f *)NULL) {
        sender = new ComponentSender3f(reader, func3f);
      }
      break;

    case 4:
      if (func4f != (Func4f *)NULL) {
        sender = new ComponentSender4f(reader, func4f);
      }
      break;
    }

    if (sender != (ComponentSender *)NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::add_texcoord_column
//       Access: Public
//  Description: Creates a new ComponentSender for the named data
//               column, if it exists in the vertex data, and adds it
//               to the list of senders for this object.
//
//               This works like add_column(), but it specifically
//               handles a texcoord-style column, which requires one
//               additional parameter to OpenGL: the texture stage.
//
//               The return value is true if the column is added,
//               false if it is not for some reason (for instance, the
//               named column doesn't exist in the vertex data).
////////////////////////////////////////////////////////////////////
bool CLP(ImmediateModeSender)::
add_texcoord_column(const GeomVertexData *vertex_data, 
                    const InternalName *name, int stage_index,
                    TexcoordFunc1f *func1f, TexcoordFunc2f *func2f, 
                    TexcoordFunc3f *func3f, TexcoordFunc4f *func4f) {
  if (vertex_data->has_column(name)) {
    GeomVertexReader *reader = new GeomVertexReader(vertex_data, name);
    ComponentSender *sender = NULL;
    const GeomVertexColumn *column = reader->get_column();
    switch (column->get_num_components()) {
    case 1:
      sender = new TexcoordSender1f(reader, func1f, stage_index);
      break;

    case 2:
      sender = new TexcoordSender2f(reader, func2f, stage_index);
      break;

    case 3:
      sender = new TexcoordSender3f(reader, func3f, stage_index);
      break;

    case 4:
      sender = new TexcoordSender4f(reader, func4f, stage_index);
      break;
    }

    if (sender != (ComponentSender *)NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::add_vector_column
//       Access: Public
//  Description: Creates a new ComponentSender for the named data
//               column, if it exists in the vertex data, and adds it
//               to the list of senders for this object.
//
//               This works like add_column(), but it specifically
//               handles a function that accepts as its first
//               parameter the size (number of components) of the
//               floating-point vector, followed by the address of the
//               vector.
//
//               The return value is true if the column is added,
//               false if it is not for some reason (for instance, the
//               named column doesn't exist in the vertex data).
////////////////////////////////////////////////////////////////////
bool CLP(ImmediateModeSender)::
add_vector_column(const GeomVertexData *vertex_data, const InternalName *name,
                  VectorFunc *func) {
  if (vertex_data->has_column(name)) {
    GeomVertexReader *reader = new GeomVertexReader(vertex_data, name);
    ComponentSender *sender = NULL;
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

    if (sender != (ComponentSender *)NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::add_vector_uint_column
//       Access: Public
//  Description: Creates a new ComponentSender for the named data
//               column, if it exists in the vertex data, and adds it
//               to the list of senders for this object.
//
//               This works like add_vector_column(), but handles a
//               function that receives a vector of unsigned ints.
//
//               The return value is true if the column is added,
//               false if it is not for some reason (for instance, the
//               named column doesn't exist in the vertex data).
////////////////////////////////////////////////////////////////////
bool CLP(ImmediateModeSender)::
add_vector_uint_column(const GeomVertexData *vertex_data, 
                       const InternalName *name, VectorUintFunc *func) {
  if (vertex_data->has_column(name)) {
    GeomVertexReader *reader = new GeomVertexReader(vertex_data, name);
    ComponentSender *sender = NULL;
    const GeomVertexColumn *column = reader->get_column();
    switch (column->get_num_components()) {
    case 1:
      sender = new VectorSender1ui(reader, func);
      break;

    case 2:
      sender = new VectorSender2ui(reader, func);
      break;

    case 3:
      sender = new VectorSender3ui(reader, func);
      break;

    case 4:
      sender = new VectorSender4ui(reader, func);
      break;
    }

    if (sender != (ComponentSender *)NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::add_sender
//       Access: Public
//  Description: Adds a new ComponentSender to the list of senders for
//               this object.  The GLImmediateModeSender object
//               becomes the owner of the ComponentSender pointer and
//               will delete it when it is done.
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::
add_sender(ComponentSender *sender) {
  _senders.push_back(sender);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::ComponentSender::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CLP(ImmediateModeSender)::ComponentSender::
~ComponentSender() {
  delete _reader;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::ComponentSender1f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::ComponentSender1f::
issue_vertex() {
  float d = _reader->get_data1f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::ComponentSender2f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::ComponentSender2f::
issue_vertex() {
  const LVecBase2f &d = _reader->get_data2f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d[0], d[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::ComponentSender3f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::ComponentSender3f::
issue_vertex() {
  const LVecBase3f &d = _reader->get_data3f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d[0], d[1], d[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::ComponentSender4f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::ComponentSender4f::
issue_vertex() {
  const LVecBase4f &d = _reader->get_data4f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d[0], d[1], d[2], d[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::TexcoordSender1f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::TexcoordSender1f::
issue_vertex() {
  float d = _reader->get_data1f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ", stage " << _stage_index
      << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(GL_TEXTURE0 + _stage_index, d);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::TexcoordSender2f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::TexcoordSender2f::
issue_vertex() {
  const LVecBase2f &d = _reader->get_data2f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ", stage " << _stage_index
      << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(GL_TEXTURE0 + _stage_index, d[0], d[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::TexcoordSender3f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::TexcoordSender3f::
issue_vertex() {
  const LVecBase3f &d = _reader->get_data3f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ", stage " << _stage_index
      << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(GL_TEXTURE0 + _stage_index, d[0], d[1], d[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::TexcoordSender4f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::TexcoordSender4f::
issue_vertex() {
  const LVecBase4f &d = _reader->get_data4f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ", stage " << _stage_index
      << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(GL_TEXTURE0 + _stage_index, d[0], d[1], d[2], d[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::VectorSender1f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::VectorSender1f::
issue_vertex() {
  float d = _reader->get_data1f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(1, &d);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::VectorSender2f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::VectorSender2f::
issue_vertex() {
  const LVecBase2f &d = _reader->get_data2f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(2, d.get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::VectorSender3f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::VectorSender3f::
issue_vertex() {
  const LVecBase3f &d = _reader->get_data3f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(3, d.get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::VectorSender4f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::VectorSender4f::
issue_vertex() {
  const LVecBase4f &d = _reader->get_data4f();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(4, d.get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::VectorSender1ui::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::VectorSender1ui::
issue_vertex() {
  int d = _reader->get_data1i();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(1, (const unsigned int *)&d);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::VectorSender2ui::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::VectorSender2ui::
issue_vertex() {
  const int *d = _reader->get_data2i();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d[0] << " " 
      << d[1] << "\n";
  }
#endif  // NDEBUG

  (*_func)(2, (const unsigned int *)d);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::VectorSender3ui::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::VectorSender3ui::
issue_vertex() {
  const int *d = _reader->get_data3i();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d[0] << " " 
      << d[1] << " " << d[2] << "\n";
  }
#endif  // NDEBUG

  (*_func)(3, (const unsigned int *)d);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(ImmediateModeSender)::VectorSender4ui::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ImmediateModeSender)::VectorSender4ui::
issue_vertex() {
  const int *d = _reader->get_data4i();
#ifndef NDEBUG
  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << *_reader->get_column()->get_name() << ": " << d[0] << " " 
      << d[1] << " " << d[2] << " " << d[3] << "\n";
  }
#endif  // NDEBUG

  (*_func)(4, (const unsigned int *)d);
}
