// Filename: tinyImmediateModeSender.h
// Created by:  drose (29Apr08)
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

#ifndef TINYIMMEDIATEMODESENDER_H
#define TINYIMMEDIATEMODESENDER_H

#include "pandabase.h"
#include "geomVertexReader.h"

// These are actually the TinyGL headers, not the system OpenGL headers.
#include "GL/gl.h"

////////////////////////////////////////////////////////////////////
//       Class : TinyImmediateModeSender
// Description : This class collects together a handful of objects
//               that will issue immediate-mode commands like
//               glVertex, glTexCoord, etc., for the purposes of
//               sending an object's vertices using the immediate mode
//               functions.
//
//               Since unlike OpenGL, TinyGL prefers the
//               immediate-mode interface, this is used all of the
//               time.
////////////////////////////////////////////////////////////////////
class TinyImmediateModeSender {
public:
  INLINE TinyImmediateModeSender();
  ~TinyImmediateModeSender();

  void clear();
  
  void set_vertex(int vertex_index);
  void issue_vertex();

  class ComponentSender;
  typedef void Func1f(GLfloat a);
  typedef void Func2f(GLfloat a, GLfloat b);
  typedef void Func3f(GLfloat a, GLfloat b, GLfloat c);
  typedef void Func4f(GLfloat a, GLfloat b, GLfloat c, GLfloat d);

  bool add_column(const GeomVertexDataPipelineReader *data_reader, 
                  const InternalName *name, Func1f *func1f, 
                  Func2f *func2f, Func3f *func3f, Func4f *func4f);

  void add_sender(ComponentSender *sender);

public:

  class ComponentSender {
  public:
    INLINE ComponentSender(GeomVertexReader *reader);
    virtual ~ComponentSender();
    INLINE void set_vertex(int vertex_index);
    virtual void issue_vertex()=0;
  protected:
    GeomVertexReader *_reader;
  };

  class ComponentSender1f : public ComponentSender {
  public:
    INLINE ComponentSender1f(GeomVertexReader *reader, Func1f *func);
    virtual void issue_vertex();
  private:
    Func1f *_func;
  };

  class ComponentSender2f : public ComponentSender {
  public:
    INLINE ComponentSender2f(GeomVertexReader *reader, Func2f *func);
    virtual void issue_vertex();
  private:
    Func2f *_func;
  };

  class ComponentSender3f : public ComponentSender {
  public:
    INLINE ComponentSender3f(GeomVertexReader *reader, Func3f *func);
    virtual void issue_vertex();
  private:
    Func3f *_func;
  };

  class ComponentSender4f : public ComponentSender {
  public:
    INLINE ComponentSender4f(GeomVertexReader *reader, Func4f *func);
    virtual void issue_vertex();
  private:
    Func4f *_func;
  };

private:
  typedef pvector<ComponentSender *> ComponentSenders;
  ComponentSenders _senders;
};

#include "tinyImmediateModeSender.I"

#endif  // SUPPORT_IMMEDIATE_MODE

