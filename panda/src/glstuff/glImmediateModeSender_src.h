// Filename: glImmediateModeSender_src.h
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

#include "pandabase.h"
#include "geomVertexReader.h"

////////////////////////////////////////////////////////////////////
//       Class : GLImmediateModeSender
// Description : This class collects together a handful of objects
//               that will issue immediate-mode commands like
//               glVertex, glTexCoord, etc., for the purposes of
//               sending an object's vertices using the immediate mode
//               functions.
//
//               Normally, this isn't used, since it's almost always
//               better to use vertex arrays or vertex buffers
//               instead.  But OpenGL is a complicated API, and some
//               drivers might have issues handling the vertex arrays;
//               this code is provided as a last-ditch fallback in
//               case you have such spectacularly buggy drivers.
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(ImmediateModeSender) {
public:
  INLINE CLP(ImmediateModeSender)();
  ~CLP(ImmediateModeSender)();

  void clear();
  
  void set_vertex(int vertex_index);
  void issue_vertex();

  class ComponentSender;
  typedef void APIENTRY Func1f(GLfloat a);
  typedef void APIENTRY Func2f(GLfloat a, GLfloat b);
  typedef void APIENTRY Func3f(GLfloat a, GLfloat b, GLfloat c);
  typedef void APIENTRY Func4f(GLfloat a, GLfloat b, GLfloat c, GLfloat d);
  typedef void APIENTRY TexcoordFunc1f(GLenum texture, GLfloat a);
  typedef void APIENTRY TexcoordFunc2f(GLenum texture, GLfloat a, GLfloat b);
  typedef void APIENTRY TexcoordFunc3f(GLenum texture, GLfloat a, GLfloat b, GLfloat c);
  typedef void APIENTRY TexcoordFunc4f(GLenum texture, GLfloat a, GLfloat b, GLfloat c, GLfloat d);

  bool add_column(const GeomVertexData *vertex_data, const InternalName *name,
                  Func1f *func1f, Func2f *func2f, Func3f *func3f, Func4f *func4f);
  bool add_texcoord_column(const GeomVertexData *vertex_data, 
                           const InternalName *name, int stage_index,
                           TexcoordFunc1f *func1f, TexcoordFunc2f *func2f, 
                           TexcoordFunc3f *func3f, TexcoordFunc4f *func4f);

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

  class TexcoordSender1f : public ComponentSender {
  public:
    INLINE TexcoordSender1f(GeomVertexReader *reader, TexcoordFunc1f *func,
                            int stage_index);
    virtual void issue_vertex();
  private:
    TexcoordFunc1f *_func;
    int _stage_index;
  };

  class TexcoordSender2f : public ComponentSender {
  public:
    INLINE TexcoordSender2f(GeomVertexReader *reader, TexcoordFunc2f *func,
                            int stage_index);
    virtual void issue_vertex();
  private:
    TexcoordFunc2f *_func;
    int _stage_index;
  };

  class TexcoordSender3f : public ComponentSender {
  public:
    INLINE TexcoordSender3f(GeomVertexReader *reader, TexcoordFunc3f *func,
                            int stage_index);
    virtual void issue_vertex();
  private:
    TexcoordFunc3f *_func;
    int _stage_index;
  };

  class TexcoordSender4f : public ComponentSender {
  public:
    INLINE TexcoordSender4f(GeomVertexReader *reader, TexcoordFunc4f *func,
                            int stage_index);
    virtual void issue_vertex();
  private:
    TexcoordFunc4f *_func;
    int _stage_index;
  };

private:
  typedef pvector<ComponentSender *> ComponentSenders;
  ComponentSenders _senders;
};

#include "glImmediateModeSender_src.I"

