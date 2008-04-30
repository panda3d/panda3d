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

#include "tinyImmediateModeSender.h"
#include "config_tinydisplay.h"

////////////////////////////////////////////////////////////////////
//     Function: TinyImmediateModeSender::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TinyImmediateModeSender::
~TinyImmediateModeSender() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyImmediateModeSender::clear
//       Access: Public
//  Description: Removes (and deletes) all of the senders from the
//               object.
////////////////////////////////////////////////////////////////////
void TinyImmediateModeSender::
clear() {
  ComponentSenders::iterator si;
  for (si = _senders.begin(); si != _senders.end(); ++si) {
    delete (*si);
  }
  _senders.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyImmediateModeSender::set_vertex
//       Access: Public
//  Description: Specifies the vertex index of the next vertex to
//               send.  If this is not called, the next consecutive
//               vertex will be sent.
////////////////////////////////////////////////////////////////////
void TinyImmediateModeSender::
set_vertex(int vertex_index) {
  ComponentSenders::iterator si;
  for (si = _senders.begin(); si != _senders.end(); ++si) {
    (*si)->set_vertex(vertex_index);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyImmediateModeSender::issue_vertex
//       Access: Public
//  Description: Sends the next vertex to the OpenGL API.
////////////////////////////////////////////////////////////////////
void TinyImmediateModeSender::
issue_vertex() {
  ComponentSenders::iterator si;
  for (si = _senders.begin(); si != _senders.end(); ++si) {
    (*si)->issue_vertex();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyImmediateModeSender::add_column
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
bool TinyImmediateModeSender::
add_column(const GeomVertexDataPipelineReader *data_reader, const InternalName *name,
           Func1f *func1f, Func2f *func2f, Func3f *func3f, Func4f *func4f) {
  if (data_reader->has_column(name)) {
    GeomVertexReader *reader = new GeomVertexReader(data_reader, name);
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
//     Function: TinyImmediateModeSender::add_sender
//       Access: Public
//  Description: Adds a new ComponentSender to the list of senders for
//               this object.  The GLImmediateModeSender object
//               becomes the owner of the ComponentSender pointer and
//               will delete it when it is done.
////////////////////////////////////////////////////////////////////
void TinyImmediateModeSender::
add_sender(ComponentSender *sender) {
  _senders.push_back(sender);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyImmediateModeSender::ComponentSender::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TinyImmediateModeSender::ComponentSender::
~ComponentSender() {
  delete _reader;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyImmediateModeSender::ComponentSender1f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TinyImmediateModeSender::ComponentSender1f::
issue_vertex() {
  float d = _reader->get_data1f();
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyImmediateModeSender::ComponentSender2f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TinyImmediateModeSender::ComponentSender2f::
issue_vertex() {
  const LVecBase2f &d = _reader->get_data2f();
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d[0], d[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyImmediateModeSender::ComponentSender3f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TinyImmediateModeSender::ComponentSender3f::
issue_vertex() {
  const LVecBase3f &d = _reader->get_data3f();
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d[0], d[1], d[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyImmediateModeSender::ComponentSender4f::issue_vertex
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TinyImmediateModeSender::ComponentSender4f::
issue_vertex() {
  const LVecBase4f &d = _reader->get_data4f();
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam()
      << *_reader->get_column()->get_name() << ": " << d << "\n";
  }
#endif  // NDEBUG

  (*_func)(d[0], d[1], d[2], d[3]);
}
