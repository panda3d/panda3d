// Filename: spheretexReflector.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
#ifndef SPHERETEXREFLECTOR_H
#define SPHERETEXREFLECTOR_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "shader.h"
#include "spheretexShader.h"
#include "casterShader.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : SpheretexReflector
// Description : Creates a reflection texture map for a given object
////////////////////////////////////////////////////////////////////
class EXPCL_SHADER SpheretexReflector : public CasterShader
{
public:

  SpheretexReflector(int size = 256);
  ~SpheretexReflector(void);

  virtual void pre_apply(Node *node, const AllAttributesWrapper &init_state,
                         const AllTransitionsWrapper &net_trans,
                         GraphicsStateGuardian *gsg);
  virtual void apply(Node *node, const AllAttributesWrapper &init_state,
                     const AllTransitionsWrapper &net_trans,
                     GraphicsStateGuardian *gsg);

  // MPG - should we make this check for powers of 2?
  INLINE void set_size(int size) { _size = size; }
  INLINE int get_size(void) const { return _size; }

  INLINE bool is_reflector(void) const { return _is_reflector; }
  INLINE void make_reflector(void) { _is_reflector = true; }
  INLINE void make_refractor(void) { _is_reflector = false; }

  INLINE void set_near(float fnear) { _fnear = fnear; }
  INLINE float get_near(void) const { return _fnear; }
  INLINE void set_far(float ffar) { _ffar = ffar; }
  INLINE float get_far(void) const { return _ffar; }

  //Override base class shader's set_priority to allow
  //us to set the priority of the contained projtex_shader at
  //the same time
  virtual void set_priority(int priority);
  virtual void set_multipass(bool on);

protected:

  SpheretexShader               _spheretex_shader;
  int                           _size;
  bool                  _is_reflector;
  float                 _fnear;
  float                 _ffar;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CasterShader::init_type();
    register_type(_type_handle, "SpheretexReflector",
                  CasterShader::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;
};

#endif
