// Filename: glImmediateModeSender_src.h
// Created by:  drose (15Aug05)
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

#include "pandabase.h"
#include "geomVertexReader.h"

#ifdef SUPPORT_IMMEDIATE_MODE

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

#ifndef STDFLOAT_DOUBLE
  typedef void APIENTRY Func1f(GLfloat a);
  typedef void APIENTRY Func2f(GLfloat a, GLfloat b);
  typedef void APIENTRY Func3f(GLfloat a, GLfloat b, GLfloat c);
  typedef void APIENTRY Func4f(GLfloat a, GLfloat b, GLfloat c, GLfloat d);
  typedef void APIENTRY TexcoordFunc1f(GLenum texture, GLfloat a);
  typedef void APIENTRY TexcoordFunc2f(GLenum texture, GLfloat a, GLfloat b);
  typedef void APIENTRY TexcoordFunc3f(GLenum texture, GLfloat a, GLfloat b, GLfloat c);
  typedef void APIENTRY TexcoordFunc4f(GLenum texture, GLfloat a, GLfloat b, GLfloat c, GLfloat d);
  typedef void APIENTRY VectorFunc(GLint, const GLfloat *);
#else  // STDFLOAT_DOUBLE
  typedef void APIENTRY Func1f(GLdouble a);
  typedef void APIENTRY Func2f(GLdouble a, GLdouble b);
  typedef void APIENTRY Func3f(GLdouble a, GLdouble b, GLdouble c);
  typedef void APIENTRY Func4f(GLdouble a, GLdouble b, GLdouble c, GLdouble d);
  typedef void APIENTRY TexcoordFunc1f(GLenum texture, GLdouble a);
  typedef void APIENTRY TexcoordFunc2f(GLenum texture, GLdouble a, GLdouble b);
  typedef void APIENTRY TexcoordFunc3f(GLenum texture, GLdouble a, GLdouble b, GLdouble c);
  typedef void APIENTRY TexcoordFunc4f(GLenum texture, GLdouble a, GLdouble b, GLdouble c, GLdouble d);
  typedef void APIENTRY VectorFunc(GLint, const GLdouble *);
#endif  // STDFLOAT_DOUBLE
  typedef void APIENTRY VectorUintFunc(GLint, const GLuint *);

  bool add_column(const GeomVertexDataPipelineReader *data_reader, 
                  const InternalName *name, Func1f *func1f, 
                  Func2f *func2, Func3f *func3, Func4f *func4);
  bool add_texcoord_column(const GeomVertexDataPipelineReader *data_reader, 
                           const InternalName *name, int stage_index,
                           TexcoordFunc1f *func1f, TexcoordFunc2f *func2, 
                           TexcoordFunc3f *func3, TexcoordFunc4f *func4);

  bool add_vector_column(const GeomVertexDataPipelineReader *data_reader, 
                         const InternalName *name, VectorFunc *func);
  bool add_vector_uint_column(const GeomVertexDataPipelineReader *data_reader, 
                              const InternalName *name, VectorUintFunc *func);

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

  class VectorSender1f : public ComponentSender {
  public:
    INLINE VectorSender1f(GeomVertexReader *reader, VectorFunc *func);
    virtual void issue_vertex();
  private:
    VectorFunc *_func;
  };

  class VectorSender2f : public ComponentSender {
  public:
    INLINE VectorSender2f(GeomVertexReader *reader, VectorFunc *func);
    virtual void issue_vertex();
  private:
    VectorFunc *_func;
  };

  class VectorSender3f : public ComponentSender {
  public:
    INLINE VectorSender3f(GeomVertexReader *reader, VectorFunc *func);
    virtual void issue_vertex();
  private:
    VectorFunc *_func;
  };

  class VectorSender4f : public ComponentSender {
  public:
    INLINE VectorSender4f(GeomVertexReader *reader, VectorFunc *func);
    virtual void issue_vertex();
  private:
    VectorFunc *_func;
  };

  class VectorSender1ui : public ComponentSender {
  public:
    INLINE VectorSender1ui(GeomVertexReader *reader, VectorUintFunc *func);
    virtual void issue_vertex();
  private:
    VectorUintFunc *_func;
  };

  class VectorSender2fui : public ComponentSender {
  public:
    INLINE VectorSender2fui(GeomVertexReader *reader, VectorUintFunc *func);
    virtual void issue_vertex();
  private:
    VectorUintFunc *_func;
  };

  class VectorSender3fui : public ComponentSender {
  public:
    INLINE VectorSender3fui(GeomVertexReader *reader, VectorUintFunc *func);
    virtual void issue_vertex();
  private:
    VectorUintFunc *_func;
  };

  class VectorSender4fui : public ComponentSender {
  public:
    INLINE VectorSender4fui(GeomVertexReader *reader, VectorUintFunc *func);
    virtual void issue_vertex();
  private:
    VectorUintFunc *_func;
  };

private:
  typedef pvector<ComponentSender *> ComponentSenders;
  ComponentSenders _senders;
};

#include "glImmediateModeSender_src.I"

#endif  // SUPPORT_IMMEDIATE_MODE

